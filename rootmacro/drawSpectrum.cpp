#include <inc/shinclude.h>

int drawSpectrum()
{
    const std::string rootfilepath = "/nadb/nadb55/namai/newage/mod1/CsGammaMeasurement/analysis/137Cs/raw.root";
    const std::string outputfilepath = "/nadb/nadb55/namai/newage/mod1/CsGammaMeasurement/analysis/plot/spectrum_137Cs.png";

    const double cal_factor_slope = 3.993;
    const double cal_factor_offset = 407.520;
    const int ene_bin = 100;
    const int ene_min = 100;
    const int ene_max = 2000;
    // const int ene_max = 8192;
    const double rate_min = 0.;
    const double rate_max = 1.5;

    const int rebinN = 1.;

    const int fit_flag = 1;
    const int fit1_min = 600;
    const int fit1_max = 750;
    // const int fit2_min = 750;
    // const int fit2_max = 1000;

    gROOT->SetBatch();
    SetShStyle();

    TFile *rootfile = new TFile(rootfilepath.c_str(), "READ");
    if (!rootfile->IsOpen())
    {
        std::cerr << "Error: Cannot open the ROOT file." << std::endl;
        return 1;
    }
    TTree *tree = (TTree *)rootfile->Get("tree");
    if (!tree)
    {
        std::cerr << "Error: Cannot find the tree." << std::endl;
        return 1;
    }

    TCanvas *c1 = new TCanvas("c1", "c1", 600, 600);
    TH1D *hist = new TH1D("hist", "hist", ene_bin, ene_min, ene_max);

    std::vector<int> *event = 0;
    float livetime;
    float livetime_sum = 0;
    tree->SetBranchAddress("event", &event);
    tree->SetBranchAddress("livetime", &livetime);

    int nentries = tree->GetEntries();

    for (int i = 0; i < nentries; i++)
    {
        tree->GetEntry(i);
        for (int j = 0; j < event->size(); j++)
        {
            hist->Fill((event->at(j) - cal_factor_offset) / cal_factor_slope);
        }
        livetime_sum += livetime;
    }
    hist->Rebin(rebinN);
    // hist->Scale(1.0 / livetime_sum / (ene_max - ene_min) * ene_bin / rebinN);
    hist->Scale(1.0 / livetime_sum / rebinN);

    c1->SetGrid(1);
    hist->GetXaxis()->SetTitle("Energy [keV]");
    hist->GetYaxis()->SetTitle("Rate [counts/sec./keV]");
    hist->GetXaxis()->SetLabelSize(0.03);
    hist->GetYaxis()->SetLabelSize(0.03);
    hist->GetYaxis()->SetRangeUser(rate_min, rate_max);
    hist->Draw("hist");

    if (fit_flag)
    {
        TF1 *fit1 = new TF1("fit", "gaus(0)+pol1(3)", fit1_min, fit1_max);
        fit1->SetParameter(0, 1.00e+00);
        fit1->SetParameter(1, 6.62e+02);
        fit1->SetParameter(2, 1.00e+02);
        fit1->SetLineColor(kRed);
        hist->Fit(fit1, "R");
        fit1->Draw("same");

        TText *t_const = new TText(0.4, 0.8, Form("const: %5.2f +- %5.2f", fit1->GetParameter(0), fit1->GetParError(0)));
        TText *t_mean = new TText(0.4, 0.75, Form("mean: %5.2f +- %5.2f", fit1->GetParameter(1), fit1->GetParError(1)));
        TText *t_sigma = new TText(0.4, 0.7, Form("sigma: %5.2f +- %5.2f", fit1->GetParameter(2), fit1->GetParError(2)));
        t_const->SetNDC();
        t_mean->SetNDC();
        t_sigma->SetNDC();
        t_const->Draw();
        t_mean->Draw();
        t_sigma->Draw();

        // calc gaus integral
        const double integral_min = fit1->GetParameter(1) - 2 * fit1->GetParameter(2);
        const double integral_max = fit1->GetParameter(1) + 2 * fit1->GetParameter(2);
        double gaus_integral = fit1->Integral(integral_min, integral_max);
        double gaus_integral_err = fit1->IntegralError(integral_min, integral_max);
        std::cout << "Gaus integral: " << gaus_integral << " +- " << gaus_integral_err << " counts/sec." << std::endl;
    }

    c1->SaveAs(outputfilepath.c_str());

    return 0;
}
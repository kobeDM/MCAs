#include <inc/shinclude.h>

int fitConstant() {
    SetShStyle();

    const std::string rootfilepath = "/nadb/nadb55/namai/miraclue/gamma/aist2024/mca/ana/beam/raw.root";
    const std::string outputfilepath = "/nadb/nadb55/namai/miraclue/gamma/aist2024/mca/ana/beam/fitSpectrum.png";
    const std::string resultfilepath = "/nadb/nadb55/namai/miraclue/gamma/aist2024/mca/ana/beam/fitConstantParameters.dat";

    const double cal_slope = 3.200;
    const double cal_slope_err = 0.025;
    const double cal_offset = -99.682;
    const double cal_offset_err = 16.425;
    const int ene_bin = 8192;
    const int ene_min = 0;
    const int ene_max = 2600;
    const double rate_min = 0;
    const double rate_max = 1.2;

    const int rebinN = 32;

    TFile *rootfile = new TFile(rootfilepath.c_str(), "READ");
    if (!rootfile->IsOpen()) {
        std::cerr << "Error: Cannot open the ROOT file." << std::endl;
        return 1;
    }
    TTree *tree = (TTree*)rootfile->Get("tree");
    if (!tree) {
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

    for (int i = 0; i < nentries; i++) {
        tree->GetEntry(i);
        for (int j = 0; j < event->size(); j++) {
            hist->Fill((event->at(j) - cal_offset) / cal_slope);
        }
        livetime_sum += livetime;
    }
    hist->Rebin(rebinN);
    hist->Scale(1.0 / livetime_sum / rebinN);

    c1->SetGrid(1);
    hist->GetXaxis()->SetTitle("Energy [keV]");
    hist->GetYaxis()->SetTitle("Rate [counts/sec./keV]");
    hist->GetXaxis()->SetLabelSize(0.03);
    hist->GetYaxis()->SetLabelSize(0.03);
    hist->GetYaxis()->SetRangeUser(rate_min, rate_max);
    hist->Draw("hist");

    // peak
    const int n_gaus = 2;
    const double gaus_const[n_gaus] = {9.43781e-02, 3.83369e-02};
    const double gaus_mean[n_gaus] = {5.15240e+02, 8.83702e+02};
    const double gaus_sigma[n_gaus] = {3.92877e+01, 4.75822e+01};
    const double gaus_min[n_gaus] = {0, 700};
    const double gaus_max[n_gaus] = {700, 2600};
    TF1 *gaus[n_gaus];
    for (int i = 0; i < n_gaus; i++) {
        gaus[i] = new TF1(Form("gaus%d", i), "gaus", gaus_min[i], gaus_max[i]);
        gaus[i]->FixParameter(0, gaus_const[i]);
        gaus[i]->FixParameter(1, gaus_mean[i]);
        gaus[i]->FixParameter(2, gaus_sigma[i]);
        gaus[i]->SetLineColor(kBlue);
        gaus[i]->Draw("same");
    }

    //fitting
    const int pol_min = 200;
    const int pol_max = 2500;
    const int interval = 100;

    const int n = (pol_max - pol_min) / interval;
    TF1 *pol[n];
    std::ofstream ofs(resultfilepath.c_str());
    ofs << "# ene_min ene_max rate rate_err" << std::endl;
    for (int i = 0; i < n; i++) {
        if (i == 2 || i == 3 || i == 6 || i == 7) continue;
        pol[i] = new TF1(Form("pol%d", i), "gaus(0)+expo(3)", pol_min + i * interval, pol_min + (i + 1) * interval);
        hist->Fit(Form("pol%d", i), "RQ");
        pol[i]->SetLineColor(kRed);
        pol[i]->Draw("same");
        double integral = pol[i]->Integral(pol_min + i * interval, pol_min + (i + 1) * interval);
        double integral_err = pol[i]->IntegralError(pol_min + i * interval, pol_min + (i + 1) * interval);
        std::cout << pol_min + i * interval << "-" << pol_min + (i + 1) * interval << "keV: " << integral << " +- " << integral_err << " counts/sec" << std::endl;
        ofs << pol_min + i * interval << " " << pol_min + (i + 1) * interval << " " << integral << " " << integral_err << std::endl;
    }

    const int n_gaus_pol = 4;
    const double gaus_pol_min[n_gaus_pol] = {400, 500, 800, 900};
    const double gaus_pol_max[n_gaus_pol] = {500, 600, 900, 1000};
    TF1 *gaus_pol[n_gaus_pol];
    TF1 *pol_wo_gaus[n_gaus_pol];
    for (int i = 0; i < n_gaus_pol; i++) {
        gaus_pol[i] = new TF1(Form("gaus_pol%d", i), "gaus(0)+pol1(3)", gaus_pol_min[i], gaus_pol_max[i]);
        if (i == 0 || i == 1) {
            gaus_pol[i]->FixParameter(0, gaus_const[0]);
            gaus_pol[i]->FixParameter(1, gaus_mean[0]);
            gaus_pol[i]->FixParameter(2, gaus_sigma[0]);
        } else {
            gaus_pol[i]->FixParameter(0, gaus_const[1]);
            gaus_pol[i]->FixParameter(1, gaus_mean[1]);
            gaus_pol[i]->FixParameter(2, gaus_sigma[1]);
        }
        hist->Fit(Form("gaus_pol%d", i), "RQ");
        // gaus_pol[i]->SetLineColor(kRed);
        // gaus_pol[i]->Draw("same");
        pol_wo_gaus[i] = new TF1(Form("pol_wo_gaus%d", i), "pol1", gaus_pol_min[i], gaus_pol_max[i]);
        pol_wo_gaus[i]->SetParameters(gaus_pol[i]->GetParameter(3), gaus_pol[i]->GetParameter(4));
        pol_wo_gaus[i]->SetLineColor(kRed);
        pol_wo_gaus[i]->Draw("same");
        double integral = pol_wo_gaus[i]->Integral(gaus_pol_min[i], gaus_pol_max[i]);
        double integral_err = pol_wo_gaus[i]->IntegralError(gaus_pol_min[i], gaus_pol_max[i]);
        std::cout << gaus_pol_min[i] << "-" << gaus_pol_max[i] << "keV: " << integral << " +- " << integral_err << " counts/sec" << std::endl;
        ofs << gaus_pol_min[i] << " " << gaus_pol_max[i] << " " << integral << " " << integral_err << std::endl;
    }

    ofs.close();
    c1->SaveAs(outputfilepath.c_str());

    return 0;
}
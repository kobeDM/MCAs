#include "inc/shinclude.h"

void calcCalibrationFactor()
{
    SetShStyle();
    const std::string datafilepath = "/nadb/nadb55/namai/newage/mod1/CsGammaMeasurement/analysis/calibration/calibration.dat";
    const std::string outputfilepath = "//nadb/nadb55/namai/newage/mod1/CsGammaMeasurement/analysis/plot/calibration.png";

    const double tge_ene_min = 0;
    const double tge_ene_max = 1500;
    const double tge_mca_ch_min = 0;
    const double tge_mca_ch_max = 8192;

    std::ifstream ifs(datafilepath.c_str());
    if (!ifs)
    {
        std::cerr << "Error: Cannot open the data file." << std::endl;
        return 1;
    }

    TGraphErrors *tge_cal = new TGraphErrors();

    std::string line;
    while (std::getline(ifs, line))
    {
        if (line[0] == '#')
            continue;
        std::istringstream iss(line);
        double ene, mca_ch, mca_ch_err;
        iss >> ene >> mca_ch >> mca_ch_err;
        tge_cal->SetPoint(tge_cal->GetN(), ene, mca_ch);
        tge_cal->SetPointError(tge_cal->GetN() - 1, 0, mca_ch_err);
    }

    TCanvas *c1 = new TCanvas("c1", "c1", 600, 600);
    c1->SetGrid();
    tge_cal->GetXaxis()->SetTitle("Energy [keV]");
    tge_cal->GetYaxis()->SetTitle("MCA channel");
    tge_cal->GetXaxis()->SetLabelSize(0.04);
    tge_cal->GetYaxis()->SetLabelSize(0.04);
    tge_cal->GetXaxis()->SetLimits(tge_ene_min, tge_ene_max);
    tge_cal->GetYaxis()->SetRangeUser(tge_mca_ch_min, tge_mca_ch_max);
    tge_cal->SetMarkerStyle(20);
    tge_cal->SetMarkerSize(1);
    tge_cal->Draw("AP");

    TF1 *fit1 = new TF1("fit1", "pol1", tge_ene_min, tge_ene_max);
    fit1->SetLineColor(kRed);
    tge_cal->Fit(fit1, "R");

    const double cal_factor_offset = fit1->GetParameter(0);
    const double cal_factor_offset_err = fit1->GetParError(0);
    const double cal_factor_slope = fit1->GetParameter(1);
    const double cal_factor_slope_err = fit1->GetParError(1);

    TLatex t_cal_factor;
    t_cal_factor.SetTextSize(0.03);
    t_cal_factor.DrawLatexNDC(0.25, 0.85, Form("Calibration factor slope: %.3f #pm %.3f", cal_factor_slope, cal_factor_slope_err));
    t_cal_factor.DrawLatexNDC(0.25, 0.80, Form("Calibration factor offset: %.3f #pm %.3f", cal_factor_offset, cal_factor_offset_err));

    c1->SaveAs(outputfilepath.c_str());
}
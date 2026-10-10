#include "inc/shinclude.h"

void drawFitParameters() {
    SetShStyle();

    const std::string resultfilepath = "/nadb/nadb55/namai/miraclue/gamma/aist2024/mca/ana/beam/fitParameters.dat";
    const std::string EnergyResolutionPath = "/nadb/nadb55/namai/miraclue/gamma/aist2024/mca/ana/beam/EnergyResolution.png";
    const std::string EnergyRatePath = "/nadb/nadb55/namai/miraclue/gamma/aist2024/mca/ana/beam/EnergyRate.png";

    std::ifstream ifs(resultfilepath.c_str());
    if (!ifs) {
        std::cerr << "Error: Cannot open the result file." << std::endl;
        return;
    }
    TCanvas *c1 = new TCanvas("c1", "c1", 600, 600);
    TCanvas *c2 = new TCanvas("c2", "c2", 600, 600);
    TGraphErrors *g_resolution = new TGraphErrors();
    TGraphErrors *g_rate = new TGraphErrors();
    std::string line;
    while (!ifs.eof()) {
        std::getline(ifs, line);
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        double mean, mean_err, sigma, sigma_err, integral, integral_err;
        iss >> mean >> mean_err >> sigma >> sigma_err >> integral >> integral_err;
        g_resolution->SetPoint(g_resolution->GetN(), mean, sigma / mean);
        g_resolution->SetPointError(g_resolution->GetN() - 1, mean_err, sqrt(pow(sigma_err / mean, 2) + pow(sigma * mean_err / mean / mean, 2)));
        // g_rate->SetPoint(g_flux->GetN(), mean, integral);
        // g_rate->SetPointError(g_flux->GetN() - 1, mean_err, integral_err);
    }

    c1->cd();
    c1->SetGrid();
    g_resolution->SetMarkerStyle(20);
    g_resolution->SetMarkerSize(0.5);
    g_resolution->SetMarkerColor(kBlack);
    g_resolution->SetLineColor(kBlack);
    g_resolution->SetTitle(";E [keV];#sigma / E");
    g_resolution->GetXaxis()->SetRangeUser(0, 1000);
    g_resolution->GetYaxis()->SetRangeUser(0, 0.1);
    g_resolution->GetXaxis()->SetLabelSize(0.04);
    g_resolution->GetYaxis()->SetLabelSize(0.04);
    g_resolution->Draw("AP");
    
    c2->cd();
    c2->SetGrid();
    g_rate->SetMarkerStyle(20);
    g_rate->SetMarkerSize(0.5);
    g_rate->SetMarkerColor(kBlack);
    g_rate->SetLineColor(kBlack);
    g_rate->SetTitle(";E [keV];Rate [counts/sec.]");
    g_rate->GetXaxis()->SetRangeUser(0, 1000);
    g_rate->GetYaxis()->SetRangeUser(0, 10);
    g_rate->GetXaxis()->SetLabelSize(0.04);
    g_rate->GetYaxis()->SetLabelSize(0.04);
    g_rate->Draw("AP");

    c1->SaveAs(EnergyResolutionPath.c_str());
    c2->SaveAs(EnergyRatePath.c_str());
}
#include "inc/shinclude.h"

void drawConstant() {
    SetShStyle();

    const std::string datafilePath = "/nadb/nadb55/namai/miraclue/gamma/aist2024/mca/ana/beam/fitConstantParameters.dat";
    const std::string outputfilePath = "/nadb/nadb55/namai/miraclue/gamma/aist2024/mca/ana/beam/Constant.png";

    std::ifstream ifs(datafilePath.c_str());
    if (!ifs) {
        std::cerr << "Error: " << datafilePath << " does not exist." << std::endl;
        return;
    }

    std::string line;
    int ene_min, ene_max;
    double rate, rate_err;

    TCanvas *c1 = new TCanvas("c1", "c1", 600, 600);
    TGraphErrors *g_fit = new TGraphErrors();

    while (!ifs.eof()) {
        std::getline(ifs, line);
        if (line[0] == '#') continue;
        std::istringstream iss(line);
        iss >> ene_min >> ene_max >> rate >> rate_err;
        g_fit->SetPoint(g_fit->GetN(), (ene_min + ene_max) / 2, rate);
        g_fit->SetPointError(g_fit->GetN() - 1, (ene_max - ene_min) / 2, rate_err);
    }

    c1->SetGrid(1);
    g_fit->SetMarkerStyle(20);
    g_fit->Draw();
    g_fit->GetXaxis()->SetTitle("Energy [keV]");
    g_fit->GetYaxis()->SetTitle("Rate [counts/sec.]");
    g_fit->GetXaxis()->SetLabelSize(0.03);
    g_fit->GetYaxis()->SetLabelSize(0.03);
    g_fit->Draw("ap");

    c1->SaveAs(outputfilePath.c_str());
}
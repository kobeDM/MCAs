#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "TStyle.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TAxis.h"

std::vector<std::string> split(const std::string& src, const char* delim = ",") {
    std::vector<std::string> vec;
    std::string::size_type len = src.length();

    for (std::string::size_type i = 0, n; i < len; i = n + 1) {
        n = src.find_first_of(delim, i);
        if (n == std::string::npos) {
            n = len;
        }
        vec.push_back(src.substr(i, n - i));
    }
    return vec;
}

int calibration(const std::string datapath) {
    std::ifstream datafile(datapath);
    if (!datafile.is_open()) {
        std::cerr << "Error: file not found" << std::endl;
        return 1;
    }

    double mca, mca_err, current, current_err;
    std::string line, str;
    TCanvas *csv = new TCanvas("csv", "csv", 800, 600);
    TGraphErrors *gr = new TGraphErrors();
    
    while (std::getline(datafile, line)) {
        if (line[0] == '#') continue;
        std::vector<std::string> v_data = split(line);
        // if (v_data[0] == "3869") continue;
        gr->SetPoint(gr->GetN(), std::stod(v_data[0]), std::stod(v_data[2]));
        gr->SetPointError(gr->GetN()-1, std::stod(v_data[1]), std::stod(v_data[3]));
    }

    gr->SetMarkerStyle(20);
    gr->SetMarkerSize(0.5);
    gr->SetMarkerColor(kBlue);
    gr->GetXaxis()->SetRangeUser(3650, 3900);
    gr->GetYaxis()->SetRangeUser(0.590, 0.597);
    gr->GetXaxis()->SetTitle("MCA channel");
    gr->GetYaxis()->SetTitle("Current (mA)");
    gr->GetYaxis()->SetTitleOffset(1.4);
    gr->GetYaxis()->SetLabelSize(0.03);
    gr->Draw("AP");

    // fit
    gStyle->SetOptFit(1111);
    TF1 *fit = new TF1("fit", "pol1", 3650, 3900);
    fit->SetLineColor(kRed);
    gr->Fit(fit, "R");

    return 0;
}
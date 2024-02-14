#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "TStyle.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TF1.h"

void drawInflux(std::string datafilepath) {
    TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
    TGraph *gr = new TGraph();

    std::ifstream datafile(datafilepath);
    if (!datafile) {
        std::cerr << "Error: file not found" << std::endl;
        return;
    }
    int start = 1706177934, end = 1706231777;
    double current_min = 0.59, current_max = 0.597;
    double current_sum = 0;
    int current_n = 0;
    std::string line;
    int unixtime;
    double current;
    int headerN = 0, lineN = 0;
    while (getline(datafile, line)) {
        if (lineN == 0) {
            lineN++;
            continue;
        }
        std::stringstream ss(line);
        while (getline(ss, line, ',')) {
            if (headerN == 1 ) unixtime = std::stoi(line.substr(0, 10));
            if (headerN == 11) current  = std::stof(line);
            headerN++;
        }
        if (unixtime > start && unixtime < end) {
            gr->SetPoint(gr->GetN(), unixtime, current);
            current_sum += current;
            current_n++;
        }
        headerN = 0;
        lineN++;
    }

    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1);

    // gr->SetMarkerStyle(20);
    gr->GetXaxis()->SetRangeUser(start, end);
    gr->GetYaxis()->SetRangeUser(current_min, current_max);
    gr->Draw("ap");
    std::cout << "Average current: " << current_sum / current_n << std::endl;

    TF1 *f1 = new TF1("f1", "pol0", start, end);
    gr->Fit(f1, "R");
    gr->Draw("same");
    std::cout << "Fit result: " << f1->GetParameter(0) << std::endl;
}
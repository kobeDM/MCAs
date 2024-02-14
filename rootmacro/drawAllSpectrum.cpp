#include <inc/shinclude.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "TStyle.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TF1.h"
#include "TAxis.h"

void drawAllSpectrum() {
    SetShStyle();
    gStyle->SetOptStat(0);

    // config
    // const int rootfileN = 1;
    // const int rootfileN = 3;
    const int rootfileN = 4;
    const std::string rootfilepath_array[rootfileN] = {
        // "/home/user/miraclue/ana/beam1/raw.root",
        // "/home/user/miraclue/ana/beam2/raw.root",
        // "/home/user/miraclue/ana/beam3/raw.root",
        // "/home/user/miraclue/ana/BG1/raw.root",
        "/home/user/miraclue/ana/BG2/raw.root",
        "/home/user/miraclue/ana/BG3/raw.root",
        "/home/user/miraclue/ana/BG4/raw.root",
        "/home/user/miraclue/ana/BG5/raw.root",
    };
    const double calfactor_array[rootfileN] = {
        0.298, 
        0.298, 
        0.298,
        0.298,
        // 0.300,
        // 0.300,
        // 0.300,
    };

    TFile *file_array[rootfileN];
    TTree *tree_array[rootfileN];
    TH1D *event_hist[rootfileN];
    TH1D *scaled_hist[rootfileN];
    TH1D *livetime_hist[rootfileN];
    std::vector<int> *event = new std::vector<int>;
    float livetime, livetime_sum = 0;

    for (int i = 0; i < rootfileN; i++) {
        // open root files
        file_array[i] = new TFile(rootfilepath_array[i].c_str());
        if (!file_array[i]) {
            std::cerr << "Error: file not found" << std::endl;
            return;
        }
        tree_array[i] = (TTree*)file_array[i]->Get("tree");
        if (!tree_array[i]) {
            std::cerr << "Error: tree not found" << std::endl;
            return;
        }
        tree_array[i]->SetBranchAddress("event", &event);
        tree_array[i]->SetBranchAddress("livetime", &livetime);

        event_hist[i] = new TH1D(Form("h%d", i), Form("h%d", i), 8192, 0, 8192);
        livetime_hist[i] = new TH1D(Form("livetime%d", i), Form("livetime%d", i), 2000, 90, 110);
        tree_array[i]->Draw(Form("livetime>>livetime%d", i));
        tree_array[i]->Draw(Form("event*%f>>h%d",calfactor_array[i], i));
        // scaling
        livetime_sum += livetime_hist[i]->GetMean() * livetime_hist[i]->GetEntries();
        std::cout << "livetime: " << livetime_sum << std::endl;

        event->clear();
    }

    TCanvas *cvs = new TCanvas("cvs", "cvs", 800, 600);
    TH1D *event_sum = new TH1D("hs", "hs", 8192, 0, 8192);

    // sum histograms
    double bin_content[8192];
    for (int i = 0; i < 8192; i++) {
        bin_content[i] = 0;
        for (int j = 0; j < rootfileN; j++) {
            bin_content[i] += event_hist[j]->GetBinContent(i);
        }
        event_sum->SetBinContent(i, bin_content[i]);
    }

    const int rebin = 8;
    event_sum->Scale(1. / livetime_sum / event_sum->GetBinWidth(1) / rebin);
    event_sum->Rebin(rebin);
    event_sum->SetTitle("");
    event_sum->GetXaxis()->SetRangeUser(0, 2700);
    // event_sum->GetYaxis()->SetRangeUser(1e-2, 1e1);
    // event_sum->GetYaxis()->SetRangeUser(1e-3, 1e0);
    event_sum->GetYaxis()->SetRangeUser(0, 0.3);
    event_sum->GetXaxis()->SetTitle("keV");
    event_sum->GetYaxis()->SetTitle("counts/sec./keV");
    event_sum->Draw("hist");
    // cvs->SetLogy();
    cvs->SetGrid();

    int offset = 120;
    TH1D *h_expo = new TH1D("h_expo", "h_expo", 8192-offset, offset, 8192);
    for (int i = 0; i < 8192; i++) {
        h_expo->SetBinContent(i, 0.1 * exp(-0.002 * i));
    }

    // rescale with exponential
    TH1D *h_rescale = (TH1D*)event_sum->Clone("h_rescale");
    for (int i = 0; i < 8192; i++) {
        h_rescale->SetBinContent(i, event_sum->GetBinContent(i) / h_expo->GetBinContent(i));
    }

    h_rescale->SetLineColor(kBlack);
    // h_rescale->Draw("same");

    h_expo->SetLineColor(kRed);
    // h_expo->Draw("same");

    // fit
    TF1 *fit = new TF1("fit", "gaus(0)+pol1(3)", 400, 630);
    // TF1 *fit = new TF1("fit", "gaus(0)", 1550, 1850);
    fit->SetParameter(0, 0);
    fit->SetParameter(1, 500);
    fit->SetParameter(2, 100);
    fit->SetParameter(3, 0);
    fit->SetParameter(4, 0);

    event_sum->Fit(fit, "R");
    fit->SetLineColor(kRed);
    fit->Draw("same");

    cvs->SaveAs("all_spectrum.png");

    delete event;

    return;
}
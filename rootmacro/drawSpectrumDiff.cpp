#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "TROOT.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TF1.h"
#include "TAxis.h"
#include "TText.h"

void drawSpectrumDiff(const std::string rootfilepath1, const std::string rootfilepath2) {
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1);
    const int chN = 8192;

    // open root files
    TFile *file1 = new TFile(rootfilepath1.c_str());
    TFile *file2 = new TFile(rootfilepath2.c_str());
    if (!file1 || !file2) {
        std::cerr << "Error: file not found" << std::endl;
        return;
    }
    TTree *tree1 = (TTree*)file1->Get("tree");
    TTree *tree2 = (TTree*)file2->Get("tree");
    if (!tree1 || !tree2) {
        std::cerr << "Error: tree not found" << std::endl;
        return;
    }

    // read root files
    std::vector<int> *event1 = new std::vector<int>;
    std::vector<int> *event2 = new std::vector<int>;
    const int fileN1 = tree1->GetEntries();
    const int fileN2 = tree2->GetEntries();
    tree1->SetBranchAddress("event", &event1);
    tree2->SetBranchAddress("event", &event2);
    TH1D *h1 = new TH1D("h1", "h1", chN, 0, chN);
    TH1D *h2 = new TH1D("h2", "h2", chN, 0, chN);
    tree1->Draw("event>>h1");
    tree2->Draw("event>>h2");

    // decoration
    h1->Rebin(8);
    h2->Rebin(8);
    h1->SetTitle("");
    h2->SetTitle("");
    h1->SetLineColor(kRed);
    h2->SetLineColor(kBlue);
    const int ymax_h1 = h1->GetMaximum();
    const int ymax_h2 = h2->GetMaximum();
    h1->GetYaxis()->SetRangeUser(0, ymax_h1 * 1.1);
    h2->GetYaxis()->SetRangeUser(0, ymax_h1 * 1.1);
    TCanvas *cvs = new TCanvas("cvs", "cvs", 1200, 600);
    cvs->Divide(2, 1);
    cvs->cd(1);
    h1->Draw();
    cvs->cd(2);
    h2->Draw();

    // fit
    TF1 *f1 = new TF1("f1", "gaus", 3200, 3900);
    TF1 *f2 = new TF1("f2", "gaus", 3500, 4000);
    h1->Fit("f1", "", "", 3200, 3900);
    h2->Fit("f2", "", "", 3500, 4000);
    f1->SetLineColor(kBlue);
    f2->SetLineColor(kRed);
    cvs->cd(1);
    f1->Draw("same");
    cvs->cd(2);
    f2->Draw("same");


    // save
    cvs->SaveAs("spectrumDiff.png");

    delete event1;
    delete event2;

    return;
}

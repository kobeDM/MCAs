#include <iostream>
#include <fstream>

#include "TROOT.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1D.h"

void draw() {
    gROOT->SetBatch();
    gStyle->SetOptStat(0);
    TFile *file = new TFile("./raw.root");
    if (!file) {
        std::cout << "File ./raw.root not found." << std::endl;
        return;
    }
    TTree *tree = (TTree*)file->Get("tree");
    if (!tree) {
        std::cout << "Tree not found." << std::endl;
        return;
    }
    const int chN = 8192;
    int ch[chN];
    int eventN = tree->GetEntries();
    tree->SetBranchAddress("ch", ch);

    TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
    TH1D *h1 = new TH1D("h1", "h1", chN, 0, chN);
    for (int i = 0; i < eventN; i++) {
        tree->GetEntry(i);
        for (int j = 0; j < chN; j++) {
            h1->SetBinContent(j, ch[j]);
        }
    }
    h1->Rebin(64);
    h1->SetTitle("");
    h1->GetXaxis()->SetTitle("Channel");
    h1->GetYaxis()->SetTitle("Counts");
    h1->Draw();
    c1->SaveAs("./sp_raw.png");

    return;
}
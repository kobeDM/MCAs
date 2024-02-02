#include <iostream>
#include <fstream>
#include <vector>

#include "TROOT.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1D.h"

void draw() {
    gROOT->SetBatch();
    gStyle->SetOptStat(0);

    // config
    const int chN = 8192;
    const int rebin = 64;
    const std::string rootfliepath = "raw.root";

    // read rootfile
    TFile *file = new TFile(rootfliepath.c_str());
    if (!file) {
        std::cerr << "File ./raw.root not found." << std::endl;
        return;
    }
    TTree *tree = (TTree*)file->Get("tree");
    if (!tree) {
        std::cerr << "Tree not found." << std::endl;
        return;
    }

    // draw
    std::vector<int> *event = new std::vector<int>;
    int eventN = tree->GetEntries();
    tree->SetBranchAddress("event", event);

    TCanvas *cvs = new TCanvas("cvs", "cvs", 800, 600);
    cvs->SetLogy();
    TH1D *sp_raw = new TH1D("sp_raw", "sp_raw", chN, 0, chN);
    int event_size;
    for (int i = 0; i < eventN; i++) {
        tree->GetEntry(i);
        event_size = event->size();
        for (int j = 0; j < event_size; j++) {
            // sp_raw->SetBinContent(event->at(j), j);
            sp_raw->Fill(event->at(j));
        }
    }
    sp_raw->Rebin(rebin);
    sp_raw->SetTitle("");
    sp_raw->GetXaxis()->SetTitle("Channels");
    sp_raw->GetYaxis()->SetTitle("Counts");
    sp_raw->Draw();
    cvs->SaveAs("./sp_raw.png");
    cvs->SaveAs("./sp_raw.pdf");

    delete event;
    return;
}
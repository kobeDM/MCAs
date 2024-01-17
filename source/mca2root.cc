#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>

#include "TFile.h"
#include "TTree.h"

void mca2root() {
    std::string filelistpath = "./datafiles.txt";
    std::ifstream filelist(filelistpath.c_str());
    if (!filelist) {
        std::cout << "File " << filelistpath << " not found." << std::endl;
        return;
    }
    std::string filename, line, filename_line;
    const int chN = 8192;
    int ch[chN];

    TFile *file = new TFile("./raw.root", "RECREATE");
    TTree *tree = new TTree("tree", "tree");
    tree->Branch("ch", ch, Form("ch[%i]/I", chN));

    int i = 0;
    int count;
    while (!filelist.eof()) {
        std::getline(filelist, filename);
        if (filename == "") break;
        std::stringstream ss(filename);
        ss >> filename_line;
        std::cout << filename << std::endl;
        std::ifstream file(filename.c_str());
        while (!file.eof()) {
            std::getline(file, line);
            std::stringstream ss(line);
            ss >> count;
            if  (std::isdigit(line[0])) {
                ch[i] = count;
                i++;
            }
        }
        tree->Fill();
        i = 0;
    }
    std::cout << "Total events: " << tree->GetEntries() << std::endl;
    tree->Write();
    file->Close();
    return;
}
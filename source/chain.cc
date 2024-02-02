#include <iostream>
#include <fstream>
#include <string>

#include "TTree.h"
#include "TFile.h"
#include "TChain.h"

void chain(std::string chainListPath, std::string outputPath) {
    TChain *chain = new TChain("tree");
    std::ifstream chainList(chainListPath.c_str());
    std::string line;
    while (std::getline(chainList, line)) {
        chain->Add(line.c_str());
    }
    chain->Merge(outputPath.c_str());
}
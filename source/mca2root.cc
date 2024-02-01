#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>

#include "TFile.h"
#include "TTree.h"

std::string erase_str(std::string str, std::string erasedStr) {
    size_t pos = str.find(erasedStr);
    if (pos != std::string::npos) {
        str.erase(pos, erasedStr.length());
    }
    return str;
}

int str2ut(std::string dateStr) {
    std::string date, time;
    std::stringstream ss1(dateStr);
    ss1 >> date >> time;

    int year, month, day;
    std::stringstream ss2(date);
    ss2 >> month;
    ss2.ignore();
    ss2 >> day;
    ss2.ignore();
    ss2 >> year;

    int hour, minute, second;
    std::stringstream ss3(time);
    ss3 >> hour;
    ss3.ignore();
    ss3 >> minute;
    ss3.ignore();
    ss3 >> second;

    std::tm timeStruct = {};
    timeStruct.tm_year = year - 1900;
    timeStruct.tm_mon = month - 1;
    timeStruct.tm_mday = day;
    timeStruct.tm_hour = hour;
    timeStruct.tm_min = minute;
    timeStruct.tm_sec = second;

    std::time_t unixTime = std::mktime(&timeStruct);
    int unixTimeInt = static_cast<int>(unixTime);

    return unixTimeInt;
}

void mca2root() {
    // config
    const std::string filelistpath = "./datafiles.txt";
    const std::string rootfliepath = "./raw.root";
    const int chN = 8192;

    std::ifstream filelist(filelistpath.c_str());
    if (!filelist) {
        std::cerr << "File " << filelistpath << " not found." << std::endl;
        return;
    }

    int fileN = 0;
    int ch = 0;
    int total_event = 0;
    int cch[chN], event[chN], ut;
    float livetime, realtime, liverate, realrate;

    TFile *file = new TFile(rootfliepath.c_str(), "RECREATE");
    TTree *tree = new TTree("tree", "tree");
    tree->Branch("fileN"   , &fileN,    "fileN/I"               );
    tree->Branch("ch"      , cch,       Form("ch[%i]/I",    chN));
    tree->Branch("event"   , event,     Form("event[%i]/I", chN));
    tree->Branch("total_event", &total_event, "total_event/I"   );
    tree->Branch("unixtime", &ut,       "unixtime/I"            );
    tree->Branch("livetime", &livetime, "livetime/F"            );
    tree->Branch("realtime", &realtime, "realtime/F"            );
    tree->Branch("liverate", &liverate, "liverate/F"            );
    tree->Branch("realrate", &realrate, "realrate/F"            );

    int count, tmp;
    std::string filename, line, filename_line;
    while (!filelist.eof()) {
        // Read file
        std::getline(filelist, filename);
        if (filename == "") break;
        std::stringstream ss(filename);
        ss >> filename_line;
        std::cout << filename << std::endl;
        std::ifstream file(filename.c_str());
        while (!file.eof()) {
            // read line
            std::getline(file, line);
            std::stringstream ss(line);
            ss >> count;
            if (strncmp(line.c_str(), "START_TIME", 10) == 0) ut = str2ut(erase_str(line, "START_TIME - "));
            if (strncmp(line.c_str(), "LIVE_TIME" ,  9) == 0) livetime = std::stof(erase_str(line, "LIVE_TIME - "));
            if (strncmp(line.c_str(), "REAL_TIME" ,  9) == 0) realtime = std::stof(erase_str(line, "REAL_TIME - "));
            if (std::isdigit(line[0])) {
                event[ch] = count;
                total_event += count;
                cch[ch] = ch;
                ch++;
            }
        }
        liverate = static_cast<float>(total_event) / livetime;
        realrate = static_cast<float>(total_event) / realtime;
        tree->Fill();
        ch = 0;
        total_event = 0;
        fileN++;
        std::cout << "unixtime   : " << ut       << std::endl;
        std::cout << "live  time : " << livetime << std::endl;
        std::cout << "real  time : " << realtime << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }
    std::cout << "Total files : " << tree->GetEntries() << std::endl;
    tree->Write();
    file->Close();
    return;
}
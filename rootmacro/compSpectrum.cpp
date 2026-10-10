#include <inc/shinclude.h>

int compSpectrum()
{
    gROOT->SetBatch();
    SetShStyle();

    // parameters
    // const bool normalize = true;
    const bool normalize = false;

    const int evtNum = 3;
    String pername[evtNum] = {
        "BG",
        "^{137}Cs",
        "^{60}Co",
    };
    String rootfilepath[evtNum] = {
        "/nadb/nadb55/namai/newage/mod1/CsGammaMeasurement/analysis/BG/raw.root",
        "/nadb/nadb55/namai/newage/mod1/CsGammaMeasurement/analysis/137Cs/raw.root",
        "/nadb/nadb55/namai/newage/mod1/CsGammaMeasurement/analysis/60Co/raw.root",
    };
    double cal_factor[evtNum] = {
        1.000,
        1.000,
        1.000,
    };

    String outputfilepath = "/nadb/nadb55/namai/newage/mod1/CsGammaMeasurement/analysis/plot/compareSpectrum_mca.png";

    const double h_bin = 100;
    const double h_min = 200;
    const double h_max = 8192;
    const double n_min = 0;
    // const double n_max = 0.1;
    const double n_max = 40;
    const int rebinN = 1;
    const String tree_name = "tree";

    TFile *file[evtNum];
    TTree *tree[evtNum];

    TCanvas *c1 = new TCanvas("c1", "c1", 600, 600);
    c1->SetGrid();
    TH1D *h1[evtNum];
    TLegend *leg = new TLegend(0.6, 0.7, 0.9, 0.9);
    // TText *t_livetime[evtNum];
    TLatex t_livetime;
    t_livetime.SetTextSize(0.03);

    for (int i = 0; i < evtNum; i++)
    {
        file[i] = new TFile(rootfilepath[i].c_str(), "READ");
        if (!file[i]->IsOpen())
        {
            std::cerr << "Error: cannot open file " << rootfilepath[i] << std::endl;
            return 1;
        }
        tree[i] = (TTree *)file[i]->Get(tree_name.c_str());
        if (!tree[i])
        {
            std::cerr << "Error: cannot find tree" << std::endl;
            return 1;
        }

        int nentries = tree[i]->GetEntries();
        std::vector<int> *event = 0;
        float livetime = 0;
        float livetime_sum = 0;
        tree[i]->SetBranchAddress("event", &event);
        tree[i]->SetBranchAddress("livetime", &livetime);
        h1[i] = new TH1D(Form("h1_%d", i), "", h_bin, h_min, h_max);

        for (int evt = 0; evt < nentries; evt++)
        {
            tree[i]->GetEntry(evt);
            for (int instance = 0; instance < event->size(); instance++)
            {
                h1[i]->Fill(event->at(instance) * cal_factor[i]);
            }
            livetime_sum += livetime;
        }
        // h1[i]->GetXaxis()->SetTitle("Energy [keV]");
        h1[i]->GetXaxis()->SetTitle("MCA channel");
        if (normalize)
        {
            h1[i]->GetYaxis()->SetTitle("Normalized rate [A.U.]");
        }
        else
        {
            h1[i]->GetYaxis()->SetTitle("Rate [counts/sec./keV]");
        }

        h1[i]->SetLineColor((i + 1) * 2);
        h1[i]->SetFillColor((i + 1) * 2);
        h1[i]->SetFillStyle(3004 + i);
        h1[i]->Rebin(rebinN);
        h1[i]->Scale(1. / livetime_sum / rebinN);

        if (normalize)
            h1[i]->Scale(1. / h1[i]->Integral());
        h1[i]->GetYaxis()->SetRangeUser(n_min, n_max);
        h1[i]->Draw("hist same");

        leg->AddEntry(h1[i], Form("%s", pername[i].c_str()), "f");
        leg->SetBorderSize(0);
        leg->SetFillColor(0);
        leg->SetFillStyle(0);
        leg->Draw();
        // t_livetime[i] = new TText(0.5, 0.65 - 0.05 * i, Form("%s livetime: %5.2f sec.", pername[i].c_str(), livetime_sum));
        // t_livetime[i]->SetNDC();
        // t_livetime[i]->SetTextColor((i + 1) * 2);
        // t_livetime[i]->SetTextSize(0.03);
        // t_livetime[i]->Draw();
        t_livetime.DrawLatex((h_max - h_min) * 0.5 + h_min, n_max * (0.65 - 0.05 * i), Form("%s livetime: %5.2f sec.", pername[i].c_str(), livetime_sum));

        if (normalize && i == 0)
        {
            TText *t_norm = new TText(0.5, 0.55, "Normalized");
            t_norm->SetNDC();
            t_norm->Draw();
        }
    }
    c1->SaveAs(outputfilepath.c_str());

    return 0;
}
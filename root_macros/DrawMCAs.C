// ----------------------- //
// DrawMCA.C -- root macro
// Author: K. Mizukoshi
// Date  : Apr. 12 2019
// Cs-137 data collected
// by MCA module
// ----------------------- //

void DrawMCAs( const std::string file_head,const int file_num){
  //int DrawMCAs(char finame){
  //const int MCACh = 8191;
  TCanvas *Resultscv = new TCanvas("summarycv","summarycv",800,600);
  Resultscv->SetGrid(1);
  const int MCACh = 8192;
  //const int showMax=3000;
  const int showMax=8192;
	const int header = 12;
	const int th_line=4;
	const int live_line=7;
	const int real_line=8;
	const int rebin =4;
	string inputFile,outputFile;
	TH1D* hist = new TH1D("hist","hist",MCACh,0,MCACh);
	TH1D* hist_sum = new TH1D("hist_sum","hist_sum",MCACh,0,MCACh);
	double val;
	double live=0,live_sum=0;
	string str;
	string str_live,str_real,str_th;
	outputFile=file_head+".root";
	for (int i=0;i<file_num;i++){
	  inputFile=file_head+"_"+to_string(i)+".mca";
	  cerr<<i<<"/"<<file_num<<"\t"<<inputFile;//<<endl;

	  ifstream ifs(inputFile.c_str());
	  for(int ich=0; ich<header; ++ich){
	    //		ifs >> sval;
	    getline(ifs,str);
	    if (ich==live_line)	      str_live=str;
	    if (ich==real_line)	      str_real=str;
	    if (ich==th_line)	      str_th=str;
	    //cerr << str<<endl;
	  }
	  //getline(ifs,str);
	  
	  live=stod(str_live.substr(str_live.find("-")+2));
	  cerr << "\t"<<live<<"sec"<<endl;
	  live_sum+=live;
	  //}
	
	for(int ich=0; ich<MCACh; ++ich){
	  ifs >> val;
	  //cout<<ich<<"\t"<<val;
	  hist->SetBinContent(ich, val);
	}
	hist_sum->Add(hist);
	}
//cerr<<endl;
	//ifstream ifs("SN718_0.mca");
 
	hist_sum->Rebin(rebin);
	//hist->Draw();
	//hist_sum->GetXaxis()->SetRangeUser(0,MCACh/2);
	hist_sum->GetXaxis()->SetRangeUser(0,showMax);
	
	hist_sum->Sumw2();
	hist_sum->Scale(1./live_sum);
	cerr<<"total "<< live_sum<<"sec"<<endl;

	
	hist_sum->GetXaxis()->SetTitle( "MCA channel" );
	hist_sum->GetYaxis()->SetTitle( "count/bin/sec" );

	hist_sum->Draw();
	//hist_sum->GetYaxis()->SetRangeUser(0,0.10);



	TF1 *gaus = new TF1("gaus","gaus",1250,1350);

	gaus->SetParameters(hist_sum->GetMaximum(),220,20);

	hist_sum->Fit(gaus,"R");

	gaus->SetLineColor(kRed);
	gaus->SetLineWidth(2);
	gaus->Draw("same");

	double mean  = gaus->GetParameter(1);
	double sigma = gaus->GetParameter(2);

	double fwhm = 2.355*sigma;
	double res  = 100.0*fwhm/mean;

	TPaveText *pt = new TPaveText(0.60,0.65,0.88,0.88,"NDC");

	pt->SetFillColor(0);
	pt->SetBorderSize(1);
	pt->SetTextSize(0.03);

	pt->AddText(Form("Mean = %.2f",mean));
	pt->AddText(Form("Sigma = %.2f",sigma));
	pt->AddText(Form("FWHM = %.2f",fwhm));
	pt->AddText(Form("Resolution = %.2f %%",res));
	double meanErr  = gaus->GetParError(1);
	double sigmaErr = gaus->GetParError(2);

	pt->AddText(Form("Mean = %.2f #pm %.2f",mean,meanErr));
	pt->AddText(Form("Sigma = %.2f #pm %.2f",sigma,sigmaErr));

	pt->Draw();



	
	hist_sum->SaveAs( outputFile.c_str( ) );
	Resultscv->SaveAs("hist.png");
	//	return 0;
}

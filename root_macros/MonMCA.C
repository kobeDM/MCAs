// ----------------------- //
// DrawMCA.C -- root macro
// Author: K. Mizukoshi
// Date  : Apr. 12 2019
// Cs-137 data collected
// by MCA module
// ----------------------- //

//int MonMCA(){
int MonMCA(string tmp_filename){
	const int MCACh = 8192;
	const int header = 12;
	const int live_line=7;
	const int real_line=8;
	const int th_line=4;
	const int dev_line=10;
	//const char tmp_filename[32]="tmp.mca";
	TCanvas *c = new TCanvas("c","",600,400);

	double val;
	string str;
	string str_live,str_real,str_th,str_dev;
	//ifstream ifs("718.dat");
	//ifstream ifs("794.dat");
	while(1){
	  TH1D* hist = new TH1D("hist",str_dev.c_str(),MCACh,0,MCACh);
	  //ifstream ifs("tmp.mca");
	  ifstream ifs(tmp_filename.c_str());
	  for(int ich=0; ich<header; ++ich){
	    getline(ifs,str);
	    if (ich==live_line)	      str_live=str;
	    if (ich==real_line)	      str_real=str;
	    if (ich==th_line)	      str_th=str;
	    if (ich==dev_line)	      str_dev=str;

	    //cerr << str<<endl;
	  }
	  for(int ich=0; ich<MCACh; ++ich){
	    ifs >> val;
	    //cout<<ich<<"\t"<<val;
	    hist->SetBinContent(ich, val);
	  }
	  //TText *tdev = new TText(0.6*MCACh, 0.4*hist->GetMaximum(),str_dev.c_str());
	  TText *tn = new TText(0.6*MCACh, 0.8*hist->GetMaximum(),tmp_filename.c_str());
	  TText *tl = new TText(0.6*MCACh, 0.7*hist->GetMaximum(), str_live.c_str());
	  TText *tr= new TText(0.6*MCACh, 0.6*hist->GetMaximum(), str_real.c_str());
	  TText *tth= new TText(0.6*MCACh, 0.5*hist->GetMaximum(), str_th.c_str());
	  //	  tl->SetTextAlign(22);
	  tn->SetTextSize(0.04);
	  //tdev->SetTextSize(0.04);
	  tl->SetTextSize(0.04);
	  tth->SetTextSize(0.04);
	  tr->SetTextSize(0.04);
	  hist->Draw();
	  tth->Draw();
	  //tdev->Draw();
	  tn->Draw(); 
	  tl->Draw(); 
	  tr->Draw(); 
	  c->Update();
	  ifs.close();
	  sleep(1);
	}
	return 0;
}

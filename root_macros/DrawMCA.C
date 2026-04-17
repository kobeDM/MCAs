// ----------------------- //
// DrawMCA.C -- root macro
// Author: K. Mizukoshi
// Date  : Apr. 12 2019
// Cs-137 data collected
// by MCA module
// ----------------------- //

int DrawMCA(){
	const int MCACh = 8191;
	const int header = 12;
	TH1D* hist = new TH1D("hist","hist",MCACh,0,MCACh);

	double val;
	string str;
	//ifstream ifs("718.dat");
	//ifstream ifs("794.dat");
	ifstream ifs("SN718_0.mca");
 
	for(int ich=0; ich<header; ++ich){
	  //		ifs >> sval;	
	  getline(ifs,str);
	  cerr << str<<endl;
	}
	for(int ich=0; ich<MCACh; ++ich){
		ifs >> val;
		//cout<<ich<<"\t"<<val;
		hist->SetBinContent(ich, val);
	}

	hist->Draw();

	return 0;
}

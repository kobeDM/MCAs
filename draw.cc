#include "inc/shinclude.h"

void draw( const String& inputDir )
{
    SetAtlasStyle( );

    std::vector< String > filepathArr;
    String fileKey = "mca";
    
    if( ShUtil::GetFilePathArr( inputDir, &filepathArr, false, fileKey ) == false ) false;

    double totalLivetime = 0.0;
    TH1D hist( "hist", "hist", 1024, 0.0, 1024.0 );
    for( auto name : filepathArr ) {
        std::cout << "Start persing " << name << " ..." << std::endl;
        std::ifstream ifs( name );
        if( ifs.is_open( ) == false ) continue;

        bool daqStart = false;
        int mcaCount = 0;
        double livetime = 0.0;
        while( !ifs.eof( ) ) {
            String line = "";
            getline( ifs, line );
            StringStream ss( line );

            if( line.find( "LIVE_TIME" ) != String::npos ) {
                String dummy = "";
                ss >> dummy >> dummy >> livetime;
            }
            
            if( line.find("<<DATA>>") != String::npos ) {
                daqStart = true;
                // std::cout << "test" << std::endl;
                continue;
            }
            if( line.find("<<END>>") != String::npos ) break;

            // std::cout << line << std::endl;
            if( daqStart == true ) {

                int mcaEvents = std::stoi( line );
                // std::cout << mcaEvents << std::endl;
                for( int i = 0; i < mcaEvents; ++i ) hist.Fill( mcaCount );
                ++mcaCount;
            }
        }
        
        totalLivetime += livetime;
    }

    std::cout << "total livetime = " << totalLivetime << std::endl;
    hist.Sumw2( );
    hist.Scale( 1.0 / totalLivetime );
    // hist.Rebin( );
    TCanvas cvs( "cvs", "cvs", 800, 600 );

    hist.SetXTitle( "MCA count" );
    hist.SetYTitle( "Events / MCA count / sec" );
    hist.Draw();

    TF1 funcPrim("funcPrim","gaus(0)",120,1024);
    funcPrim.SetLineColor( kRed );
    // funcPrim.SetParameter(0,100);
    // funcPrim.SetParLimits(0,10,1000); 
    funcPrim.SetParameter(1,500);
    funcPrim.SetParLimits(1,120,1000); 
    funcPrim.SetParameter(2,100);
    funcPrim.SetParLimits(2,0,1000); 
    hist.Fit( &funcPrim, "LMI", "", 120, 1024 );
    hist.Fit( &funcPrim, "LMI", "", funcPrim.GetParameter(1) - funcPrim.GetParameter(2) * 0.4, funcPrim.GetParameter(1) + funcPrim.GetParameter(2) * 1.5);
    cvs.SaveAs( "prim.png" );

    TF1 func("func","gaus",funcPrim.GetParameter(1) - funcPrim.GetParameter(2),funcPrim.GetParameter(1) + funcPrim.GetParameter(2));
    func.SetLineColor( kRed );
    func.SetParameter(0,funcPrim.GetParameter( 0 ) );
    func.SetParameter(1,funcPrim.GetParameter( 1 ) );
    func.SetParameter(2,funcPrim.GetParameter( 2 ) );
    hist.Fit( &func, "L", "", funcPrim.GetParameter(1) - funcPrim.GetParameter(2)*0.4, funcPrim.GetParameter(1) + funcPrim.GetParameter(2)*1.5 );

    cvs.SaveAs("test.png");
    
    return;
}

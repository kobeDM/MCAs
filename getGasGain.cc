#include "inc/shinclude.h"


class CalParams {
public:
    CalParams  ( );
    ~CalParams ( );
    
    double m_gasGain;
    double m_gasGainErr;
    double m_eneReso;
    double m_eneResoErr;
};

CalParams::CalParams( )
    : m_gasGain    ( 0.0 )
    , m_gasGainErr ( 0.0 )
    , m_eneReso    ( 0.0 )
    , m_eneResoErr ( 0.0 )
{
    
}

CalParams::~CalParams( )
{
    
}

CalParams getGasGain( const String& inputDir, const String& outputDir, const double& gainFactor, const String& calSource )
{
    SetShStyle( );
    ShUtil::ExistCreateDir( outputDir );

    std::vector< String > filepathArr;
    String fileKey = "mca";

    CalParams params;
    
    if( ShUtil::GetFilePathArr( inputDir, &filepathArr, false, fileKey ) == false ) return params;

    double totalLivetime = 0.0;
    TH1D hist( "hist", "hist", 1024, 0.0, 1024.0 );
    for( auto name : filepathArr ) {
        std::cout << "Start persing " << name << " ..." << std::endl;
        std::ifstream ifs( name );
        if( ifs.is_open( ) == false ) continue;

        bool parseStart = false;
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
                parseStart = true;
                // std::cout << "test" << std::endl;
                continue;
            }
            if( line.find("<<END>>") != String::npos ) break;

            // std::cout << line << std::endl;
            if( parseStart == true ) {

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

    // return 0.0;
    
    TF1 funcPrim("funcPrim","gaus(0)",120,1024);
    funcPrim.SetLineColor( kRed );
    // funcPrim.SetParameter(0,100);
    // funcPrim.SetParLimits(0,10,1000); 
    funcPrim.SetParameter(1,500);
    funcPrim.SetParLimits(1,120,1000); 
    funcPrim.SetParameter(2,100);
    funcPrim.SetParLimits(2,0,1000); 
    hist.Fit( &funcPrim, "LMI", "", 120, 1024 );
    hist.Fit( &funcPrim, "LMI", "", funcPrim.GetParameter(1) - funcPrim.GetParameter(2) * 0.2, funcPrim.GetParameter(1) + funcPrim.GetParameter(2) * 1.2);
    // cvs.SaveAs( "prim.png" );

    TF1 func("func","gaus",funcPrim.GetParameter(1) - funcPrim.GetParameter(2),funcPrim.GetParameter(1) + funcPrim.GetParameter(2));
    func.SetLineColor( kRed );
    func.SetParameter(0,funcPrim.GetParameter( 0 ) );
    func.SetParameter(1,funcPrim.GetParameter( 1 ) );
    func.SetParameter(2,funcPrim.GetParameter( 2 ) );
    hist.Fit( &func, "L", "", funcPrim.GetParameter(1) - funcPrim.GetParameter(2)*0.4, funcPrim.GetParameter(1) + funcPrim.GetParameter(2)*1.2 );

    double mean     = func.GetParameter( 1 );
    double meanErr  = func.GetParError( 1 );
    double sigma    = func.GetParameter( 2 );
    double sigmaErr = func.GetParError( 2 );
    
    double gasGain    = mean * gainFactor;
    double gasGainErr = meanErr * gainFactor;
    double eneReso    = sigma / mean;
    double eneResoErr = sqrt( mean * mean * sigmaErr * sigmaErr + sigma * sigma * meanErr * meanErr ) / mean / mean;
    
    ShTUtil::CreateDrawText( 0.55, 0.8, Form( "Cal. source: %s", calSource.c_str( ) ) );
    ShTUtil::CreateDrawText( 0.55, 0.72, Form( "Gas gain: %.0lf #pm %.0lf", gasGain, gasGainErr ) );
    ShTUtil::CreateDrawText( 0.55, 0.64, Form( "#sigma_{E}/E: %.2lf #pm %.2lf", eneReso, eneResoErr ) );
    ShTUtil::CreateDrawText( 0.55, 0.56, Form( "Rate: %.1lf Hz", hist.Integral( ) ) );
    
    cvs.SaveAs( Form( "%s/hist.png", outputDir.c_str( ) ) );

    params.m_gasGain    = gasGain;
    params.m_gasGainErr = gasGainErr;
    params.m_eneReso    = eneReso;
    params.m_eneResoErr = eneResoErr;
    
    return params;
}

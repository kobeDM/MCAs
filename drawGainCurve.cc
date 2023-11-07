#include "inc/shinclude.h"
#include "getGasGain.cc"

void drawGainCurve( const String& inputFileList, const String& outputDir, const double& gainFactor, const String& calSource )
{
    // SetShStyle( );

    ShUtil::ExistCreateDir( outputDir );
    
    std::ifstream ifs( inputFileList );
    if( ifs.is_open( ) == false ) return;
    std::map< double, String > fileTable;
    while( !ifs.eof( ) ) {
        String line = "";
        std::getline( ifs, line );
        if( line.size( ) <= 0 ) continue;
        String filePath = "";
        double voltage = 0.0;
        StringStream ss( line );
        ss >> filePath >> voltage;
        fileTable.insert( std::make_pair( voltage, filePath ) );
    }

    double voltageArray[100] = { };
    double gainArray[100] = { };
    double voltageErrArray[100] = { };
    double gainErrArray[100] = { };
    int idx = 0;
    for( auto pair : fileTable ) {
        double voltage = pair.first;
        String perDir = ShUtil::GetFileName( pair.second );
        CalParams params = getGasGain( pair.second, perDir, gainFactor, calSource );
        std::cout << pair.first << "\t" << params.m_gasGain << std::endl;

        voltageArray[idx] = voltage;
        gainArray[idx] = params.m_gasGain;
        voltageErrArray[idx] = 0.001;
        gainErrArray[idx] = params.m_gasGainErr;

        ++idx;
    }

    TGraphErrors gErr( fileTable.size( ), voltageArray, gainArray, voltageErrArray, gainErrArray );
    TCanvas cvs( "cvs", "cvs", 800, 600 );
    cvs.SetLogy( 1 );
    gErr.SetMarkerStyle( 20 );
    gErr.Draw( "AP" );
    gErr.GetXaxis( )->SetTitle( "Anode voltage [V]" );
    gErr.GetXaxis( )->SetRangeUser( 440, 510 );
    gErr.GetYaxis( )->SetTitle( "Gas gain" );
    gErr.GetYaxis( )->SetRangeUser( 5000, 50000 );
    gErr.Draw( "AP" );

    TF1 func( "func", "expo", 445, 495 );
    gErr.Fit( &func, "", "", 445, 495 );

    ShTUtil::CreateDrawText( 0.2, 0.85, Form( "gain = exp(%0.3lf + %0.3lfV)", func.GetParameter( 0 ), func.GetParameter( 1 ) ) );
    
    cvs.SaveAs( Form( "%s/gainCurve.png", outputDir.c_str( ) ) );

    TFile file( Form( "%s/gainCurve.root", outputDir.c_str( ) ), "RECREATE" );
    gErr.Write( );
    func.Write( );
    file.Close( );
    
    return;
}

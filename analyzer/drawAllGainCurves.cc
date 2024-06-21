#include "def.h"

void drawAllGainCurves( const String& inputFileList, const String& outputDir )
{
    SetShStyle( );
    ShUtil::ExistCreateDir( outputDir );
    
    std::ifstream ifs( inputFileList );
    if( ifs.is_open( ) == false ) return;
    std::map< String, String > fileTable;
    while( !ifs.eof( ) ) {
        String line = "";
        std::getline( ifs, line );
        if( line.size( ) <= 0 ) continue;
        String filePath = "";
        String gasMixture = "";
        StringStream ss( line );
        ss >> filePath >> gasMixture;
        fileTable.insert( std::make_pair( gasMixture, filePath ) );
    }

    std::ofstream ofs( Form( "%s/gainCurveList.txt", outputDir.c_str( ) ) );
    ofs << "#GasMixture\tConstant\tSlope" << std::endl;
    TCanvas cvs( "cvs", "cvs", 800, 600 );
    cvs.SetLogy( 1 );
    TMultiGraph multiGraph;
    int colIdx = 1;
    TLegend* pLeg = ShTUtil::CreateLegend( 0.6, 0.2, 0.95, 0.5 );
    std::vector< TF1* > funcArray;
    for( auto pair : fileTable ) {
        String filePath = pair.second;
        String gasMixture = pair.first;

        TFile file( filePath.c_str( ) );
        TGraph* pOrgGraph = dynamic_cast< TGraph* >( file.Get( "Graph" ) );
        TF1* pOrgFunc = dynamic_cast< TF1* >( file.Get( "func" ) );
        if( pOrgGraph == nullptr || pOrgFunc == nullptr ) continue;
        
        TGraph* pGraph = dynamic_cast< TGraph* >( pOrgGraph->Clone( Form( "graph_%s", gasMixture.c_str( ) ) ) );
        TF1*    pFunc  = dynamic_cast< TF1*    >( pOrgFunc->Clone( Form( "func_%s", gasMixture.c_str( ) ) ) );
        if( pGraph == nullptr || pFunc == nullptr ) continue;

        pGraph->SetMarkerColor( colIdx );
        pGraph->SetLineColor( colIdx );
        pFunc->SetLineColor( colIdx );
        pLeg->AddEntry( pGraph, gasMixture.c_str( ), "lep" );
        funcArray.push_back( pFunc );
        multiGraph.Add( pGraph, "p" );

        ofs << gasMixture << "\t" << pFunc->GetParameter( 0 ) << "\t" << pFunc->GetParameter( 1 ) << std::endl;
        
        ++colIdx;
    }
    ofs.close( );
    
    multiGraph.Draw( "a" );

    multiGraph.GetXaxis( )->SetTitle( "Anode voltage [V]" );
    multiGraph.GetXaxis( )->SetLimits( 400.0, 700.0 );
    multiGraph.GetXaxis( )->SetRangeUser( 400.0, 700.0 );
    multiGraph.GetYaxis( )->SetTitle( "Gas gain" );
    multiGraph.GetYaxis( )->SetRangeUser( 1000.0, 1000000.0 );
    multiGraph.Draw( "a" );
    for( auto pFunc : funcArray ) pFunc->Draw( "same" );
    pLeg->Draw( );
    
    cvs.SaveAs( Form("%s/gasMix.png", outputDir.c_str( ) ) );
    
    return;
}

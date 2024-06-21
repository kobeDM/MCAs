#include "def.h"

bool getRate( const String& inputDir, RunParams* pParam );
TGraphErrors* getGraph( std::map< String, TGraphErrors* >* pTable, const String& gasMixture );


void drawGainVsRate( const String& inputFileList, const String& gainCurveParamList, const String& outputDir )
{
    SetShStyle( );
    ShUtil::ExistCreateDir( outputDir );

    // obtain gain curve parameters
    std::ifstream ifsParam( gainCurveParamList );
    if( ifsParam.is_open( ) == false ) return;
    std::map< String, GainCurveParams > gainCurveTable;
    while( !ifsParam.eof( ) ) {
        String line = "";
        std::getline( ifsParam, line );
        if( line.size( ) <= 0 ) continue;
        GainCurveParams params;
        StringStream ss( line );
        ss >> params.m_gasMixture >> params.m_gainCurveConst >> params.m_gainCurveSlope;
        gainCurveTable.insert( std::make_pair( params.m_gasMixture, params ) );
    }
    
    // obtain target file lists
    std::ifstream ifsInput( inputFileList );
    if( ifsInput.is_open( ) == false ) return;
    std::vector<  RunParams > runArray;
    while( !ifsInput.eof( ) ) {
        String line = "";
        std::getline( ifsInput, line );
        if( line.size( ) <= 0 ) continue;
        RunParams params;
        StringStream ss( line );
        ss >> params.m_dirPath >> params.m_anodeVoltage >> params.m_gasMixture;
        
        runArray.push_back( params );
    }

    std::cout << runArray.size( ) << std::endl;
    
    std::map< String, TGraphErrors* > graphTable;
    for( auto runParam : runArray ) {
        // get gain curve parameter
        if( gainCurveTable.find( runParam.m_gasMixture ) == gainCurveTable.end( ) ) continue;
        GainCurveParams gainParam = gainCurveTable.at( runParam.m_gasMixture );
        double gasGain = TMath::Exp( gainParam.m_gainCurveConst + gainParam.m_gainCurveSlope * runParam.m_anodeVoltage );
        
        if( getRate( runParam.m_dirPath, &runParam ) == false ) continue;

        TGraphErrors* pGraph = getGraph( &graphTable, runParam.m_gasMixture );
        if( pGraph == nullptr ) continue;
        int nPoint = pGraph->GetN( );

        double rate = static_cast< double >( runParam.m_totEvt ) / runParam.m_livetime;
        double rateErr = sqrt( static_cast< double >( runParam.m_totEvt ) ) / runParam.m_livetime;
        std::cout << gasGain << "\t" << rate << "\t" << rateErr << std::endl;

        pGraph->SetPoint( nPoint, gasGain, rate );
        pGraph->SetPointError( nPoint, 0.001, rateErr );
    }
    
    TMultiGraph multiGraph;
    int colIdx = 1;
    TLegend* pLeg = ShTUtil::CreateLegend( 0.5, 0.6, 0.9, 0.9 );
    for( auto pair : graphTable ) {
        TGraphErrors* pGraph = pair.second;
        if( pGraph == nullptr ) continue;
        std::cout << pGraph->GetName( ) << std::endl;
        pGraph->SetMarkerColor( colIdx );
        pGraph->SetMarkerStyle( colIdx + 19 );
        pGraph->SetLineColor( colIdx );
        multiGraph.Add( pair.second, "p" );
        pLeg->AddEntry( pGraph, pair.first.c_str( ), "lep" );
        ++colIdx;
    }

    std::cout << graphTable.size( ) << std::endl;
    TCanvas cvs( "cvs", "cvs", 800, 600 );
    gPad->SetRightMargin(0.1);
    multiGraph.Draw( "a" );
    multiGraph.GetXaxis( )->SetTitle( "Gas gain (expected)" );
    multiGraph.GetYaxis( )->SetTitle( "Rate [Hz]" );
    multiGraph.Draw( "a" );
    pLeg->Draw( );
    cvs.SaveAs( Form( "%s/gainVsRate.png", outputDir.c_str( ) ) );
    cvs.SaveAs( Form( "%s/gainVsRate.pdf", outputDir.c_str( ) ) );
    
    return;
}


bool getRate( const String& inputDir, RunParams* pParam )
{
    if( pParam == nullptr ) return false;
    
    std::vector< String > filepathArr;
    String fileKey = "mca";
    if( ShUtil::GetFilePathArr( inputDir, &filepathArr, false, fileKey ) == false ) return false;

    double totalLivetime = 0.0;
    double totalEvents = 0.0;
    for( auto name : filepathArr ) {
        std::cout << "Start persing " << name << " ..." << std::endl;
        std::ifstream ifs( name );
        if( ifs.is_open( ) == false ) continue;

        bool parseStart = false;
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
                continue;
            }
            if( line.find("<<END>>") != String::npos ) break;

            if( parseStart == true ) {
                int mcaEvents = std::stoi( line );
                totalEvents += static_cast< double >( mcaEvents );
                // if( mcaEvents > 100 ) std::cout << mcaEvents << std::endl;
            }
        }

        totalLivetime += livetime;
    }

    pParam->m_totEvt = totalEvents;
    pParam->m_livetime = totalLivetime;
    
    return true;
}

TGraphErrors* getGraph( std::map< String, TGraphErrors* >* pTable, const String& gasMixture )
{
    if( pTable == nullptr ) return nullptr;
    TGraphErrors* pGraph = nullptr;

    if( pTable->find( gasMixture ) == pTable->end( ) ) {
        pGraph = new TGraphErrors( );
        pGraph->SetName( Form( "graph_%s", gasMixture.c_str( ) ) );
        std::cout << "create graph" << std::endl;
        pTable->insert( std::make_pair( gasMixture, pGraph ) );
    }
    else {
        pGraph = pTable->at( gasMixture );
    }
    
    return pGraph;
}


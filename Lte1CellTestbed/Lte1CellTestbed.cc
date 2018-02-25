// Single Cell Test bed for LTE Cellular communications

/*
Run Commands:
1.  ./waf --run scratch/Lte1CellTestbed/Lte1CellTestbed --cwd scratch/Lte1CellTestbed/logs

2. ./waf --command-template="%s --ns3::ConfigStore::Filename=run.cfg --ns3::ConfigStore::Mode=Load --ns3::ConfigStore::FileFormat=RawText" --run "scratch/Lte1CellTestbed/Lte1CellTestbed" --cwd "scratch/Lte1CellTestbed/logs"
*/

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include <ns3/config-store.h>

#include "progress-bar.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE( "Lte1CellTestbed" );

int main( int argc, char *argv[] ) {

    // Load experiment configuration
    CommandLine cmd;
    cmd.Parse( argc, argv );
    ConfigStore inputConfig;
    inputConfig.ConfigureDefaults();
    cmd.Parse( argc, argv );    

    // Position for Cell Tower -- aa points in 'm'
    double enbX = 0.00, enbY = 0.00, enbZ = 30.00;
    double cellRadius   = 1000.00;

    uint32_t nMobileUes = 4;
    uint32_t nStaticUes = 4;

    double simDuration    = 75.00;
    Time::SetResolution( Time::NS );

    // Create Nodes
    NodeContainer nodesEnb; nodesEnb.Create( 1 );
    NodeContainer nodesUesMobile;   nodesUesMobile.Create( nMobileUes );
    NodeContainer nodesUesStatic;   nodesUesStatic.Create( nStaticUes );
    NodeContainer nodesUes; nodesUes.Add( nodesUesMobile ); nodesUes.Add( nodesUesStatic );

    // Setup mobility models for eNodeBs and UEs
    // Setup mobility for eNodeB -- ConstantPositionMobilityModel
    Ptr<ListPositionAllocator> posAllocEnb = CreateObject<ListPositionAllocator>();
    posAllocEnb->Add( Vector(enbX,enbY,enbZ) );
    MobilityHelper mobilityHelperEnb;
    mobilityHelperEnb.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );
    mobilityHelperEnb.SetPositionAllocator( posAllocEnb );
    mobilityHelperEnb.Install( nodesEnb );
    // Setup mobility for mobile UE nodes
    Ptr<UniformRandomVariable> wpRadSampler = CreateObject<UniformRandomVariable>();
    wpRadSampler->SetAttribute( "Max", DoubleValue(1.00*cellRadius) );
    wpRadSampler->SetAttribute( "Min", DoubleValue(0.98*cellRadius) );
    Ptr<ListPositionAllocator> posAllocUeMobile = CreateObject<ListPositionAllocator>();
    for( unsigned ueIdx = 0; ueIdx < nMobileUes; ueIdx++ ) {
        posAllocUeMobile->Add( Vector(enbX,enbY,1.5) );
    }
    Ptr<RandomDiscPositionAllocator> wpAllocUeMobile = CreateObject<RandomDiscPositionAllocator>();
    wpAllocUeMobile->SetX(enbX);    wpAllocUeMobile->SetX(enbY);    wpAllocUeMobile->SetRho(wpRadSampler);
    MobilityHelper mobilityHelperUeMobile;
    mobilityHelperUeMobile.SetMobilityModel( "ns3::RandomWaypointMobilityModel",
        "Speed", StringValue("ns3::ConstantRandomVariable[Constant=16.67]"),    // in 'm/s'
        "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.00]"),
        "PositionAllocator", PointerValue(wpAllocUeMobile) );
    mobilityHelperUeMobile.SetPositionAllocator( posAllocUeMobile );
    mobilityHelperUeMobile.Install( nodesUesMobile );
    // Setup mobility for static UEs
    Ptr<UniformDiscPositionAllocator> posAllocUeStatic  = CreateObject<UniformDiscPositionAllocator>();
    posAllocUeStatic->SetX(enbX);   posAllocUeStatic->SetY(enbY);   posAllocUeStatic->SetRho(cellRadius);
    MobilityHelper mobilityHelperUeStatic;
    mobilityHelperUeStatic.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );
    mobilityHelperUeStatic.SetPositionAllocator( posAllocUeStatic );
    mobilityHelperUeStatic.Install( nodesUesStatic );

    // Setup LTE network
    Ptr<LteHelper> lteHelper    = CreateObject<LteHelper>();
    NetDeviceContainer devsEnb; devsEnb = lteHelper->InstallEnbDevice( nodesEnb );
    NetDeviceContainer devsUes; devsUes = lteHelper->InstallUeDevice( nodesUes );
    lteHelper->Attach( devsUes, devsEnb.Get(0) );

    // Activate saturation traffic
    enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer( q );
    lteHelper->ActivateDataRadioBearer( devsUes, bearer );

    // Enable Logging
    lteHelper->EnableTraces();

    // Setup simulation durtation
    Simulator::Stop( Seconds(simDuration) );

    // Setup Progress bar
    sim::ProgressBar progressBar( simDuration );
    progressBar.Enable();

    Simulator::Run();

    Simulator::Destroy();

    return 0;
}
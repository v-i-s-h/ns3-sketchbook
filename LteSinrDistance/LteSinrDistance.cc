// Simple LTE simulation
// export NS_LOG=LteSinrDistance=info
#include <iomanip>
#include <iostream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE( "LteSinrDistance" );

// Definitions for Mobility Tracking -- comment to disable trace
// #define ENABLE_MOBILITY_TRACKING

#ifdef ENABLE_MOBILITY_TRACKING
static void CourseChange (std::string traceStr, Ptr<const MobilityModel> mobility) {
  Vector pos = mobility->GetPosition ();
  Vector vel = mobility->GetVelocity ();
  std::cout << Simulator::Now ()
            << " Trace:" << traceStr << "    "
            << " POS: (" << pos.x << "," << pos.y << "," << pos.z << ")"
            << " VEL: (" << vel.x << "," << vel.y << "," << vel.z << ")" << std::endl;
}

static void ReportPosition( Ptr<Node> thisNode ) {
    Vector pos = thisNode->GetObject<MobilityModel>()->GetPosition();
    Vector vel = thisNode->GetObject<MobilityModel>()->GetVelocity();
    std::cout << Simulator::Now()
              << " Node#" << thisNode->GetId()
              << " POS: (" << pos.x << "," << pos.y << "," << pos.z << ")"
              << " VEL: (" << vel.x << "," << vel.y << "," << vel.z << ")"
              << std::endl;
}
#endif


std::stringstream ue1LogStream;
std::stringstream ue2LogStream;
static void ReportUeMeasurements( std::string traceStr, uint16_t cellId, uint16_t rnti, double rsrp, double sinr ) {

    // Divide the stream to ue1 and ue2
    if( rnti == 1 ) {
        ue1LogStream << Simulator::Now().GetNanoSeconds() / (double) 1e9 << "\t" << rsrp << "\t" << sinr << "\n";
    } else if( rnti == 2 ) {
        ue2LogStream << Simulator::Now().GetNanoSeconds() / (double) 1e9 << "\t" << rsrp << "\t" << sinr << "\n";
    }
}

int main(int argc, char const *argv[]) {

    double enbX = 0.00, enbY = 0.00, enbZ = 30.00;  // Base Station Position in canvas
    // double ueMaxRadius  = 1000.00;  // Radius around eNodeB for users to allocate positions and waypoints

    uint32_t noOfUeNodes = 2;

    // All timing params for experiment
    Time simDuration                = Seconds(60.00);  // In Seconds
    Time startTime_FTPSrcApp        = Seconds(60.10);
    Time startTime_FTPSnkApp        = Seconds(60.00);

    Vector ue0StartPoint    = Vector(enbX+100,enbY,1);
    Vector ue1StartPoint    = Vector(enbX,enbY+100,1);
    Vector ue0EndPoint      = Vector(enbX+1000,enbY,1);
    Vector ue1EndPoint      = Vector(enbX,enbY+1000,1);

    // Setup time resolution
    Time::SetResolution( Time::NS );

    // Create eNodeBs
    NS_LOG_INFO( "Creating eNodeB nodes..." );
    NodeContainer eNodeBs;  eNodeBs.Create(1);
    // Create UEs
    NS_LOG_INFO( "Creating UE nodes..." );
    NodeContainer ueNodes;  ueNodes.Create(noOfUeNodes);

    NS_LOG_INFO( "Creating a single remote node..." );
    NodeContainer remoteNode;   remoteNode.Create(1);

    // ####################   SETUP MOBILTY MODELS FOR SIMUALTION ##############
    // NS_LOG_INFO( "Setting up mobility model..." );
    // MobilityHelper  mobModel;
    // mobModel.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );  mobModel.Install( eNodeBs );
    // mobModel.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );  mobModel.Install( ueNodes );

    NS_LOG_INFO( "Setting up mobility model..." );
    // Setup mobility for eNodeB
    MobilityHelper enbMobility;
    Ptr<ListPositionAllocator> enbPosAlloc = CreateObject<ListPositionAllocator>();
    enbPosAlloc->Add( Vector(enbX,enbY,enbZ) );
    enbMobility.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );
    enbMobility.SetPositionAllocator( enbPosAlloc );
    enbMobility.Install( eNodeBs );

    // Setup mobility waypoints for UEs
    Ptr<ListPositionAllocator> uePosAlloc = CreateObject<ListPositionAllocator>();
    uePosAlloc->Add( ue0StartPoint );
    uePosAlloc->Add( ue1StartPoint );
    Ptr<ListPositionAllocator> ueWpAlloc  = CreateObject<ListPositionAllocator>();
    ueWpAlloc->Add( ue0EndPoint );
    ueWpAlloc->Add( ue1EndPoint );
    MobilityHelper ueMobility;
    ueMobility.SetMobilityModel( "ns3::RandomWaypointMobilityModel",
        "Speed", StringValue("ns3::ConstantRandomVariable[Constant=16.67]"),
        "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"),
        "PositionAllocator", PointerValue(ueWpAlloc) );
    ueMobility.SetPositionAllocator( uePosAlloc );
    NS_LOG_INFO( "Installing MobilityModel to UEs..." );
    ueMobility.Install( ueNodes );

#ifdef ENABLE_MOBILITY_TRACKING
    NS_LOG_INFO( "Install MobilityModel tracking hooks..." );
    // Enable tracing the UE positions
    Config::Connect( "/NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange) );
    // Create events every second to report back the current position of UEs
    for( uint32_t ueIdx = 0; ueIdx < 2; ueIdx ++ ) {
        for( uint32_t stepIdx = 0; stepIdx < simDuration.GetSeconds(); stepIdx++ ) {
            Simulator::Schedule( Time(Seconds(stepIdx)), ReportPosition, ueNodes.Get(ueIdx) );
        }
    }
#endif
    // ######################## END OF MOBILITY SETUP ##########################

    // ########################## LTE NETWORK SETUP ############################
    // Create an Lte Helper
    Ptr<LteHelper> lteHelper    = CreateObject<LteHelper>();
    Ptr<PointToPointEpcHelper> epcHelper    = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper( epcHelper );   // Install EPC Helper to LTE
    Ptr<Node> pgwNode   = epcHelper->GetPgwNode();

    lteHelper->SetFadingModel( "ns3::TraceFadingLossModel" );
    lteHelper->SetFadingModelAttribute( "TraceFilename", StringValue ("../../src/lte/model/fading-traces/fading_trace_EVA_60kmph.fad") );
    lteHelper->SetFadingModelAttribute( "TraceLength", TimeValue (Seconds (10.0)) );
    lteHelper->SetFadingModelAttribute( "SamplesNum", UintegerValue (10000) );
    lteHelper->SetFadingModelAttribute( "WindowSize", TimeValue (Seconds (0.5)) );
    lteHelper->SetFadingModelAttribute( "RbNum", UintegerValue (100) );

    // Set up remote node for internet access
    InternetStackHelper inetHelper; inetHelper.Install( remoteNode );

    // Create a Point To Point link between ENB's PGW and remote node
    PointToPointHelper p2pHelper;
    p2pHelper.SetDeviceAttribute( "DataRate", DataRateValue(DataRate("100Gb/s")) );
    p2pHelper.SetDeviceAttribute( "Mtu", UintegerValue(1500) );
    p2pHelper.SetChannelAttribute( "Delay", TimeValue(Seconds(0.010)) );
    NetDeviceContainer inetDev = p2pHelper.Install( pgwNode, remoteNode.Get(0) );

    // Assign IP addresses
    Ipv4AddressHelper ipv4Helper;
    ipv4Helper.SetBase( "1.0.0.0", "255.0.0.0" );
    Ipv4InterfaceContainer ipv4interfaces = ipv4Helper.Assign( inetDev );

    // State the routing
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting( remoteNode.Get(0)->GetObject<Ipv4>() );
    remoteNodeStaticRouting->AddNetworkRouteTo( Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1 );

    // Install LTE Network device at eNodeBs
    NS_LOG_INFO( "Installing LTE network device at eNodeB" );
    NetDeviceContainer enbDevs  = lteHelper->InstallEnbDevice( eNodeBs );
    // Install LTE Network device at ueNodes
    NS_LOG_INFO( "Installing LTE network device at UE" );
    NetDeviceContainer ueDevs   = lteHelper->InstallUeDevice( ueNodes );

    NS_LOG_INFO( "Installing Internet Stack on UEs..." );
    // Install Internet Stack to UEs
    inetHelper.Install( ueNodes );

    // assign IP addresses to UEs and add apps
    NS_LOG_INFO( "Assigning IP Addresses to UEs..." );
    Ipv4InterfaceContainer ueIpInf = epcHelper->AssignUeIpv4Address( ueDevs );
    for( uint32_t idx = 0; idx < ueNodes.GetN(); idx++ ) {
        Ptr<Node> thisUe    = ueNodes.Get( idx );
        Ptr<NetDevice> thisUeDev = ueDevs.Get( idx );
        // Setup default gateway
        Ptr<Ipv4StaticRouting>ueNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting( thisUe->GetObject<Ipv4>() );
        ueNodeStaticRouting->SetDefaultRoute( epcHelper->GetUeDefaultGatewayAddress(), 1 );
    }

    // Attach UEs to eNodeB
    NS_LOG_INFO( "Attaching UEs to eNodeB" );
    lteHelper->Attach( ueDevs, enbDevs.Get(0) );

    // Get IP address of all nodes
    std::cout << "IP v4 Addresses: " << std::endl
              << "    Remote Node: " << remoteNode.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal() << std::endl
              << "    Node 0     : " << ueNodes.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal() << std::endl
              << "    Node 1     : " << ueNodes.Get(1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal() << std::endl
              << "    PGW        : " << pgwNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal() << std::endl;

    // Enable tracing functionality for UE
    Config::Connect( "/NodeList/*/DeviceList/*/LteUePhy/ReportCurrentCellRsrpSinr", MakeCallback(&ReportUeMeasurements) );
    
    // ####################### END OF LTE SETUP ################################


    // ################### SETUP A LARGE FILE TRANSFER #########################
    Ptr<Node> sourceNode = ueNodes.Get(0);
    Ptr<Node> sinkNode   = remoteNode.Get(0);

    ApplicationContainer FTPSrcApps;
    ApplicationContainer FTPSnkApps;

    // Ipv4Address srcNodeAddress = sourceNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
    Ipv4Address snkNodeAddress = sinkNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();

    // Create a general FTP Application
    OnOffHelper FTPApplication( "ns3::UdpSocketFactory", InetSocketAddress(snkNodeAddress,21) );
    FTPApplication.SetAttribute( "MaxBytes", UintegerValue(2*1024*1024) );    // 20MB File
    FTPApplication.SetAttribute( "PacketSize", UintegerValue(1024) );  // 1KB packet size
    FTPApplication.SetAttribute( "DataRate", DataRateValue(2*1024*1024) ); // Am I asking for too much?
    FTPApplication.SetAttribute( "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]") );  // chatter always :P
    FTPApplication.SetAttribute( "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]") ); // why shutup??
    // Install FTP Application at Source Node
    FTPSrcApps.Add( FTPApplication.Install(sourceNode) );

    // Create a generic packet sink application
    PacketSinkHelper FTPSinkApp( "ns3::UdpSocketFactory", InetSocketAddress(snkNodeAddress,21) );
    // Install packet sink at Sink Node
    FTPSnkApps.Add( FTPSinkApp.Install(sinkNode) );

    // Schedule Start of File Transfer
    FTPSrcApps.Start( startTime_FTPSrcApp );   // Do you really want to stop?
    FTPSnkApps.Start( startTime_FTPSnkApp );
    // Not Stopping the data transfer explicitly, they should stop by themselves once the file is send(?)
    // ################### END  OF LARGE FILE TRANSFER #########################

    // Enable LTE Traces
    NS_LOG_INFO( "Enabling LTE Traces..." );
    lteHelper->EnableTraces();

    Simulator::Stop( simDuration );

    NS_LOG_INFO( "Starting Simulator..." );
    Simulator::Run();
    NS_LOG_INFO( "Stoping Simulator..." );

    // Save the UE traces
    std::ofstream ue1File( "ue1Traces.txt", std::ios::in|std::ios::trunc );
    ue1File << ue1LogStream.str();    ue1File.close();
    std::ofstream ue2File( "ue2Traces.txt", std::ios::in|std::ios::trunc );
    ue2File << ue2LogStream.str();    ue2File.close();

    Simulator::Destroy();

    return 0;
}

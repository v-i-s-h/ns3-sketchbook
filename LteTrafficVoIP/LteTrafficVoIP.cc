// Simple LTE simulation
// export NS_LOG=LteTrafficVoIP=info
#include <iomanip>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE( "LteTrafficVoIP" );

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

int main(int argc, char const *argv[]) {

    double enbX = 0.00, enbY = 0.00, enbZ = 30.00;  // Base Station Position in canvas
    double ueMaxRadius  = 1000.00;  // Radius around eNodeB for users to allocate positions and waypoints

    uint32_t noOfUeNodes = 2;
    uint32_t simDuration = 20;  // In Seconds

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
    Ptr<UniformRandomVariable> radiusSampler    = CreateObject<UniformRandomVariable>();
    radiusSampler->SetAttribute( "Max", DoubleValue(ueMaxRadius) );
    radiusSampler->SetAttribute( "Min", DoubleValue( 0.00) );

    Ptr<RandomDiscPositionAllocator> uePosAlloc = CreateObject<RandomDiscPositionAllocator>();
    uePosAlloc->SetX(enbX);   uePosAlloc->SetY(enbY);   uePosAlloc->SetRho(radiusSampler);

    Ptr<RandomDiscPositionAllocator> ueWpAlloc  = CreateObject<RandomDiscPositionAllocator>();
    ueWpAlloc->SetX(enbX);  ueWpAlloc->SetY(enbY);  ueWpAlloc->SetRho(radiusSampler);
    MobilityHelper ueMobility;
    ueMobility.SetMobilityModel( "ns3::RandomWaypointMobilityModel",
        "Speed", StringValue("ns3::ConstantRandomVariable[Constant=0.9]"),
        "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"),
        "PositionAllocator", PointerValue(ueWpAlloc) );
    ueMobility.SetPositionAllocator( uePosAlloc );
    ueMobility.Install( ueNodes );

#ifdef ENABLE_MOBILITY_TRACKING
    // Enable tracing the UE positions
    Config::Connect( "/NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange) );
    // Create events every second to report back the current position of UEs
    for( uint32_t ueIdx = 0; ueIdx < 2; ueIdx ++ ) {
        for( uint32_t stepIdx = 0; stepIdx < simDuration; stepIdx++ ) {
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

    // lteHelper->SetFadingModel("ns3::TraceFadingLossModel");
    // lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("../../src/lte/model/fading-traces/fading_trace_ETU_3kmph.fad"));
    // lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
    // lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
    // lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
    // lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (100));

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


    // ####################### END OF LTE SETUP ################################

    // ########################## SETUP UDP ECHO APP ###########################
    LogComponentEnable( "UdpEchoClientApplication", LOG_LEVEL_INFO );
    LogComponentEnable( "UdpEchoServerApplication", LOG_LEVEL_INFO );

    Ptr<Node> svrNode       = ueNodes.Get(1); //remoteNode.Get(0);
    Ptr<Node> clientNode    = ueNodes.Get(0);

    NS_LOG_INFO( "Setting up UDP Echo Server..." );
    UdpEchoServerHelper echoServer( 9 );
    ApplicationContainer svrApp = echoServer.Install( svrNode );
    svrApp.Start( Seconds(1.5) );   svrApp.Stop( Seconds(8.0) );

    NS_LOG_INFO( "Setting up UDP Echo Client..." );
    UdpEchoClientHelper echoClient( svrNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 9 );
    echoClient.SetAttribute( "MaxPackets", UintegerValue(2) );
    echoClient.SetAttribute( "Interval", TimeValue(Seconds(1.0)) );
    echoClient.SetAttribute( "PacketSize", UintegerValue(1024) );
    ApplicationContainer clientApp = echoClient.Install( clientNode );
    clientApp.Start( Seconds(2.1) );    clientApp.Stop( Seconds(5.0) );
    // ########################### END OF APP SETUP ############################

    // ######################## SETUP VoIP BETWEEN UEs #########################
    LogComponentEnable( "OnOffApplication", LOG_LEVEL_INFO );
    LogComponentEnable( "PacketSink", LOG_LEVEL_INFO );
    // Create two Data Streams
    uint32_t inPort     = 1729;

    Ptr<Node> voipNode0 = ueNodes.Get(0);
    Ptr<Node> voipNode1 = ueNodes.Get(1);

    Ipv4Address node0Address = voipNode0->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
    Ipv4Address node1Address = voipNode1->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();

    NS_LOG_INFO( "Creating two traffic streams between nodes " << node0Address << " and " << node1Address );

    ApplicationContainer sourceApps;
    ApplicationContainer sinkApps;

    ExponentialRandomVariable onTimeRV;
    ExponentialRandomVariable offTImeRV;

    OnOffHelper ooHelper0( "ns3::UdpSocketFactory", InetSocketAddress(node0Address,inPort) );
    ooHelper0.SetAttribute( "MaxBytes", UintegerValue(774172) );
    ooHelper0.SetAttribute( "PacketSize", UintegerValue(172) );
    ooHelper0.SetAttribute( "DataRate", DataRateValue(68800) );
    ooHelper0.SetAttribute( "OnTime", StringValue("ns3::ExponentialRandomVariable[Mean=0.352][Bound=2.0]") );
    ooHelper0.SetAttribute( "OffTime", StringValue("ns3::ExponentialRandomVariable[Mean=0.650][Bound=2.0]") );
    sourceApps.Add( ooHelper0.Install(voipNode1) );

    OnOffHelper ooHelper1( "ns3::UdpSocketFactory", InetSocketAddress(node1Address,inPort) );
    ooHelper1.SetAttribute( "MaxBytes", UintegerValue(774172) );
    ooHelper1.SetAttribute( "PacketSize", UintegerValue(172) );
    ooHelper1.SetAttribute( "DataRate", DataRateValue(68800) );
    ooHelper1.SetAttribute( "OnTime", StringValue("ns3::ExponentialRandomVariable[Mean=0.352][Bound=2.0]") );
    ooHelper1.SetAttribute( "OffTime", StringValue("ns3::ExponentialRandomVariable[Mean=0.650][Bound=2.0]") );
    sourceApps.Add( ooHelper1.Install(voipNode0) );

    PacketSinkHelper sinkHelper0( "ns3::UdpSocketFactory", InetSocketAddress(node0Address,inPort) );
    sinkApps.Add( sinkHelper0.Install(voipNode0) );
    PacketSinkHelper sinkHelper1( "ns3::UdpSocketFactory", InetSocketAddress(node1Address,inPort) );
    sinkApps.Add( sinkHelper1.Install(voipNode1) );

    sourceApps.Start( Seconds(3.5) );    sourceApps.Stop( Seconds(9.5) );   // A 6 sec chit chat
    sinkApps.Start( Seconds(3.4) );  sinkApps.Stop( Seconds(9.6) );

    // ########################### END OF APP SETUP ############################

    // ################### SETUP A LARGE FILE TRANSFER #########################
    Ptr<Node> sourceNode = ueNodes.Get(0);
    Ptr<Node> sinkNode   = remoteNode.Get(0);

    ApplicationContainer FTPSrcApps;
    ApplicationContainer FTPSnkApps;

    // Ipv4Address srcNodeAddress = sourceNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
    Ipv4Address snkNodeAddress = sinkNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();

    // Create a general FTP Application
    OnOffHelper FTPApplication( "ns3::UdpSocketFactory", InetSocketAddress(snkNodeAddress,21) );
    FTPApplication.SetAttribute( "MaxBytes", UintegerValue(20*1024*1024) );    // 20MB File
    FTPApplication.SetAttribute( "PacketSize", UintegerValue(1024) );  // 1KB packet size
    FTPApplication.SetAttribute( "DataRate", DataRateValue(2*1024*1024) ); // I'm I asking for too much?
    FTPApplication.SetAttribute( "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]") );  // chatter always :P
    FTPApplication.SetAttribute( "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]") ); // why shutup??
    // Install FTP Application at Source Node
    FTPSrcApps.Add( FTPApplication.Install(sourceNode) );

    // Create a generic packet sink application
    PacketSinkHelper FTPSinkApp( "ns3::UdpSocketFactory", InetSocketAddress(snkNodeAddress,21) );
    // Install packet sink at Sink Node
    FTPSnkApps.Add( FTPSinkApp.Install(sinkNode) );

    // Schedule Start of File Transfer
    FTPSrcApps.Start( Seconds(5.0) );   // Do you really want to stop?
    FTPSnkApps.Start( Seconds(4.9) );
    // No Stopping the dats transfer explicitly, they should stop by themselves once the tht file is send(?)
    // ################### END  OF LARGE FILE TRANSFER #########################

    // Enable LTE Traces
    NS_LOG_INFO( "Enabling LTE Traces..." );
    lteHelper->EnableTraces();

    Simulator::Stop( Seconds(simDuration) );

    NS_LOG_INFO( "Starting Simulator..." );
    Simulator::Run();
    NS_LOG_INFO( "Stoping Simulator..." );

    Simulator::Destroy();

    return 0;
}

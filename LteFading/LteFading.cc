// Simple LTE simulation
// export NS_LOG=LteModel0=info
#include <iomanip>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE( "LteModel0" );

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

void
PrintGnuplottableUeListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
          if (uedev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"" << uedev->GetImsi ()
                      << "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,4\" textcolor rgb \"grey\" front point pt 1 ps 0.3 lc rgb \"grey\" offset 0,0"
                      << std::endl;
            }
        }
    }
}

void
PrintGnuplottableEnbListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
          if (enbdev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"" << enbdev->GetCellId ()
                      << "\" at "<< pos.x << "," << pos.y
                      << " left font \"Helvetica,4\" textcolor rgb \"white\" front  point pt 2 ps 0.3 lc rgb \"white\" offset 0,0"
                      << std::endl;
            }
        }
    }
}

int main(int argc, char const *argv[]) {

    double enbX = 0.00, enbY = 0.00, enbZ = 30.00;  // Base Station Position in canvas
    double ueMaxRadius  = 1000.00;  // Radius around eNodeB for users to allocate positions and waypoints


    uint32_t noOfUeNodes = 2;
    uint32_t simDuration = 5;  // In Seconds


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
        "Speed", StringValue("ns3::ConstantRandomVariable[Constant=8.3]"),
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

    //Setting Fading Model
    //Fading trace configuration
     NS_LOG_INFO("Fading model settings");

    lteHelper->SetFadingModel("ns3::TraceFadingLossModel");

    lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("../../src/lte/model/fading-traces/fading_trace_ETU_3kmph.fad"));



    lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
    lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
    lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
    lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (100));


    Ptr<PointToPointEpcHelper> epcHelper    = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper( epcHelper );   // Install EPC Helper to LTE
    Ptr<Node> pgwNode   = epcHelper->GetPgwNode();

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
    uint16_t dlPort = 1729;
    for( uint32_t idx = 0; idx < ueNodes.GetN(); idx++ ) {
        Ptr<Node> thisUe    = ueNodes.Get( idx );
        Ptr<NetDevice> thisUeDev = ueDevs.Get( idx );
        // Setup default gateway
        Ptr<Ipv4StaticRouting>ueNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting( thisUe->GetObject<Ipv4>() );
        ueNodeStaticRouting->SetDefaultRoute( epcHelper->GetUeDefaultGatewayAddress(), 1 );
        // Add Apps
        PacketSinkHelper packetSinkHelper( "ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(),dlPort) );
        ApplicationContainer serverApps = packetSinkHelper.Install( thisUe );
        serverApps.Start( Seconds(0.01) );
        UdpClientHelper client( ueIpInf.GetAddress(idx), dlPort );
        ApplicationContainer clientApps = client.Install( remoteNode );
        clientApps.Start( Seconds(0.01) );
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

    // ########################### ACTIVATE EPS BEARER #######################
    // Ptr<EpcTft> tft = Create<EpcTft>();
    // EpcTft::PacketFilter pf;
    // pf.localPortStart = 1729;
    // pf.localPortEnd = 1729;
    // tft->Add (pf);
    // lteHelper->ActivateDedicatedEpsBearer (ueDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), tft);
    // #########################################################################

    // ######################## SETUP DATA APPLICATIONS ########################
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    Ptr<Node> svrNode       = remoteNode.Get(0);
    Ptr<Node> clientNode    = ueNodes.Get(0);

    NS_LOG_INFO( "Setting up UDP Echo Server..." );
    UdpEchoServerHelper echoServer( 9 );
    ApplicationContainer svrApp = echoServer.Install( svrNode );
    svrApp.Start( Seconds(1.5) );   svrApp.Stop( Seconds(8.0) );

    NS_LOG_INFO( "Setting up UDP Echo Client..." );
    UdpEchoClientHelper echoClient( svrNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 9 );
    echoClient.SetAttribute( "MaxPackets", UintegerValue(3) );
    echoClient.SetAttribute( "Interval", TimeValue(Seconds(1.0)) );
    echoClient.SetAttribute( "PacketSize", UintegerValue(1024) );
    ApplicationContainer clientApp = echoClient.Install( clientNode );
    clientApp.Start( Seconds(2.0) );    clientApp.Stop( Seconds(7.9) );

    // ########################### END OF APP SETUP ############################

    lteHelper->EnableTraces();

    // Ptr<RadioEnvironmentMapHelper> remHelper;
    // PrintGnuplottableEnbListToFile( "enbs.txt" );
    // PrintGnuplottableUeListToFile( "ues.txt" );

    Simulator::Stop( Seconds(simDuration) );

    NS_LOG_INFO( "Starting Simulator..." );
    Simulator::Run();
    NS_LOG_INFO( "Stoping Simulator..." );

    Simulator::Destroy();

    return 0;
}

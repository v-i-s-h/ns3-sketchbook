// LTE 1 Cell testbed
// export NS_LOG=Lte1CellTestbed=info
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

NS_LOG_COMPONENT_DEFINE( "Lte1CellTestbed" );

int main( int argc, char *argv[] ) {

    double enbX = 0.00, enbY = 0.00, enbZ = 30.00;
    double cellRadius  = 500.00;

    uint32_t noOfMobileUEs  = 2;
    uint32_t noOfStaticUEs  = 4;

    // Timind specification for Simulation
    Time simDuration    = Seconds( 15.00 );

    Time::SetResolution( Time::NS );

    // ############################# CREATE NODES ##############################
    NS_LOG_INFO( "Creating eNodeB nodes..." );
    NodeContainer enbNodes;  enbNodes.Create( 1 );
    NS_LOG_INFO( "Creating Mobile UE nodes..." );
    NodeContainer ueNodesMobile;    ueNodesMobile.Create( noOfMobileUEs );
    NS_LOG_INFO( "Creating Static UE nodes..." );
    NodeContainer ueNodesStatic;    ueNodesStatic.Create( noOfStaticUEs );
    NodeContainer ueNodes;
    ueNodes.Add( ueNodesMobile );   ueNodes.Add( ueNodesStatic );
    NS_LOG_INFO( "Creating Remote node..." );
    NodeContainer remoteNode;   remoteNode.Create( 1 );

    // ###################### SETUP MOBILITY MODELS ############################
    NS_LOG_INFO( "Setting up mobility for eNodeB..." );
    Ptr<ListPositionAllocator> posAllocEnb = CreateObject<ListPositionAllocator>();
    posAllocEnb->Add( Vector(enbX,enbY,enbZ) );
    MobilityHelper mobHlprEnb;
    mobHlprEnb.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );
    mobHlprEnb.SetPositionAllocator( posAllocEnb );
    mobHlprEnb.Install( enbNodes );

    NS_LOG_INFO( "Setting up mobility for Mobile UEs..." );
    // Random direction for each mobile UE
    Ptr<UniformRandomVariable> wpRadiusSampler  = CreateObject<UniformRandomVariable>();
    wpRadiusSampler->SetAttribute( "Max", DoubleValue(1.00*cellRadius) );
    wpRadiusSampler->SetAttribute( "Min", DoubleValue(0.98*cellRadius) );
    // Position allocator for Mobile UEs
    Ptr<ListPositionAllocator> posAllocMobileUe = CreateObject<ListPositionAllocator>();
    for( unsigned ueIdx = 0; ueIdx < noOfMobileUEs; ueIdx++ ) {
        posAllocMobileUe->Add( Vector(enbX,enbY,0.00) );
    }
    Ptr<RandomDiscPositionAllocator> wpAllocMobileUe    = CreateObject<RandomDiscPositionAllocator>();
    wpAllocMobileUe->SetX(enbX);    wpAllocMobileUe->SetY(enbY);    wpAllocMobileUe->SetRho(wpRadiusSampler);
    MobilityHelper mobHlprNodeUeMobile;
    mobHlprNodeUeMobile.SetMobilityModel( "ns3::RandomWaypointMobilityModel",
        "Speed", StringValue("ns3::ConstantRandomVariable[Constant=16.67]"),
        "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.00]"),
        "PositionAllocator", PointerValue(wpAllocMobileUe) );
    mobHlprNodeUeMobile.SetPositionAllocator( posAllocMobileUe );
    mobHlprNodeUeMobile.Install( ueNodesMobile );

    NS_LOG_INFO( "Setting up mobility for Static UEs..." );
    Ptr<UniformDiscPositionAllocator> posAllocStaticUe  = CreateObject<UniformDiscPositionAllocator>();
    posAllocStaticUe->SetX(enbX);   posAllocStaticUe->SetY(enbY);   posAllocStaticUe->SetRho(cellRadius);
    MobilityHelper mobHlprNodeUeStatic;
    mobHlprNodeUeStatic.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );
    mobHlprNodeUeStatic.SetPositionAllocator( posAllocStaticUe );
    mobHlprNodeUeStatic.Install( ueNodesStatic );

    // ############################ SETUP LTE ##################################
    Ptr<LteHelper> lteHelperStatic  = CreateObject<LteHelper>();
    Ptr<LteHelper> lteHelperMobile  = CreateObject<LteHelper>();
    Ptr<PointToPointEpcHelper> epcHelper    = CreateObject<PointToPointEpcHelper>();
    lteHelperStatic->SetEpcHelper( epcHelper );
    lteHelperMobile->SetEpcHelper( epcHelper );
    Ptr<Node> pgwNode   = epcHelper->GetPgwNode();

    // @TODO: Setup Fading and Pathloss models
    Config::SetDefault( "ns3::LteUePhy::TxPower", DoubleValue(40) );         // Transmission power in dBm
    Config::SetDefault( "ns3::LteUePhy::NoiseFigure", DoubleValue(10) );     // Default 5
    Config::SetDefault( "ns3::LteEnbPhy::TxPower", DoubleValue(24) );        // Transmission power in dBm
    Config::SetDefault( "ns3::LteEnbPhy::NoiseFigure", DoubleValue(20) );    // Default 5
    lteHelperStatic->SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel"));
    // lteHelperStatic->SetFadingModel("ns3::TraceFadingLossModel");
    lteHelperMobile->SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel"));
    lteHelperMobile->SetFadingModel("ns3::TraceFadingLossModel");
    lteHelperMobile->SetFadingModelAttribute ("TraceFilename", StringValue ("/Users/vish/developer/ns3/bake/source/ns-3-dev/scratch/Lte1CellTestbed/fading-traces/fading_trace_EVA_60kmph.fad"));
    lteHelperMobile->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
    lteHelperMobile->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
    lteHelperMobile->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
    lteHelperMobile->SetFadingModelAttribute ("RbNum", UintegerValue (100));

    InternetStackHelper iNetHlpr;   iNetHlpr.Install( remoteNode );

    // Setup connection between remoteNode and PGW
    PointToPointHelper p2pHlpr;
    p2pHlpr.SetDeviceAttribute( "DataRate", DataRateValue(DataRate("100Gbps")) );
    p2pHlpr.SetDeviceAttribute( "Mtu", UintegerValue(1500) );
    p2pHlpr.SetChannelAttribute( "Delay", TimeValue(Seconds(0.010)) );
    NetDeviceContainer netDevP2PNodes   = p2pHlpr.Install( pgwNode, remoteNode.Get(0) );

    Ipv4AddressHelper ipv4HlprExternNwrk;
    ipv4HlprExternNwrk.SetBase( "1.0.0.0", "255.0.0.0" );
    Ipv4InterfaceContainer ipv4InfContainer = ipv4HlprExternNwrk.Assign( netDevP2PNodes );

    // State the routing
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting( remoteNode.Get(0)->GetObject<Ipv4>() );
    remoteNodeStaticRouting->AddNetworkRouteTo( Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1 );

    NS_LOG_INFO( "Installing LTE network device in eNodeBs..." );
    NetDeviceContainer netDevEnb    = lteHelperStatic->InstallEnbDevice( enbNodes );
    NS_LOG_INFO( "Installing LTE network device in static UEs..." );
    NetDeviceContainer netDevUeStatic   = lteHelperStatic->InstallUeDevice( ueNodesStatic );
    NS_LOG_INFO( "Installing LTE network device in mobile UEs..." );
    NetDeviceContainer netDevUeMobile   = lteHelperMobile->InstallUeDevice( ueNodesMobile );
    NetDeviceContainer netDevUe     = NetDeviceContainer( netDevUeMobile, netDevUeStatic );
    // NS_LOG_INFO( "installing LTE network device in UEs..." );
    // NetDeviceContainer netDevUe = lteHelper->InstallUeDevice( ueNodes );
    NS_LOG_INFO( "Installing Internet Stack in UEs..." );
    iNetHlpr.Install( ueNodes );

    NS_LOG_INFO( "Assigning IP Addresses to UEs..." );
    Ipv4InterfaceContainer ueIpInf = epcHelper->AssignUeIpv4Address( netDevUe );
    for( uint32_t idx = 0; idx < ueNodesStatic.GetN(); idx++ ) {
        Ptr<Node> thisUe    = ueNodesStatic.Get( idx );
        Ptr<NetDevice> thisUeDev = netDevUeStatic.Get( idx );
        // Setup default gateway
        Ptr<Ipv4StaticRouting>ueNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting( thisUe->GetObject<Ipv4>() );
        ueNodeStaticRouting->SetDefaultRoute( epcHelper->GetUeDefaultGatewayAddress(), 1 );
    }

    NS_LOG_INFO( "Attaching UEs to eNodeBs..." );
    lteHelperMobile->Attach( netDevUeMobile, netDevEnb.Get(0) );
    lteHelperStatic->Attach( netDevUeStatic, netDevEnb.Get(0) );

    // ##################### SETUP APPLICATIONS ################################
    Ptr<Node> sourceNode = ueNodesMobile.Get(0);
    Ptr<Node> sinkNode   = remoteNode.Get(0);

    Ipv4Address srcNodeAddress = sourceNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
    Ipv4Address snkNodeAddress = sinkNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();

    NS_LOG_INFO( "Setting up Large file transfer between " << srcNodeAddress << " and " << snkNodeAddress );

    LogComponentEnable( "OnOffApplication", LOG_LEVEL_INFO );
    LogComponentEnable( "PacketSink", LOG_LEVEL_INFO );

    ApplicationContainer FTPSrcApps;
    ApplicationContainer FTPSnkApps;

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
    FTPSrcApps.Start( Seconds(4.00) );
    FTPSnkApps.Start( Seconds(3.80) );

    // ###################### SETUP SIMULATION #################################

    NS_LOG_INFO( "Enabling tracing..." );
    lteHelperMobile->EnableTraces();

    Simulator::Stop( simDuration );
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

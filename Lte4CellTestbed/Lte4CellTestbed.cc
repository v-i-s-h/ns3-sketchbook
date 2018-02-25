// export NS_LOG=Lte4CellTestBed=info
// ./waf --run "scratch/Lte4CellTestbed/Lte4CellTestbed" --cwd "scratch/Lte4CellTestbed/"

/*

    LTE TEST BED
    Configuration:
        No.of Cells     : 4
        Cell Radius     : 500m
        No.Of users     : ? static users + ? moving user per cell
        User Speed      : ??
        Fading Model    :
        Path Loss Model :
        eNodeB Configuration:
            Tx Power        : ?
        UE Configuration :
            AMC Selection Scheme    : ?
*/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"         // For LteHelper, EpcHelper
#include "ns3/point-to-point-module.h"  // For point-to-point Helper
#include "ns3/csma-module.h"        // For CsmaHelper
#include "ns3/internet-module.h"    // For InternetStackHelper
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"

#include <assert.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE( "Lte4CellTestBed" );

static void ReportPosition( Ptr<Node> thisNode ) {
    Vector pos = thisNode->GetObject<MobilityModel>()->GetPosition();
    Vector vel = thisNode->GetObject<MobilityModel>()->GetVelocity();
    std::cout << "[" << Simulator::Now().GetSeconds() << "]"
              << " Node#" << thisNode->GetId()
              << " POS: (" << pos.x << "," << pos.y << "," << pos.z << ")"
              << " VEL: (" << vel.x << "," << vel.y << "," << vel.z << ")"
              << std::endl;
}

int main( int argc, char *argv[] ) {

    // Positions of ENBs
    double cellRadius   = 500.00;
    double enb0X = -1.500*cellRadius, enb0Y = +0.000, enb0Z = 10.00;
    double enb1X = +0.000, enb1Y = +0.866*cellRadius, enb1Z = 10.00;
    double enb2X = +0.000, enb2Y = -0.866*cellRadius, enb2Z = 10.00;
    double enb3X = +1.500*cellRadius, enb3Y = +0.000, enb3Z = 10.00;

    // Number of users per cell
    unsigned enb0StaticUsers    = 2;    unsigned enb0MobileUsers    = 2;
    unsigned enb1StaticUsers    = 2;    unsigned enb1MobileUsers    = 2;
    unsigned enb2StaticUsers    = 2;    unsigned enb2MobileUsers    = 2;
    unsigned enb3StaticUsers    = 2;    unsigned enb3MobileUsers    = 2;

    unsigned noOfRemoteNodes    = 1;

    Time simDuration = Seconds(3.00);

    //--------------------------- Setup nodes ----------------------------------
    NS_LOG_INFO( "No.of Nodes " << NodeList::GetNNodes() );
    uint32_t nodesCounter = NodeList::GetNNodes();
    NodeContainer enbNodes;  enbNodes.Create(4);
    NS_LOG_INFO( "eNodeB Nodes in range " << nodesCounter << "-" << NodeList::GetNNodes()-1 );    nodesCounter = NodeList::GetNNodes();
    NodeContainer ueNodeStaticCell0;    ueNodeStaticCell0.Create( enb0StaticUsers );
    if( enb0StaticUsers > 0 ) {
    NS_LOG_INFO( "Static Nodes of Cell0 in range " << nodesCounter << "-" << NodeList::GetNNodes()-1 );    nodesCounter = NodeList::GetNNodes(); }
    NodeContainer ueNodeMobileCell0;    ueNodeMobileCell0.Create( enb0MobileUsers );
    if( enb0MobileUsers > 0 ) {
    NS_LOG_INFO( "Mobile Nodes of Cell0 in range " << nodesCounter << "-" << NodeList::GetNNodes()-1 );    nodesCounter = NodeList::GetNNodes(); }
    NodeContainer ueNodeStaticCell1;    ueNodeStaticCell1.Create( enb1StaticUsers );
    if( enb1StaticUsers > 0 ) {
    NS_LOG_INFO( "Static Nodes of Cell1 in range " << nodesCounter << "-" << NodeList::GetNNodes()-1 );    nodesCounter = NodeList::GetNNodes(); }
    NodeContainer ueNodeMobileCell1;    ueNodeMobileCell1.Create( enb1MobileUsers );
    if( enb1MobileUsers > 0 ) {
    NS_LOG_INFO( "Mobile Nodes of Cell1 in range " << nodesCounter << "-" << NodeList::GetNNodes()-1 );    nodesCounter = NodeList::GetNNodes(); }
    NodeContainer ueNodeStaticCell2;    ueNodeStaticCell2.Create( enb2StaticUsers );
    if( enb2StaticUsers > 0 ) {
    NS_LOG_INFO( "Static Nodes of Cell2 in range " << nodesCounter << "-" << NodeList::GetNNodes()-1 );    nodesCounter = NodeList::GetNNodes(); }
    NodeContainer ueNodeMobileCell2;    ueNodeMobileCell2.Create( enb2MobileUsers );
    if( enb2MobileUsers > 0 ) {
    NS_LOG_INFO( "Mobile Nodes of Cell2 in range " << nodesCounter << "-" << NodeList::GetNNodes()-1 );    nodesCounter = NodeList::GetNNodes(); }
    NodeContainer ueNodeStaticCell3;    ueNodeStaticCell3.Create( enb3StaticUsers );
    if( enb3StaticUsers > 0 ) {
    NS_LOG_INFO( "Static Nodes of Cell3 in range " << nodesCounter << "-" << NodeList::GetNNodes()-1 );    nodesCounter = NodeList::GetNNodes(); }
    NodeContainer ueNodeMobileCell3;    ueNodeMobileCell3.Create( enb3MobileUsers );
    if( enb3MobileUsers > 0 ) {
    NS_LOG_INFO( "Mobile Nodes of Cell3 in range " << nodesCounter << "-" << NodeList::GetNNodes()-1 );    nodesCounter = NodeList::GetNNodes(); }

    assert( noOfRemoteNodes <= 1 );
    NodeContainer remoteNodes;  remoteNodes.Create( noOfRemoteNodes );
    NS_LOG_INFO( "Remote Nodes in range " << nodesCounter << "-" << NodeList::GetNNodes()-1 );  nodesCounter = NodeList::GetNNodes();

    NodeContainer ueNodesStatic;
    ueNodesStatic.Add( ueNodeStaticCell0 ); ueNodesStatic.Add( ueNodeStaticCell1 );
    ueNodesStatic.Add( ueNodeStaticCell2 ); ueNodesStatic.Add( ueNodeStaticCell3 );
    NodeContainer ueNodesMobile;
    ueNodesMobile.Add( ueNodeMobileCell0 ); ueNodesMobile.Add( ueNodeMobileCell1 );
    ueNodesMobile.Add( ueNodeMobileCell2 ); ueNodesMobile.Add( ueNodeMobileCell3 );
    NodeContainer ueNodes;
    ueNodes.Add( ueNodesStatic );   ueNodes.Add( ueNodesMobile );
    // ueNodes.Add( ueNodeStaticCell0 );   ueNodes.Add( ueNodeMobileCell0 );
    // ueNodes.Add( ueNodeStaticCell1 );   ueNodes.Add( ueNodeMobileCell1 );
    // ueNodes.Add( ueNodeStaticCell2 );   ueNodes.Add( ueNodeMobileCell2 );
    // ueNodes.Add( ueNodeStaticCell3 );   ueNodes.Add( ueNodeMobileCell3 );

    //------------------------- End Of Nodes Setup -----------------------------

    //--------------------------- Setup Mobility -------------------------------
    // Mobility for eNodeBs
    NS_LOG_INFO( "Setting up mobility for eNodeBs..." );
    MobilityHelper mobilityEnb;
    Ptr<ListPositionAllocator> posAllocEnb  = CreateObject<ListPositionAllocator>();
    posAllocEnb->Add( Vector(enb0X,enb0Y,enb0Z) );
    posAllocEnb->Add( Vector(enb1X,enb1Y,enb1Z) );
    posAllocEnb->Add( Vector(enb2X,enb2Y,enb2Z) );
    posAllocEnb->Add( Vector(enb3X,enb3Y,enb3Z) );
    mobilityEnb.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );
    mobilityEnb.SetPositionAllocator( posAllocEnb );
    mobilityEnb.Install( enbNodes );
    NS_LOG_INFO( "ENodeBs positions: " 
                    << "(" << enb0X << "," << enb0Y << "), "
                    << "(" << enb1X << "," << enb1Y << "), " 
                    << "(" << enb2X << "," << enb2Y << "), "
                    << "(" << enb3X << "," << enb3Y << ")" );

    // Mobility for UEs
    NS_LOG_INFO( "Setting up mobility for UEs..." );
    // Deploy static Users randomly around each eNodeBs
    // Setup a random variable for distance of each UE
    Ptr<UniformRandomVariable> ueRadiusSampler  = CreateObject<UniformRandomVariable>();    // Randomly sample radius for static users
    ueRadiusSampler->SetAttribute( "Max", DoubleValue(0.866*cellRadius) );   // 0.866 to confine the users within the hexagon
    ueRadiusSampler->SetAttribute( "Min", DoubleValue(0.00) );

    // Setup position allocator for UE associated with each cell x 4 cells
    Ptr<UniformDiscPositionAllocator> _posAllocStaticUeCell0  = CreateObject<UniformDiscPositionAllocator>();
    _posAllocStaticUeCell0->SetX(enb0X); _posAllocStaticUeCell0->SetY(enb0Y); _posAllocStaticUeCell0->SetRho(0.866*cellRadius);

    Ptr<UniformDiscPositionAllocator> _posAllocStaticUeCell1  = CreateObject<UniformDiscPositionAllocator>();
    _posAllocStaticUeCell1->SetX(enb1X); _posAllocStaticUeCell1->SetY(enb1Y); _posAllocStaticUeCell1->SetRho(0.866*cellRadius);

    Ptr<UniformDiscPositionAllocator> _posAllocStaticUeCell2  = CreateObject<UniformDiscPositionAllocator>();
    _posAllocStaticUeCell2->SetX(enb2X); _posAllocStaticUeCell2->SetY(enb2Y); _posAllocStaticUeCell2->SetRho(0.866*cellRadius);

    Ptr<UniformDiscPositionAllocator> _posAllocStaticUeCell3  = CreateObject<UniformDiscPositionAllocator>();
    _posAllocStaticUeCell3->SetX(enb3X); _posAllocStaticUeCell3->SetY(enb3Y); _posAllocStaticUeCell3->SetRho(0.866*cellRadius);

    // NS_LOG_INFO( "\nCell0: << " << _posAllocStaticUeCell0->GetNext() << ", " << _posAllocStaticUeCell0->GetNext() << ", " << _posAllocStaticUeCell0->GetNext()
    //             << "\nCell1: << " << _posAllocStaticUeCell1->GetNext() << ", " << _posAllocStaticUeCell1->GetNext() << ", " << _posAllocStaticUeCell1->GetNext()
    //             << "\nCell2: << " << _posAllocStaticUeCell2->GetNext() << ", " << _posAllocStaticUeCell2->GetNext() << ", " << _posAllocStaticUeCell2->GetNext()
    //             << "\nCell3: << " << _posAllocStaticUeCell3->GetNext() << ", " << _posAllocStaticUeCell3->GetNext() << ", " << _posAllocStaticUeCell3->GetNext() );

    // Setup locations with non-zero Z values
    Ptr<ListPositionAllocator> posAllocStaticUeCell0    = CreateObject<ListPositionAllocator>();
    for( unsigned _ueIdx = 0; _ueIdx < enb0StaticUsers; _ueIdx++ ) {
        Vector thisUePos  = _posAllocStaticUeCell0->GetNext();    thisUePos.z = 1.50;   posAllocStaticUeCell0->Add( thisUePos );
    }
    Ptr<ListPositionAllocator> posAllocStaticUeCell1    = CreateObject<ListPositionAllocator>();
    for( unsigned _ueIdx = 0; _ueIdx < enb1StaticUsers; _ueIdx++ ) {
        Vector thisUePos  = _posAllocStaticUeCell1->GetNext();    thisUePos.z = 1.50;   posAllocStaticUeCell1->Add( thisUePos );
    }
    Ptr<ListPositionAllocator> posAllocStaticUeCell2    = CreateObject<ListPositionAllocator>();
    for( unsigned _ueIdx = 0; _ueIdx < enb2StaticUsers; _ueIdx++ ) {
        Vector thisUePos  = _posAllocStaticUeCell2->GetNext();    thisUePos.z = 1.50;   posAllocStaticUeCell2->Add( thisUePos );
    }
    Ptr<ListPositionAllocator> posAllocStaticUeCell3    = CreateObject<ListPositionAllocator>();
    for( unsigned _ueIdx = 0; _ueIdx < enb3StaticUsers; _ueIdx++ ) {
        Vector thisUePos  = _posAllocStaticUeCell3->GetNext();    thisUePos.z = 1.50;   posAllocStaticUeCell3->Add( thisUePos );
    }

    // for all mobile UEs, start point is from the exact cell center and goes to cell edge in radom direction;
    // Setup a random sample for waypoint navigation
    Ptr<UniformRandomVariable> wpRadiusSampler  = CreateObject<UniformRandomVariable>();
    wpRadiusSampler->SetAttribute( "Max", DoubleValue(0.866*cellRadius) );   // maximum radius within cell boundary
    wpRadiusSampler->SetAttribute( "Min", DoubleValue(0.800*cellRadius) );   // minimum radius not much lees than cell boundary

    // Setup position allocators for Mobile UEs
    Ptr<ListPositionAllocator> posAllocMobileUeCell0    = CreateObject<ListPositionAllocator>();
    for( unsigned ueIdx = 0; ueIdx < enb0MobileUsers; ueIdx++ ) { posAllocMobileUeCell0->Add( Vector(enb0X+50,enb0Y+50,1.50) ); }
    Ptr<RandomDiscPositionAllocator> wpAllocMobileUeCell0   = CreateObject<RandomDiscPositionAllocator>();
    wpAllocMobileUeCell0->SetX(enb0X);  wpAllocMobileUeCell0->SetY(enb0Y);  wpAllocMobileUeCell0->SetRho(wpRadiusSampler);

    Ptr<ListPositionAllocator> posAllocMobileUeCell1    = CreateObject<ListPositionAllocator>();
    for( unsigned ueIdx = 0; ueIdx < enb1MobileUsers; ueIdx++ ) { posAllocMobileUeCell1->Add( Vector(enb1X+50,enb1Y+50,1.50) ); }
    Ptr<RandomDiscPositionAllocator> wpAllocMobileUeCell1   = CreateObject<RandomDiscPositionAllocator>();
    wpAllocMobileUeCell1->SetX(enb1X);  wpAllocMobileUeCell1->SetY(enb1Y);  wpAllocMobileUeCell1->SetRho(wpRadiusSampler);

    Ptr<ListPositionAllocator> posAllocMobileUeCell2    = CreateObject<ListPositionAllocator>();
    for( unsigned ueIdx = 0; ueIdx < enb2MobileUsers; ueIdx++ ) { posAllocMobileUeCell2->Add( Vector(enb2X+50,enb2Y+50,1.50) ); }
    Ptr<RandomDiscPositionAllocator> wpAllocMobileUeCell2   = CreateObject<RandomDiscPositionAllocator>();
    wpAllocMobileUeCell2->SetX(enb2X);  wpAllocMobileUeCell2->SetY(enb2Y);  wpAllocMobileUeCell2->SetRho(wpRadiusSampler);

    Ptr<ListPositionAllocator> posAllocMobileUeCell3    = CreateObject<ListPositionAllocator>();
    for( unsigned ueIdx = 0; ueIdx < enb3MobileUsers; ueIdx++ ) { posAllocMobileUeCell3->Add( Vector(enb3X+50,enb3Y+50,1.50) ); }
    Ptr<RandomDiscPositionAllocator> wpAllocMobileUeCell3   = CreateObject<RandomDiscPositionAllocator>();
    wpAllocMobileUeCell3->SetX(enb3X);  wpAllocMobileUeCell3->SetY(enb3Y);  wpAllocMobileUeCell3->SetRho(wpRadiusSampler);

    // Setup Mobility Models for Static Users in each cell
    MobilityHelper mobilityStaticUeCell0;
    mobilityStaticUeCell0.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );
    mobilityStaticUeCell0.SetPositionAllocator( posAllocStaticUeCell0 );
    MobilityHelper mobilityStaticUeCell1;
    mobilityStaticUeCell1.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );
    mobilityStaticUeCell1.SetPositionAllocator( posAllocStaticUeCell1 );
    MobilityHelper mobilityStaticUeCell2;
    mobilityStaticUeCell2.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );
    mobilityStaticUeCell2.SetPositionAllocator( posAllocStaticUeCell2 );
    MobilityHelper mobilityStaticUeCell3;
    mobilityStaticUeCell3.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );
    mobilityStaticUeCell3.SetPositionAllocator( posAllocStaticUeCell3 );
    // Install Mobility models to UEs
    mobilityStaticUeCell0.Install( ueNodeStaticCell0 );
    mobilityStaticUeCell1.Install( ueNodeStaticCell1 );
    mobilityStaticUeCell2.Install( ueNodeStaticCell2 );
    mobilityStaticUeCell3.Install( ueNodeStaticCell3 );

    // Setup mobility for Mobile users in eNodes
    MobilityHelper mobilityMobileUeCell0;
    mobilityMobileUeCell0.SetMobilityModel( "ns3::RandomWaypointMobilityModel",
        "Speed", StringValue("ns3::ConstantRandomVariable[Constant=33.36]"),
        "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.50]"),
        "PositionAllocator", PointerValue(wpAllocMobileUeCell0) );
    mobilityMobileUeCell0.SetPositionAllocator( posAllocMobileUeCell0 );
    mobilityMobileUeCell0.Install( ueNodeMobileCell0 );

    MobilityHelper mobilityMobileUeCell1;
    mobilityMobileUeCell1.SetMobilityModel( "ns3::RandomWaypointMobilityModel",
        "Speed", StringValue("ns3::ConstantRandomVariable[Constant=33.36]"),
        "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.50]"),
        "PositionAllocator", PointerValue(wpAllocMobileUeCell1) );
    mobilityMobileUeCell1.SetPositionAllocator( posAllocMobileUeCell1 );
    mobilityMobileUeCell1.Install( ueNodeMobileCell1 );

    MobilityHelper mobilityMobileUeCell2;
    mobilityMobileUeCell2.SetMobilityModel( "ns3::RandomWaypointMobilityModel",
        "Speed", StringValue("ns3::ConstantRandomVariable[Constant=33.36]"),
        "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.50]"),
        "PositionAllocator", PointerValue(wpAllocMobileUeCell2) );
    mobilityMobileUeCell2.SetPositionAllocator( posAllocMobileUeCell2 );
    mobilityMobileUeCell2.Install( ueNodeMobileCell2 );

    MobilityHelper mobilityMobileUeCell3;
    mobilityMobileUeCell3.SetMobilityModel( "ns3::RandomWaypointMobilityModel",
        "Speed", StringValue("ns3::ConstantRandomVariable[Constant=33.36]"),
        "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.50]"),
        "PositionAllocator", PointerValue(wpAllocMobileUeCell3) );
    mobilityMobileUeCell3.SetPositionAllocator( posAllocMobileUeCell3 );
    mobilityMobileUeCell3.Install( ueNodeMobileCell3 );

    NS_LOG_INFO( "Setting up mobility tracking for cell0 users..." );
    for( uint32_t ueIdx = 0; ueIdx < ueNodeMobileCell0.GetN(); ueIdx++ ) {
        for( uint32_t stepIdx = 0; stepIdx < simDuration.GetSeconds(); stepIdx++ ) {
            NS_LOG_INFO( "    UeId: " << ueNodeMobileCell0.Get(ueIdx)->GetId() << "    @" << stepIdx << " sec");
            Simulator::Schedule( Time(Seconds(stepIdx)), ReportPosition, ueNodeMobileCell0.Get(ueIdx) );
        }
    }

    NS_LOG_INFO( "End of Mobility Setup      Number of Nodes: " << NodeList::GetNNodes() );
    //------------------------ End Of mobility Setup ---------------------------

    //--------------------------- Setup LTE Network ----------------------------
    // LogComponentEnable( "LteHelper", LOG_LEVEL_INFO );
    // LogComponentEnable( "PhyStatsCalculator", LOG_LEVEL_FUNCTION );
    // LogComponentEnable( "LteAmc", LOG_LEVEL_INFO );
    // LogComponentEnable( "LteUePhy", LOG_LEVEL_DEBUG );
    NS_LOG_INFO( "Setting up LTE network" );
    Ptr<LteHelper> lteHelper                = CreateObject<LteHelper>();
    Ptr<PointToPointEpcHelper> epcHelper    = CreateObject<PointToPointEpcHelper>();    // @TODO: one unknown node gets created here, PGW?
    lteHelper->SetEpcHelper( epcHelper );

    Config::SetDefault( "ns3::LteUePhy::TxPower", DoubleValue(24) );         // Transmission power in dBm
    Config::SetDefault( "ns3::LteUePhy::NoiseFigure", DoubleValue(6) );     // Default 5
    Config::SetDefault( "ns3::LteEnbPhy::TxPower", DoubleValue(40) );        // Transmission power in dBm
    Config::SetDefault( "ns3::LteEnbPhy::NoiseFigure", DoubleValue(6) );    // Default 5
    // lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel"));

    lteHelper->SetAttribute("PathlossModel",StringValue("ns3::OkumuraHataPropagationLossModel"));
    lteHelper->SetPathlossModelAttribute("Environment", StringValue("Urban"));
    // Config::SetDefault ("ns3::RadioBearerStatsCalculator::EpochDuration", TimeValue (Seconds(1.00)));


    lteHelper->SetFadingModel("ns3::TraceFadingLossModel");
    lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("./../fading-traces/fading_trace_EVA_60kmph.fad"));
    lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
    lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
    lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
    lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (100));

    Ptr<Node> pgwNode   = epcHelper->GetPgwNode();
    NS_LOG_INFO( "LTE EPC PGW nodes at " << nodesCounter << "-" << NodeList::GetNNodes()-1 );

    // Install INternet Stack on all nodes
    InternetStackHelper isHlpr;
    isHlpr.Install( remoteNodes );

    // // Create a bus topology connecting remote nodes and PGW node
    // CsmaHelper csmaBus;
    // csmaBus.SetChannelAttribute( "DataRate", StringValue("8Gbps") );    // Gigabit bus
    // csmaBus.SetChannelAttribute( "Delay", TimeValue(NanoSeconds(6560)) );    // aa...
    //
    // NodeContainer csmaNodes = NodeContainer( pgwNode, remoteNodes );
    // NetDeviceContainer netDevCsmaNodes = csmaBus.Install( csmaNodes );

    // Create a P2P topology for remote nodes and PGW
    PointToPointHelper p2pBus;
    p2pBus.SetDeviceAttribute( "DataRate", DataRateValue(DataRate("100Gbps")) );
    p2pBus.SetDeviceAttribute( "Mtu", UintegerValue(1500) );
    p2pBus.SetChannelAttribute( "Delay", TimeValue(NanoSeconds(6560)) );
    NetDeviceContainer netDevP2PNodes   = p2pBus.Install( pgwNode, remoteNodes.Get(0) );

    Ipv4AddressHelper ipv4HlprExternNwrk;     // allocator of IP addresses for remote nodes and PGW?
    ipv4HlprExternNwrk.SetBase( "1.0.0.0", "255.0.0.0" );
    Ipv4InterfaceContainer ipv4InfCtnr = ipv4HlprExternNwrk.Assign( netDevP2PNodes ); // Assign IP address in range 1.X.Y.Z

    NS_LOG_INFO( "Setting up Internet Stack in UE nodes..." );
    isHlpr.Install( ueNodes );

    NS_LOG_INFO( "Installing network devices in eNodeBs..." );
    NetDeviceContainer netDevENB    = lteHelper->InstallEnbDevice( enbNodes );

    NS_LOG_INFO( "Installing network devices in UEs..." );
    NetDeviceContainer netDevCell0Ues   = lteHelper->InstallUeDevice( NodeContainer(ueNodeStaticCell0,ueNodeMobileCell0) );
    NetDeviceContainer netDevCell1Ues   = lteHelper->InstallUeDevice( NodeContainer(ueNodeStaticCell1,ueNodeMobileCell1) );
    NetDeviceContainer netDevCell2Ues   = lteHelper->InstallUeDevice( NodeContainer(ueNodeStaticCell2,ueNodeMobileCell2) );
    NetDeviceContainer netDevCell3Ues   = lteHelper->InstallUeDevice( NodeContainer(ueNodeStaticCell3,ueNodeMobileCell3) );

    // Assigning IP addresses to UE nodes
    NS_LOG_INFO( "Assigning IP Address to UE nodes..." );
    Ipv4InterfaceContainer ipInfUe  = epcHelper->AssignUeIpv4Address( netDevCell0Ues );
    ipInfUe.Add( epcHelper->AssignUeIpv4Address(netDevCell1Ues) );
    ipInfUe.Add( epcHelper->AssignUeIpv4Address(netDevCell2Ues) );
    ipInfUe.Add( epcHelper->AssignUeIpv4Address(netDevCell3Ues) );

    // Fix Routing
    // @TODO: Fix this to handle mulitple remote nodes
    // State the routing
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting( remoteNodes.Get(0)->GetObject<Ipv4>() );
    remoteNodeStaticRouting->AddNetworkRouteTo( Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1 );
    // Ipv4StaticRoutingHelper ipv4RoutingHelper;
    // for( unsigned nodeIdx = 0; nodeIdx < noOfRemoteNodes; nodeIdx++ ) {
    //     Ptr<Ipv4StaticRouting> staticRoutingRemoteNodes = ipv4RoutingHelper.GetStaticRouting( remoteNodes.Get(nodeIdx)->GetObject<Ipv4>() );
    //     staticRoutingRemoteNodes->AddNetworkRouteTo( Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1 );
    // }

    // PGW Routing
    Ptr<Ipv4StaticRouting> staticRoutingPgw = ipv4RoutingHelper.GetStaticRouting( pgwNode->GetObject<Ipv4>() );
    staticRoutingPgw->AddNetworkRouteTo( Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1 );
    staticRoutingPgw->AddNetworkRouteTo( Ipv4Address("1.0.0.0"), Ipv4Mask("255.0.0.0"), 2 );

    NS_LOG_INFO( "Setting up Routing for UE nodes..." );
    for( uint32_t idx = 0; idx < ueNodes.GetN(); idx++ ) {
        Ptr<Node> thisUe    = ueNodes.Get( idx );
        Ptr<Ipv4StaticRouting>staticRoutingUes   = ipv4RoutingHelper.GetStaticRouting( thisUe->GetObject<Ipv4>() );
        staticRoutingUes->SetDefaultRoute( epcHelper->GetUeDefaultGatewayAddress(), 1 );
    }

    NS_LOG_INFO( "Attaching UEs to eNodeBs..." );
    lteHelper->Attach( netDevCell0Ues, netDevENB.Get(0) );
    lteHelper->Attach( netDevCell1Ues, netDevENB.Get(1) );
    lteHelper->Attach( netDevCell2Ues, netDevENB.Get(2) );
    lteHelper->Attach( netDevCell3Ues, netDevENB.Get(3) );

    // Activate a data radio bearer each UE
    // enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    // EpsBearer bearer (q);
    // lteHelper->ActivateDataRadioBearer ( netDevCell0Ues, bearer);
    // lteHelper->ActivateDataRadioBearer ( netDevCell1Ues, bearer);
    // lteHelper->ActivateDataRadioBearer ( netDevCell2Ues, bearer);
    // lteHelper->ActivateDataRadioBearer ( netDevCell3Ues, bearer);

    // Print IP addresses
    std::cout << "IPv4 Addresses:" << std::endl
              << "    Remote Node 0: " << remoteNodes.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal() << std::endl
              << "    PGW Node  [0]: " << pgwNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal() << std::endl
              << "    PGW Node  [1]: " << pgwNode->GetObject<Ipv4>()->GetAddress(2,0).GetLocal() << std::endl;
    //--------------------------- End Of LTE Setup -----------------------------

    // ########################## SETUP UDP ECHO APP ###########################
    // LogComponentEnable( "UdpEchoClientApplication", LOG_LEVEL_INFO );
    // LogComponentEnable( "UdpEchoServerApplication", LOG_LEVEL_INFO );
    //
    // Ptr<Node> svrNode       = ueNodeStaticCell3.Get(0);//ueNodes.Get(0);
    // Ptr<Node> clientNode    = ueNodeMobileCell2.Get(0);//pgwNode;
    //
    // NS_LOG_INFO( "Setting up UDP Echo Server..." );
    // UdpEchoServerHelper echoServer( 9 );
    // ApplicationContainer svrApp = echoServer.Install( svrNode );
    // svrApp.Start( Seconds(0.50) );   svrApp.Stop( Seconds(5.00) );
    //
    // NS_LOG_INFO( "Setting up UDP Echo Client..." );
    // UdpEchoClientHelper echoClient( svrNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 9 );
    // echoClient.SetAttribute( "MaxPackets", UintegerValue(2) );
    // echoClient.SetAttribute( "Interval", TimeValue(Seconds(1.0)) );
    // echoClient.SetAttribute( "PacketSize", UintegerValue(1024) );
    // ApplicationContainer clientApp = echoClient.Install( clientNode );
    // clientApp.Start(Seconds(0.50));    clientApp.Stop(Seconds(5.0));
    // ########################### END OF APP SETUP ############################

    // LogComponentEnable( "OnOffApplication", LOG_LEVEL_INFO );
    // LogComponentEnable( "PacketSink", LOG_LEVEL_INFO );

    // // ####################### SETUP AN FTP APPLICATION ########################
    NS_LOG_INFO( "Setting up large file transfer..." );
    Ptr<Node> sourceNode = remoteNodes.Get(0);
    Ptr<Node> sinkNode   = ueNodeMobileCell0.Get(0);

    ApplicationContainer FTPSrcApps;
    ApplicationContainer FTPSnkApps;

    // Ipv4Address srcNodeAddress = sourceNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
    Ipv4Address snkNodeAddress = sinkNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();

    // Create a general FTP Application
    OnOffHelper FTPApplication( "ns3::UdpSocketFactory", InetSocketAddress(snkNodeAddress,21) );
    // FTPApplication.SetAttribute( "MaxBytes", UintegerValue(2*1024*1024) );    // 20MB File
    FTPApplication.SetAttribute( "PacketSize", UintegerValue(1024) );  // 1KB packet size
    FTPApplication.SetAttribute( "DataRate", DataRateValue(5*1024*1024) ); // I'm I asking for too much?
    FTPApplication.SetAttribute( "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]") );  // chatter always :P
    FTPApplication.SetAttribute( "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]") ); // why shutup??
    // Install FTP Application at Source Node
    FTPSrcApps.Add( FTPApplication.Install(sourceNode) );

    // Create a generic packet sink application
    PacketSinkHelper FTPSinkApp( "ns3::UdpSocketFactory", InetSocketAddress(snkNodeAddress,21) );
    // Install packet sink at Sink Node
    FTPSnkApps.Add( FTPSinkApp.Install(sinkNode) );

    // Schedule Start of File Transfer
    FTPSrcApps.Start( Seconds(1.00) );   // Do you really want to stop?
    FTPSnkApps.Start( Seconds(0.80) );
    // No Stopping the dats transfer explicitly, they should stop by themselves once the tht file is send(?)
    FTPSrcApps.Stop( Seconds(28.00) );
    FTPSnkApps.Stop( Seconds(28.20) );
    // #########################################################################

    // Enable traces
    lteHelper->EnableTraces();

    // GtkConfigStore config;
    // config.ConfigureDefaults ();
    // config.ConfigureAttributes ();

    Simulator::Stop( simDuration );
    NS_LOG_INFO( "Starting Simulation......" );
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

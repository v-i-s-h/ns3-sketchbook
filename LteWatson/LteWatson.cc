// export NS_LOG=UrbanScenario=info

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Antonino Masaracchia<antonino.masaracchia@gmail.com>
 */

#include <iostream>
#include <sys/resource.h>
#include <sys/time.h>
#include <math.h>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/flow-monitor-helper.h"
#include "ns3/lte-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/epc-helper.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store.h"
#include "ns3/animation-interface.h"
#include "ns3/config-store-module.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-environment.h"
#include "ns3/okumura-hata-propagation-loss-model.h"
#include "src/core/model/log.h"
#include "src/core/model/config.h"

#define SAT 20000000
#define NODE 10 //Node of nodelist
#define APP 28 //App of node
using namespace ns3;

static uint32_t nodo=0;
static std::ofstream outfile_pos;

//function for tracing
void CourseChange (Ptr<Node> node){
    Vector position = node->GetObject<MobilityModel>()->GetPosition();
    nodo = node->GetId();
    outfile_pos<<(nodo - 2)<<'\t'<<Simulator::Now()<<' '<<position.x<<' '<<position.y<<' '<<position.z<<std::endl;
}

void ReportProgress( void ) {
    std::cout << "\r Simulation: " << Simulator::Now();
}

NS_LOG_COMPONENT_DEFINE ("UrbanScenario");


int main(int argc, char** argv) {

    #if 1
  LogComponentEnable ("UrbanScenario", LOG_LEVEL_INFO);
  //LogComponentEnable ("RrFfMacScheduler", LOG_LEVEL_INFO);
  //LogComponentEnable ("FlowMonitor", LOG_LEVEL_ALL);
  #endif

  outfile_pos.open("PositionTrace.txt");

  //Default Configuration
    uint32_t nUes=10; //number of interferent Ue's;
    uint32_t nEnbs = 1; //Single Cell
    uint32_t fading = 1; //Presence of Fading

    double simTime=20; //Simulation Time
    double epochDuration=1;
    double ray=1500;

    int64_t stream = -1;

    std::stringstream rate;//saturation Condition
    std::string traceFadingPath="";
    std::string environment ="";
    std::string citySize="";

    //External Parameters
    CommandLine cmd;
    cmd.AddValue("nUes","Numero delle stazioni mobili[Default=10] ",nUes);
    cmd.AddValue("nEnbs", "Numero delle stazioni base[Default=1]",nEnbs);
    cmd.AddValue("simTime","Durata della simulazione[Default=200]",simTime);
    cmd.AddValue("epochDuration","Campinamento Traccia[Default=1]",epochDuration);
    cmd.AddValue("ray","Raggio della cella in metri[Default=1500]",ray);
    cmd.AddValue("fading","Indicare se si vuole fading[Default=1]",fading);
    cmd.AddValue("tracePath","Path delle tracce di fading",traceFadingPath);
    cmd.AddValue("environment","Ambiente di propagazione [Default=OpenAreas]",environment);
    cmd.AddValue("citySize","Larghezza della città [Default=Large]",citySize);
    cmd.AddValue("stream","Indice di Stream di numeri casuali",stream);
    cmd.Parse(argc, argv);

    uint32_t totalNodes = nUes;
    rate<<(SAT/totalNodes)<<"b/s";
    //LTE Devices parameters
    NS_LOG_INFO("Lte-Epc helper creation");
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
    Ptr<PointToPointEpcHelper> epcHelper    = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper(epcHelper);

    NS_LOG_INFO("Pgw creation");
    Ptr<Node> pgw = epcHelper->GetPgwNode ();

    Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (25)); // 5 MHz channel
    Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue (25)); // 5 MHz channel
    Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue(10));
    //Config::SetDefault ("ns3::LteEnbPhy::NoiseFigure", DoubleValue(20)); //Deafault 5
    Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue(43));
    //Config::SetDefault ("ns3::LteUePhy::NoiseFigure", DoubleValue(20)); //Deafult 9
    //Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity",UintegerValue(160));
    lteHelper->SetEnbDeviceAttribute("DlEarfcn", UintegerValue(100));
    lteHelper->SetEnbDeviceAttribute("UlEarfcn", UintegerValue(18100));
    lteHelper->SetAttribute("Scheduler",StringValue("ns3::RrFfMacScheduler"));
    //Config::SetDefault("ns3::RrFfMacScheduler::CqiTimerThreshold",UintegerValue(1));
    Config::SetDefault ("ns3::LteEnbRrc::EpsBearerToRlcMapping", EnumValue (LteEnbRrc::RLC_UM_ALWAYS));
    Config::SetDefault("ns3::LteSpectrumPhy::CtrlErrorModelEnabled",BooleanValue(false));
    Config::SetDefault("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(4294967295));

    //Propagation Model Configuration
    NS_LOG_INFO("Propagation model settings");

    lteHelper->SetAttribute("PathlossModel",StringValue("ns3::OkumuraHataPropagationLossModel"));

    if(!environment.empty()){
        lteHelper->SetPathlossModelAttribute("Environment", StringValue(environment));
        NS_LOG_INFO("Environment "<<environment);
    }

    else{
        lteHelper->SetPathlossModelAttribute("Environment", StringValue("Urban"));
        NS_LOG_INFO("Default environment");
    }

    if(!citySize.empty())
        lteHelper->SetPathlossModelAttribute("CitySize", StringValue(citySize));


    if (epochDuration > 0)
    {
        Config::SetDefault ("ns3::RadioBearerStatsCalculator::EpochDuration", TimeValue (Seconds (epochDuration)));
    }

    //Fading trace configuration
    NS_LOG_INFO("Fading model settings");
    if (fading)
    {
        lteHelper->SetFadingModel("ns3::TraceFadingLossModel");
        if(!traceFadingPath.empty())
        {
            std::stringstream track_fad;
            track_fad<<"src/lte/model/fading-traces/"<<traceFadingPath;
            lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue (track_fad.str()));
            NS_LOG_INFO("FadingTrace: "<<track_fad.str());

        }
        else
        {
            lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("/Users/vish/developer/ns3/bake/source/ns-3-dev/scratch/LteWatson/fading-traces/fading_trace_EVA_60kmph.fad"));
            NS_LOG_INFO("Default fading trace EVA 60kmph");
        }

    lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
    lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
    lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
    lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (100));
    }


    // Create a single RemoteHost
    NS_LOG_INFO("Remote Host creation");
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create (1);
    Ptr<Node> remoteHost = remoteHostContainer.Get (0);
    InternetStackHelper internet;
    internet.Install (remoteHostContainer);

    // Create the Internet
    NS_LOG_INFO("P2P instauration");
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
    p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
    p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.0)));
    p2ph.SetQueue("ns3::DropTailQueue","MaxPackets", StringValue ("4294967295"));
    NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
    // interface 0 is localhost, 1 is the p2p device
  //  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

    //Instradamento verso la rete di default LTE
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
    remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

    NS_LOG_INFO("Lte Node Creation and positionation");

    NodeContainer enbNodes;
    enbNodes.Create(1);
    NodeContainer ueNodes;
    ueNodes.Create(totalNodes);

    double x=1500.0;
    double y=1500.0;

    MobilityHelper mobilityEnb;
    Ptr<ListPositionAllocator> positionAllocEnb = CreateObject<ListPositionAllocator> ();
    positionAllocEnb->Add(Vector(x,y,30));

    mobilityEnb.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityEnb.SetPositionAllocator(positionAllocEnb);
    mobilityEnb.Install(enbNodes);

    // Ptr<RandomDiscPositionAllocator> allocUe = CreateObject<RandomDiscPositionAllocator>();
    // allocUe->SetX(x);
    // allocUe->SetY(y);
    // allocUe->SetRho(rho);
    // allocUe->AssignStreams(stream);
    Ptr<UniformRandomVariable> rho = CreateObject<UniformRandomVariable>();
    rho->SetAttribute("Max",DoubleValue(ray));
    rho->SetAttribute("Min",DoubleValue(0));
    Ptr<ListPositionAllocator> allocUe    = CreateObject<ListPositionAllocator>();
    for( unsigned ueIdx = 0; ueIdx < nUes; ueIdx++ ) { allocUe->Add( Vector(x,y,1.50) ); }
    Ptr<RandomDiscPositionAllocator> allocWaypoint = CreateObject<RandomDiscPositionAllocator>();
    allocWaypoint->SetX(x);
    allocWaypoint->SetY(y);
    allocWaypoint->SetRho(rho);
    // allocWaypoint->AssignStreams(stream);
    MobilityHelper mobilityUe;
    mobilityUe.SetMobilityModel("ns3::RandomWaypointMobilityModel",
           "Speed",StringValue("ns3::ConstantRandomVariable[Constant=16.67]"),
           "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.50]"),
           "PositionAllocator",PointerValue(allocWaypoint));
   mobilityUe.SetPositionAllocator(allocUe);
   mobilityUe.Install(ueNodes);
   //----------------
   // Ptr<UniformRandomVariable> wpRadiusSampler  = CreateObject<UniformRandomVariable>();
   // wpRadiusSampler->SetAttribute( "Max", DoubleValue(0.866*cellRadius) );   // maximum radius within cell boundary
   // wpRadiusSampler->SetAttribute( "Min", DoubleValue(0.800*cellRadius) );   // minimum radius not much lees than cell boundary
   // // Setup position allocators for Mobile UEs
   // Ptr<ListPositionAllocator> posAllocMobileUeCell0    = CreateObject<ListPositionAllocator>();
   // for( unsigned ueIdx = 0; ueIdx < enb0MobileUsers; ueIdx++ ) { posAllocMobileUeCell0->Add( Vector(enb0X,enb0Y,1.50) ); }
   // Ptr<RandomDiscPositionAllocator> wpAllocMobileUeCell0   = CreateObject<RandomDiscPositionAllocator>();
   // wpAllocMobileUeCell0->SetX(enb0X);  wpAllocMobileUeCell0->SetY(enb0Y);  wpAllocMobileUeCell0->SetRho(wpRadiusSampler);
   //
   // // Setup mobility for Mobile users in eNodes
   // MobilityHelper mobilityMobileUeCell0;
   // mobilityMobileUeCell0.SetMobilityModel( "ns3::RandomWaypointMobilityModel",
   //     "Speed", StringValue("ns3::ConstantRandomVariable[Constant=16.67]"),
   //     "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.50]"),
   //     "PositionAllocator", PointerValue(wpAllocMobileUeCell0) );
   // mobilityMobileUeCell0.SetPositionAllocator( posAllocMobileUeCell0 );
   // mobilityMobileUeCell0.Install( ueNodeMobileCell0 );
   //----------------


   NetDeviceContainer enbDevs;
   NetDeviceContainer ueDevs;

   enbDevs = lteHelper->InstallEnbDevice (enbNodes);
   ueDevs = lteHelper->InstallUeDevice (ueNodes);


   //Installazione di Internet e assegnazion indirizzi di default
    internet.Install(ueNodes);
    Ipv4InterfaceContainer ueIpIfaces;
    ueIpIfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDevs));

    for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

    //necessario per i nodi sonda

    //Attacchiamo il nodo mobile alla stazione base
    for (uint32_t i = 0; i < ueDevs.GetN(); i++)
      {
        lteHelper->Attach (ueDevs.Get(i), enbDevs.Get(0));

        // side effect: the default EPS bearer will be activated
      }


    uint16_t dlPort = 1234;

    ApplicationContainer centralClientApps;
    ApplicationContainer centralServerApps;
    NS_LOG_INFO("Application Creation");
    for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
        OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress(ueIpIfaces.GetAddress(u), dlPort));
        onoff.SetAttribute("OnTime",
            StringValue("ns3::ConstantRandomVariable[Constant=1000]"));
        onoff.SetAttribute("OffTime",
            StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        onoff.SetAttribute("PacketSize",
            UintegerValue(1024));
        onoff.SetAttribute("DataRate",
            StringValue(rate.str()));
        centralClientApps.Add(onoff.Install(remoteHost));

        PacketSinkHelper sink ("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(), dlPort));
        centralServerApps.Add (sink.Install(ueNodes.Get(u)));
    }

   lteHelper->EnableTraces ();

    centralServerApps.Start (Seconds (0.001));
    centralClientApps.Start (Seconds (0.001));


    /*AnimationInterface::SetNodeDescription(enbNodes,"ENB");
    AnimationInterface::SetConstantPosition(pgw,800,800,0);
    //AnimationInterface::SetNodeColor(intEnbs,255,255,0);
    AnimationInterface::SetNodeDescription(remoteHostContainer,"RH");
    AnimationInterface::SetConstantPosition(remoteHost,2500,2500,0);
    //AnimationInterface::SetNodeColor(intEnbs,255,255,0);
    AnimationInterface anim ("urban_eval.xml");
    //centralClientApps.Stop(Seconds(10));
    Simulator::Stop (Seconds (simTime));*/

   //Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteNetDevice/$ns3::LteEnbNetDevice/LteEnbPhy/DlSpectrumPhy/TxStart", MakeCallback(&SpectrumTx));
   //Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteNetDevice/$ns3::LteUeNetDevice/LteUePhy/DlSpectrumPhy/RxStart", MakeCallback(&SpectrumRx));
   //Config::Connect("/NodeList/*/$ns3::MobilityModel/$ns3::SteadyStateRandomWaypointMobilityModel/CourseChange", MakeCallback(&CourseChange));
   //Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteNetDevice/$ns3::LteUeNetDevice/LteUePhy/DlSpectrumPhy/RxEndOk", MakeCallback(&RxOkPhy));
   //Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteNetDevice/$ns3::LteUeNetDevice/LteUePhy/DlSpectrumPhy/RxEndError", MakeCallback(&RxErrorPhy));
   //Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteEnbNetDevice/LteEnbRrc/UeMap/*/Srb1/LtePdcp/TxPDU", MakeCallback(&PdcpTx));
   //Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/LteUeRrc/DataRadioBearerMap/*/LtePdcp/RxPDU", MakeCallback(&PdcpRx));

    NS_LOG_INFO("Scheduling events for position tracing");

    for(uint32_t u=0;u<nUes;++u){
        for(double j=1;j<=simTime;++j){
            Simulator::Schedule(Time(Seconds(j)),CourseChange,ueNodes.Get(u));
        }
    }

    for( uint32_t t = 0; t <= simTime; t++ ) {
        Simulator::Schedule( Time(Seconds(simTime)), ReportProgress );
        Simulator::Schedule( Time(Seconds(simTime+0.250)), ReportProgress );
        Simulator::Schedule( Time(Seconds(simTime+0.500)), ReportProgress );
        Simulator::Schedule( Time(Seconds(simTime+0.750)), ReportProgress );
    }


    NS_LOG_INFO( "Simulation RUN " << std::endl
                 << "TotalNodes = " << ueNodes.GetN()
                 << " stream_x = " << stream
                 << " rateSingle = " <<rate.str() );

   Simulator::Run ();

   //Prendiamo i risultati
    rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    NS_LOG_INFO("Done");
    //outfile_transmission.close();
    //outfile_pos.close();

    NS_LOG_INFO("Tempo di simulazione "<<(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec)<<" sec.");
    //std::cout<<"Tempo di simulazione "<<(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec)<<" sec."<<std::endl;
    //NS_LOG_INFO();
    //std::cout<<"TxTotBytes= "<<size_tx<<std::endl;
    //NS_LOG_INFO("TxTotBytes= "<<size_tx);
    //std::cout<<"RxTotBytes= "<<size_rx<<std::endl;
    //NS_LOG_INFO("RxTotBytes= "<<size_rx);
    //std::cout<<"RxTotBytesCorrected= "<<size_rx_corr<<std::endl;
    //NS_LOG_INFO("RxTotBytesCorrected= "<<size_rx_corr);
    //std::cout<<"RxTotBytesError= "<<size_rx_err<<std::endl;
    //NS_LOG_INFO("RxTotBytesError= "<<size_rx_err);

    outfile_pos.close();

   Simulator::Destroy ();

    return 0;
}

// Lte Base example program from User Documentation

#include<iostream>

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include <ns3/config-store.h>

using namespace ns3;

int main( int argc, char *argv[] ) {

    CommandLine cmd;
    cmd.Parse( argc, argv );
    ConfigStore inputConfig;
    inputConfig.ConfigureDefaults();
    cmd.Parse( argc, argv );


    // Create a LteHelper
    Ptr<LteHelper> lteHelper    = CreateObject<LteHelper>();

    // Create eNodeBs and UEs
    NodeContainer enbNodes;     enbNodes.Create( 1 );
    NodeContainer ueNodes;      ueNodes.Create( 2 );

    // Setup mobility
    MobilityHelper  mobility;
    mobility.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );  mobility.Install( enbNodes );
    mobility.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );  mobility.Install( ueNodes );

    // Install LTE stack
    NetDeviceContainer  enbDevs;    enbDevs = lteHelper->InstallEnbDevice( enbNodes );
    NetDeviceContainer  ueDevs;     ueDevs  = lteHelper->InstallUeDevice( ueNodes );

    // Attach UEs to eNodeBs
    lteHelper->Attach( ueDevs, enbDevs.Get(0) );

    // Activate saturation traffic
    enum EpsBearer::Qci q   = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer( q );
    lteHelper->ActivateDataRadioBearer( ueDevs, bearer );

    // Setup simulation time
    Simulator::Stop( Seconds(10) );

    // Configure Logs
    lteHelper->EnablePhyTraces();
    lteHelper->EnableMacTraces();
    lteHelper->EnableRlcTraces();
    lteHelper->EnablePdcpTraces();

    // Run Simulation
    Simulator::Run();

    // End Simulation
    Simulator::Destroy();

    return 0;
}
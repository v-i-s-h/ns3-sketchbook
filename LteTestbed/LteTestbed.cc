// LTE Testbed for multiple cell simulation

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"

using namespace ns3;

int main( int argc, char *argv[] ) {

    // Configuration
    // double cellRadius = 1500.00;

    // number of users
    // @TODO

    Ptr<LteHelper> lteHelper    = CreateObject<LteHelper>();

    Ptr<LteHexGridEnbTopologyHelper> cellTopology = CreateObject<LteHexGridEnbTopologyHelper>();


    return 0;
}

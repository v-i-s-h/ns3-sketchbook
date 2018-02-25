# ns3 Sketchbook

## A Collection of ns3 simulation codes
This is repo of my ns3/scratch folder. Includes simulation codes for various scenarios. Codes may refer to some proprietary algorithms/modules which are not a part of this repo. This repository also contains julia scripts to plot performance metrics.

#### LTE Module
1. Lte1CellTestbed
2. Lte4CellTestbed
3. LteBasic
4. LteFading
5. LteMultiTraffic
6. LteSinrDistance
7. LteTestbed
8. LteThroughput
9. LteTrafficVoIP
10. LteWatson

#### How to run?
To run any of the above scenarios, use the following template:
```
./waf --run "scratch/Lte4CellTestbed/Lte4CellTestbed" --cwd "scratch/Lte4CellTestbed/"
```

If you want to specify configuration parameters,
* Use following template to save current simulation configuration
```
./waf --command-template="%s --ns3::ConfigStore::Filename=input-defaults.txt --ns3::ConfigStore::Mode=Save --ns3::ConfigStore::FileFormat=RawText" --run "scratch/LteBasic/LteBasic" --cwd "scratch/LteBasic/"
```
* Use following template to load a configuration
```
./waf --command-template="%s --ns3::ConfigStore::Filename=input-defaults.txt --ns3::ConfigStore::Mode=Load --ns3::ConfigStore::FileFormat=RawText" --run "scratch/LteBasic/LteBasic" --cwd "scratch/LteBasic/"
```

#### Fading Traces
This repository also contains fading traces distributed with ns3 and a matlab script to generate fading traces under ```fading-traces``` folder. This is used by some of the other scenarios in this repo. A dummy simulator is also included in the folder to avoid waf erros during building.
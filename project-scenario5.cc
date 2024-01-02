/*
CN PROJECT (SCENARIO-5)

CLASS = BESE-13 B

GROUP MEMBERS:
Jaweria Manahil(419118)
Sameed Ilyas(426125)
Sara Adnan(411228)
Zainab Athar(405094)

SUBMITTED TO: Dr. Huma Ghafoor
*/


#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <cstdlib>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/aodv-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
//#include "ns3/assert.h" used while testing
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-module.h"

//MADE USING NS-3.40 some methods or classes will be implemented differently on previous versions

#define NODES 50
#define ANIMATION_TIME 10

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ProjectScenario5");


int main(int argc, char *argv[])
{
    // Seed and logging setup
    LogComponentEnable("ProjectScenario5", LOG_LEVEL_ALL);

double totalAverageDelay = 0.0;
    double totalPdr = 0.0;
    int successfullExp = 0, flowCounter = 0;
    int numExperiments = 20;
    
    
     for (int experiment = 0; experiment < numExperiments; ++experiment) {
     
     
    RngSeedManager::SetSeed(experiment + 1);
    
    std::cout << "Iteration Number " << experiment + 1 << " :" <<std::endl;
    
    // Define the number of packets to send in each burst
const uint32_t numPackets = 5;
const uint32_t packetSize = 1024; // Size of each packet
const DataRate dataRate("500kb/s");
    
      // Create nodes
    
    NodeContainer nodes;
    nodes.Create(NODES);

       // Mobility configuration
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                  "X", StringValue("300.0"),
                                  "Y", StringValue("300.0"),
                                  "Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=150]"));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(0, 500, 0, 500)),
                              "Distance", DoubleValue(5.0),
                              "Time", TimeValue(Seconds(1.0)));
    mobility.Install(nodes);
    

// Wi-Fi configuration
WifiHelper wifi;
YansWifiPhyHelper wifiPhy;

YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
wifiPhy.SetChannel(wifiChannel.Create());

WifiMacHelper wifiMac;
wifiMac.SetType("ns3::AdhocWifiMac");
NetDeviceContainer wifiDevices = wifi.Install(wifiPhy, wifiMac, nodes);

    
    //AODV Routing Protocol
    AodvHelper aodv;
    InternetStackHelper stack;
    stack.SetRoutingHelper(aodv);
    stack.Install(nodes);

            // IP address assignment
    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(wifiDevices);
    
    
// Server (PacketSink) setup
uint16_t port = 9;  // Use any port number

// Client (UdpClient) setup
UdpClientHelper client(interfaces.GetAddress(49), port); // The address of the destination node
client.SetAttribute("MaxPackets", UintegerValue(numPackets));
client.SetAttribute("Interval", TimeValue(Seconds(1.0))); // Interval between packets
client.SetAttribute("PacketSize", UintegerValue(packetSize));

ApplicationContainer clientApps = client.Install(nodes.Get(0)); // Install client on the source node
clientApps.Start(Seconds(1.0));
clientApps.Stop(Seconds(ANIMATION_TIME));


// Server (PacketSink) setup
UdpServerHelper server(port);
ApplicationContainer serverApps = server.Install(nodes.Get(49)); // Install server on the destination node
serverApps.Start(Seconds(1.0));
serverApps.Stop(Seconds(ANIMATION_TIME));

//FlowMonitor SetUp
        FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor = flowmon.InstallAll();


    // Animation setup
    std::string animFilename = "project-scenario5-" + std::to_string(experiment) + ".xml";
    AnimationInterface anim(animFilename.c_str());
    anim.EnablePacketMetadata(true); // Enable packet metadata for NetAnim
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(10));


        // Run the simulation
        Simulator::Stop(Seconds(ANIMATION_TIME));
        Simulator::Run();
    
        //FlowMonitor Statistics Calculation
 Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i) {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
    std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
    std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
    std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
    flowCounter++;
    double throughput = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1024 / 1024;
    std::cout << "  Throughput: " << throughput << " Mbps\n";

    if (i->second.rxPackets > 0) {
        double averageDelay = i->second.delaySum.GetSeconds() / i->second.rxPackets;
        double pdr = static_cast<double>(i->second.rxPackets) / i->second.txPackets;
        std::cout << "  Average Delay: " << averageDelay << " seconds\n";
        std::cout << "  Packet Delivery Ratio: " << pdr << "\n";
        totalAverageDelay += averageDelay;
        totalPdr += pdr;
        successfullExp++;
       
    } else {
        std::cout << "  Average Delay: N/A\n";
        std::cout << "  Packet Delivery Ratio: N/A\n";
    }

    std::cout << "\n";
}
    
    Simulator::Destroy();
    }
 // Calculate and print total averages
    double finalAverageDelay = totalAverageDelay / successfullExp;
    double finalPdr = totalPdr / flowCounter;

    std::cout << "Total Average Delay over " << numExperiments << " experiments: " << finalAverageDelay << " seconds" << std::endl;
    std::cout << "Total Packet Delivery Ratio over " << numExperiments << " experiments: " << finalPdr << std::endl;
    return 0;
}

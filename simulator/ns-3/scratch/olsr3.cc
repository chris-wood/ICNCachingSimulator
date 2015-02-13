/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:"nill"; -*- */
/*
 * Copyright (c) 2012 Amir Reda
 *
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
 *Based on AeroRP
 *Authors: Dr/ Sherif Khatab <s.khattab@fci-cu.edu.eg>
 *         Eng/ Amir mohamed Reda <amiralex32@gmail.com>
 *
 * this scenario is described as follows two main parts
 * 1- ground station
 *  a- fixed mobility model
 *  b- routing protocol aerorp with flag true to use it in routing protocol and fixed IP
 *  c- it acts like a server for appliction clint server
 * 2- airborne nodes
 *  a- gauss markov mobility model
 *  b- routing protocol aerorp
 *  c- TDMA tx range is 28
 *  d- client and adhoc protocol for application
 */
 #include "ns3/core-module.h"
 #include "ns3/netanim-module.h"
 #include "ns3/network-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/mobility-module.h"
 #include "ns3/config-store-module.h"
 #include "ns3/wifi-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/dsdv-helper.h"
 #include "ns3/flow-monitor-module.h"
 #include "ns3/stats-module.h"
//#include "olsr-routing-protocol.h"
#include "ns3/socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-route.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/ipv4-header.h"
#include "ns3/node-list.h"
// #include "ns3/simple-wireless-tdma-module.h"
 //#include "ns3/aerorp-module.h"
 #include <iostream>
 #include <cmath>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"
#include <iostream>
#include <cmath>
#include "ns3/socket.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
 #include "ns3/core-module.h"
 #include "ns3/netanim-module.h"
 #include "ns3/network-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/mobility-module.h"
 #include "ns3/config-store-module.h"
 #include "ns3/wifi-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/dsdv-helper.h"
 #include "ns3/flow-monitor-module.h"
 #include "ns3/stats-module.h"
#include "ns3/tag.h"

using namespace ns3;
using namespace std;
uint16_t port = 5000;

NS_LOG_COMPONENT_DEFINE ("AeroRPSimulationExample");

class TimestampTag : public Tag {
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);

  // these are our accessors to our tag structure
  void SetTimestamp (Time time);
  Time GetTimestamp (void) const;

  void Print (std::ostream &os) const;

private:
  Time m_timestamp;

  // end class TimestampTag
};

TypeId
TimestampTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("TimestampTag")
    .SetParent<Tag> ()
    .AddConstructor<TimestampTag> ()
    .AddAttribute ("Timestamp",
                   "Some momentous point in time!",
                   EmptyAttributeValue (),
                   MakeTimeAccessor (&TimestampTag::GetTimestamp),
                   MakeTimeChecker ())
  ;
  return tid;
}
TypeId
TimestampTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
TimestampTag::GetSerializedSize (void) const
{
  return 8;
}
void
TimestampTag::Serialize (TagBuffer i) const
{
  int64_t t = m_timestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&t, 8);
}
void
TimestampTag::Deserialize (TagBuffer i)
{
  int64_t t;
  i.Read ((uint8_t *)&t, 8);
  m_timestamp = NanoSeconds (t);
}

void
TimestampTag::SetTimestamp (Time time)
{
  m_timestamp = time;
}
Time
TimestampTag::GetTimestamp (void) const
{
  return m_timestamp;
}

void
TimestampTag::Print (std::ostream &os) const
{
  os << "t=" << m_timestamp;
}

class AeroRPSimulation
{
public:
  AeroRPSimulation ();
  void PrintTime();
  void CaseRun (uint32_t nWifis,
                uint32_t nSinks,
                double totalTime,
                std::string rate,
                uint32_t nodeSpeed,
                double dataStart,
                bool printRoutes,
                std::string CSVfileName);

	void SetOnoffDelayTracker(Ptr<TimeMinMaxAvgTotalCalculator> onoffDelay);
	void SetCounter(Ptr<CounterCalculator<> > calc);

private:
  ///define data members
  //no of total nodes
  uint32_t m_nWifis;
  //no of sink nodes
  uint32_t m_nSinks;
  double m_totalTime;
  //rate of physical layer 8kbbs
  std::string m_rate;
  //speed of nodes
  uint32_t m_nodeSpeed;
  // time of start of data transmission
  double m_dataStart;
  //total received bytes
  uint32_t bytesTotal;
  //total packets received
  uint32_t packetsReceived;
  //to calculate the on off application delay
  Ptr<TimeMinMaxAvgTotalCalculator> m_onoffDelay;

  Ptr<CounterCalculator<> > m_calc;
  //print routes to neighbors
  bool m_printRoutes;
  //the file to save
  std::string m_CSVfileName;

  ///define data members of nodes attributes
  NodeContainer anNodes;
  NodeContainer gNodes;
  NetDeviceContainer devices;
  NetDeviceContainer gdevices;
  Ipv4InterfaceContainer interfaces;

private:
  ///behaviors for creating network
  void CreateNodes ();
  ///tr_name is the name of the file
  void CreateDevices (std::string tr_name);
  ///the file name tr_name of print routes
  void InstallInternetStack (std::string tr_name);
  void InstallApplications ();
  void SetupMobility ();
  //method used to receive packet and return the packets received
  void ReceivePacket (Ptr <Socket> );
  //return socket
  Ptr <Socket> SetupPacketReceive (Ipv4Address, Ptr <Node> );
  //calculate throughput
  void CheckThroughput ();

};

DataCollector data;
uint64_t packetsTransmitted = 0;
uint64_t n = 0;
Time t_delay = Seconds(0.0), t2_delay = Seconds(0.0);
Ptr<TimeMinMaxAvgTotalCalculator> m_macDelay;

void
OnoffTxTrace (std::string context, Ptr<const Packet> p)
{
	packetsTransmitted +=1;
}

int main (int argc, char **argv)
{
  AeroRPSimulation test;
  uint32_t nWifis = 60;
  uint32_t nSinks = 1;
  double totalTime = 1500.0;
  std::string rate ("4kbps");
  uint32_t nodeSpeed = 1200; // in m/s
  double dataStart = 100.0;
  bool printRoutingTable = false;
  std::string CSVfileName = "AeroRPSimulation.csv";

  CommandLine cmd;
  cmd.AddValue ("nWifis", "Number of wifi nodes[Default:60]", nWifis);
  cmd.AddValue ("nSinks", "Number of wifi sink nodes[Default:10]", nSinks);
  cmd.AddValue ("totalTime", "Total Simulation time[Default:1000]", totalTime);
  cmd.AddValue ("rate", "CBR traffic rate[Default:4kbps]", rate);
  cmd.AddValue ("nodeSpeed", "Node speed in RandomWayPoint model[Default:1200]", nodeSpeed);
  cmd.AddValue ("dataStart", "Time at which nodes start to transmit data[Default=100.0]", dataStart);
  cmd.AddValue ("printRoutingTable", "print routing table for nodes[Default:1]", printRoutingTable);
  cmd.AddValue ("CSVfileName", "The name of the CSV output file name[Default:AeroRPSimulation.csv]", CSVfileName);
  cmd.Parse (argc, argv);

  std::ofstream out (CSVfileName.c_str ());
  out << "SimulationSecond," <<
  "throughput," <<
  "PacketsReceived," <<
  "PacketsTransmitted," <<
  "NumberOfSinks," <<
  std::endl;
  out.close ();
/*
   LogComponentEnable("AeroRPRoutingProtocol", LOG_LEVEL_ALL);
   LogComponentEnable("AeroRPNeighborTable", LOG_LEVEL_ALL);
   LogComponentEnable("AeroRPPositionTable", LOG_LEVEL_ALL);
   LogComponentEnable("AeroRPPacketQueue", LOG_LEVEL_ALL);
*/
///*
   //LogComponentEnable("AeroRPRoutingProtocol", LOG_NONE);
   //LogComponentEnable("AeroRPNeighborTable", LOG_NONE);
   //LogComponentEnable("AeroRPPositionTable", LOG_NONE);
//*/
  SeedManager::SetSeed (12345);

 // Config::SetDefault ("ns3::aerorp::NeighborTable::MaxRange", DoubleValue (27800));
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", StringValue ("512"));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (rate));

  test = AeroRPSimulation ();
  test.CaseRun (nWifis,nSinks, totalTime, rate,nodeSpeed,dataStart, printRoutingTable, CSVfileName);

  Ptr<TimeMinMaxAvgTotalCalculator> onoff_delay = CreateObject<TimeMinMaxAvgTotalCalculator>();
  onoff_delay->SetKey("onoffDelay");
  test.SetOnoffDelayTracker(onoff_delay);
  data.AddDataCalculator(onoff_delay);

  std::cout << "\nOnoff packet delay:\t" << t_delay.GetSeconds()  << "/" << n << " = "<< (t_delay.GetSeconds())/n <<"\n\n";

  return 0;
}

void
AeroRPSimulation::PrintTime()
{
   cout << "Time: " << Simulator::Now().GetSeconds() << endl;
   Simulator::Schedule(Seconds(1.0), &AeroRPSimulation::PrintTime, this);
}

void
AeroRPSimulation::SetOnoffDelayTracker(Ptr<TimeMinMaxAvgTotalCalculator> onoffDelay)
{
  m_onoffDelay = onoffDelay;
}

void
AeroRPSimulation::SetCounter(Ptr<CounterCalculator<> > calc)
{
	m_calc = calc;
}

AeroRPSimulation::AeroRPSimulation ()
  : bytesTotal (0),
    packetsReceived (0),
    m_onoffDelay (0)
{
}
//is this only packets for control or data
void
AeroRPSimulation::ReceivePacket (Ptr <Socket> socket)
{
 // NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << " Received one packet!");
  Ptr <Packet> packet;
  while (packet = socket->Recv ())
    {
      		TimestampTag timestamp;
		if (packet->FindFirstMatchingByteTag(timestamp))
		{
			Time tx = timestamp.GetTimestamp();
			t_delay = t_delay + (Simulator::Now() - tx);
			n++;
			if (m_onoffDelay != 0)
			{
				m_onoffDelay->Update((Simulator::Now() - tx));
			}
		}
      bytesTotal += packet->GetSize ();
      packetsReceived += 1;
    }
}

void
AeroRPSimulation::CheckThroughput ()
{
  double kbs = (bytesTotal * 8.0) / 1000;
  bytesTotal = 0;

  std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

  out << (Simulator::Now ()).GetSeconds () << "," << kbs << "," << packetsReceived << ","<< packetsTransmitted << ","<< m_nSinks << std::endl;

  out.close ();

  packetsReceived = 0;
  packetsTransmitted = 0;

  Simulator::Schedule (Seconds (1.0), &AeroRPSimulation::CheckThroughput, this);
}

Ptr <Socket>
AeroRPSimulation::SetupPacketReceive (Ipv4Address addr, Ptr <Node> node)
{

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr <Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback ( &AeroRPSimulation::ReceivePacket, this));

  return sink;
}

void
AeroRPSimulation::CaseRun (uint32_t nWifis,uint32_t nSinks, double totalTime, std::string rate, uint32_t nodeSpeed, double dataStart, bool printRoutes, std::string CSVfileName)
{
  m_nWifis = nWifis;
  m_nSinks = nSinks;
  m_totalTime = totalTime;
  m_rate = rate;
  m_nodeSpeed = nodeSpeed;
  m_dataStart = dataStart;
  m_printRoutes = printRoutes;
  m_CSVfileName = CSVfileName;

  std::stringstream ss;
  ss << m_nWifis;
  std::string t_nodes = ss.str ();

  std::stringstream ss3;
  ss3 << m_totalTime;
  std::string sTotalTime = ss3.str ();

  std::string tr_name = "AeroRP_" + t_nodes + "Nodes_" + sTotalTime + "SimTime";
  std::cout << "Trace file generated is " << tr_name << ".tr\n";

  CreateNodes ();
  CreateDevices (tr_name);
  SetupMobility ();
  InstallInternetStack (tr_name);
  InstallApplications ();

  NodeContainer c;
 /* for (NodeContainer::Iterator i = gNodes.Begin (); i != gNodes.End (); i++)
      {
  	c.Add(i);

        Ptr<Node> node = (*i);
        Ptr<olsr::OlsrRoutingProtocol> aerorp = node->GetObject<AeroRP::OlsrRoutingProtocol> ();
        aerorp->SetGroundStation();
        aerorp->Start();


      }*/
  OlsrHelper olsr;

  Ipv4ListRoutingHelper list;
  list.Add (olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (olsr); // has effect on the next Install ()
  internet.Install (gNodes);




/*for (NodeContainer::Iterator i = anNodes.Begin (); i != anNodes.End (); i++)
    {
      Ptr<Node> node = (*i);
      Ptr<AeroRP::AeroRoutingProtocol> aerorp = node->GetObject<AeroRP::AeroRoutingProtocol> ();

      aerorp->Start();

    }


  AeroRPHelper aerorp;
  aerorp.Install ();*/

  std::cout << "\nStarting simulation for " << m_totalTime << " s ...\n";

  CheckThroughput ();

/*
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor;

  Config::SetDefault ("ns3::FlowMonitor::StartTime",TimeValue (Seconds (m_dataStart)));
  Config::SetDefault ("ns3::FlowMonitor::PacketSizeBinWidth", DoubleValue (1000));
 //Print per flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
       std::cout << "Time: "<< Simulator :: Now() <<"Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress;
      std::cout << "Tx Packets = " << iter->second.txPackets;
      std::cout << "Rx Packets = " << iter->second.rxPackets;
      std::cout << "Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024  << " Kbps";
    }

  monitor->SerializeToXmlFile("aerorp.flowmon", true, true);
  monitor = flowmon.InstallAll();

*/
  Simulator::Stop (Seconds (m_totalTime));

  Ptr<Node> node1 = NodeList::GetNode (1);
  AnimationInterface::SetNodeColor(node1,0,255,0);

  AnimationInterface anim ("aerorp.xml");

  anim.UpdateNodeColor(node1,0,255,0);

  PrintTime();
  Simulator::Run ();


  Simulator::Destroy ();
}

void
AeroRPSimulation::CreateNodes ()
{
  std::cout << "Creating " << (unsigned) m_nWifis << " anNodes.\n";
  gNodes.Create (m_nSinks);
  anNodes.Create (m_nWifis);

  NS_ASSERT_MSG (m_nWifis > m_nSinks, "Sinks must be less or equal to the number of nodes in network");
}

void
AeroRPSimulation::SetupMobility ()
{
MobilityHelper gmobility;
  gmobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                "MinX", DoubleValue (75000.0),
                                "MinY", DoubleValue (75000.0),
                                "DeltaX", DoubleValue (0.0),
                                "DeltaY", DoubleValue (0.0),
                                "GridWidth", UintegerValue (1),
                                "LayoutType", StringValue ("RowFirst"));
  gmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  gmobility.Install (gNodes);

  Ptr<Node> node1 = NodeList::GetNode (1);
  MobilityHelper mobility1;
  Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator> ();
  positionAlloc1->Add (Vector (85000.0, 85000.0, 0.0));
  mobility1.SetPositionAllocator (positionAlloc1);
  mobility1.SetMobilityModel ("ns3::GaussMarkovMobilityModel",
                             "Bounds", BoxValue (Box (80000, 90000, 80000, 90000, 0, 1000)),
                             "TimeStep", TimeValue (Seconds (10.0)),
                             "Alpha", DoubleValue (0.85),
   "MeanVelocity", StringValue ("ns3::UniformRandomVariable[Min=800|Max=1200]"),
   "MeanDirection", StringValue ("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"),
   "MeanPitch", StringValue ("ns3::UniformRandomVariable[Min=0.05|Max=0.05]"),
   "NormalVelocity", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.0|Bound=0.0]"),
   "NormalDirection", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2|Bound=0.4]"),
   "NormalPitch", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.02|Bound=0.04]"));

  mobility1.Install (node1);

   MobilityHelper mobility;
   mobility.SetMobilityModel ("ns3::GaussMarkovMobilityModel",
   "Bounds", BoxValue (Box (0, 150000, 0, 150000, 0, 1000)),

    /*a new set of values are calculated for each node. Since the nodes’ velocity and direction are
    * fixed
    *until the next timestep, setting a large timestep will result in long periods of straight
    * movement.
    *A short timestep such as 0.25 s, will result in a path that is almost continuously changing. The
    *timestep value for our simulations is set to 10 s.
   */
   "TimeStep", TimeValue (Seconds (10.0)),
  /*
    *Setting alpha between zero and one allows us to tune the model with degrees of memory and
    *variation.
    *In order to analyze the impact of alpha on the mobility, we conducted baseline simulations.We
    *observe that as alpha increases, the node paths become less random and more predictable. For the
    *rest of the simulations we kept alpha value to be 0.85 to have some predictability in the mobility
    *of the nodes, while avoiding abrupt AN direction changes
    */
   "Alpha", DoubleValue (0.85),
   "MeanVelocity", StringValue ("ns3::UniformRandomVariable[Min=800|Max=1200]"),
   "MeanDirection", StringValue ("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"),
   "MeanPitch", StringValue ("ns3::UniformRandomVariable[Min=0.05|Max=0.05]"),
   "NormalVelocity", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.0|Bound=0.0]"),
   "NormalDirection", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2|Bound=0.4]"),
   "NormalPitch", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.02|Bound=0.04]"));
   mobility.SetPositionAllocator ("ns3::RandomBoxPositionAllocator",
   "X", StringValue ("ns3::UniformRandomVariable[Min=0|Max=150000]"),
   "Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max=150000]"),
   "Z", StringValue ("ns3::UniformRandomVariable[Min=0|Max=1000]"));

  mobility.Install (anNodes);
}

void
AeroRPSimulation::CreateDevices (std::string tr_name)
{

/*
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (m_phyMode), "ControlMode",StringValue (m_phyMode));

  devices = wifi.Install (wifiPhy, wifiMac, anNodes);
  gdevices = wifi.Install (wifiPhy, wifiMac, gNodes);

  AsciiTraceHelper ascii;
  wifiPhy.EnableAsciiAll (ascii.CreateFileStream (tr_name + ".tr"));
  wifiPhy.EnablePcapAll (tr_name);
 */
	  std::string phyMode ("DsssRate1Mbps");
	  double distance = 500;  // m
	  uint32_t numNodes = 30;  // by default, 5x5
	  double interval = 0.001; // seconds
	  uint32_t packetSize = 600; // bytes
	 // uint32_t numPackets = 10000000;
	  std::string rtslimit = "1500";

//     Config::SetDefault ("ns3::SimpleWirelessChannel::MaxRange", DoubleValue (27800));
  //   Config::SetDefault ("ns3::SimpleWirelessChannel::GroundRange", DoubleValue (150000));
   //  Config::SetDefault ("ns3::SimpleWirelessChannel::GroundID", IntegerValue (0));
      // default allocation, each node gets a slot to transmit


     // turn off RTS/CTS for frames below 2200 bytes
      Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue (rtslimit));
      // Fix non-unicast data rate to be the same as that of unicast
      Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));



      // The below set of helpers will help us to put together the wifi NICs we want
      WifiHelper wifi;

      YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
      // set it to zero; otherwise, gain will be added
      wifiPhy.Set ("RxGain", DoubleValue (-10) );
      // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
      wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

      YansWifiChannelHelper wifiChannel;
      wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
      wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
      wifiPhy.SetChannel (wifiChannel.Create ());

      // Add a non-QoS upper mac, and disable rate control
      NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
      wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                    "DataMode",StringValue (phyMode),
                                    "ControlMode",StringValue (phyMode));
      // Set it to adhoc mode
      wifiMac.SetType ("ns3::AdhocWifiMac");
      devices = wifi.Install (wifiPhy, wifiMac, gNodes);



/*      TdmaHelper tdma = TdmaHelper (anNodes.GetN ()+gNodes.GetN(), anNodes.GetN ()+gNodes.GetN()); // in this case selected, numSlots = nodes

      TdmaControllerHelper controller;
      controller.Set ("SlotTime", TimeValue (MicroSeconds (1100)));
      controller.Set ("GaurdTime", TimeValue (MicroSeconds (100)));
      controller.Set ("InterFrameTime", TimeValue (MicroSeconds (10)));
      tdma.SetTdmaControllerHelper (controller);

      devices = tdma.Install (anNodes);

      gdevices = tdma.Install (gNodes);*/
/*

      AsciiTraceHelper ascii;
      Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (tr_name + (".tr"));
      tdma.EnableAsciiAll (stream);
*/
}

void
AeroRPSimulation::InstallInternetStack (std::string tr_name)
{
  //Config::SetDefault ("ns3::aerorp::NeighborTable::MaxRange", DoubleValue (27800));
  OlsrHelper aerorp;

  InternetStackHelper stack;
  stack.SetRoutingHelper (aerorp); // has effect on the next Install ()
  stack.Install (anNodes);
  // stack.Install (gNodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  //address.Assign (gdevices);
  interfaces = address.Assign (devices);
 //


  uint16_t sinkPort = 6;
  Address sinkAddress1 (InetSocketAddress (interfaces.GetAddress (0), sinkPort)); // interface of n24
  PacketSinkHelper packetSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps1 = packetSinkHelper1.Install (gNodes.Get (1)); //n2 as sink
  sinkApps1.Start (Seconds (0.));
  sinkApps1.Stop (Seconds (100.));

  if (m_printRoutes)
    {
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ((tr_name + ".routes"), std::ios::out);
      aerorp.PrintRoutingTableAllEvery (Seconds ( 1 ), routingStream);
    }
}

void
AeroRPSimulation::InstallApplications ()
{
      Ptr<Node> appNode1 = NodeList::GetNode (1);

      Ipv4Address nodeAddress = appNode1->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
     // Ptr<Socket> sink = SetupPacketReceive (nodeAddress, appNode1);

  for (uint32_t clientNode = 1; clientNode <= m_nWifis - 1; clientNode++ )
      {
          OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address (InetSocketAddress (interfaces.GetAddress (0), port)));
          onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
          onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

              ApplicationContainer apps1 = onoff1.Install (anNodes.Get (clientNode));
              Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
              apps1.Start (Seconds (var->GetValue (m_dataStart, m_dataStart + 1)));
              apps1.Stop (Seconds (m_totalTime - 400));
      }
 Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::OnOffApplication/Tx", MakeCallback (&OnoffTxTrace));
}


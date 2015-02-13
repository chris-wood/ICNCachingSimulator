/*
 -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*-

 * Copyright (c) 2009 University of Washington
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





	LAB Assignment #5
    1. Setup a 5x5 wireless adhoc network with a grid. You may use
    examples/wireless/wifi-simple-adhoc-grid.cc as a base.

    2. Install the OLSR routing protocol.

    3. Setup three UDP traffic flows, one along each diagonal and one
    along the middle (at high rates of transmission).

    4. Setup the ns-3 flow monitor for each of these flows.

    5. Now schedule each of the flows at times 1s, 1.5s, and 2s.

    6. Now using the flow monitor, observe the throughput of each of the
    UDP flows. Furthermore, use the tracing mechanism to monitor the number of
    packet collisions/drops at intermediary nodes. Around which nodes are most
    of the collisions/drops happening?

    7. Now repeat the experiment with RTS/CTS enabled on the wifi devices.

    8. Show the difference in throughput and packet drops if any.


	Solution by: Konstantinos Katsaros (K.Katsaros@surrey.ac.uk)
	based on wifi-simple-adhoc-grid.cc


// The default layout is like this, on a 2-D grid.
//
// n20  n21  n22  n23  n24
// n15  n16  n17  n18  n19
// n10  n11  n12  n13  n14
// n5   n6   n7   n8   n9
// n0   n1   n2   n3   n4
//
// the layout is affected by the parameters given to GridPositionAllocator;
// by default, GridWidth is 5 and numNodes is 25..
//
// Flow 1: 0->24
// Flow 2: 20->4
// Flow 3: 10->4

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
#include "ns3/olsr-state.h"


// #include "ns3/simple-wireless-tdma-module.h"
 //#include "ns3/aerorp-module.h"
 #include <iostream>
 #include <cmath>


NS_LOG_COMPONENT_DEFINE ("Lab5");

using namespace ns3;
uint16_t port = 9;
Time t_delay = Seconds(0.0);
uint64_t n = 0;
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

void
TimestampTag::Print (std::ostream &os) const
{
  os << "t=" << m_timestamp;
}

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

class olsrExample
{
public:
	olsrExample ():m_onoffDelay (0)
	{};
	void InstallApplications ();
	void SetOnoffDelayTracker(Ptr<TimeMinMaxAvgTotalCalculator> onoffDelay);
	 Ptr <Socket> SetupPacketReceive (Ipv4Address addr, Ptr <Node> node);

  void CaseRun (uint32_t nWifis,
                uint32_t nSinks,
                double totalTime,
                std::string rate,
                std::string phyMode,
                uint32_t nodeSpeed,
                uint32_t periodicUpdateInterval,
                uint32_t settlingTime,
                double dataStart,
                bool printRoutes,
                std::string CSVfileName);
  Ipv4InterfaceContainer interfaces;

private:
  uint32_t m_nWifis;
  uint32_t m_nSinks;
  double m_totalTime;
  std::string m_rate;
  std::string m_phyMode;
  uint32_t m_nodeSpeed;
  uint32_t m_periodicUpdateInterval;
  uint32_t m_settlingTime;
  double m_dataStart;
  uint32_t bytesTotal;
  uint32_t packetsReceived;
  bool m_printRoutes;
  std::string m_CSVfileName;



private:
  void CreateNodes ();
  void CreateDevices (std::string tr_name);
  void InstallInternetStack (std::string tr_name);

  void SetupMobility ();
  void ReceivePacket (Ptr <Socket>);
  Ptr<TimeMinMaxAvgTotalCalculator> m_onoffDelay;
};

uint32_t MacTxDropCount, PhyTxDropCount, PhyRxDropCount;
uint64_t packetsTransmitted = 0;

void
OnoffTxTrace (std::string context, Ptr<const Packet> p)
{
	packetsTransmitted +=1;
}

void
MacTxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  MacTxDropCount++;
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
PrintDrop()
{
  std::cout << Simulator::Now().GetSeconds() << "\t" << MacTxDropCount << "\t"<< PhyTxDropCount << "\t" << PhyRxDropCount << "\n";
  Simulator::Schedule(Seconds(5.0), &PrintDrop);
}

void
PhyTxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  PhyTxDropCount++;
}
void
PhyRxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  PhyRxDropCount++;
}
int main (int argc, char *argv[])
{




  std::string phyMode ("DsssRate1Mbps");
  double distance = 500;  // m
  uint32_t numNodes = 100;  // by default, 5x5
  double interval = 0.001; // seconds
  uint32_t packetSize = 64; // bytes
 // uint32_t numPackets = 10000000;
  std::string rtslimit = "1500";
  CommandLine cmd;

  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("distance", "distance (m)", distance);
  cmd.AddValue ("packetSize", "distance (m)", packetSize);
  cmd.AddValue ("rtslimit", "RTS/CTS Threshold (bytes)", rtslimit);
  cmd.Parse (argc, argv);
  // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue (rtslimit));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  NodeContainer c;
  c.Create (numNodes);

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
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);

  ObjectFactory pos;
    pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
    pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=3000.0]"));
    pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=3000.0]"));


  MobilityHelper mobility;
  Ptr <PositionAllocator> taPositionAlloc=pos.Create ()->GetObject <PositionAllocator> ();
  mobility.SetPositionAllocator (taPositionAlloc);
//  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  std::ostringstream speedConstantRandomVariableStream;
   speedConstantRandomVariableStream << "ns3::ConstantRandomVariable[Constant="
                                    << 0.01
                                    << "]";
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel", "Speed", StringValue (speedConstantRandomVariableStream.str ()),
                               "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=2.0]"), "PositionAllocator", PointerValue (taPositionAlloc));

  mobility.Install (c);

  // Enable OLSR
  OlsrHelper olsr;

  Ipv4ListRoutingHelper list;
  list.Add (olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (olsr); // has effect on the next Install ()
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ifcont = ipv4.Assign (devices);
  olsrExample test = olsrExample ();
  test.interfaces = ifcont;
  // Create Apps

  uint16_t sinkPort = 6; // use the same for all apps

  // UDP connection from N0 to N24

   Address sinkAddress1 (InetSocketAddress (ifcont.GetAddress (50), sinkPort)); // interface of n24
   PacketSinkHelper packetSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
   ApplicationContainer sinkApps1 = packetSinkHelper1.Install (c.Get (2)); //n2 as sink
   sinkApps1.Start (Seconds (0.));
   sinkApps1.Stop (Seconds (100.));

   test.InstallApplications();
   Ptr<Socket> ns3UdpSocket1 = Socket::CreateSocket (c.Get (0), UdpSocketFactory::GetTypeId ()); //source at n0

   // Create UDP application at n0
   Ptr<MyApp> app1 = CreateObject<MyApp> ();
   app1->Setup (ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate ("1Mbps"));
   c.Get (0)->AddApplication (app1);
   app1->SetStartTime (Seconds (31.));
   app1->SetStopTime (Seconds (100.));

   // UDP connection from N10 to N14

    Address sinkAddress2 (InetSocketAddress (ifcont.GetAddress (14), sinkPort)); // interface of n14
    PacketSinkHelper packetSinkHelper2 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
    ApplicationContainer sinkApps2 = packetSinkHelper2.Install (c.Get (14)); //n14 as sink
    sinkApps2.Start (Seconds (0.));
    sinkApps2.Stop (Seconds (100.));

    Ptr<Socket> ns3UdpSocket2 = Socket::CreateSocket (c.Get (10), UdpSocketFactory::GetTypeId ()); //source at n10

    // Create UDP application at n10
    Ptr<MyApp> app2 = CreateObject<MyApp> ();
    app2->Setup (ns3UdpSocket2, sinkAddress2, packetSize, numPackets, DataRate ("1Mbps"));
    c.Get (10)->AddApplication (app2);
    app2->SetStartTime (Seconds (31.5));
    app2->SetStopTime (Seconds (100.));

    // UDP connection from N20 to N4

     Address sinkAddress3 (InetSocketAddress (ifcont.GetAddress (4), sinkPort)); // interface of n4
     PacketSinkHelper packetSinkHelper3 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
     ApplicationContainer sinkApps3 = packetSinkHelper3.Install (c.Get (4)); //n2 as sink
     sinkApps3.Start (Seconds (0.));
     sinkApps3.Stop (Seconds (100.));

     Ptr<Socket> ns3UdpSocket3 = Socket::CreateSocket (c.Get (20), UdpSocketFactory::GetTypeId ()); //source at n20

     // Create UDP application at n20
     Ptr<MyApp> app3 = CreateObject<MyApp> ();
     app3->Setup (ns3UdpSocket3, sinkAddress3, packetSize, numPackets, DataRate ("1Mbps"));
     c.Get (20)->AddApplication (app3);
     app3->SetStartTime (Seconds (32.));
     app3->SetStopTime (Seconds (100.));

  // Install FlowMonitor on all nodes
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();


  // Trace Collisions
  Config::ConnectWithoutContext("/NodeList//$ns3::WifiNetDevice/Mac/MacTxDrop", MakeCallback(&MacTxDrop));
  Config::ConnectWithoutContext("/NodeList//$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback(&PhyRxDrop));
  Config::ConnectWithoutContext("/NodeList//$ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback(&PhyTxDrop));

  Simulator::Schedule(Seconds(0), &PrintDrop);

  Simulator::Stop (Seconds (80));
  Simulator::Run ();

  PrintDrop();

  // Print per flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  uint32_t totalRx =0;
    uint32_t totalTx =0;
    double totalDelay=0;
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

      if ((t.sourceAddress == Ipv4Address("10.1.1.1") && t.destinationAddress == Ipv4Address("10.1.1.25"))
    	|| (t.sourceAddress == Ipv4Address("10.1.1.11") && t.destinationAddress == Ipv4Address("10.1.1.15"))
    	|| (t.sourceAddress == Ipv4Address("10.1.1.21") && t.destinationAddress == Ipv4Address("10.1.1.5")))
        {
    	  NS_LOG_UNCOND("Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress);
    	  NS_LOG_UNCOND("Tx Packets = " << iter->second.txPackets);
    	  NS_LOG_UNCOND("Rx Packets = " << iter->second.rxPackets);
    	  NS_LOG_UNCOND("Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024  << " Kbps");
    	  	  	  std::cout<<"Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress;
    	  	  	std::cout<<"Tx Packets = " << iter->second.txPackets;
    	  	  std::cout<<"Rx Packets = " << iter->second.rxPackets;

    	  totalRx +=  iter->second.rxPackets;
    	  totalTx +=  iter->second.txPackets;
    	  totalDelay+= iter->second.delaySum.GetMilliSeconds();
    	  	//std::cout<<"Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024  << " Kbps"<<"\n";



       // }
    }
  Ptr<TimeMinMaxAvgTotalCalculator> onoff_delay = CreateObject<TimeMinMaxAvgTotalCalculator>();
  onoff_delay->SetKey("onoffDelay");
  test.SetOnoffDelayTracker(onoff_delay);
  std::cout << "\nOnoff packet delay:\t" << t_delay.GetSeconds()  << "/" << n << " = "<< (t_delay.GetSeconds())/n <<"\n\n";
  std::cout << "  PDR = " << ((double)totalRx / (double)totalTx)*100.0  << " \n";
  std::cout << "  TotalTX = " << (double)totalTx  << " \n";
  std::cout << "  TotalRX = " << (double)totalRx  << " \n";
  std::cout << "Delay= "<<((double)totalDelay / (double)totalRx)<<"\n";
  monitor->SerializeToXmlFile("Interval.flowmon", true, true);

  Simulator::Destroy ();

  return 0;
}

void
olsrExample::SetOnoffDelayTracker(Ptr<TimeMinMaxAvgTotalCalculator> onoffDelay)
{
  m_onoffDelay = onoffDelay;
}

void
olsrExample::InstallApplications ()
{
	NodeContainer nodes;
	for(int i=0;i<NodeList::GetNNodes();i++){
		nodes.Add(NodeList::GetNode(i));
	}
  for (uint32_t i = 0; i <= 0; i++ )
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Ipv4Address nodeAddress = node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
      Ptr<Socket> sink = this->SetupPacketReceive (nodeAddress, node);
    }

  for (uint32_t clientNode = 0; clientNode <= 9; clientNode++ )
    {
	  Ptr<Node> node = NodeList::GetNode (clientNode+12);
	        Ipv4Address nodeAddress = node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
	        Ptr<Socket> sink = this->SetupPacketReceive (nodeAddress, node);


          OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address (InetSocketAddress (interfaces.GetAddress (clientNode+12), port)));
          onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
          onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));


              ApplicationContainer apps1 = onoff1.Install (nodes.Get (clientNode));
              Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
              apps1.Start (Seconds (20));
              apps1.Stop (Seconds (50));


    }
int flowCount=0;
  for (uint32_t clientNode = 0; clientNode <= flowCount; clientNode++ )
    {
          OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address (InetSocketAddress (interfaces.GetAddress (clientNode+flowCount+1), port)));
          onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
          onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));


		  ApplicationContainer apps1 = onoff1.Install (nodes.Get (clientNode));
		  apps1.Start (Seconds (20.0));
		  apps1.Stop (Seconds (30.0));


		  PacketSinkHelper sink ("ns3::UdpSocketFactory",
		                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
		  apps1 = sink.Install (nodes.Get (clientNode+flowCount+2));
		  apps1.Start (Seconds (20.0));
		  apps1.Stop (Seconds (40.0));
    }
  Config::Connect ("/NodeList//$ns3::OnOffApplication/Tx", MakeCallback (&OnoffTxTrace));

}

Ptr <Socket>
olsrExample::SetupPacketReceive (Ipv4Address addr, Ptr <Node> node)
{

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr <Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback ( &olsrExample::ReceivePacket, this));

  return sink;
}

void
olsrExample::ReceivePacket (Ptr <Socket> socket)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << " Received one packet!");
  Ptr <Packet> packet;
  while ((packet = socket->Recv ()))
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
*/

  /* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
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
 */



/*
	LAB Assignment #5
    1. Setup a 5x5 wireless adhoc network with a grid. You may use
    examples/wireless/wifi-simple-adhoc-grid.cc as a base.

    2. Install the OLSR routing protocol.

    3. Setup three UDP traffic flows, one along each diagonal and one
    along the middle (at high rates of transmission).

    4. Setup the ns-3 flow monitor for each of these flows.

    5. Now schedule each of the flows at times 1s, 1.5s, and 2s.

    6. Now using the flow monitor, observe the throughput of each of the
    UDP flows. Furthermore, use the tracing mechanism to monitor the number of
    packet collisions/drops at intermediary nodes. Around which nodes are most
    of the collisions/drops happening?

    7. Now repeat the experiment with RTS/CTS enabled on the wifi devices.

    8. Show the difference in throughput and packet drops if any.


	Solution by: Konstantinos Katsaros (K.Katsaros@surrey.ac.uk)
	based on wifi-simple-adhoc-grid.cc
*/

// The default layout is like this, on a 2-D grid.
//
// n20  n21  n22  n23  n24
// n15  n16  n17  n18  n19
// n10  n11  n12  n13  n14
// n5   n6   n7   n8   n9
// n0   n1   n2   n3   n4
//
// the layout is affected by the parameters given to GridPositionAllocator;
// by default, GridWidth is 5 and numNodes is 25..
//
// Flow 1: 0->24
// Flow 2: 20->4
// Flow 3: 10->4

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
#include "ns3/olsr-state.h"


// #include "ns3/simple-wireless-tdma-module.h"
 //#include "ns3/aerorp-module.h"
 #include <iostream>
 #include <cmath>
 #include <sstream>


NS_LOG_COMPONENT_DEFINE ("Lab5");

using namespace ns3;
uint16_t port = 9;
Time t_delay = Seconds(0.0);
uint64_t n = 0;
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

void
TimestampTag::Print (std::ostream &os) const
{
  os << "t=" << m_timestamp;
}

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

class olsrExample
{
public:
	olsrExample ():m_onoffDelay (0)
	{};
	void InstallApplications (uint32_t no_flows);
	void SetOnoffDelayTracker(Ptr<TimeMinMaxAvgTotalCalculator> onoffDelay);
	 Ptr <Socket> SetupPacketReceive (Ipv4Address addr, Ptr <Node> node);

  void CaseRun (uint32_t nWifis,
                uint32_t nSinks,
                double totalTime,
                std::string rate,
                std::string phyMode,
                uint32_t nodeSpeed,
                uint32_t periodicUpdateInterval,
                uint32_t settlingTime,
                double dataStart,
                bool printRoutes,
                std::string CSVfileName);
  Ipv4InterfaceContainer interfaces;

private:
  uint32_t m_nWifis;
  uint32_t m_nSinks;
  double m_totalTime;
  std::string m_rate;
  std::string m_phyMode;
  uint32_t m_nodeSpeed;
  uint32_t m_periodicUpdateInterval;
  uint32_t m_settlingTime;
  double m_dataStart;
  uint32_t bytesTotal;
  uint32_t packetsReceived;
  bool m_printRoutes;
  std::string m_CSVfileName;



private:
  void CreateNodes ();
  void CreateDevices (std::string tr_name);
  void InstallInternetStack (std::string tr_name);

  void SetupMobility ();
  void ReceivePacket (Ptr <Socket>);
  Ptr<TimeMinMaxAvgTotalCalculator> m_onoffDelay;
};

uint32_t MacTxDropCount, PhyTxDropCount, PhyRxDropCount;
uint64_t packetsTransmitted = 0;

void
OnoffTxTrace (std::string context, Ptr<const Packet> p)
{
	packetsTransmitted +=1;
}

void
MacTxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  MacTxDropCount++;
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
PrintDrop()
{
  std::cout << Simulator::Now().GetSeconds() << "\t" << MacTxDropCount << "\t"<< PhyTxDropCount << "\t" << PhyRxDropCount << "\n";
  Simulator::Schedule(Seconds(5.0), &PrintDrop);
}

void
PhyTxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  PhyTxDropCount++;
}
void
PhyRxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  PhyRxDropCount++;
}



namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}


int main (int argc, char *argv[])
{
	std::string phyMode ("DsssRate1Mbps");
	double rss = -80;  // -dBm
	double distance = 500;  // m
	//double interval = 0.01; // seconds
	double interval = 1; // seconds
	//uint32_t packetSize = 1; // bytes
	uint32_t packetSize = 64; // bytes
	uint32_t numPackets = 1;
	std::string rtslimit = "1500";
	//std::string rtslimit = "2200";
	//bool verbose = false;
	//bool assocMethod1 = false;
	//bool assocMethod2 = false;


	CommandLine cmd;

	uint32_t numNodes = 100;
	double   mobilityA = 0.01;
	uint32_t no_flows = 10;
	uint32_t seedA = 1;
	netArea = 3000;
	olsr_interval = 0;

	numNodes=12 ;mobilityA=0.01 ;no_flows=1 ;seedA=2 ;netArea=200 ;olsr_interval=1;

	cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
	cmd.AddValue ("distance", "distance (m)", distance);
	cmd.AddValue ("packetSize", "packetSize", packetSize);
	cmd.AddValue ("rtslimit", "RTS/CTS Threshold (bytes)", rtslimit);

	cmd.AddValue ("numNodes", "numNodes", numNodes);
	cmd.AddValue ("mobilityA", "mobilityA", mobilityA);
	cmd.AddValue ("no_flows", "no_flows", no_flows);
	cmd.AddValue ("seedA", "seedA", seedA);
	cmd.AddValue ("netArea", "netArea", netArea);
	cmd.AddValue ("olsr_interval", "olsr_interval", olsr_interval);

	cmd.Parse (argc, argv);

	std::cout << "numNodes" << " " << "mobilityA" << " " << "no_flows" << " " << "seedA" << " " << "netArea" << "olsr_interval" << std::endl;
	std::cout << numNodes << " " << mobilityA << " " << no_flows << " " << seedA << " " << netArea << " " << olsr_interval << std::endl;

	// Convert to time object
	Time interPacketInterval = Seconds (interval);

  // turn off RTS/CTS for frames below 2200 bytes
  //Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue (rtslimit));
  // Fix non-unicast data rate to be the same as that of unicast
  //Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  // disable fragmentation for frames below 2200 bytes
  //Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue (rtslimit));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  NodeContainer c;
  c.Create (numNodes);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  // set it to zero; otherwise, gain will be added
  //wifiPhy.Set ("RxGain", DoubleValue (-10) );
  wifiPhy.Set ("TxPowerStart", DoubleValue(33));
  wifiPhy.Set ("TxPowerEnd", DoubleValue(33));
  wifiPhy.Set ("TxPowerLevels", UintegerValue(1));
  wifiPhy.Set ("TxGain", DoubleValue(0));
  wifiPhy.Set ("RxGain", DoubleValue(0));
  wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue(-61.8));
  wifiPhy.Set ("CcaMode1Threshold", DoubleValue(-64.8));




  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  //wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  //wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
  wifiChannel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel",
	  	  	  	  	  	  	  	    "SystemLoss", DoubleValue(1),
		  	  	  	  	  	  	    "HeightAboveZ", DoubleValue(1.5));
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a non-QoS upper mac, and disable rate control
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);


	//Random Seed
	RngSeedManager::SetSeed (321654);
	RngSeedManager::SetRun (seedA);

	ObjectFactory pos;
	pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
	std::string str = "ns3::UniformRandomVariable[Min=0.0|Max=";
	str = str + patch::to_string(netArea);
	str = str + "]";
	pos.Set ("X", StringValue (str));
	pos.Set ("Y", StringValue (str));

  MobilityHelper mobility;

  Ptr <PositionAllocator> taPositionAlloc=pos.Create ()->GetObject <PositionAllocator> ();
  mobility.SetPositionAllocator (taPositionAlloc);

  std::ostringstream speedConstantRandomVariableStream;
  speedConstantRandomVariableStream << "ns3::ConstantRandomVariable[Constant="
                                << mobilityA
                                << "]";
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel", "Speed", StringValue (speedConstantRandomVariableStream.str ()),
								"Pause", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"), "PositionAllocator", PointerValue (taPositionAlloc));

  //mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (c);

  // Enable OLSR
  OlsrHelper olsr;

  Ipv4ListRoutingHelper list;
  list.Add (olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (olsr); // has effect on the next Install ()
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ifcont = ipv4.Assign (devices);
  olsrExample test = olsrExample ();
  test.interfaces = ifcont;
  // Create Apps

  uint16_t sinkPort = 6; // use the same for all apps

  // UDP connection from N0 to N24

 /*  Address sinkAddress1 (InetSocketAddress (ifcont.GetAddress (50), sinkPort)); // interface of n24
   PacketSinkHelper packetSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
   ApplicationContainer sinkApps1 = packetSinkHelper1.Install (c.Get (2)); //n2 as sink
   sinkApps1.Start (Seconds (0.));
   sinkApps1.Stop (Seconds (100.));*/

   test.InstallApplications(no_flows);
/*   Ptr<Socket> ns3UdpSocket1 = Socket::CreateSocket (c.Get (0), UdpSocketFactory::GetTypeId ()); //source at n0

   // Create UDP application at n0
   Ptr<MyApp> app1 = CreateObject<MyApp> ();
   app1->Setup (ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate ("1Mbps"));
   c.Get (0)->AddApplication (app1);
   app1->SetStartTime (Seconds (31.));
   app1->SetStopTime (Seconds (100.));

   // UDP connection from N10 to N14

    Address sinkAddress2 (InetSocketAddress (ifcont.GetAddress (14), sinkPort)); // interface of n14
    PacketSinkHelper packetSinkHelper2 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
    ApplicationContainer sinkApps2 = packetSinkHelper2.Install (c.Get (14)); //n14 as sink
    sinkApps2.Start (Seconds (0.));
    sinkApps2.Stop (Seconds (100.));

    Ptr<Socket> ns3UdpSocket2 = Socket::CreateSocket (c.Get (10), UdpSocketFactory::GetTypeId ()); //source at n10

    // Create UDP application at n10
    Ptr<MyApp> app2 = CreateObject<MyApp> ();
    app2->Setup (ns3UdpSocket2, sinkAddress2, packetSize, numPackets, DataRate ("1Mbps"));
    c.Get (10)->AddApplication (app2);
    app2->SetStartTime (Seconds (31.5));
    app2->SetStopTime (Seconds (100.));

    // UDP connection from N20 to N4

     Address sinkAddress3 (InetSocketAddress (ifcont.GetAddress (4), sinkPort)); // interface of n4
     PacketSinkHelper packetSinkHelper3 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
     ApplicationContainer sinkApps3 = packetSinkHelper3.Install (c.Get (4)); //n2 as sink
     sinkApps3.Start (Seconds (0.));
     sinkApps3.Stop (Seconds (100.));

     Ptr<Socket> ns3UdpSocket3 = Socket::CreateSocket (c.Get (20), UdpSocketFactory::GetTypeId ()); //source at n20

     // Create UDP application at n20
     Ptr<MyApp> app3 = CreateObject<MyApp> ();
     app3->Setup (ns3UdpSocket3, sinkAddress3, packetSize, numPackets, DataRate ("1Mbps"));
     c.Get (20)->AddApplication (app3);
     app3->SetStartTime (Seconds (32.));
     app3->SetStopTime (Seconds (100.));*/

  // Install FlowMonitor on all nodes
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();


  // Trace Collisions
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTxDrop", MakeCallback(&MacTxDrop));
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback(&PhyRxDrop));
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback(&PhyTxDrop));

  Simulator::Schedule(Seconds(0), &PrintDrop);
  Simulator::Stop (Seconds (60));

  std::cout << "Start sim!\n";

  Simulator::Run ();

  PrintDrop();

  // Print per flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
	uint32_t totalRx =0;
	uint32_t totalTx =0;
    double totalDelay=0;
    uint32_t no_of_valid_flows=0;
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

/*      if ((t.sourceAddress == Ipv4Address("10.1.1.1") && t.destinationAddress == Ipv4Address("10.1.1.25"))
    	|| (t.sourceAddress == Ipv4Address("10.1.1.11") && t.destinationAddress == Ipv4Address("10.1.1.15"))
    	|| (t.sourceAddress == Ipv4Address("10.1.1.21") && t.destinationAddress == Ipv4Address("10.1.1.5")))
        {*/
    	  //NS_LOG_UNCOND("Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress);
    	  //NS_LOG_UNCOND("Tx Packets = " << iter->second.txPackets);
    	  //NS_LOG_UNCOND("Rx Packets = " << iter->second.rxPackets);
    	  //NS_LOG_UNCOND("Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024  << " Kbps");
/*    	  	  	  std::cout<<"Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress;
    	  	  	std::cout<<"Tx Packets = " << iter->second.txPackets;
    	  	  std::cout<<"Rx Packets = " << iter->second.rxPackets;*/

			//Ditch flows with <%10 DR
			//if ((double)iter->second.rxPackets/(double)iter->second.txPackets > 0.1)
			//{
				totalRx +=  iter->second.rxPackets;
				totalTx +=  iter->second.txPackets;
				totalDelay+= iter->second.delaySum.GetMilliSeconds();
				no_of_valid_flows++;
			//}

    	  	//std::cout<<"Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024  << " Kbps"<<"\n";



       // }
    }
  Ptr<TimeMinMaxAvgTotalCalculator> onoff_delay = CreateObject<TimeMinMaxAvgTotalCalculator>();
  onoff_delay->SetKey("onoffDelay");
  test.SetOnoffDelayTracker(onoff_delay);
  std::cout << "\nOnoff packet delay:\t" << t_delay.GetSeconds()  << "/" << n << " = "<< (t_delay.GetSeconds())/n <<"\n\n";
  std::cout << "  PDR = " << ((double)totalRx / (double)totalTx)*100.0  << " \n";
  std::cout << "  TotalTX = " << (double)totalTx  << " \n";
  std::cout << "  TotalRX = " << (double)totalRx  << " \n";
  std::cout << "Delay = "<<((double)totalDelay / (double)totalRx)<<"\n";
  std::cout << "Valid Flows = "<<no_of_valid_flows<<"\n";
  monitor->SerializeToXmlFile("Interval.flowmon", true, true);

  Simulator::Destroy ();

  return 0;
}

void
olsrExample::SetOnoffDelayTracker(Ptr<TimeMinMaxAvgTotalCalculator> onoffDelay)
{
  m_onoffDelay = onoffDelay;
}

void
olsrExample::InstallApplications (uint32_t no_flows)
{
	NodeContainer nodes;
	for(int i=0;i<NodeList::GetNNodes();i++){
		nodes.Add(NodeList::GetNode(i));
	}
/*  for (uint32_t i = 0; i <= 0; i++ )
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Ipv4Address nodeAddress = node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
      Ptr<Socket> sink = this->SetupPacketReceive (nodeAddress, node);
    }*/

  uint32_t targetNode;
  for (uint32_t clientNode = 0; clientNode <= no_flows-1; clientNode++ )
    {
	  targetNode = (clientNode + 5)%(NodeList::GetNNodes());
	  Ptr<Node> node = NodeList::GetNode (targetNode);
	        Ipv4Address nodeAddress = node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
	        Ptr<Socket> sink = this->SetupPacketReceive (nodeAddress, node);


          OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address (InetSocketAddress (interfaces.GetAddress (targetNode), port)));
          onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
          onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));


              ApplicationContainer apps1 = onoff1.Install (nodes.Get (clientNode));
              Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
              apps1.Start (Seconds (15));
              apps1.Stop (Seconds (55));


    }
/*int flowCount=0;
  for (uint32_t clientNode = 0; clientNode <= flowCount; clientNode++ )
    {
          OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address (InetSocketAddress (interfaces.GetAddress (clientNode+flowCount+1), port)));
          onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
          onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));


		  ApplicationContainer apps1 = onoff1.Install (nodes.Get (clientNode));
		  apps1.Start (Seconds (20.0));
		  apps1.Stop (Seconds (30.0));


		  PacketSinkHelper sink ("ns3::UdpSocketFactory",
		                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
		  apps1 = sink.Install (nodes.Get (clientNode+flowCount+2));
		  apps1.Start (Seconds (20.0));
		  apps1.Stop (Seconds (40.0));
    }*/
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::OnOffApplication/Tx", MakeCallback (&OnoffTxTrace));

}

Ptr <Socket>
olsrExample::SetupPacketReceive (Ipv4Address addr, Ptr <Node> node)
{

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr <Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback ( &olsrExample::ReceivePacket, this));

  return sink;
}

void
olsrExample::ReceivePacket (Ptr <Socket> socket)
{
  //NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << " Received one packet!");
	std::cout<<Simulator::Now ().GetSeconds () << " Received one packet!\n";
  Ptr <Packet> packet;
  while ((packet = socket->Recv ()))
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


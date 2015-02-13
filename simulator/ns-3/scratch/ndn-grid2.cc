/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2012 University of California, Los Angeles
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
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */
// ndn-grid.cc
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ndnSIM-module.h"

using namespace ns3;

/**
 * This scenario simulates a grid topology (using PointToPointGrid module)
 *
 * (consumer) -- ( ) ----- ( )
 *     |          |         |
 *    ( ) ------ ( ) ----- ( )
 *     |          |         |
 *    ( ) ------ ( ) -- (producer)
 *
 * All links are 1Mbps with propagation 10ms delay. 
 *
 * FIB is populated using NdnGlobalRoutingHelper.
 *
 * Consumer requests data from producer with frequency 100 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-grid
 */

int
main (int argc, char *argv[])
{
  // Setting default parameters for PointToPoint links and channels
  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("10Mbps"));
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("10ms"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("10"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Creating 3x3 topology
  PointToPointHelper p2p;
  PointToPointGridHelper grid (3, 3, p2p);
  grid.BoundingBox(100,100,200,200);

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::Flooding");
  ndnHelper.InstallAll ();

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  // Getting containers for the consumer/producer
  Ptr<Node> producer1 = grid.GetNode (2, 2);
  Ptr<Node> producer2 = grid.GetNode (1, 2);

  NodeContainer consumerNodes1;
  consumerNodes1.Add (grid.GetNode (0,0));

  NodeContainer consumerNodes2;
  consumerNodes2.Add (grid.GetNode (0,1));
  // Install NDN applications

  std::string pre = "/";
  std::string prefix = "/prefix";
  std::string prefix1 = "/prefix1";
  std::string prefix2 = "/prefix2";

  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix (prefix1);
  consumerHelper.SetAttribute ("Frequency", StringValue ("100")); // 100 interests a second
  consumerHelper.Install (consumerNodes1);



  ndn::AppHelper consumerHelper2 ("ns3::ndn::ConsumerCbr");
  consumerHelper2.SetPrefix (prefix2);
  consumerHelper2.SetAttribute ("Frequency", StringValue ("100")); // 100 interests a second
  consumerHelper2.Install (consumerNodes2);

  ndn::AppHelper producerHelper2 ("ns3::ndn::Producer");
  producerHelper2.SetPrefix (prefix2);
  producerHelper2.SetAttribute ("PayloadSize", StringValue("10"));
  producerHelper2.Install (producer2);

  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  producerHelper.SetPrefix (prefix1);
  producerHelper.SetAttribute ("PayloadSize", StringValue("10"));
  producerHelper.Install (producer1);

  // Add /prefix origins to ndn::GlobalRouter
  //ndnGlobalRoutingHelper.AddOrigins (prefix, producer);
  ndnGlobalRoutingHelper.AddOrigins (prefix1, producer1);
  ndnGlobalRoutingHelper.AddOrigins (prefix2, producer2);

  // Calculate and install FIBs
  //ndn::GlobalRoutingHelper::CalculateRoutes ();
  ndn::GlobalRoutingHelper::CustomRoutes();

  Simulator::Stop (Seconds (20.0));
  ndn::AppDelayTracer::InstallAll("results/grid-Delay.txt");
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
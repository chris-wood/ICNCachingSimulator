/*
 -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*-

 * Copyright (c) 2004 Francisco J. Ros 
 * Copyright (c) 2007 INESC Porto
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
 * Authors: Francisco J. Ros  <fjrm@dif.um.es>
 *          Gustavo J. A. M. Carneiro <gjc@inescporto.pt>


///
/// \file	olsr-state.cc
/// \brief	Implementation of all functions needed for manipulating the internal
///		state of an OLSR node.
///
#include "ns3/mobility-module.h"
#include "olsr-state.h"
#include "ns3/node.h"
#include "ns3/node-list.h"
#include "ns3/network-module.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/olsr-routing-protocol.h"


#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/wifi-net-device.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/channel.h"
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/output-stream-wrapper.h"
#include "vector"


static int tbl [1000][1000]={0};



namespace ns3 {
namespace olsr {
using namespace std;
********* MPR Selector Set Manipulation *********

MprSelectorTuple*
OlsrState::FindMprSelectorTuple (Ipv4Address const &mainAddr)
{
  for (MprSelectorSet::iterator it = m_mprSelectorSet.begin ();
       it != m_mprSelectorSet.end (); it++)
    {
      if (it->mainAddr == mainAddr)
        return &(*it);
    }
  return NULL;
}

void
OlsrState::EraseMprSelectorTuple (const MprSelectorTuple &tuple)
{
  for (MprSelectorSet::iterator it = m_mprSelectorSet.begin ();
       it != m_mprSelectorSet.end (); it++)
    {
      if (*it == tuple)
        {
          m_mprSelectorSet.erase (it);
          break;
        }
    }
}

void
OlsrState::EraseMprSelectorTuples (const Ipv4Address &mainAddr)
{
  for (MprSelectorSet::iterator it = m_mprSelectorSet.begin ();
       it != m_mprSelectorSet.end ();)
    {
      if (it->mainAddr == mainAddr)
        {
          it = m_mprSelectorSet.erase (it);
        }
      else
        {
          it++;
        }
    }
}

void
OlsrState::InsertMprSelectorTuple (MprSelectorTuple const &tuple)
{
  m_mprSelectorSet.push_back (tuple);
}

std::string
OlsrState::PrintMprSelectorSet () const
{
  std::ostringstream os;
  os << "[";
  for (MprSelectorSet::const_iterator iter = m_mprSelectorSet.begin ();
       iter != m_mprSelectorSet.end (); iter++)
    {
      MprSelectorSet::const_iterator next = iter;
      next++;
      os << iter->mainAddr;
      if (next != m_mprSelectorSet.end ())
        os << ", ";
    }
  os << "]";
  return os.str ();
}


********* Neighbor Set Manipulation *********

NeighborTuple*
OlsrState::FindNeighborTuple (Ipv4Address const &mainAddr)
{
  for (NeighborSet::iterator it = m_neighborSet.begin ();
       it != m_neighborSet.end (); it++)
    {
      if (it->neighborMainAddr == mainAddr)
        return &(*it);
    }
  return NULL;
}

const NeighborTuple*
OlsrState::FindSymNeighborTuple (Ipv4Address const &mainAddr) const
{
  for (NeighborSet::const_iterator it = m_neighborSet.begin ();
       it != m_neighborSet.end (); it++)
    {
      if (it->neighborMainAddr == mainAddr && it->status == NeighborTuple::STATUS_SYM)
        return &(*it);
    }
  return NULL;
}

NeighborTuple*
OlsrState::FindNeighborTuple (Ipv4Address const &mainAddr, uint8_t willingness)
{
  for (NeighborSet::iterator it = m_neighborSet.begin ();
       it != m_neighborSet.end (); it++)
    {
      if (it->neighborMainAddr == mainAddr && it->willingness == willingness)
        return &(*it);
    }
  return NULL;
}

void
OlsrState::EraseNeighborTuple (const NeighborTuple &tuple)
{
  for (NeighborSet::iterator it = m_neighborSet.begin ();
       it != m_neighborSet.end (); it++)
    {
      if (*it == tuple)
        {
          m_neighborSet.erase (it);
          break;
        }
    }
}

void
OlsrState::EraseNeighborTuple (const Ipv4Address &mainAddr)
{
  for (NeighborSet::iterator it = m_neighborSet.begin ();
       it != m_neighborSet.end (); it++)
    {
      if (it->neighborMainAddr == mainAddr)
        {
          it = m_neighborSet.erase (it);
          break;
        }
    }
}

void
OlsrState::InsertNeighborTuple (NeighborTuple const &tuple)
{
  for (NeighborSet::iterator it = m_neighborSet.begin ();
       it != m_neighborSet.end (); it++)
    {
      if (it->neighborMainAddr == tuple.neighborMainAddr)
        {
          // Update it
          *it = tuple;
          return;
        }
    }
  m_neighborSet.push_back (tuple);
}

********* Neighbor 2 Hop Set Manipulation *********

TwoHopNeighborTuple*
OlsrState::FindTwoHopNeighborTuple (Ipv4Address const &neighborMainAddr,
                                    Ipv4Address const &twoHopNeighborAddr)
{
  for (TwoHopNeighborSet::iterator it = m_twoHopNeighborSet.begin ();
       it != m_twoHopNeighborSet.end (); it++)
    {
      if (it->neighborMainAddr == neighborMainAddr
          && it->twoHopNeighborAddr == twoHopNeighborAddr)
        {
          return &(*it);
        }
    }
  return NULL;
}

void
OlsrState::EraseTwoHopNeighborTuple (const TwoHopNeighborTuple &tuple)
{
  for (TwoHopNeighborSet::iterator it = m_twoHopNeighborSet.begin ();
       it != m_twoHopNeighborSet.end (); it++)
    {
      if (*it == tuple)
        {
          m_twoHopNeighborSet.erase (it);
          break;
        }
    }
}

void
OlsrState::EraseTwoHopNeighborTuples (const Ipv4Address &neighborMainAddr,
                                      const Ipv4Address &twoHopNeighborAddr)
{
  for (TwoHopNeighborSet::iterator it = m_twoHopNeighborSet.begin ();
       it != m_twoHopNeighborSet.end ();)
    {
      if (it->neighborMainAddr == neighborMainAddr
          && it->twoHopNeighborAddr == twoHopNeighborAddr)
        {
          it = m_twoHopNeighborSet.erase (it);
        }
      else
        {
          it++;
        }
    }
}

void
OlsrState::EraseTwoHopNeighborTuples (const Ipv4Address &neighborMainAddr)
{
  for (TwoHopNeighborSet::iterator it = m_twoHopNeighborSet.begin ();
       it != m_twoHopNeighborSet.end ();)
    {
      if (it->neighborMainAddr == neighborMainAddr)
        {
          it = m_twoHopNeighborSet.erase (it);
        }
      else
        {
          it++;
        }
    }
}

void
OlsrState::InsertTwoHopNeighborTuple (TwoHopNeighborTuple const &tuple)
{
  m_twoHopNeighborSet.push_back (tuple);
}

********* MPR Set Manipulation *********

bool
OlsrState::FindMprAddress (Ipv4Address const &addr)
{
  MprSet::iterator it = m_mprSet.find (addr);
  return (it != m_mprSet.end ());
}

void
OlsrState::SetMprSet (MprSet mprSet)
{
  m_mprSet = mprSet;
}
MprSet
OlsrState::GetMprSet () const
{
  return m_mprSet;
}

********* Duplicate Set Manipulation *********

DuplicateTuple*
OlsrState::FindDuplicateTuple (Ipv4Address const &addr, uint16_t sequenceNumber)
{
  for (DuplicateSet::iterator it = m_duplicateSet.begin ();
       it != m_duplicateSet.end (); it++)
    {
      if (it->address == addr && it->sequenceNumber == sequenceNumber)
        return &(*it);
    }
  return NULL;
}

void
OlsrState::EraseDuplicateTuple (const DuplicateTuple &tuple)
{
  for (DuplicateSet::iterator it = m_duplicateSet.begin ();
       it != m_duplicateSet.end (); it++)
    {
      if (*it == tuple)
        {
          m_duplicateSet.erase (it);
          break;
        }
    }
}

void
OlsrState::InsertDuplicateTuple (DuplicateTuple const &tuple)
{
  m_duplicateSet.push_back (tuple);
}

********* Link Set Manipulation *********

LinkTuple*
OlsrState::FindLinkTuple (Ipv4Address const & ifaceAddr)
{
  for (LinkSet::iterator it = m_linkSet.begin ();
       it != m_linkSet.end (); it++)
    {
      if (it->neighborIfaceAddr == ifaceAddr)
        return &(*it);
    }
  return NULL;
}

LinkTuple*
OlsrState::FindSymLinkTuple (Ipv4Address const &ifaceAddr, Time now)
{
  for (LinkSet::iterator it = m_linkSet.begin ();
       it != m_linkSet.end (); it++)
    {
      if (it->neighborIfaceAddr == ifaceAddr)
        {
          if (it->symTime > now)
            return &(*it);
          else
            break;
        }
    }
  return NULL;
}

void
OlsrState::EraseLinkTuple (const LinkTuple &tuple)
{
  for (LinkSet::iterator it = m_linkSet.begin ();
       it != m_linkSet.end (); it++)
    {
      if (*it == tuple)
        {
          m_linkSet.erase (it);
          break;
        }
    }
}

LinkTuple&
OlsrState::InsertLinkTuple (LinkTuple const &tuple)
{
  m_linkSet.push_back (tuple);
  return m_linkSet.back ();
}

********* Topology Set Manipulation *********

TopologyTuple*
OlsrState::FindTopologyTuple (Ipv4Address const &destAddr,
                              Ipv4Address const &lastAddr)
{
  for (TopologySet::iterator it = m_topologySet.begin ();
       it != m_topologySet.end (); it++)
    {
      if (it->destAddr == destAddr && it->lastAddr == lastAddr)
        return &(*it);
    }
  return NULL;
}

TopologyTuple*
OlsrState::FindNewerTopologyTuple (Ipv4Address const & lastAddr, uint16_t ansn)
{
  for (TopologySet::iterator it = m_topologySet.begin ();
       it != m_topologySet.end (); it++)
    {
      if (it->lastAddr == lastAddr && it->sequenceNumber > ansn)
        return &(*it);
    }
  return NULL;
}

void
OlsrState::EraseTopologyTuple (const TopologyTuple &tuple)
{
  for (TopologySet::iterator it = m_topologySet.begin ();
       it != m_topologySet.end (); it++)
    {
      if (*it == tuple)
        {
          m_topologySet.erase (it);
          break;
        }
    }
}

void
OlsrState::EraseOlderTopologyTuples (const Ipv4Address &lastAddr, uint16_t ansn)
{
  for (TopologySet::iterator it = m_topologySet.begin ();
       it != m_topologySet.end ();)
    {
      if (it->lastAddr == lastAddr && it->sequenceNumber < ansn)
        {
          it = m_topologySet.erase (it);
        }
      else
        {
          it++;
        }
    }
}

void
OlsrState::InsertTopologyTuple (TopologyTuple const &tuple)
{
  m_topologySet.push_back (tuple);
}

********* Interface Association Set Manipulation *********

IfaceAssocTuple*
OlsrState::FindIfaceAssocTuple (Ipv4Address const &ifaceAddr)
{
  for (IfaceAssocSet::iterator it = m_ifaceAssocSet.begin ();
       it != m_ifaceAssocSet.end (); it++)
    {
      if (it->ifaceAddr == ifaceAddr)
        return &(*it);
    }
  return NULL;
}

const IfaceAssocTuple*
OlsrState::FindIfaceAssocTuple (Ipv4Address const &ifaceAddr) const
{
  for (IfaceAssocSet::const_iterator it = m_ifaceAssocSet.begin ();
       it != m_ifaceAssocSet.end (); it++)
    {
      if (it->ifaceAddr == ifaceAddr)
        return &(*it);
    }
  return NULL;
}

void
OlsrState::EraseIfaceAssocTuple (const IfaceAssocTuple &tuple)
{
  for (IfaceAssocSet::iterator it = m_ifaceAssocSet.begin ();
       it != m_ifaceAssocSet.end (); it++)
    {
      if (*it == tuple)
        {
          m_ifaceAssocSet.erase (it);
          break;
        }
    }
}

void
OlsrState::InsertIfaceAssocTuple (const IfaceAssocTuple &tuple)
{
  m_ifaceAssocSet.push_back (tuple);
}

std::vector<Ipv4Address>
OlsrState::FindNeighborInterfaces (const Ipv4Address &neighborMainAddr) const
{
  std::vector<Ipv4Address> retval;
  for (IfaceAssocSet::const_iterator it = m_ifaceAssocSet.begin ();
       it != m_ifaceAssocSet.end (); it++)
    {
      if (it->mainAddr == neighborMainAddr)
        retval.push_back (it->ifaceAddr);
    }
  return retval;
}

********* Host-Network Association Set Manipulation *********

AssociationTuple*
OlsrState::FindAssociationTuple (const Ipv4Address &gatewayAddr, const Ipv4Address &networkAddr, const Ipv4Mask &netmask)
{
  for (AssociationSet::iterator it = m_associationSet.begin ();
       it != m_associationSet.end (); it++)
    {
      if (it->gatewayAddr == gatewayAddr and it->networkAddr == networkAddr and it->netmask == netmask)
        {
          return &(*it);
        }
    }
  return NULL;
}

void
OlsrState::EraseAssociationTuple (const AssociationTuple &tuple)
{
  for (AssociationSet::iterator it = m_associationSet.begin ();
       it != m_associationSet.end (); it++)
    {
      if (*it == tuple)
        {
          m_associationSet.erase (it);
          break;
        }
    }
}

void
OlsrState::InsertAssociationTuple (const AssociationTuple &tuple)
{
  m_associationSet.push_back (tuple);
}

void
OlsrState::EraseAssociation (const Association &tuple)
{
  for (Associations::iterator it = m_associations.begin ();
       it != m_associations.end (); it++)
    {
      if (*it == tuple)
        {
          m_associations.erase (it);
          break;
        }
    }
}

void
OlsrState::InsertAssociation (const Association &tuple)
{
  m_associations.push_back (tuple);
}

void
OlsrState::faceChange(){

	int a=dsdv::RoutingTable::dsdvNodes.GetN();
	a++;

	int nodeCount=NodeList::GetNNodes();
	  for(int i=0;i<nodeCount;i++){
		  for(int j=0;j<nodeCount;j++){
			  tbl[i][j]=0;
		  }
	  }

	  for(int i=0;i<nodeCount;i++){
		  Ptr<Node> node1=NodeList::GetNode(i);
		  Ptr<Ipv4> stack = node1 -> GetObject<Ipv4> ();
		  Ptr<Ipv4RoutingProtocol> rp_Gw = (stack->GetRoutingProtocol ());
		  Ptr<olsr::RoutingProtocol> olsr_Gw = DynamicCast<olsr::RoutingProtocol> (rp_Gw);
		  if(olsr_Gw==0) return;
		  for (NeighborSet::iterator it = olsr_Gw->m_state.m_neighborSet.begin ();
		  			       it != olsr_Gw->m_state.m_neighborSet.end (); it++)
		  			    {

			  	  	  	  int id1=node1->GetId();
			  	  	  	  uint32_t ifIndex;

			  	  	  	  int id2=getNodeId(it->neighborMainAddr,ifIndex);
			  	  	  	  tbl[id1][id2]=ifIndex;
			  	  	  	  //tbl[id2][id1]=true;
		  			    }
	  }


	  ofstream ofs;
	  ofs.open("addEntryOutput_olsr2.txt", ios::out | ios::app );
  for(int i=0;i<nodeCount;i++){
	  for(int j=0;j<nodeCount;j++){
		  ofs<< tbl[i][j]<< " ";
	  }
	  ofs<<"\n";
  }
  ofs<<"\n";
  ofs<<"\n";

}

void
OlsrState::RunAlgorithm(){

	int MAXX = 1000;
	int MAXY = 1000;
	int nonodes = NodeList::GetNNodes();
		int noedges = 0;

		std::vector<int> nodes;
		std::vector<int> X;
		std::vector<int> Y;
		std::vector<std::pair<int,int> > edges;

		//NODES
		for(int i=0;i<nonodes;i++)
		{
			nodes.push_back(i);
		}

		//COORDINATES
		for(int i=0;i<nonodes;i++)
		{
			Ptr<Node> node=NodeList::GetNode(i);
			Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
			Vector pos = mob->GetPosition ();
			X.push_back((int)pos.x/3);
			Y.push_back((int)pos.y/3);
		}

		//EDGES
		for(int i=0;i<nonodes;i++){
			  for(int j=0;j<nonodes;j++){
				  if(tbl[i][j]){
					  edges.push_back(std::make_pair(i,j));
					  noedges++;
				  }

			  }

		  }


		std::ifstream minInt;
		std::ifstream dirSer;
		std::ofstream myfile;
		std::string line;

		//PRINT NODES
		myfile.open ("nodes.txt");
		for(int i=0;i<nonodes;i++)
		{
			myfile << nodes[i] << " " << X[i] << " " << Y[i] << std::endl;
		}
		myfile.close();

		//PRINT EDGES
		myfile.open ("edges.txt");
		for(int i=0;i<noedges;i++)
		{
			myfile << edges[i].first << " " << edges[i].second << std::endl;
		}
		myfile.close();

		//RUN PYTHON, WAIT FOR IT
		system ("python minimalIntervalTablesC2py.py main");



		minIntervals[][4]={0};///root, node, port, intIt
		dirServices[][3]={0};///node, root, label

		//READ MINIMAL INTERVAL TABLE
		int minIntervals[100000][5]={0};///root, node, port, intIt
		int dirServices[100000][3]={0};///node, root, label
		int minIntSize=0;
		int dirSerSize=0;
		minInt.open ("minIntervals.txt");
		if (minInt.is_open())
		{
			int line_no=0;
			while ( getline (minInt,line) )
			{
				char a;
				std::istringstream iss(line);
				if(!(iss >> minIntervals[line_no][0] >> minIntervals[line_no][1] >> minIntervals[line_no][2]>>a>>minIntervals[line_no][3]>>a>>minIntervals[line_no][4]>>a)){
					iss >> minIntervals[line_no][0] >> minIntervals[line_no][1] >> minIntervals[line_no][2]>>a>>minIntervals[line_no][3]>>a;
					minIntervals[line_no][4]=minIntervals[line_no][3];
				}
				//iss >> minIntervals[line_no][0] >> minIntervals[line_no][1] >> minIntervals[line_no][2]>>a>>minIntervals[line_no][3]>>a>>minIntervals[line_no][4]>>a;
				line_no++;
			}
			minIntSize=line_no;
			minInt.close();

		}
		else std::cout << "Unable to open file";

		//READ DIRECTORY SERVICES
		dirSer.open ("dirServices.txt");
		std::string line2;
		if (dirSer.is_open())
		{
			int line_no=0;
			getline (dirSer,line2);
			while ( getline (dirSer,line2) )
			{
				std::istringstream iss(line2);
				iss >> dirServices[line_no][0] >> dirServices[line_no][1] >> dirServices[line_no][2];


				line_no++;
			}
			dirSerSize=line_no;
			dirSer.close();
		}
		else std::cout << "Unable to open file";


		std::ofstream routes;
		int ne_inface;
		int des_inface;
		int ne_node;
		routes.open ("routes.txt");
		for(int i=0;i<nonodes;i++){
			for(int j=0;j<nonodes;j++){
				if(i==j) continue;
				int match;
				if(!findBestIntervalMatch(i,j,match,minIntSize, minIntervals, dirSerSize, dirServices)) continue;
				//if(!findDesInFace(match,des_inface, minIntervals)) continue;
				ne_node=minIntervals[match][2];
				ne_inface=tbl[ne_node][i];
				std::stringstream out;
				out << i;
				routes<<out.str()<<" "<<j<<" "<<ne_node<<" "<<ne_inface<<" "<<des_inface<<"\n";
				//routes.close();
			}
		}
		routes.close();
}


bool
OlsrState::findBestIntervalMatch(int src, int des, int &match, int minIntSize, int minIntervals[][5], int dirSerSize, int dirServices[][3]){
	int bestDis=255;
	int bestMatch=0;
	bool flag=false;
	//int count=sizeof(minIntervals)/sizeof(minIntervals[0]);
	for(int i=0;i<minIntSize;i++){
		int label;
		if(src!=minIntervals[i][1]) continue;
		if(des==minIntervals[i][2]){
			match=i;
			return true;
		}
		if(!FindLabelforRoot(minIntervals[i][0],des,label,dirSerSize, dirServices)) continue;
		if(label>=minIntervals[i][3]&&label<=minIntervals[i][4]){
			if(label-minIntervals[i][3]<bestDis){
				bestDis=label-minIntervals[i][3];
				bestMatch=i;
				flag=true;
			}
		}
	}
	match=bestMatch;
	return flag;

}

bool
OlsrState::FindLabelforRoot(int root, int node, int &label, int dirSerSize, int dirServices[][3]){
	//int count=sizeof(dirServices)/sizeof(dirServices[0]);
	for(int i=0;i<dirSerSize;i++){
		if(dirServices[i][0]==node&&dirServices[i][1]==root){
			label=dirServices[i][2];
			return true;
		}
	}
	return false;
}

bool
OlsrState::findDesInFace(int match, int &inFace, int minIntervals[][5]){
	for(int i=minIntervals[match][3];i<=minIntervals[match][4];i++){
		if(!tbl[i][minIntervals[match][1]]){
			inFace= tbl[i][minIntervals[match][1]];
			return true;
		}
	}
	return false;
}

int
OlsrState::getNodeId(Ipv4Address add, uint32_t &ifindex){
	//return 0;

	int nodeCount=NodeList::GetNNodes();
	for(int i=0;i<nodeCount;i++){
		Ptr<Node> node=NodeList::GetNode(i);
		int ndevices=node->GetNDevices();
		for(int j=0;j<ndevices;j++){
			Ptr<Ipv4> stack = node->GetObject<Ipv4> ();
			Ptr<Ipv4L3Protocol> l3 = stack->GetObject<Ipv4L3Protocol> ();
			//Ptr<NetDevice> device=node->GetDevice(j);
			 Ipv4Address address = l3->GetAddress (j,0).GetLocal();

			if(address==add){
				ifindex=j;
				return node->GetId();
			}
		}
	}

}

}} // namespace olsr, ns3
*/
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2004 Francisco J. Ros
 * Copyright (c) 2007 INESC Porto
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
 * Authors: Francisco J. Ros  <fjrm@dif.um.es>
 *          Gustavo J. A. M. Carneiro <gjc@inescporto.pt>
 */

///
/// \file	olsr-state.cc
/// \brief	Implementation of all functions needed for manipulating the internal
///		state of an OLSR node.
///
#include "ns3/mobility-module.h"
#include "olsr-state.h"
#include "ns3/node.h"
#include "ns3/node-list.h"
#include "ns3/network-module.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/olsr-routing-protocol.h"


#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/wifi-net-device.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/channel.h"
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/output-stream-wrapper.h"


#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/breadth_first_search.hpp>

#include <vector>
#include <iostream>
#include <map>

using namespace std;
using namespace boost;

typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS> MyGraph;
//typedef boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS> MyGraph;
typedef boost::graph_traits<MyGraph>::vertex_descriptor MyVertex;
typedef boost::graph_traits<MyGraph>::edge_descriptor MyEdge;
typedef boost::graph_traits<MyGraph>::edge_iterator edge_iterator;
typedef std::map <int, int> mapii;
typedef vector <int> iVector;
typedef vector <iVector> iVVector;
typedef std::map <int, iVVector> int_VVector;
typedef std::map <int, iVector> int_Vector;
typedef std::map <int, int_VVector> int_int_VVector;
typedef std::map <int, int_int_VVector> int_int_int_VVector;
typedef int_int_int_VVector::iterator it1;
typedef int_int_VVector::iterator it2;
typedef int_VVector::iterator it3;
typedef iVVector::iterator it4;
typedef iVector::iterator it5;

MyGraph g;
MyGraph tree [5];
mapii parents [5];
int_Vector children [5];
iVector tempV;
mapii labels [5];
mapii r_labels [5];

int_int_int_VVector nodesListPerPortPerNodePerRoot;

iVector routes_currNode, routes_destNode, routes_nextHop, routes_ne_inface, routes_des_inface;

double netArea;
bool olsr_interval;
static int tbl [1000][1000]={0};
static int tbl_bk [1000][1000]={0};

#define NOROOTS 5

//class MyVisitor : public boost::default_dfs_visitor
class MyVisitorBFS : public boost::default_bfs_visitor
{
public:

	int root;

	MyVisitorBFS(int roots)
	{
		root = roots;
	}

	void tree_edge(MyEdge e, const MyGraph& g)
	{
		//cerr << e << endl;
		int src = source(e, g);
		int tar = target(e, g);
		//add_edge(MyVertex(src),MyVertex(tar),tree[root]);
		add_edge(src,tar,tree[root]);
	}
};

class MyVisitorDFS : public boost::default_dfs_visitor
{
public:

	int dfslabel;
	int root;

	MyVisitorDFS(int roots)
	{
		root = roots;
		dfslabel = 0;
	}

	void discover_vertex(MyVertex v, const MyGraph& g)
	{
		//cerr << v << " " << dfslabel << endl;
		labels[root][v] = dfslabel;
		r_labels[root][dfslabel] = v;
		dfslabel++;
		return;
	}

	//back_edge: parents
	//tree_edge: children

	void tree_edge(MyEdge e, const MyGraph& g)
	{
		//cerr << e << " " << dfslabel << endl;
		int paren = source(e, g);
		int child = target(e, g);
		parents[root][child] = paren;
		children[root][paren].push_back(child);
	}
};

bool isVectorConsecutive(iVector &vec) //input: sorted vector
{
	bool consec = true;
	iVector::iterator it = vec.begin();
	it++;
	while (it != vec.end())
	{
		if (abs(*it - *(it-1)) != 1)
		{
			consec = false;
			break;
		}
		it++;
	}
	return consec;
}


void cleanGraphsTrees()
{
    g.clear();
	tempV.clear();
	nodesListPerPortPerNodePerRoot.clear();
	routes_currNode.clear();
	routes_destNode.clear();
	routes_nextHop.clear();
	routes_ne_inface.clear();
	routes_des_inface.clear();
	for(int k=0;k<NOROOTS;k++)
	{
		tree[k].clear();
		parents[k].clear();
		labels[k].clear();
		r_labels[k].clear();
		children[k].clear();
	}
}

double distance(int dX0, int dY0, int dX1, int dY1)
{
    return sqrt((dX1 - dX0)*(dX1 - dX0) + (dY1 - dY0)*(dY1 - dY0));
}

int returnQuadrant(int x, int y)
{
	if (x<0 && y<0)
		return 3;
	else if (x<0 && y>=0)
		return 2;
	else if (x>=0 && y>=0)
		return 1;
	else
		return 4;
}

void find_ancestors(mapii (&parents)[5],int root, int src)
{
	if (parents[root][src] == src)
	{
		//cout << src << " ";
		tempV.push_back(src);
		return;
	}
	else
	{
		find_ancestors(parents,root,parents[root][src]);
		//cout << src << " ";
		tempV.push_back(src);
		return;
	}
}

void find_descendants(iVector &desList, int_Vector (&children)[5],int root, int src)
{
	//cout << src << " ";
	desList.push_back(labels[root][src]);
	if ( children[root].find(src) == children[root].end() )
	{
		//cout << src << " ";
		return;
	}
	else
	{
		iVector::iterator it = children[root][src].begin();
		while (it != children[root][src].end()) {
			//cout << *it << ":\n";
			find_descendants(desList,children,root,*it);
			it++;
		}
		//cout << src << " ";
		return;
	}
}

void find_descendants_atPort(int_Vector (&children)[5],int root, int src)
{
	iVector desList;

	iVector::iterator it = children[root][src].begin();
	//cout << "Root: " << root << " , Src: " << src << "\n";
	while (it != children[root][src].end()) {
		//cout << "Port: " << *it << "\n";
		find_descendants(desList,children,root,*it);
		sort(desList.begin(), desList.end());
		nodesListPerPortPerNodePerRoot[root][src][*it].push_back(desList);
		desList.clear();
		//cout << "\n";
		it++;
	}
	//cout << "\n";
}

void find_descendants_atPort_except(int_Vector (&children)[5],int root, int src, int except, int node, int port)
{
	iVector desList;

	iVector::iterator it = children[root][src].begin();
	//cout << "Root: " << root << " , Src: " << src << "\n";
	while (it != children[root][src].end()) {
		if (*it != except)
		{
			find_descendants(desList,children,root,*it);
		}
		else
		{
			desList.push_back(labels[root][src]);
		}
		//cout << "Port: " << port << "\n";
		//desList.push_back(labels[root][src]);
		sort(desList.begin(), desList.end());
		nodesListPerPortPerNodePerRoot[root][node][port].push_back(desList);
		desList.clear();
		it++;
	}
}

void print_nodeTable(int_int_int_VVector &nodesListPerPortPerNodePerRoot)
{
	for (it1 it1i = nodesListPerPortPerNodePerRoot.begin(); it1i != nodesListPerPortPerNodePerRoot.end(); it1i++)
	{
		cout << "Root: " << it1i->first << endl;
		for (it2 it2i = it1i->second.begin(); it2i != it1i->second.end(); it2i++)
		{
			cout << "Node: " << it2i->first << endl;
			for (it3 it3i = it2i->second.begin(); it3i != it2i->second.end(); it3i++)
			{
				cout << "Port: " << it3i->first << endl;
				for (it4 it4i = it3i->second.begin(); it4i != it3i->second.end(); it4i++)
				{
					for (it5 it5i = it4i->begin(); it5i != it4i->end(); it5i++)
					{
						cout << *it5i << " ";
					}
					cout << endl;

					//Check vectors are consecutive
					assert(isVectorConsecutive(*it4i));
				}
				cout << endl;
			}
			cout << endl;
		}
		cout << endl;
	}
	cout << endl;
}

namespace ns3 {
namespace olsr {
using namespace std;
using namespace boost;
/********** MPR Selector Set Manipulation **********/

MprSelectorTuple*
OlsrState::FindMprSelectorTuple (Ipv4Address const &mainAddr)
{
  for (MprSelectorSet::iterator it = m_mprSelectorSet.begin ();
       it != m_mprSelectorSet.end (); it++)
    {
      if (it->mainAddr == mainAddr)
        return &(*it);
    }
  return NULL;
}

void
OlsrState::EraseMprSelectorTuple (const MprSelectorTuple &tuple)
{
  for (MprSelectorSet::iterator it = m_mprSelectorSet.begin ();
       it != m_mprSelectorSet.end (); it++)
    {
      if (*it == tuple)
        {
          m_mprSelectorSet.erase (it);
          break;
        }
    }
}

void
OlsrState::EraseMprSelectorTuples (const Ipv4Address &mainAddr)
{
  for (MprSelectorSet::iterator it = m_mprSelectorSet.begin ();
       it != m_mprSelectorSet.end ();)
    {
      if (it->mainAddr == mainAddr)
        {
          it = m_mprSelectorSet.erase (it);
        }
      else
        {
          it++;
        }
    }
}

void
OlsrState::InsertMprSelectorTuple (MprSelectorTuple const &tuple)
{
  m_mprSelectorSet.push_back (tuple);
}

std::string
OlsrState::PrintMprSelectorSet () const
{
  std::ostringstream os;
  os << "[";
  for (MprSelectorSet::const_iterator iter = m_mprSelectorSet.begin ();
       iter != m_mprSelectorSet.end (); iter++)
    {
      MprSelectorSet::const_iterator next = iter;
      next++;
      os << iter->mainAddr;
      if (next != m_mprSelectorSet.end ())
        os << ", ";
    }
  os << "]";
  return os.str ();
}


/********** Neighbor Set Manipulation **********/

NeighborTuple*
OlsrState::FindNeighborTuple (Ipv4Address const &mainAddr)
{
  for (NeighborSet::iterator it = m_neighborSet.begin ();
       it != m_neighborSet.end (); it++)
    {
      if (it->neighborMainAddr == mainAddr)
        return &(*it);
    }
  return NULL;
}

const NeighborTuple*
OlsrState::FindSymNeighborTuple (Ipv4Address const &mainAddr) const
{
  for (NeighborSet::const_iterator it = m_neighborSet.begin ();
       it != m_neighborSet.end (); it++)
    {
      if (it->neighborMainAddr == mainAddr && it->status == NeighborTuple::STATUS_SYM)
        return &(*it);
    }
  return NULL;
}

NeighborTuple*
OlsrState::FindNeighborTuple (Ipv4Address const &mainAddr, uint8_t willingness)
{
  for (NeighborSet::iterator it = m_neighborSet.begin ();
       it != m_neighborSet.end (); it++)
    {
      if (it->neighborMainAddr == mainAddr && it->willingness == willingness)
        return &(*it);
    }
  return NULL;
}

void
OlsrState::EraseNeighborTuple (const NeighborTuple &tuple)
{
  for (NeighborSet::iterator it = m_neighborSet.begin ();
       it != m_neighborSet.end (); it++)
    {
      if (*it == tuple)
        {
          m_neighborSet.erase (it);
          break;
        }
    }
}

void
OlsrState::EraseNeighborTuple (const Ipv4Address &mainAddr)
{
  for (NeighborSet::iterator it = m_neighborSet.begin ();
       it != m_neighborSet.end (); it++)
    {
      if (it->neighborMainAddr == mainAddr)
        {
          it = m_neighborSet.erase (it);
          break;
        }
    }
}

void
OlsrState::InsertNeighborTuple (NeighborTuple const &tuple)
{
  for (NeighborSet::iterator it = m_neighborSet.begin ();
       it != m_neighborSet.end (); it++)
    {
      if (it->neighborMainAddr == tuple.neighborMainAddr)
        {
          // Update it
          *it = tuple;
          return;
        }
    }
  m_neighborSet.push_back (tuple);
}

/********** Neighbor 2 Hop Set Manipulation **********/

TwoHopNeighborTuple*
OlsrState::FindTwoHopNeighborTuple (Ipv4Address const &neighborMainAddr,
                                    Ipv4Address const &twoHopNeighborAddr)
{
  for (TwoHopNeighborSet::iterator it = m_twoHopNeighborSet.begin ();
       it != m_twoHopNeighborSet.end (); it++)
    {
      if (it->neighborMainAddr == neighborMainAddr
          && it->twoHopNeighborAddr == twoHopNeighborAddr)
        {
          return &(*it);
        }
    }
  return NULL;
}

void
OlsrState::EraseTwoHopNeighborTuple (const TwoHopNeighborTuple &tuple)
{
  for (TwoHopNeighborSet::iterator it = m_twoHopNeighborSet.begin ();
       it != m_twoHopNeighborSet.end (); it++)
    {
      if (*it == tuple)
        {
          m_twoHopNeighborSet.erase (it);
          break;
        }
    }
}

void
OlsrState::EraseTwoHopNeighborTuples (const Ipv4Address &neighborMainAddr,
                                      const Ipv4Address &twoHopNeighborAddr)
{
  for (TwoHopNeighborSet::iterator it = m_twoHopNeighborSet.begin ();
       it != m_twoHopNeighborSet.end ();)
    {
      if (it->neighborMainAddr == neighborMainAddr
          && it->twoHopNeighborAddr == twoHopNeighborAddr)
        {
          it = m_twoHopNeighborSet.erase (it);
        }
      else
        {
          it++;
        }
    }
}

void
OlsrState::EraseTwoHopNeighborTuples (const Ipv4Address &neighborMainAddr)
{
  for (TwoHopNeighborSet::iterator it = m_twoHopNeighborSet.begin ();
       it != m_twoHopNeighborSet.end ();)
    {
      if (it->neighborMainAddr == neighborMainAddr)
        {
          it = m_twoHopNeighborSet.erase (it);
        }
      else
        {
          it++;
        }
    }
}

void
OlsrState::InsertTwoHopNeighborTuple (TwoHopNeighborTuple const &tuple)
{
  m_twoHopNeighborSet.push_back (tuple);
}

/********** MPR Set Manipulation **********/

bool
OlsrState::FindMprAddress (Ipv4Address const &addr)
{
  MprSet::iterator it = m_mprSet.find (addr);
  return (it != m_mprSet.end ());
}

void
OlsrState::SetMprSet (MprSet mprSet)
{
  m_mprSet = mprSet;
}
MprSet
OlsrState::GetMprSet () const
{
  return m_mprSet;
}

/********** Duplicate Set Manipulation **********/

DuplicateTuple*
OlsrState::FindDuplicateTuple (Ipv4Address const &addr, uint16_t sequenceNumber)
{
  for (DuplicateSet::iterator it = m_duplicateSet.begin ();
       it != m_duplicateSet.end (); it++)
    {
      if (it->address == addr && it->sequenceNumber == sequenceNumber)
        return &(*it);
    }
  return NULL;
}

void
OlsrState::EraseDuplicateTuple (const DuplicateTuple &tuple)
{
  for (DuplicateSet::iterator it = m_duplicateSet.begin ();
       it != m_duplicateSet.end (); it++)
    {
      if (*it == tuple)
        {
          m_duplicateSet.erase (it);
          break;
        }
    }
}

void
OlsrState::InsertDuplicateTuple (DuplicateTuple const &tuple)
{
  m_duplicateSet.push_back (tuple);
}

/********** Link Set Manipulation **********/

LinkTuple*
OlsrState::FindLinkTuple (Ipv4Address const & ifaceAddr)
{
  for (LinkSet::iterator it = m_linkSet.begin ();
       it != m_linkSet.end (); it++)
    {
      if (it->neighborIfaceAddr == ifaceAddr)
        return &(*it);
    }
  return NULL;
}

LinkTuple*
OlsrState::FindSymLinkTuple (Ipv4Address const &ifaceAddr, Time now)
{
  for (LinkSet::iterator it = m_linkSet.begin ();
       it != m_linkSet.end (); it++)
    {
      if (it->neighborIfaceAddr == ifaceAddr)
        {
          if (it->symTime > now)
            return &(*it);
          else
            break;
        }
    }
  return NULL;
}

void
OlsrState::EraseLinkTuple (const LinkTuple &tuple)
{
  for (LinkSet::iterator it = m_linkSet.begin ();
       it != m_linkSet.end (); it++)
    {
      if (*it == tuple)
        {
          m_linkSet.erase (it);
          break;
        }
    }
}

LinkTuple&
OlsrState::InsertLinkTuple (LinkTuple const &tuple)
{
  m_linkSet.push_back (tuple);
  return m_linkSet.back ();
}

/********** Topology Set Manipulation **********/

TopologyTuple*
OlsrState::FindTopologyTuple (Ipv4Address const &destAddr,
                              Ipv4Address const &lastAddr)
{
  for (TopologySet::iterator it = m_topologySet.begin ();
       it != m_topologySet.end (); it++)
    {
      if (it->destAddr == destAddr && it->lastAddr == lastAddr)
        return &(*it);
    }
  return NULL;
}

TopologyTuple*
OlsrState::FindNewerTopologyTuple (Ipv4Address const & lastAddr, uint16_t ansn)
{
  for (TopologySet::iterator it = m_topologySet.begin ();
       it != m_topologySet.end (); it++)
    {
      if (it->lastAddr == lastAddr && it->sequenceNumber > ansn)
        return &(*it);
    }
  return NULL;
}

void
OlsrState::EraseTopologyTuple (const TopologyTuple &tuple)
{
  for (TopologySet::iterator it = m_topologySet.begin ();
       it != m_topologySet.end (); it++)
    {
      if (*it == tuple)
        {
          m_topologySet.erase (it);
          break;
        }
    }
}

void
OlsrState::EraseOlderTopologyTuples (const Ipv4Address &lastAddr, uint16_t ansn)
{
  for (TopologySet::iterator it = m_topologySet.begin ();
       it != m_topologySet.end ();)
    {
      if (it->lastAddr == lastAddr && it->sequenceNumber < ansn)
        {
          it = m_topologySet.erase (it);
        }
      else
        {
          it++;
        }
    }
}

void
OlsrState::InsertTopologyTuple (TopologyTuple const &tuple)
{
  m_topologySet.push_back (tuple);
}

/********** Interface Association Set Manipulation **********/

IfaceAssocTuple*
OlsrState::FindIfaceAssocTuple (Ipv4Address const &ifaceAddr)
{
  for (IfaceAssocSet::iterator it = m_ifaceAssocSet.begin ();
       it != m_ifaceAssocSet.end (); it++)
    {
      if (it->ifaceAddr == ifaceAddr)
        return &(*it);
    }
  return NULL;
}

const IfaceAssocTuple*
OlsrState::FindIfaceAssocTuple (Ipv4Address const &ifaceAddr) const
{
  for (IfaceAssocSet::const_iterator it = m_ifaceAssocSet.begin ();
       it != m_ifaceAssocSet.end (); it++)
    {
      if (it->ifaceAddr == ifaceAddr)
        return &(*it);
    }
  return NULL;
}

void
OlsrState::EraseIfaceAssocTuple (const IfaceAssocTuple &tuple)
{
  for (IfaceAssocSet::iterator it = m_ifaceAssocSet.begin ();
       it != m_ifaceAssocSet.end (); it++)
    {
      if (*it == tuple)
        {
          m_ifaceAssocSet.erase (it);
          break;
        }
    }
}

void
OlsrState::InsertIfaceAssocTuple (const IfaceAssocTuple &tuple)
{
  m_ifaceAssocSet.push_back (tuple);
}

std::vector<Ipv4Address>
OlsrState::FindNeighborInterfaces (const Ipv4Address &neighborMainAddr) const
{
  std::vector<Ipv4Address> retval;
  for (IfaceAssocSet::const_iterator it = m_ifaceAssocSet.begin ();
       it != m_ifaceAssocSet.end (); it++)
    {
      if (it->mainAddr == neighborMainAddr)
        retval.push_back (it->ifaceAddr);
    }
  return retval;
}

/********** Host-Network Association Set Manipulation **********/

AssociationTuple*
OlsrState::FindAssociationTuple (const Ipv4Address &gatewayAddr, const Ipv4Address &networkAddr, const Ipv4Mask &netmask)
{
  for (AssociationSet::iterator it = m_associationSet.begin ();
       it != m_associationSet.end (); it++)
    {
      if (it->gatewayAddr == gatewayAddr and it->networkAddr == networkAddr and it->netmask == netmask)
        {
          return &(*it);
        }
    }
  return NULL;
}

void
OlsrState::EraseAssociationTuple (const AssociationTuple &tuple)
{
  for (AssociationSet::iterator it = m_associationSet.begin ();
       it != m_associationSet.end (); it++)
    {
      if (*it == tuple)
        {
          m_associationSet.erase (it);
          break;
        }
    }
}

void
OlsrState::InsertAssociationTuple (const AssociationTuple &tuple)
{
  m_associationSet.push_back (tuple);
}

void
OlsrState::EraseAssociation (const Association &tuple)
{
  for (Associations::iterator it = m_associations.begin ();
       it != m_associations.end (); it++)
    {
      if (*it == tuple)
        {
          m_associations.erase (it);
          break;
        }
    }
}

void
OlsrState::InsertAssociation (const Association &tuple)
{
  m_associations.push_back (tuple);
}

void
OlsrState::faceChange(){

	/*int a=dsdv::RoutingTable::dsdvNodes.GetN();
	a++;*/

	int nodeCount=NodeList::GetNNodes();
	  for(int i=0;i<nodeCount;i++){
		  for(int j=0;j<nodeCount;j++){
			  tbl[i][j]=0;
		  }
	  }

	  for(int i=0;i<nodeCount;i++){
		  Ptr<Node> node1=NodeList::GetNode(i);
		  Ptr<Ipv4> stack = node1 -> GetObject<Ipv4> ();
		  Ptr<Ipv4RoutingProtocol> rp_Gw = (stack->GetRoutingProtocol ());
		  Ptr<olsr::RoutingProtocol> olsr_Gw = DynamicCast<olsr::RoutingProtocol> (rp_Gw);
		  if(olsr_Gw==0) return;
		  for (NeighborSet::iterator it = olsr_Gw->m_state.m_neighborSet.begin ();
		  			       it != olsr_Gw->m_state.m_neighborSet.end (); it++)
		  			    {

			  	  	  	  int id1=node1->GetId();
			  	  	  	  uint32_t ifIndex;

			  	  	  	  int id2=getNodeId(it->neighborMainAddr,ifIndex);
			  	  	  	  tbl[id1][id2]=ifIndex;
			  	  	  	  //tbl[id2][id1]=true;
		  			    }
	  }


	  //ofstream ofs;
	  //ofs.open("addEntryOutput_olsr2.txt", ios::out | ios::app );
  //for(int i=0;i<nodeCount;i++){
//	  for(int j=0;j<nodeCount;j++){
	//	  ofs<< tbl[i][j]<< " ";
	//  }
	//  ofs<<"\n";
  //}
  //ofs<<"\n";
  //ofs<<"\n";

}



void
OlsrState::RunAlgorithm(){

	//clean graphs from prev. run
	cleanGraphsTrees();

	int nonodes = NodeList::GetNNodes();

	//compare old and new tbl, if equal don't run algorithm.
	bool tableSame = true;
	for(int i=0; i<nonodes;i++)
	{
		for(int j=0;j<nonodes;j++)
		{
			if(tbl[i][j] != tbl_bk[i][j])
			{
				tableSame = false;
				i=j=nonodes; //break nested loops
			}
		}
	}
	if (tableSame)
	{
		//cout << "Saved time by checking if tbl is same!" << endl;
		return;
	}
	else
	{
		//cout << "Couldn't save any time, strange!" << endl;
	}

	int XMAX = netArea;
	int YMAX = netArea;

	int noedges = 0;
	std::vector<int> nodes;
	std::vector<double> X;
	std::vector<double> Y;
	std::vector<std::pair<int,int> > edges;

	//NODES
	cout << "Number of nodes: " << nonodes << endl;
	for(int i=0;i<nonodes;i++)
	{
		nodes.push_back(i);
	}

	//COORDINATES
	for(int i=0;i<nonodes;i++)
	{
		Ptr<Node> node=NodeList::GetNode(i);
		Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
		Vector pos = mob->GetPosition ();
		X.push_back(pos.x);
		Y.push_back(pos.y);
	}

	//EDGES
	for(int i=0;i<nonodes;i++){
		for(int j=0;j<nonodes;j++){
			if(tbl[i][j]){
				edges.push_back(std::make_pair(i,j));
				noedges++;
			}
		}
	}

	//ADD NODES (not needed in boost)
	for(int i=0;i<nonodes;i++){
		//cout << nodes[i] << " " << X[i] << " " << Y[i] << std::endl;
		//boost::add_vertex(g);
	}

	//ADD EDGES
	for(int i=0;i<noedges;i++)
	{
		//cout << edges[i].first << " " << edges[i].second << endl;
		boost::add_edge(edges[i].first, edges[i].second, g);
	}
	write_graphviz(cout, g);

	//Choose 5 roots (4 corner roots and the center root)
	double distance;
	double corners [NOROOTS] = {0};
	corners [NOROOTS-1] = sqrt(pow(XMAX,2) + pow(YMAX,2));
	int cornersIndex [NOROOTS];
	int signX, signY, quadrant;
	for(int j=0;j<nonodes;j++)
	{
		signX = X[j] - (XMAX / 2);
		signY = Y[j] - (YMAX / 2);
		distance = sqrt(pow(signX,2) + pow(signY,2));
		quadrant = returnQuadrant(signX,signY);
		//FOUR QUADRANTS
		if (distance > corners[quadrant-1])
		{
			corners[quadrant-1] = distance;
			cornersIndex[quadrant-1] = j;
		}
		//CENTER CASE
		if (distance < corners[NOROOTS-1])
		{
			corners[NOROOTS-1] = distance;
			cornersIndex[NOROOTS-1] = j;
		}
	}

	//Print the corner point index and coordinates
	for(int k=0;k<NOROOTS;k++)
	{
		cout << "Root: " << k << ", node: " << cornersIndex[k] << endl;
	} cout << endl;

	//BFS Pruning, DFS Labeling
	vector <MyVisitorBFS> bvis;
	vector <MyVisitorDFS> dvis;
	for(int k=0;k<NOROOTS;k++)
	{
		//cout << "Pruning/Labeling, Root: " << k << endl;
		bvis.push_back(MyVisitorBFS(k));
		dvis.push_back(MyVisitorDFS(k));
		boost::breadth_first_search(g, MyVertex(cornersIndex[k]), boost::visitor(bvis[k]));
		boost::depth_first_search(tree[k], boost::visitor(dvis[k]).root_vertex(MyVertex(cornersIndex[k])));
		parents[k][cornersIndex[k]] = cornersIndex[k];
		//write_graphviz(cout,  tree[k]);
	}

	for(int k=0;k<NOROOTS;k++)
	{
		for(int l=0;l<nonodes;l++)
		{
			find_descendants_atPort(children, k, l);
			find_ancestors(parents,k,l); //saves to temp vector tempV
			iVector::reverse_iterator it = tempV.rbegin();
			it++; //skip node itself
			int port = *it;
			while (it != tempV.rend())
			{
				//cout << *it << " ";
				find_descendants_atPort_except(children, k, *it,*(it-1),l,port);
				it++;
			}
			//cout << "\n\n";
			tempV.clear();
		}
	}

	//print_nodeTable(nodesListPerPortPerNodePerRoot);
	//exit(0);

	int ne_node;
	int ne_inface;
	//int des_inface;
	for(int i=0;i<nonodes;i++)
	{
		for(int j=0;j<nonodes;j++)
		{
			if(i==j) continue;
			if(!findBestIntervalMatch(i,j,ne_node)) continue;
			ne_inface=tbl[ne_node][i];

			routes_currNode.push_back(i);
			routes_destNode.push_back(j);
			routes_nextHop.push_back(ne_node);
			routes_ne_inface.push_back(ne_inface);
			//routes_des_inface.push_back(des_inface);

			cout << i << " " << j << " " << ne_node << " " << ne_inface << endl;
		}
	}

	for(int i=0; i<1000;i++)
	{
		for(int j=0;j<1000;j++)
		{
			tbl_bk[i][j] = tbl[i][j];
		}
	}

	//exit(0);
}

bool
OlsrState::findBestIntervalMatch(int src, int des, int &ne_node){
	int bestDis=1000;
	int bestMatch=0;
	bool flag=false;
	int deslabel;
	iVector::iterator it;

	for (it1 it1i = nodesListPerPortPerNodePerRoot.begin(); it1i != nodesListPerPortPerNodePerRoot.end(); it1i++) //root
	{
		deslabel = labels[it1i->first][des]; //Might need error handling!
		//cout << "Destination: " << des << ", deslabel: " << deslabel << endl;
		for (it2 it2i = it1i->second.begin(); it2i != it1i->second.end(); it2i++) 	//node
		{
			if(src!=it2i->first) continue;
			for (it3 it3i = it2i->second.begin(); it3i != it2i->second.end(); it3i++)	//port
			{
				if(des==it3i->first)
				{
					//cout << "Next hop!" << endl;
					ne_node=it3i->first;
					return true;
				}
			}
			for (it3 it3i = it2i->second.begin(); it3i != it2i->second.end(); it3i++)	//port
			{
				for (it4 it4i = it3i->second.begin(); it4i != it3i->second.end(); it4i++)	//label list
				{
					it = find (it4i->begin(), it4i->end(), deslabel);
					if (it != it4i->end()) {
						//cout << "Found candidate, " << r_labels[it1i->first][*it] << endl;
						if ((it - it4i->begin()) < bestDis) {
							bestDis = it - it4i->begin();
							assert(bestDis>=0);
							//bestMatch=*it;
							bestMatch=it3i->first;
							flag=true;
						}
					}
				}
			}
		}
	}
	ne_node=bestMatch;
	return flag;
}

/*bool
OlsrState::findDesInFace(int match, int &inFace, int minIntervals[][5]){
	for(int i=minIntervals[match][3];i<=minIntervals[match][4];i++){
		if(!tbl[i][minIntervals[match][1]]){
			inFace= tbl[i][minIntervals[match][1]];
			return true;
		}
	}
	return false;
}*/

int
OlsrState::getNodeId(Ipv4Address add, uint32_t &ifindex){
	//return 0;

	int nodeCount=NodeList::GetNNodes();
	for(int i=0;i<nodeCount;i++){
		Ptr<Node> node=NodeList::GetNode(i);
		for(int j=0;j<node->GetNDevices();j++){
			Ptr<Ipv4> stack = node->GetObject<Ipv4> ();
			Ptr<Ipv4L3Protocol> l3 = stack->GetObject<Ipv4L3Protocol> ();
			//Ptr<NetDevice> device=node->GetDevice(j);
			 Ipv4Address address = l3->GetAddress (j,0).GetLocal();

			if(address==add){
				ifindex=j;
				return node->GetId();
			}
		}
	}

}

}} // namespace olsr, ns3

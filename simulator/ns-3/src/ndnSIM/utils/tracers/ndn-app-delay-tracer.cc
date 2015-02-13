/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2013 University of California, Los Angeles
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

#include "ndn-app-delay-tracer.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/config.h"
#include "ns3/names.h"
#include "ns3/callback.h"

#include "ns3/ndn-app.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"
#include "ns3/simulator.h"
#include "ns3/node-list.h"
#include "ns3/log.h"

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

#include <fstream>
#include <map>

NS_LOG_COMPONENT_DEFINE ("ndn.AppDelayTracer");
double avg_hopCount;
double avg_delay;
bool flag1;
bool flag2;
int Count;
int m_AggCountNonEdge;
int m_AggCountTot;
int HopCountTotal;
int HopCountDecreased;
std::map<std::string,int> prefixLog [1000000];
int frwIntCnt [1000000]={0};
int ObjLog[1000000]={0};
static int m_aggCount[100000][1000]={0};
static int m_reqCount[100000][1000]={0};
double avgAggHop;
int aggCount;
using namespace std;

namespace ns3 {
namespace ndn {

static std::list< boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<AppDelayTracer> > > > g_tracers;

template<class T>
static inline void
NullDeleter (T *ptr)
{
}

void
AppDelayTracer::Destroy ()
{
  g_tracers.clear ();
}

double
AppDelayTracer::GetAvrageHopCount()
{
	return avg_hopCount;
}

double
AppDelayTracer::GetAvrageAggHop()
{
	return avgAggHop;
}

int
AppDelayTracer::GetIncreaseAggCount(bool isTotal)
{
	if(isTotal){
		return m_AggCountTot++;
	}
	else
		return m_AggCountNonEdge++;
}

int
AppDelayTracer::GetIncreaseAggCountPerObj(int Obj, int NodeID)
{
	return m_aggCount[Obj][NodeID]++;
}

int
AppDelayTracer::GetIncreaseHopCount(int hopCnt, bool isTotal)
{
	if(isTotal)
		return HopCountTotal+=hopCnt;
	else{
		aggCount++;
		avgAggHop=(double)(avgAggHop*(aggCount-1)+hopCnt)/(double)aggCount;
		return HopCountDecreased+=hopCnt;
	}
}

int
AppDelayTracer::GetIncreasePrefixRequest(int NodeID, string str){

	std::map<string, int>::iterator pre;
	pre=prefixLog[NodeID].find(str);
	int result=pre->second+1;
	frwIntCnt[NodeID]++;
	prefixLog[NodeID].erase(str);
	prefixLog[NodeID].insert(std::pair<string,double>(str,result));
	return result;
}

int
AppDelayTracer::GetIncreaseObjRequest(int Obj,int NodeId){
	return m_reqCount[Obj][NodeId]++;
}

int
AppDelayTracer::GetIncreaseObjRequest(int Obj){
	return ObjLog[Obj]++;
}

void
AppDelayTracer::InstallAll (const std::string &file)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<AppDelayTracer> > tracers;
  boost::shared_ptr<std::ostream> outputStream;
  if (file != "-")
    {
      boost::shared_ptr<std::ofstream> os (new std::ofstream ());
      os->open (file.c_str (), std::ios_base::out | std::ios_base::trunc);

      if (!os->is_open ())
        {
          NS_LOG_ERROR ("File " << file << " cannot be opened for writing. Tracing disabled");
          return;
        }

      outputStream = os;
    }
  else
    {
      outputStream = boost::shared_ptr<std::ostream> (&std::cout, NullDeleter<std::ostream>);
    }

  for (NodeList::Iterator node = NodeList::Begin ();
       node != NodeList::End ();
       node++)
    {
      Ptr<AppDelayTracer> trace = Install (*node, outputStream);
      tracers.push_back (trace);
    }

  if (tracers.size () > 0)
    {
      // *m_l3RateTrace << "# "; // not necessary for R's read.table
      tracers.front ()->PrintHeader (*outputStream);
      *outputStream << "\n";
    }

  g_tracers.push_back (boost::make_tuple (outputStream, tracers));
}

void
AppDelayTracer::Install (const NodeContainer &nodes, const std::string &file)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<AppDelayTracer> > tracers;
  boost::shared_ptr<std::ostream> outputStream;
  if (file != "-")
    {
      boost::shared_ptr<std::ofstream> os (new std::ofstream ());
      os->open (file.c_str (), std::ios_base::out | std::ios_base::trunc);

      if (!os->is_open ())
        {
          NS_LOG_ERROR ("File " << file << " cannot be opened for writing. Tracing disabled");
          return;
        }

      outputStream = os;
    }
  else
    {
      outputStream = boost::shared_ptr<std::ostream> (&std::cout, NullDeleter<std::ostream>);
    }

  for (NodeContainer::Iterator node = nodes.Begin ();
       node != nodes.End ();
       node++)
    {
      Ptr<AppDelayTracer> trace = Install (*node, outputStream);
      tracers.push_back (trace);
    }

  if (tracers.size () > 0)
    {
      // *m_l3RateTrace << "# "; // not necessary for R's read.table
      tracers.front ()->PrintHeader (*outputStream);
      *outputStream << "\n";
    }

  g_tracers.push_back (boost::make_tuple (outputStream, tracers));
}

void
AppDelayTracer::Install (Ptr<Node> node, const std::string &file)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<AppDelayTracer> > tracers;
  boost::shared_ptr<std::ostream> outputStream;
  if (file != "-")
    {
      boost::shared_ptr<std::ofstream> os (new std::ofstream ());
      os->open (file.c_str (), std::ios_base::out | std::ios_base::trunc);

      if (!os->is_open ())
        {
          NS_LOG_ERROR ("File " << file << " cannot be opened for writing. Tracing disabled");
          return;
        }

      outputStream = os;
    }
  else
    {
      outputStream = boost::shared_ptr<std::ostream> (&std::cout, NullDeleter<std::ostream>);
    }

  Ptr<AppDelayTracer> trace = Install (node, outputStream);
  tracers.push_back (trace);

  if (tracers.size () > 0)
    {
      // *m_l3RateTrace << "# "; // not necessary for R's read.table
      tracers.front ()->PrintHeader (*outputStream);
      *outputStream << "\n";
    }

  g_tracers.push_back (boost::make_tuple (outputStream, tracers));
}


Ptr<AppDelayTracer>
AppDelayTracer::Install (Ptr<Node> node,
                         boost::shared_ptr<std::ostream> outputStream)
{
  NS_LOG_DEBUG ("Node: " << node->GetId ());

  Ptr<AppDelayTracer> trace = Create<AppDelayTracer> (outputStream, node);

  return trace;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

AppDelayTracer::AppDelayTracer (boost::shared_ptr<std::ostream> os, Ptr<Node> node)
: m_nodePtr (node)
, m_os (os)
{
  m_node = boost::lexical_cast<string> (m_nodePtr->GetId ());

  Connect ();

  string name = Names::FindName (node);
  if (!name.empty ())
    {
      m_node = name;
    }
}

AppDelayTracer::AppDelayTracer (boost::shared_ptr<std::ostream> os, const std::string &node)
: m_node (node)
, m_os (os)
{
  Connect ();
}

AppDelayTracer::~AppDelayTracer ()
{
};


void
AppDelayTracer::Connect ()
{
  Config::ConnectWithoutContext ("/NodeList/"+m_node+"/ApplicationList/*/LastRetransmittedInterestDataDelay",
                                 MakeCallback (&AppDelayTracer::LastRetransmittedInterestDataDelay, this));

  Config::ConnectWithoutContext ("/NodeList/"+m_node+"/ApplicationList/*/FirstInterestDataDelay",
                                 MakeCallback (&AppDelayTracer::FirstInterestDataDelay, this));
}

void
AppDelayTracer::PrintHeader (std::ostream &os) const
{
	  os << "Time" << "\t"
	     << "Node" << "\t"
	   //  << "AppId" << "\t"
	     << "SeqNo" << "\t"

	    // << "Type" << "\t"
	     << "DelayS" << "\t"
	     << "DelayUS" << "\t"
	   //  << "RetxCount" << "\t"
	     << "HopCount"  << "\t"
	     << "Avg Delay"  << "\t"
	     << "Avg HopCount" <<"";
}

void
AppDelayTracer::LastRetransmittedInterestDataDelay (Ptr<App> app, uint32_t seqno, Time delay, int32_t hopCount)
{
//	Ptr<Node> node=app->GetNode();
//	int a=node->m_getObjectCount;
//	if (a<100)
//		return;
	//if(Simulator::Now ().ToDouble (Time::S)<=10000){
	//	return;
	//}
/*
	if(seqno==10000)
		return;
*/
	hopCount=(hopCount/2)+1;
	if (flag1){
		avg_hopCount=((avg_hopCount*Count)+(hopCount))/(double)(Count+1);
		avg_delay=((avg_delay*Count)+(delay.ToDouble (Time::S)))/(double)(Count+1);
			  	  Count++;
			  	  flag2=false;
	}
	else{

		flag1=true;
		return;
	}

	  //avg_hopCount=((avg_hopCount*Count)+(hopCount))/(double)(Count+1);
	  //Count++;
*m_os << Simulator::Now ().ToDouble (Time::S) << "\t"
      << m_node << "\t"
      //<<2 app->GetId () << "\t"
      << seqno << "\t"
     // << "LastDelay" << "\t"
      << delay.ToDouble (Time::S) << "\t"
      << delay.ToDouble (Time::US) << "\t"
     // << 1 << "\t"
     << hopCount << "\t"
     <<avg_delay<<"\t"
         <<avg_hopCount<<"\n";
//hop_Count1=avg_hopCount;
//avgHopCount=avg_hopCount;
}

void
AppDelayTracer::FirstInterestDataDelay (Ptr<App> app, uint32_t seqno, Time delay, uint32_t retxCount, int32_t hopCount)
{
	//Ptr<Node> node=app->GetNode();
	//int a=node->m_getObjectCount;
	//if (a<100)
	//	return;
	//if(Simulator::Now ().ToDouble (Time::S)<=10000){
	//		return;
	//	}
/*	if(seqno==10000)
			return;*/
	hopCount=(hopCount/2)+1;
if (flag2){
	avg_hopCount=((avg_hopCount*Count)+(hopCount))/(double)(Count+1);
	avg_delay=((avg_delay*Count)+(delay.ToDouble (Time::S)))/(double)(Count+1);
		  	  Count++;
		  	  flag2=false;
}
else{
	flag2=true;
	return;
}

	 //avg_hopCount=((avg_hopCount*Count)+(hopCount))/(double)(Count+1);
	 // 	  Count++;
*m_os << Simulator::Now ().ToDouble (Time::S) << "\t"
       << m_node << "\t"
       //<< app->GetId () << "\t"
       << seqno << "\t"
      // << "FullDelay" << "\t"
       << delay.ToDouble (Time::S) << "\t"
       << delay.ToDouble (Time::US) << "\t"
     //  << retxCount << "\t"
       << hopCount << "\t"
       <<avg_delay<<"\t"
       <<avg_hopCount<<"\n";
 	  	  //hop_Count1=avg_hopCount;

 	 	// avgHopCount=avg_hopCount;
}




} // namespace ndn
} // namespace ns3

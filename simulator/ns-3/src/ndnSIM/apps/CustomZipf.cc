/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013-2014 University of California, Santa Cruz
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
 * Author: Maziar Mirzazad Barijough <maziar@soe.ucsc.edu>
 */

#include "CustomZipf.h"

#include "ns3/ndn-app-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"

#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include <iostream>
#include <string>
#include <fstream>
#include <math.h>


NS_LOG_COMPONENT_DEFINE ("ndn.CustomZipf");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (CustomZipf);
double currentReq[3];

//std::ifstream infile;
static int count;
static int DiffCount;
TypeId
CustomZipf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::CustomZipf")
    .SetGroupName ("Ndn")
    .SetParent<ConsumerCbr> ()
    .AddConstructor<CustomZipf> ()

    .AddAttribute ("NumberOfContents", "Number of the Contents in total",
                   StringValue ("100"),
                   MakeUintegerAccessor (&CustomZipf::SetNumberOfContents, &CustomZipf::GetNumberOfContents),
                   MakeUintegerChecker<uint32_t> ())

    .AddAttribute ("q", "parameter of improve rank",
                   StringValue ("0.7"),
                   MakeDoubleAccessor (&CustomZipf::SetQ, &CustomZipf::GetQ),
                   MakeDoubleChecker<double> ())

    .AddAttribute ("s", "parameter of power",
                   StringValue ("0.7"),
                   MakeDoubleAccessor (&CustomZipf::SetS, &CustomZipf::GetS),
                   MakeDoubleChecker<double> ())
   .AddAttribute ("ConsumerNode", "The requester node name",
				  StringValue ("0"),
                  MakeUintegerAccessor (&CustomZipf::SetConsumerNode, &CustomZipf::GetConsumerNode),
                  MakeUintegerChecker<uint32_t> ());



  return tid;
}


CustomZipf::CustomZipf()
  : m_N (100) // needed here to make sure when SetQ/SetS are called, there is a valid value of N
  , m_q (0.7)
  , m_s (0.7)
  , node_num(1)
  , m_SeqRng (0.0, 1.0)
{

	/*currentReq[0]=0;
	currentReq[1]=0;
	std::stringstream out;

	out << node_num;
	std::string str="temp/"+out.str()+".txt";
	//const char* s=str.c_str();
	std::ifstream infile(str.c_str());
	//infile.open(s,std::ifstream::in | std::ifstream::app);
	std::cout<<str.c_str();
	std::string line;
	if (std::getline(infile, line)){
		std::istringstream iss(line);
		iss >> nextReq[0] >>nextReq[1];
	}*/

  // SetNumberOfContents is called by NS-3 object system during the initialization
}

CustomZipf::~CustomZipf()
{

}

void
CustomZipf::SetNumberOfContents (uint32_t numOfContents)
{

  m_N = numOfContents;

  NS_LOG_DEBUG (m_q << " and " << m_s << " and " << m_N);

  m_Pcum = std::vector<double> (m_N + 1);

  m_Pcum[0] = 0.0;
  for (uint32_t i=1; i<=m_N; i++)
    {
      m_Pcum[i] = m_Pcum[i-1] + 1.0 / std::pow(i+m_q, m_s);
    }

  for (uint32_t i=1; i<=m_N; i++)
    {
      m_Pcum[i] = m_Pcum[i] / m_Pcum[m_N];
      NS_LOG_LOGIC ("Cumulative probability [" << i << "]=" << m_Pcum[i]);
  }
}

uint32_t
CustomZipf::GetNumberOfContents () const
{
  return m_N;
}

uint32_t
CustomZipf::GetConsumerNode() const
{
  return node_num;
}

void CustomZipf::SetConsumerNode(uint32_t NO)
{
	//node_num=NO;
	//std::stringstream out;
    //out << node_num;
	//std::string str="temp/"+out.str()+".txt";
	//infile.open(str.c_str(),std::ifstream::in | std::ifstream::app);
}

void
CustomZipf::SetQ (double q)
{
  m_q = q;
  SetNumberOfContents (m_N);
}

double
CustomZipf::GetQ () const
{
  return m_q;
}

void
CustomZipf::SetS (double s)
{
  m_s = s;
  SetNumberOfContents (m_N);
}

double
CustomZipf::GetS () const
{
  return m_s;
}

void
CustomZipf::SendPacket() {

  int Nodeid=GetNode()->GetId();
  std::stringstream out;
  out << Nodeid;
  std::string str="temp/"+out.str()+".txt";
  if(!infile.is_open()){
	  infile.open(str.c_str(),std::ifstream::in | std::ifstream::app);
  }

  if (!m_active) return;

  NS_LOG_FUNCTION_NOARGS ();

  uint32_t seq=std::numeric_limits<uint32_t>::max (); //invalid

  // std::cout << Simulator::Now ().ToDouble (Time::S) << "s max -> " << m_seqMax << "\n";

  while (m_retxSeqs.size ())
    {
      seq = *m_retxSeqs.begin ();
      m_retxSeqs.erase (m_retxSeqs.begin ());

      // NS_ASSERT (m_seqLifetimes.find (seq) != m_seqLifetimes.end ());
      // if (m_seqLifetimes.find (seq)->time <= Simulator::Now ())
      //   {

      //     NS_LOG_DEBUG ("Expire " << seq);
      //     m_seqLifetimes.erase (seq); // lifetime expired. Trying to find another unexpired sequence number
      //     continue;
      //   }
      NS_LOG_DEBUG("=interest seq "<<seq<<" from m_retxSeqs");
      break;
    }

	/*std::stringstream out;
	out << node_num;
	std::string str="temp/"+out.str()+".txt";
	//const char* s=str.c_str();
	std::ifstream infile(str.c_str(),std::ifstream::in | std::ifstream::app);*/
	//infile.open(str.c_str(),std::ifstream::in | std::ifstream::app);

	//std::cout<<str.c_str();
  if(n_firstTime){


		std::string line;
		if (std::getline(infile, line)){
			std::istringstream iss(line);
			iss >> nextReq[0] >>nextReq[1]>>nextReq[2];

			NextSeq[Nodeid]=nextReq[2];
		}
		n_firstTime=false;

  }
  //seq=nextReq[2];
  seq=NextSeq[GetNode()->GetId()];

  if (seq == std::numeric_limits<uint32_t>::max ()) //no retransmission
    {
      if (m_seqMax != std::numeric_limits<uint32_t>::max ())
        {
          if (m_seq >= m_seqMax)
            {
              return; // we are totally done
            }
        }

      //seq = CustomZipf::GetNextSeq();
      //seq=nextReq[2];
      seq=NextSeq[GetNode()->GetId()];
      m_seq ++;
    }
  if(seq!=nextReq[2])
	  DiffCount++;

  // std::cout << Simulator::Now ().ToDouble (Time::S) << "s -> " << seq << "\n";

  //
  std::stringstream outt;
  outt << (int)nextReq[1];
  //Name temp=m_interestName.append(outt.str());
  //temp.append(outt.str());
  //m_interestName=m_interestName+outt.str();

  //std::cout<<"name temp :"<<temp<<"\n";
  //std::cout<<outt.str()<<"\n";
  std::string outtstr=outt.str();
  if(outtstr.length()==1) outtstr="00"+outtstr;
  else if  (outtstr.length()==2) outtstr="0"+outtstr;
  Ptr<ndn::Name> name = Create<ndn::Name> ();
  name->append("pre"+outtstr);

  //name->append(outt.str());

  name->appendSeqNum (seq);
  //
  //std::cout<<outtstr<<" :"<<(*name).toUri().substr(0,7)<<"\n";


  //Ptr<ndn::Name> prefix = Create<ndn::Name> (std::string("/prefix"));
  std::string strt = "/prefix" + outtstr;

  //Ptr<ndn::Name> prefix = Create<ndn::Name> ();
  //prefix->append("prefix"+outt.str());
  //prefix->append("prefix"+outt.str());
  Ptr<Interest> interest = Create<Interest> ();
  interest->SetConsumerNode(Nodeid);
  interest->SetNonce (m_rand.GetValue ());
  interest->SetName  (name);
  interest->SetInterestLifetime(Seconds (65534));
  //std::cout<<"packet from "<< Nodeid<<" "<<*name<<" "<<count<<" "<<DiffCount<<"\n";
  //std::cout<<count<<"\n";
count++;
  // NS_LOG_INFO ("Requesting Interest: \n" << *interest);
  NS_LOG_INFO ("> Interest for " << seq<<", Total: "<<m_seq<<", face: "<<m_face->GetId());
  NS_LOG_DEBUG ("Trying to add " << seq << " with " << Simulator::Now () << ". already " << m_seqTimeouts.size () << " items");

  m_seqTimeouts.insert (SeqTimeout (seq, Simulator::Now ()));
  m_seqFullDelay.insert (SeqTimeout (seq, Simulator::Now ()));

  m_seqLastDelay.erase (seq);
  m_seqLastDelay.insert (SeqTimeout (seq, Simulator::Now ()));

  m_seqRetxCounts[seq] ++;

  m_rtt->SentSeq (SequenceNumber32 (seq), 1);

  FwHopCountTag hopCountTag;
  interest->GetPayload ()->AddPacketTag (hopCountTag);

  m_transmittedInterests (interest, this, m_face);
  m_face->ReceiveInterest (interest);

  currentReq[0]=nextReq[0];
  currentReq[1]=nextReq[1];
  currentReq[2]=nextReq[2];
  //std::cout<<currentReq[0]<<"\n";
  std::string line;
  if (std::getline(infile, line)){
  		std::istringstream iss(line);
  		iss >> nextReq[0] >>nextReq[1]>>nextReq[2];
  		NextSeq[Nodeid]=nextReq[2];

  }
  else{
	  nextReq[0]=10000000;
  }
int a=0;
if(nextReq[2]==0){
	a++;
}
  CustomZipf::ScheduleNextPacket ();
}

uint32_t
CustomZipf::GetNextSeq()
{
  uint32_t content_index = 1; //[1, m_N]
  double p_sum = 0;

  double p_random = m_SeqRng.GetValue();
  while (p_random == 0)
    {
      p_random = m_SeqRng.GetValue();
    }
  //if (p_random == 0)
  NS_LOG_LOGIC("p_random="<<p_random);
  for (uint32_t i=1; i<=m_N; i++)
    {
      p_sum = m_Pcum[i];   //m_Pcum[i] = m_Pcum[i-1] + p[i], p[0] = 0;   e.g.: p_cum[1] = p[1], p_cum[2] = p[1] + p[2]
      if (p_random <= p_sum)
        {
          content_index = i;
          break;
        } //if
    } //for
  //content_index = 1;
  NS_LOG_DEBUG("RandomNumber="<<content_index);
  if(node_num==0){
	  content_index=1;
  }
  return content_index;
}

void
CustomZipf::ScheduleNextPacket() {

/*  if (m_firstTime)
    {
      m_sendEvent = Simulator::Schedule (Seconds (0.0),
                                         &CustomZipf::SendPacket, this);
      m_firstTime = false;
    }
  else if (!m_sendEvent.IsRunning ())
    m_sendEvent = Simulator::Schedule (
                                       (m_random == 0) ?
                                       Seconds(1.0 / m_frequency)
                                       :
                                       Seconds(m_random->GetValue ()),
                                       &CustomZipf::SendPacket, this);*/
	 if (m_firstTime)
	    {
	      m_sendEvent = Simulator::Schedule (Seconds (0.0),
	                                         &CustomZipf::SendPacket, this);
	      m_firstTime = false;
	    }
	  else if (!m_sendEvent.IsRunning ())
	    m_sendEvent = Simulator::Schedule (Seconds(nextReq[0]),
	                                       &CustomZipf::SendPacket, this);

}

} /* namespace ndn */
} /* namespace ns3 */

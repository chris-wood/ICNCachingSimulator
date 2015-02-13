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
// ndn-grid.cc
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include <iostream>
#include <string>
#include <fstream>
#include <math.h>
#include <vector>
#include <string>
#include <sstream>
#include "ns3/ndnSIM/ndn.cxx/detail/error.h"

using namespace std;
using namespace ns3;


NodeContainer nodes;
double cacheNodes [100][2];
//static double time_tot;
int objNodeMapper[10000];

void mapObjectsToNodes(int ObjectCount,int NodeCount){
	for(int i=0;i<ObjectCount;i++){
		objNodeMapper[i]=rand()%NodeCount;
	}
}

void ReqsPreProcess(int ObjectCount,int NodeCount, int time_factor, int xy_min, int xy_max, int k, string traceName){

	//int depth=((int)(log10(NodeCount)/log10(k)));
    //int firstLeafIndex=pow(k,depth)-1;
	double *lastReq=new double[NodeCount]();
	std::ifstream infile(traceName.c_str());
	int numberofLine=0;
	//std::ofstream *outFiles = new ofstream [NodeCount];
	stringstream ss;
	//std::ofstream outfile("modReqs.txt");
	for(int i=0;i<NodeCount;i++){
		std::stringstream out;
	    out << i;
	    string fileName="temp/"+out.str()+std::string(".txt");
		ofstream ofs(fileName.c_str());
	}


	int count=0;
	//for(int round=0;round<2;round++){
		infile.clear();
		infile.seekg(0, ios::beg);
		std::string line;
			std::getline(infile, line);
		//double Timeindex=round*time_factor;
			double Timeindex=0;
	while (std::getline(infile, line))
	{
		numberofLine++;
		std::replace( line.begin(), line.end(), ',',' ');
		std::istringstream iss(line);
		double time,x,y;
		int obj;

		Ptr<MobilityModel> mob = nodes[2]->GetObject<MobilityModel>();
		Vector pos = mob->GetPosition ();


		if (!(iss >> time >> x >> y >> obj)) { break; }
		time=Timeindex+time*time_factor;
		x=x*xy_max;
		y=y*xy_max;
		double minDistance=sqrt(pow(x-pos.x,2.0)+pow(y-pos.y,2.0));
				int minNode=2;

		for(int i=0; i<NodeCount;i++){
			mob = nodes[i]->GetObject<MobilityModel>();
			pos = mob->GetPosition ();
			double dis=sqrt(pow(x-pos.x,2.0)+pow(y-pos.y,2.0));
			if(dis < minDistance){
				minDistance=dis;
				minNode=i;
			}
		}


		std::stringstream out;
			    out << minNode;
			    string fileName="temp/"+out.str()+std::string(".txt");
				ofstream ofs;
				ofs.open(fileName.c_str(), ios::out | ios::app );
				double difference=time-lastReq[minNode];
				ofs<<difference<<" "<<objNodeMapper[obj]<<" "<<obj<<"\n";
				lastReq[minNode]=time;
		//outFiles[minNode]<<time<<objNodeMapper[obj]<<obj<<"\n";
				count++;
	}
	//}
	cout<<"NumberOfLines"<<numberofLine<<"\n";
	cout<<count;
	for(int i=0;i<NodeCount;i++){
		std::stringstream out;
		out << i;
		string fileName="temp/"+out.str()+std::string(".txt");
		ofstream ofs;
		ofs.open(fileName.c_str(), ios::out | ios::app );
		ofs<<time_factor<<" "<<"0"<<" "<<"0"<<"\n";
		ofs.close();
	}

}

void CreateTree(int NodeCount, int k){


	nodes.Create (NodeCount);
	MobilityHelper mobility;
	Ptr<ListPositionAllocator> initialAlloc =
    CreateObject<ListPositionAllocator> ();
	for(int i=0;i<NodeCount;i++){
		int rowNum=(int)((log10(i+1)/log10(k)));
		int colNum=i-(int)pow(k,rowNum)+1;
		double x_interval=100.0/(pow(k,rowNum)+1);
		double y_interval=100.0/((int)(log10(NodeCount)/log10(k)));
		double x_pos=(colNum+1)*x_interval;
		double y_pos=rowNum*y_interval;
		Vector3D locVec(x_pos, y_pos, 0);



		      initialAlloc->Add (locVec);

		  mobility.SetPositionAllocator(initialAlloc);
		//mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel",
		//		"Position",Vector3DValue(locVec));
		mobility.Install(nodes[i]);


//		Ptr<ConstantPositionMobilityModel> loc = nodes[i]->GetObject<ConstantPositionMobilityModel> ();

	//	loc->SetPosition (locVec);
	}
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (nodes);
	PointToPointHelper p2p;
	for(int i=NodeCount-1;i>0;i--){
		p2p.Install (nodes.Get (i), nodes.Get ((i-1)/k));
	}


	Ptr<Node> producer = nodes[0];
	ndn::AppHelper producerHelper ("ns3::ndn::Producer");
	producerHelper.SetPrefix ("/");
	producerHelper.SetAttribute ("PayloadSize", StringValue("100"));
	producerHelper.Install (producer);

	//ndn::GlobalRoutingHelper ccnxGlobalRoutingHelper;
	//ccnxGlobalRoutingHelper.InstallAll ();
	//ccnxGlobalRoutingHelper.AddOrigins ("/prefix", producer);

	// Calculate and install FIBs
	//ccnxGlobalRoutingHelper.CalculateRoutes ();


	NodeContainer consumerNodes;
	int depth=((int)(log10(NodeCount)/log10(k)));
	int s=pow(k,depth)-1;


	ndn::AppHelper consumerHelper("ns3::ndn::CustomZipf");
	//ndn::AppHelper consumerHelper("ns3::ndn::ConsumerZipfMandelbrot2");
	consumerHelper.SetAttribute ("NumberOfContents", StringValue ("1000"));
	for(int i=NodeCount-1;i>=s;i--){
	  consumerNodes.Add (nodes[i]);
	}
	//consumerNodes.Add(nodes[s]);
		//ndn::AppHelper consumerHelper ("ns3::ndn::CustomZipf");
		//std::stringstream out;
		//out << (int)i;

		//consumerHelper.SetAttribute ("NumberOfContents", StringValue ("100")); // 10 different contents
		//consumerHelper.SetAttribute("ConsumerNode", StringValue(out.str()));
		consumerHelper.Install (consumerNodes);

}

void createTopology(int NodeCount, double range, int xy_min, int xy_max, int timeFactor){

	  //create nodes
        //NodeContainer nodes;
	    nodes.Create (NodeCount);


	  //set coordinations
	    Ptr<UniformRandomVariable> randomizer = CreateObject<UniformRandomVariable> ();
		randomizer->SetAttribute ("Min", DoubleValue (xy_min));
		randomizer->SetAttribute ("Max", DoubleValue (xy_max));

		MobilityHelper mobility;
		mobility.SetPositionAllocator ("ns3::RandomBoxPositionAllocator",
									   "X", PointerValue (randomizer),
									   "Y", PointerValue (randomizer),
									   "Z", PointerValue(0));

		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		mobility.Install (nodes);


		//ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
		// ndnGlobalRoutingHelper.InstallAll ();

		//ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot");
		//ndn::AppHelper consumerHelper ("ns3::ndn::CustomZipf");
		 for(int i=0;i<NodeCount;i++){
			 ndn::AppHelper producerHelper ("ns3::ndn::Producer");
			 producerHelper.SetAttribute ("PayloadSize", StringValue("100"));
			 std::stringstream out;
			 out << (int)i+100;
			 std::string outtstr=out.str();
			  if(outtstr.length()==1) outtstr="00"+outtstr;
			  else if  (outtstr.length()==2) outtstr="0"+outtstr;
			 std::string s=std::string("/pre")+outtstr+std::string("/");
			 //std::string s=std::string("/")+outtstr;
			 //std::string s=std::string("/prefix");
			 //cout<<"producer :"<<s.c_str()<<"\n";

			 producerHelper.SetPrefix(s.c_str());
			 producerHelper.Install (nodes[i]);

			 //ndnGlobalRoutingHelper.AddOrigins (s.c_str(), nodes[i]);

			 //consumerHelper.SetPrefix ("/prefix");
			 //consumerHelper.SetAttribute ("NumberOfContents", StringValue ("1000")); // 10 different contents
			 // consumerHelper.SetAttribute("ConsumerNode", StringValue(out.str()));
			 // consumerHelper.Install(nodes[i]);
		 }
		 //ndn::GlobalRoutingHelper::CalculateRoutes ();


			NodeContainer consumerNodes;
			ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot2");
			consumerHelper.SetPrefix ("/pre");
			consumerHelper.SetAttribute ("Frequency", StringValue ("100")); // 100 interests a second
			consumerHelper.SetAttribute ("NumberOfContents", StringValue ("1000000"));

			for(int i=0;i<NodeCount;i++){
			  consumerNodes.Add (nodes[i]);
			}
			ApplicationContainer consumer = consumerHelper.Install (consumerNodes);
			consumer.Start (Seconds (0));    // start consumers at 0s, 1s, 2s, 3s
			consumer.Stop  (Seconds (timeFactor-5));


	  //make p2p links
		 PointToPointHelper p2p;
		 int n=0;
		 for(int i=0;i<NodeCount;i++){
			 for(int j=0;j<NodeCount;j++){
				 Ptr<MobilityModel> mob = nodes[i]->GetObject<MobilityModel>();
				 Vector pos_i = mob->GetPosition ();
				 mob = nodes[j]->GetObject<MobilityModel>();
				 Vector pos_j = mob->GetPosition ();
				 double distance=sqrt(pow(pos_i.x-pos_j.x,2.0)+pow(pos_i.y-pos_j.y,2.0));
				 if(distance<=range){
					 p2p.Install (nodes.Get (i), nodes.Get (j));
					 n++;
				 }
			 }
		 }
		 std::cout<<"n is :"<< n<<"\n";




}

void assignCache(int NodeCount, int k, int cache){


	std::ostringstream str;
	//str<<(int)cache;
	// Install CCNx stack on all nodes
	  ndn::StackHelper ccnxHelper;
	 ccnxHelper.SetPit("ns3::ndn::pit::Persistent","MaxSize", "3000");
	  ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
	  ccnxHelper.SetContentStore ("ns3::ndn::cs::Lru", "MaxSize", "1000");
	  ccnxHelper.InstallAll ();


	  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
	  			  ndnGlobalRoutingHelper.InstallAll ();
	  			 for(int i=0;i<NodeCount;i++){
	  				 std::stringstream out;
	  				 out << (int)i+100;
	  				  std::string outtstr=out.str();
	  				  if(outtstr.length()==1) outtstr="00"+outtstr;
	  				  else if  (outtstr.length()==2) outtstr="0"+outtstr;
	  				 std::string s=std::string("/pre")+outtstr+std::string("/");
	  				//std::string s=std::string("/");
	  				 ndnGlobalRoutingHelper.AddOrigins (s.c_str(), nodes[i]);
	  			 }
	  			 ndn::GlobalRoutingHelper::CalculateRoutes();




}

void makeVoronoiRegions(int nodeCount){

	/*int j=0;
	Ptr<MobilityModel> mob;
	for(int i=0;i<nodeCount;i++){
		mob=nodes[i]->GetObject<MobilityModel>();
		Vector pos=mob->GetPosition();
		if(nodes[i]->CSsize>1){
			cacheNodes[j][0]=pos.x;
			cacheNodes[j][1]=pos.y;
			j++;
		}
	}*/
}

void simulate(string traceName, string resultsName, double cache){

	Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("10Mbps"));
		Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("15ms"));
		Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("10000"));

		//string traceName="traces/3d/sample_trace.txt";
		int k=2;
		int NodeCount=200;
		int objectCount=1000000;
		int xy_min=0;
		int xy_max=100;
		int time_factor=10;
		double range=12;


		createTopology(NodeCount, range,xy_min,xy_max, time_factor);
		assignCache(NodeCount,k, cache);
		/*std::ifstream infile("cacheInput.txt");
			std::string line;*/

		//CreateTree(NodeCount,k);
		//mapObjectsToNodes(objectCount,NodeCount);

		//ReqsPreProcess(objectCount,NodeCount, time_factor,xy_min,xy_max,k, traceName);



			//CommandLine cmd;



/*			ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
			  ndnGlobalRoutingHelper.InstallAll ();
			 for(int i=0;i<NodeCount;i++){
				 std::stringstream out;
				 out << (int)i;
				  std::string outtstr=out.str();
				  if(outtstr.length()==1) outtstr="00"+outtstr;
				  else if  (outtstr.length()==2) outtstr="0"+outtstr;
				 std::string s=std::string("/pre")+outtstr;
				 ndnGlobalRoutingHelper.AddOrigins (s.c_str(), nodes[i]);
			 }
			 ndn::GlobalRoutingHelper::CalculateRoutes();*/


		  Simulator::Stop (Seconds (time_factor));
		  //L2RateTracer::InstallAll ("drop-trace.txt", Seconds (0.5));
		  std::stringstream out;


		  std::string ss = out.str();

		  ndn::AppDelayTracer::InstallAll(resultsName);

		  //ndn::CsTracer::InstallAll ("cs-trace.txt", Seconds (1));

		  //ndn::AppDelayTracer::InstallAll (std::string("results/app-delays-trace")+ss.c_str()+std::string(".txt"));



		   Simulator::Run ();
		  Simulator::Destroy ();




}

int
main (int argc, char *argv[])
{


	int runNo=1;
	int clsNo=9;
	int cache=100;
    CommandLine cmd;
    cmd.AddValue("runNo", "Run Number", runNo);
    cmd.AddValue("clsNo", "Cluster No", clsNo);
    cmd.AddValue("cache", "Cache Size", cache);
    cmd.Parse (argc, argv);



		std::stringstream cls;
		std::stringstream run;
		cls << clsNo;
		run << runNo;
		string traceName="traces/3d/trace_0."+cls.str()+std::string("_1.dat");
		//cout<<traceName<<"\n";
		//string traceName="traces/3d/trace_0."+cls.str()+"_"+run.str()+std::string(".dat");
		//string resultsName="results/wireless/onPath/"+run.str()+"/onPath_res_0."+cls.str()+"_"+run.str()+std::string(".txt");
		string resultsName="results/resEdge2.txt";

		//traceName="1000Zipf.txt";
		std::cout<<"\n"<<cache<<" "<<clsNo<<"\n";
		simulate(traceName,resultsName,cache);
		//std::cout<<runNo;
		std::stringstream out;



		string fileName="results/Wireless_Edge.txt";
		ofstream ofs;
		ofs.open(fileName.c_str(), ios::out | ios::app );

		double avgHopCount=ndn::AppDelayTracer::GetAvrageHopCount();
		cout<<"AggCountTot: "<<ndn::AppDelayTracer::GetIncreaseAggCount(true)<<"\n";
		cout<<"AggCountNonEdge: "<<ndn::AppDelayTracer::GetIncreaseAggCount(false)<<"\n";
		cout<<"HopCountTot: "<<ndn::AppDelayTracer::GetIncreaseHopCount(0,true)<<"\n";
		cout<<"HopCountDecreased: "<<ndn::AppDelayTracer::GetIncreaseHopCount(0,false)<<"\n";
		cout<<"Average Agg hop Count: "<<ndn::AppDelayTracer::GetAvrageAggHop()<<"\n";
		cout<<"Average hop Count: "<<ndn::AppDelayTracer::GetAvrageHopCount()<<"\n";

		//ofs<<cache<<" "<<clsNo<<" "<<avgHopCount<<"\n";

		string frwCnt="results/FrwCnt"+cls.str()+"zipf.txt";
		string frwPer="results/FrwPer"+cls.str()+"zipf.txt";
		ofstream ofsCnt;
		ofstream ofsPer;
		ofsCnt.open(frwCnt.c_str(), ios::out | ios::app );
		ofsPer.open(frwPer.c_str(), ios::out | ios::app );
		for(int i=0;i<200;i++){
			double tot=0;
			int max=0;
			for (int j=0;j<200;j++){
				 std::stringstream out;
				 out << (int)j;
				 std::string outtstr=out.str();
				 if(outtstr.length()==1) outtstr="00"+outtstr;
				 else if  (outtstr.length()==2) outtstr="0"+outtstr;
				 std::string s=std::string("/pre")+outtstr;
				int per=ndn::AppDelayTracer::GetIncreasePrefixRequest(i,s);
				max=per>max?per:max;
				tot+=per;
				ofsCnt<<per<<" ";
			}

			ofsCnt<<"\n"<<tot<<"\n";
			ofsPer<<max<<" "<<tot<<" "<<(double)max/(double)tot<<"\n";
		}

	return 0;
}




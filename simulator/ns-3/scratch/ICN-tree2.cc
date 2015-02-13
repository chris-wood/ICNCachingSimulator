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
#include "ns3/ndnSIM/ndn.cxx/detail/error.h"

using namespace std;
using namespace ns3;


NodeContainer nodes;
double cacheNodes [100][2];

int objNodeMapper[10000];

void mapObjectsToNodes(int ObjectCount,int NodeCount){
	for(int i=0;i<ObjectCount;i++){
		objNodeMapper[i]=0;
	}
}

void ReqsPreProcess(int ObjectCount,int NodeCount,int levels, double time_factor, int xy_min, int xy_max, int k, string traceName){

	//int depth=((int)(log10(NodeCount)/log10(k)));
    int firstLeafIndex=(k*(pow(k,levels-2)-1)/(k-1))+1;
	double *lastReq=new double[NodeCount]();
	std::ifstream infile(traceName.c_str());
	//std::ofstream *outFiles = new ofstream [NodeCount];
	stringstream ss;
	//std::ofstream outfile("modReqs.txt");
	for(int i=firstLeafIndex;i<NodeCount;i++){
		std::stringstream out;
	    out << i;
	    string fileName="temp/"+out.str()+std::string(".txt");
		ofstream ofs(fileName.c_str());
	}

	std::string line;
	std::getline(infile, line);
	int count=0;
	while (std::getline(infile, line))
	{
		std::replace( line.begin(), line.end(), ',',' ');
		std::istringstream iss(line);
		double time,x;
		int obj;

		Ptr<MobilityModel> mob = nodes[firstLeafIndex]->GetObject<MobilityModel>();
		Vector pos = mob->GetPosition ();
		//y=pos.y;


		if (!(iss >> time >> x >> obj)) { break; }
		time=time*time_factor;
		x=x*xy_max;
		double minDistance=sqrt(pow(x-pos.x,2.0));
				int minNode=firstLeafIndex;
		//y=y*y_factor;
		for(int i=firstLeafIndex; i<NodeCount;i++){
			mob = nodes[i]->GetObject<MobilityModel>();
			pos = mob->GetPosition ();
			double dis=sqrt(pow(x-pos.x,2.0));
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
	cout<<count;
	for(int i=firstLeafIndex;i<NodeCount;i++){
		std::stringstream out;
		out << i;
		string fileName="temp/"+out.str()+std::string(".txt");
		ofstream ofs;
		ofs.open(fileName.c_str(), ios::out | ios::app );
		ofs<<time_factor<<" "<<"0"<<" "<<"10000"<<"\n";
		ofs.close();
	}

}

void CreateTree(int NodeCount, int k, int levels){
	nodes.Create (NodeCount);
	MobilityHelper mobility;

	Ptr<ListPositionAllocator> initialAlloc =
    CreateObject<ListPositionAllocator> ();
	int nodeNum=0;
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		mobility.Install (nodes);
	for(int rowNum=0;rowNum<levels;rowNum++){
		int temp=pow(k,rowNum);
		for(int colNum=0;colNum<temp;colNum++){

			double x_interval=100.0/(pow(k,rowNum));
					double y_interval=100.0/(levels-1);
					double x_pos=((2*colNum)+1)*(x_interval/2.0);
					double y_pos=rowNum*y_interval;
					//Vector3D locVec(x_pos, y_pos, 0);

				      Ptr<Node> n0 = nodes[nodeNum];
				      Ptr<ConstantPositionMobilityModel> nLoc =  n0->GetObject<ConstantPositionMobilityModel> ();
				      if (nLoc == 0)
				        {
				          nLoc = CreateObject<ConstantPositionMobilityModel> ();
				          n0->AggregateObject (nLoc);
				        }

				      Vector nVec (x_pos, y_pos, 0);
				      nLoc->SetPosition (nVec);

					nodeNum++;

		}
	}


	PointToPointHelper p2p;
	for(int i=NodeCount-1;i>0;i--){
		p2p.Install (nodes.Get (i), nodes.Get ((i-1)/k));
	}


	Ptr<Node> producer = nodes[0];
	ndn::AppHelper producerHelper ("ns3::ndn::Producer");
	producerHelper.SetPrefix ("/");
	producerHelper.SetAttribute ("PayloadSize", StringValue("1"));
	producerHelper.Install (producer);



	// Calculate and install FIBs
	//ccnxGlobalRoutingHelper.CalculateRoutes ();


	NodeContainer consumerNodes;
	int firstLeafIndex=(k*(pow(k,levels-2)-1)/(k-1))+1;
	//int depth=((int)(log10(NodeCount)/log10(k)));
	//int s=pow(k,depth)-1;


	 //ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot");
	ndn::AppHelper consumerHelper ("ns3::ndn::CustomZipf");
	 consumerHelper.SetPrefix ("/pre");
	  consumerHelper.SetAttribute ("Frequency", StringValue ("1")); // 100 interests a second
	consumerHelper.SetAttribute ("NumberOfContents", StringValue ("100"));
	for(int i=NodeCount-1;i>=firstLeafIndex;i--){
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

void createTopology(int NodeCount, double range, int xy_min, int xy_max){

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
									   "Z", PointerValue (randomizer));

		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		mobility.Install (nodes);



		//ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerZipfMandelbrot");
		ndn::AppHelper consumerHelper ("ns3::ndn::CustomZipf");
		 for(int i=0;i<NodeCount;i++){
			 ndn::AppHelper producerHelper ("ns3::ndn::Producer");
			 producerHelper.SetAttribute ("PayloadSize", StringValue("1"));
			 std::stringstream out;
			 out << (int)i;
			 std::string s=std::string("/pre")+out.str();
			 //std::string s=std::string("/prefix");
			 //cout<<"producer :"<<s.c_str()<<"\n";
			 producerHelper.SetPrefix(s.c_str());
			 producerHelper.Install (nodes[i]);

			 //consumerHelper=new ndn::AppHelper ("ns3::ndn::CustomZipf");
			 consumerHelper.SetPrefix ("/prefix");
			 consumerHelper.SetAttribute ("NumberOfContents", StringValue ("100")); // 10 different contents
			 consumerHelper.SetAttribute("ConsumerNode", StringValue(out.str()));
			 consumerHelper.Install(nodes[i]);
		 }

	  //make p2p links
		 PointToPointHelper p2p;

		 for(int i=0;i<NodeCount;i++){
			 for(int j=i+1;j<NodeCount;j++){
				 Ptr<MobilityModel> mob = nodes[i]->GetObject<MobilityModel>();
				 Vector pos_i = mob->GetPosition ();
				 mob = nodes[j]->GetObject<MobilityModel>();
				 Vector pos_j = mob->GetPosition ();
				 double distance=sqrt(pow(pos_i.x-pos_j.x,2.0)+pow(pos_i.y-pos_j.y,2.0));
				 if(distance<=range){
					 p2p.Install (nodes.Get (i), nodes.Get (j));
				 }
			 }
		 }



}

void assignCache(int NodeCount, int k, double* cache, int levels){
	// Install CCNx stack on all nodes
	  ndn::StackHelper ccnxHelper;
	  ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
	  ccnxHelper.SetContentStore ("ns3::ndn::cs::Lru", "MaxSize", "0");
	  ccnxHelper.InstallAll ();


		ndn::GlobalRoutingHelper ccnxGlobalRoutingHelper;
		ccnxGlobalRoutingHelper.InstallAll ();
		ccnxGlobalRoutingHelper.AddOrigins ("/", nodes[0]);
		ccnxGlobalRoutingHelper.CalculateRoutes ();

	  //int depth=((int)(log10(NodeCount)/log10(k)));
	  //int firstLeafIndex=pow(k,depth)-1;
	  int nodeNum=1;
	  for(int i=1;i<levels;i++){
		  int budjet=(int)(cache[i]/pow(k,i));
		  int remain=cache[i]-budjet*pow(k,i);
		  for(int j=0;j<pow(k,i);j++){


			  std::stringstream out;
			  			  out << nodeNum;
			  			  std::string ss = out.str();
			  			  std::string conf=std::string("/NodeList/")+ ss.c_str()+std::string("/$ns3::ndn::ContentStore/MaxSize");
			  			  if(remain>0){
			  				  Config::Set (conf, UintegerValue (budjet+1)); // number after nodeList is global ID of the node (= node->GetId ())
			  				  remain--;
			  			  }
			  			  else{
			  				Config::Set (conf, UintegerValue (budjet)); // number after nodeList is global ID of the node (= node->GetId ())
			  			  }
			  			  nodeNum++;
		  }
	  }
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

void simulate(string line, string traceName, int levels, int k){

	Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("1000000Mbps"));
		Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("200ms"));
		Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("1000000"));
		//int levels=5;
		//string traceName="traces/trace_0.3-1.dat";
		//int k=4;
		int NodeCount=(pow(k,levels)-1)/(k-1);
		int objectCount=100;
		int xy_min=0;
		int xy_max=100;
		double time_factor=95350/32;
		double total_Cache=0;
		//createTopology(NodeCount, 100,xy_min,xy_max,k);
		//std::ifstream infile("cacheInput.txt", ios::out | ios::app ););
			//std::string line;

		CreateTree(NodeCount,k,levels);
		mapObjectsToNodes(objectCount,NodeCount);

		ReqsPreProcess(objectCount,NodeCount,levels, time_factor,xy_min,xy_max,k, traceName);
	int a=0;
		//while (std::getline(infile, line)&&a==0)
		//{
			a++;
			//line="16384 0 0.4015 0 0 0.5985";
			//line="100	0.32   0.2    0.1    0.1";
			double *cache;
			int sim_run=0;

			std::replace(line.begin(),line.end(),',',' ');
			cout<<line<<"\n";
			istringstream iss(line);
			vector<string> tokens;
			copy(istream_iterator<string>(iss),
			         istream_iterator<string>(),
			         back_inserter<vector<string> >(tokens));
			string str=tokens[0];
			total_Cache=atof(str.c_str());
			//int size=tokens.size();
			cache=new double[10]();
			for(int i=1;i<levels;i++){
				str=tokens[i];
				cache[levels-i]=atof(str.c_str())*total_Cache;
			}


			assignCache(NodeCount,k, cache,levels);

			//CommandLine cmd;
			//cmd.Parse (argc, argv);

		  // Calculate and install FIBs
			//ndn::GlobalRoutingHelper ccnxGlobalRoutingHelper;
			//ndn::GlobalRoutingHelper ccnxGlobalRoutingHelper;
			//ccnxGlobalRoutingHelper.InstallAll ();
			//ccnxGlobalRoutingHelper.AddOrigins ("/", nodes[0]);


		//  ccnxGlobalRoutingHelper.CalculateRoutes ();

		  Simulator::Stop (Seconds (time_factor));
		  //L2RateTracer::InstallAll ("drop-trace.txt", Seconds (0.5));
		  std::stringstream out;
		  out << sim_run;

		  std::string ss = out.str();

		  //ndn::AppDelayTracer::InstallAll (std::string("results/app-delays-trace")+ss.c_str()+std::string(".txt"));
		  ndn::AppDelayTracer::InstallAll (std::string("results/app-delays-trace-tree.txt"));


		   Simulator::Run ();
		   Simulator::Destroy ();

		//}
}

int
main (int argc, char *argv[])
{
	//int runNo=1;
	//int clsNo=0;

    int k=2;
    int objectCount=100;

   /* int levels=3;
	string input="0.0,2,0,1";*/

    int levels=6;
	string input="0.0,30,0,0.54,0.27,0.14,0.07";

    CommandLine cmd;
    cmd.AddValue("input", "input", input);

    cmd.Parse (argc, argv);
    cout<<input<<"\n";
	string str=input.substr(0,3);
	input.erase(0,4);

	std::stringstream cls;
	std::stringstream run;
	cls << str;
	//run<<runNo;
	//string traceName="traces/trace_maz.dat";
	string traceName="traces/2d/100obj/2d_obj100_alpha1_irm.dat";
	//string traceName="traces/2d/100obj/trace_"+cls.str()+std::string(".dat");
	//string traceName="traces/2d/trace_"+cls.str()+std::string("_1.dat");
	string resultsName="results/resTree.txt";

	//traceName="1000Zipf.txt";
	//cout<<traceName<<""<<resultsName<<"\n";
	simulate(input,traceName, levels, k);
	//std::cout<<runNo;
	std::stringstream out;

	int n_num=0;
	int sum=0;
	int AggPLevelPerObject[10][10000]={0};
	int ReqPLevelPerObject[10][10000]={0};
	for(int rowNum=0;rowNum<levels;rowNum++){
			int temp=pow(k,rowNum);
			for(int colNum=0;colNum<temp;colNum++){
				for(int obj=0;obj<=objectCount;obj++){
					int tmp=ndn::AppDelayTracer::GetIncreaseAggCountPerObj(obj,n_num);
					sum+=tmp;
					AggPLevelPerObject[rowNum][obj]+=tmp;
					ReqPLevelPerObject[rowNum][obj]+=ndn::AppDelayTracer::GetIncreaseObjRequest(obj,n_num);

				}
				n_num++;
			}
	}


	//ofstream ofs2;
	ofstream ofs2("results/aggStats.txt");
	//ofs2.open("results/aggStats.txt", ios::out | ios::app );
	for(int i=0;i<=objectCount;i++){
		ofs2<<i<<" ";

		for(int j=0;j<levels;j++){
			ofs2<<(double)AggPLevelPerObject[j][i]/(double)ReqPLevelPerObject[j][i]<<" ";
		}
		for(int j=0;j<levels;j++){
					ofs2<<(double)AggPLevelPerObject[j][i]<<" "<<(double)ReqPLevelPerObject[j][i]<<" ";
		}
		ofs2<<"\n";
	}
	int reqSum=0;
	for(int i=0;i<=objectCount;i++){
		reqSum+=ndn::AppDelayTracer::GetIncreaseObjRequest(i);
	}

	for(int i=0;i<=objectCount;i++){
		ofs2<<100*(double)(ndn::AppDelayTracer::GetIncreaseObjRequest(i)-1)/(double)reqSum<<"\n";
	}

	std::cout<<"sum: "<<sum<<"\n";
	cout<<"AggCountTot: "<<ndn::AppDelayTracer::GetIncreaseAggCount(true)<<"\n";
	cout<<"AggCountNonEdge: "<<ndn::AppDelayTracer::GetIncreaseAggCount(false)<<"\n";

	cout<<"HopCountTot: "<<ndn::AppDelayTracer::GetIncreaseHopCount(0,true)<<"\n";
	cout<<"HopCountDecreased: "<<ndn::AppDelayTracer::GetIncreaseHopCount(0,false)<<"\n";

	cout<<"Average Agg hop Count: "<<ndn::AppDelayTracer::GetAvrageAggHop()<<"\n";
	cout<<"Average hop Count: "<<ndn::AppDelayTracer::GetAvrageHopCount()<<"\n";

	string fileName="results/tree.txt";
	ofstream ofs;
	ofs.open(fileName.c_str(), ios::out | ios::app );
	double avgHopCount=ndn::AppDelayTracer::GetAvrageHopCount();
	ofs<<str<<" "<< input <<" "<<avgHopCount<<"\n";


  return 0;
}





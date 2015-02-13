// basic file operations
#include <iostream>
#include <string>
#include <vector>	
#include <utility>	//pair
#include <fstream>
#include <cstdlib>     /* srand, rand */
#include <ctime>       /* time */
using namespace std;

int MAXX = 1000;
int MAXY = 1000;

int main () {

	srand (time(NULL));
  
	int nonodes = 10;
	int noedges = 3;
	vector<int> nodes;
	vector<int> X;
	vector<int> Y;
	vector<pair<int,int> > edges;
	
	//NODES
	for(int i=0;i<nonodes;i++)
	{
		nodes.push_back(i);
	}
	
	//COORDINATES
	for(int i=0;i<nonodes;i++)
	{
		X.push_back(rand() % MAXX);
		Y.push_back(rand() % MAXY);
	}
	
	//EDGES
	edges.push_back(make_pair(3,4));
	edges.push_back(make_pair(1,9));
	edges.push_back(make_pair(2,5));
  
	ifstream minInt;
	ifstream dirSer;
	ofstream myfile;
	string line;
	
	//PRINT NODES
	myfile.open ("nodes.txt");
	for(int i=0;i<nonodes;i++)
	{
		myfile << nodes[i] << " " << X[i] << " " << Y[i] << endl;
	}
	myfile.close();
	
	//PRINT EDGES
	myfile.open ("edges.txt");
	for(int i=0;i<noedges;i++)
	{
		myfile << edges[i].first << " " << edges[i].second << endl;
	}
	myfile.close();
	
	//RUN PYTHON, WAIT FOR IT
	system ("python minimalIntervalTablesC2py.py main");
	
	//READ MINIMAL INTERVAL TABLE
	minInt.open ("minIntervals.txt");
	if (minInt.is_open())
	{
		while ( getline (minInt,line) )
		{
			cout << line << endl;
		}
		minInt.close();
		cout << endl;
	}
	else cout << "Unable to open file"; 
	
	//READ DIRECTORY SERVICES
	dirSer.open ("dirServices.txt");
	if (dirSer.is_open())
	{
		while ( getline (dirSer,line) )
		{
			cout << line << endl;
		}
		dirSer.close();
	}
	else cout << "Unable to open file"; 
	
	
	return 0;
}
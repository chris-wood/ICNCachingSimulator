#python -m cProfile -s 'cumtime' minimalIntervalTables.py > profile.txt

################ Libraries ###############################################################
import re 
import sys
import getopt
import math
import subprocess
import copy
import os
import os.path
import getopt
import fnmatch
import shutil
import random
random.seed(1)

from itertools import groupby, count

#import matplotlib.pyplot as plt
import networkx as nx
from networkx.algorithms.flow import shortest_augmenting_path

#NEED TO COMPUTE ORIGIN NODE! HOW? FOR NOW 500,500

XMAX = 1000
YMAX = XMAX
NOROOTS 	= 5

def sublist(list, sublist):
 seta = set(list)
 setb = set(sublist)
 if seta.issubset(setb):
  return 0
 elif setb.issubset(seta):
  return 1
 else:
  return -1

# def labels2Nodes(labels, G):
 # nodeList = []
 # for it in labels:
  # nodeList.append(list(n for n in G if G.node[n]['label']==it)[0])
 # return nodeList
 
def labels2Nodes(labels, bfsTreesLabelDict):
 nodeList = []
 for it in labels:
  nodeList.append(bfsTreesLabelDict[it])
 return nodeList

def consecutive(n1, n2):
 return (abs(n1-n2) == 1)
 
def interval2Labels(n1, n2):
 frange = range(n1,n2+1)
 if not frange:
  frange = range(n2,n1+1)
 return frange
 
def intervalTables(T,node):
 #print T.nodes(data=True)
 labels = {}
 intervals = {}
 nodeTable = nodesAtPorts(T,node)
 
 #Nodes-->Labels
 #labels = copy.deepcopy(nodeTable)
 ports = nodeTable.keys()
 #print ports
 for port in ports:
  labels[port] = []
  for i,ls in enumerate(nodeTable[port]):
   temp = []
   labels[port].append(temp)
   for j,lsit in enumerate(nodeTable[port][i]):
	#print T.node[lsit]['label']
	labels[port][i].append(T.node[lsit]['label'])
   labels[port][i].sort()
 #print labels
 
 #Parent node for reverse intervals
 parentport = T.predecessors(node)
 
 
 #Labels-->Intervals
 for port in ports:
  intervals[port] = []
  for lsi,ls in enumerate(labels[port]):
   for k, g in groupby(ls, lambda n, c=count(): n-next(c)):
    temp = list(g)
    #print temp
    if len(temp) > 1:
     if ((len(parentport) > 0) and (port==parentport[0]) and (lsi==len(labels[port])-1)):
      inter = [temp[-1], temp[0]]
     else:
      inter = [temp[0], temp[-1]]
    else:
     inter = temp
    intervals[port].append(inter)
    #if len(intervals[port]) > 2:
    # print intervals[port]	
 return intervals

def nodesAtPorts(T,node):
 nodeTable = {}
 #Children
 children = T.neighbors(node);
 #print children
 for c in children:
  nodeTable[c] = [list(nx.descendants(T,c)) + [c]]
 #Parent
 ancestors = []
 curNodeList = []
 current = node
 parent = T.predecessors(current)
 parentport = T.predecessors(current)
 if len(parentport) > 0:
  nodeTable[parentport[0]] = []
 while len(parent) > 0: #Not root
  #print parent
  curNodeList.append(parent[0])
  parentChildren = [item for item in T.neighbors(parent[0]) if item not in [current]]
  #If parent has consecutive labels with any of parentChildren, then group them in the same label list
  #If not, add it to curNodeList, until one of the parents is not consecutive. Then move that list to nodeTable, and empty curNodeList.
  #It seems two lines above are not necessary, fall back to simpler scheme!  
  for ps in parentChildren:
   descendants = list(nx.descendants(T,ps)) + [ps]
   #print descendants
   nodeTable[parentport[0]].append(descendants)
  current = parent[0]
  parent = T.predecessors(current)
 if len(parentport) > 0:
  nodeTable[parentport[0]].append(curNodeList)
 #print nodeTable
 return nodeTable
  
def nodeDistance(n1,n2):
 return math.sqrt((n1['x'] - n2['x'])*(n1['x'] - n2['x']) + (n1['y'] - n2['y'])*(n1['y'] - n2['y']));
 
def returnQuadrant(x,y):
 if (x<0 and y<0):
  return 3
 elif (x<0 and y>-1):
  return 2
 elif (x>-1 and y>-1):
  return 1
 else:
  return 4	

################ Main **************** ####################################################
def main():
 G = nx.Graph()
  
 #READ nodes with XY coordinates
 with open("nodes.txt") as f:
  for line in f:
   tmp = map(int, line.split(' '))
   G.add_node(tmp[0])
   G.node[tmp[0]]['x'] = tmp[1]
   G.node[tmp[0]]['y'] = tmp[2]
   #G.node[n]['gid'] = n
   #G.node[n]['label'] = ""
 #print G.nodes(data=True)
 
 #READ edges
 with open("edges.txt") as f:
  for line in f:
   tmp = map(int, line.split(' '))
   G.add_edge(tmp[0],tmp[1])
 #print G.edges()
 
 #degree =  G.degree().values()
 #Print AVG DEGREE
 #print reduce(lambda x, y: x + y, degree) / float(len(degree))
 
 #Choose 4+1 corners
 Corners = [0] * NOROOTS
 CornerInd = [0] * NOROOTS
 Corners[4] = math.sqrt(math.pow(XMAX,2) + math.pow(YMAX,2))
 
 for n in range(0,G.number_of_nodes()):
  signX = G.node[n]['x'] - (XMAX / 2)
  signY = G.node[n]['y'] - (YMAX / 2)
  distance = math.sqrt(math.pow(signX,2) + math.pow(signY,2))
  quadrant = returnQuadrant(signX,signY)
  #FOUR QUADRANT NODES
  if (distance > Corners[quadrant-1]):
   Corners[quadrant-1] = distance
   CornerInd[quadrant-1] = n
  #CENTER NODE
  if (distance < Corners[NOROOTS-1]):
   Corners[NOROOTS-1] = distance
   CornerInd[NOROOTS-1] = n
 #print Corners
 #print CornerInd
 
 #BFS truncate 5 trees (directed graphs)
 bfsTrees = []
 bfsTreesLabelDict = {}
 for c in CornerInd:
  bfsTrees.append(nx.bfs_tree(G,c))
 #print((bfsTrees[0]).edges())
 
 #Instantiate reverse label-gid lookup
 for c in range(NOROOTS):
  bfsTreesLabelDict[c] = {}
 #print bfsTreesLabelDict
 
 #DFS label 5 trees (directed graphs)
 for i in range(0,len(CornerInd)):
  dfsList = list(nx.dfs_preorder_nodes(bfsTrees[i],CornerInd[i]))
  #print dfsList
  for n in bfsTrees[i]:
   bfsTrees[i].node[n]['label'] = dfsList.index(n)
   bfsTreesLabelDict[i][dfsList.index(n)] = n
  #print ""
  #print bfsTrees[i].nodes(data=True)
  #print bfsTrees[i].edges()
 #print bfsTreesLabelDict
 
 #All descendants
 #print list(nx.descendants(bfsTrees[0],2))
 #All levels
 #print(nx.bfs_successors(G,CornerInd[0]))
 #Use this to print siblings
 #print nx.bfs_successors(G,CornerInd[0])[bfsTrees[0].predecessors(2)[0]]
 #Parent
 #print bfsTrees[0].predecessors(2)
 #Children
 #print bfsTrees[0].neighbors(2)
 #Ancestors
 #print nx.ancestors(bfsTrees[0],2)
 
 #Create intervals per PORT per NODE per BFS/DFS TREE
 intervalTable = {}
 for root in range(0,len(CornerInd)):
  intervalTable[root] = {}
  #print "Root %s:"%(root)
  for n in bfsTrees[root]:
   intervalTable[root][n] = intervalTables(bfsTrees[root],n)
 #print ""
 #print intervalTable
 #print ""

 #Minimize interval information
 minIntervalTable = copy.deepcopy(intervalTable) #For minimization
 #tmpIntervalTable = copy.deepcopy(intervalTable) #For iteration
 for root in range(0,len(CornerInd)-1):
  nodes = minIntervalTable[root].keys()
  for node in nodes:
   ports = minIntervalTable[root][node].keys()
   for port in ports:
	#for i,intIt in enumerate(list(minIntervalTable[root][node][port])):
    for intIt in list(minIntervalTable[root][node][port]):
	 labelList = interval2Labels(intIt[0],intIt[-1])
	 nodeList = labels2Nodes(labelList,bfsTreesLabelDict[root])
	 nodeList.sort()
	 #print labelList
	 #print "%s %s %s %s"%(root, node, port, nodeList)
	 for rootcmp in range(root+1,len(CornerInd)):
	  if (node in list(minIntervalTable[rootcmp])) and (port in list(minIntervalTable[rootcmp][node])):
	   #for j,intItcmp in enumerate(list(minIntervalTable[rootcmp][node][port])):
	   for intItcmp in list(minIntervalTable[rootcmp][node][port]):
		labelListcmp = interval2Labels(intItcmp[0],intItcmp[-1])
		nodeListcmp = labels2Nodes(labelListcmp,bfsTreesLabelDict[rootcmp])
		nodeListcmp.sort()
		#print labelListcmp
		#print "%s %s %s %s"%(rootcmp, node, port, nodeListcmp)
		issublist = sublist(nodeList,nodeListcmp)
		if issublist == 1:
		 #print "1"
		 minIntervalTable[rootcmp][node][port].remove(intItcmp)
		elif issublist == 0:
		 #print "0"
		 try:
		  minIntervalTable[root][node][port].remove(intIt)
		 except:
		  pass
 #print ""
 #print minIntervalTable
 #print ""
 
 #Count number of intervals
 count = 0
 for root in range(0,len(CornerInd)):
  nodes = intervalTable[root].keys()
  for node in nodes:
   ports = intervalTable[root][node].keys()
   for port in ports:
    for i,intIt in enumerate(intervalTable[root][node][port]):
	 #print intIt
	 count=count+1
 #print count
 
 #Count number of intervals (minimized)
 mincount = 0
 for root in range(0,len(CornerInd)):
  nodes = minIntervalTable[root].keys()
  for node in nodes:
   ports = minIntervalTable[root][node].keys()
   for port in ports:
    for i,intIt in enumerate(minIntervalTable[root][node][port]):
	 #print intIt
	 mincount=mincount+1
 #print mincount
 
 #Print minIntervalTable for C++ Code
 intFile = open("minIntervals.txt", "w")
 for root in range(0,len(CornerInd)):
  nodes = minIntervalTable[root].keys()
  for node in nodes:
   ports = minIntervalTable[root][node].keys()
   for port in ports:
    for i,intIt in enumerate(minIntervalTable[root][node][port]):
	 intFile.write("%s %s %s %s\n"%(root, node, port, intIt))
	 
 #Print dirServices for C++ Code
 dirFile = open("dirServices.txt", "w")
 for i in range(0,len(CornerInd)):
   for n in bfsTrees[i]:
    dirFile.write("%s %s %s\n"%(n, i, bfsTrees[i].node[n]['label']))


main() 

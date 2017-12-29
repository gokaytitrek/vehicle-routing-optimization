//
//  main.cpp
//  
//
//  Created by Gokay on 29.12.2017.
//

#include <iostream>
#include <vector>
#include <cstdlib>
#include <math.h>

#define NO_OF_CUSTOMER 5
#define NO_OF_VEHICLES 30
#define VEHICLE_Cap 90

using namespace std;

class Node {
   public:
      	int nodeId, nodeX, nodeY, demand;
      	bool isRouted, isDepot;
      	
      	Node(int coordinateX, int coordinateY);
		Node(int id);
};

Node::Node(int coordinateX, int coordinateY)
{
	this->nodeId = 0;
	this->nodeX = coordinateX;
	this->nodeY = coordinateY;	
	this->isDepot = true;
}

Node::Node(int id)
{
	this->nodeId = id;
	this->nodeX = (rand() % 100 );//Generate random number betweend 0-99 for coordinate x
	this->nodeY = (rand() % 100 );//Generate random number betweend 0-99 for coordinate y
	this->demand = (rand() % 9 ) + 1;//Generate random demnand betweend 1-9
	this->isRouted = false;
	this->isDepot = false;
}

class Init {
	public:
		vector<Node> nodes;
		double distanceMatrix[NO_OF_CUSTOMER+1][NO_OF_CUSTOMER+1] = {{0}};
		int noOfCustomers;
	    int noOfVehicles;
	    int vehicleCap;
	    
	public:
		Init(int noOfCustomers, int noOfVehicles, int vehicleCap)
		{
			this->noOfCustomers = noOfCustomers;
			this->noOfVehicles = noOfVehicles;
			this->vehicleCap = vehicleCap;
					
			nodes.reserve(this->noOfCustomers);
		}

	public:
		generateNodes(){
			//Depot Coordinates
  	  		int depotX = 50;
   		 	int depotY = 50;
    
			//Create depot node
			nodes.push_back(Node(depotX,depotY));  
			
			//Initialize nodes
			//Node[0] is allocated for depot node. Start from 1
			for(int i = 1; i <= this->noOfCustomers; i++) {
				nodes.push_back(Node(i));
			}
		}
		
		printNodes()
		{
			cout << "Nodes" << endl;
			
			//Print Nodes
			for(int i = 0; i <= this->noOfCustomers; i++) {
				cout << "Node ID:"<<  nodes[i].nodeId  << "\tX:"<<  nodes[i].nodeX << "\tY:" << nodes[i].nodeY << "\tDemand:" << nodes[i].demand <<endl;
			}
			cout << endl << endl;
		}
		
		calculateDistanceMatrix(){
			double deltaX,deltaY;
			for(int i=0;i<=NO_OF_CUSTOMER;i++){
				for(int j=i+1;j<=NO_OF_CUSTOMER;j++){
					deltaX = this->nodes[i].nodeX - this->nodes[j].nodeX;
					deltaY = this->nodes[i].nodeY - this->nodes[j].nodeY;
					
					double distance = round(sqrt((deltaX * deltaX) + (deltaY * deltaY)));
					
					this->distanceMatrix[i][j] = distance;
					this->distanceMatrix[j][i] = distance;
				}
			}
		}
		
		printDistanceMatrix(){
			cout << "Distance Matrix" << endl;
			for (int i = 0; i <= NO_OF_CUSTOMER; i++) {
                for (int j = 0; j <= NO_OF_CUSTOMER; j++) {
                    cout << this->distanceMatrix[i][j] << "\t";
                }
                cout << endl;
            }
            cout << endl << endl;
		}
};


int main() 
{
    cout << "Vehicle routing optimization" <<endl;
    
	Init init(NO_OF_CUSTOMER,NO_OF_VEHICLES,VEHICLE_Cap);
	init.generateNodes();
   	init.printNodes();
	init.calculateDistanceMatrix();
	init.printDistanceMatrix();
	

    return 0;
}




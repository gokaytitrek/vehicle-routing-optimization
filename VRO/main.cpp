// VRO.cpp : Defines the entry point for the console application.
//
//  main.cpp
//  
//
//  Created by Gokay on 29.12.2017.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <math.h>
#include <float.h>

#define NO_OF_CUSTOMER 50
#define NO_OF_VEHICLES 10
#define VEHICLE_CAPACITY 80

using namespace std;

class Node {
public:
	int nodeId, nodeX, nodeY, demand;
	bool isRouted, isDepot,isValid;

	//Depot node
	Node(int coordinateX, int coordinateY);
	Node(int id);
	Node();
};

Node::Node(int coordinateX, int coordinateY)
{
	this->nodeId = 0;
	this->nodeX = coordinateX;
	this->nodeY = coordinateY;
	this->demand = 0;
	this->isDepot = true;
	this->isValid = true;
}

Node::Node(int id)
{
	this->nodeId = id;
	this->nodeX = (rand() % 100);//Generate random number betweend 0-99 for coordinate x
	this->nodeY = (rand() % 100);//Generate random number betweend 0-99 for coordinate y
	this->demand = (rand() % 9) + 1;//Generate random demnand betweend 1-9
	this->isRouted = false;
	this->isDepot = false;
	this->isValid = true;
}

Node::Node(){
	this->isValid = false;
}

class Vehicle{
public:
	int vehicleId, load, currentLocation;
	int capacity = VEHICLE_CAPACITY;
	vector<Node> nodes;
	bool closed;

	Vehicle(int id);
	Vehicle();

public:
	void addNode(Node customer){
		this->nodes.emplace_back(customer);
		this->load += customer.demand;
		this->currentLocation = customer.nodeId;
	}

	bool checkCapacity(int demand){
		if (this->load + demand <= this->capacity){
			return true;
		}
		else{
			return false;
		}
	}
};

Vehicle::Vehicle(int id)
{
	this->vehicleId = id;
	this->capacity = VEHICLE_CAPACITY;
	this->load = 0;
	this->currentLocation = 0;
	this->closed = false;
	this->nodes.clear();
}

//Default Constructor for Vehicle
Vehicle::Vehicle()
{

}

class Init {
public:
	vector<Node> nodes;
	vector<Vehicle> vehicles;
	double distanceMatrix[NO_OF_CUSTOMER + 1][NO_OF_CUSTOMER + 1];// = { { 0 } };
	int noOfCustomers;
	int noOfVehicles;
	int vehicleCap;
	double cost = 0;

public:
	Init(int noOfCustomers, int noOfVehicles, int vehicleCap)
	{
		this->noOfCustomers = noOfCustomers;
		this->noOfVehicles = noOfVehicles;
		this->vehicleCap = vehicleCap;

		nodes.reserve(this->noOfCustomers);
		vehicles.reserve(this->noOfVehicles);
	}

public:
	void generateNodes(){
		//Depot Coordinates
		int depotX = 50;
		int depotY = 50;

		//Create depot node
		nodes.push_back(Node(depotX, depotY));

		//Initialize nodes
		//Node[0] is allocated for depot node. Start from 1
		for (int i = 1; i <= this->noOfCustomers; i++) {
			nodes.push_back(Node(i));
		}
	}
	void printNodes()
	{
		cout << "Nodes" << endl;

		//Print Nodes
		for (int i = 0; i <= this->noOfCustomers; i++) {
			cout << "Node ID:" << nodes[i].nodeId << "\tX:" << nodes[i].nodeX << "\tY:" << nodes[i].nodeY << "\tDemand:" << nodes[i].demand << endl;
		}
		cout << endl << endl;
	}

	void calculateDistanceMatrix(){
		double deltaX, deltaY;
		for (int i = 0; i <= NO_OF_CUSTOMER; i++){
			for (int j = i + 1; j <= NO_OF_CUSTOMER; j++){
				deltaX = this->nodes[i].nodeX - this->nodes[j].nodeX;
				deltaY = this->nodes[i].nodeY - this->nodes[j].nodeY;

				double distance = round(sqrt((deltaX * deltaX) + (deltaY * deltaY)));

				this->distanceMatrix[i][j] = distance;
				this->distanceMatrix[j][i] = distance;
			}
		}
	}

	void printDistanceMatrix(){
		cout << "Distance Matrix" << endl;
		for (int i = 0; i <= NO_OF_CUSTOMER; i++) {
			for (int j = 0; j <= NO_OF_CUSTOMER; j++) {
				cout << this->distanceMatrix[i][j] << "\t";
			}
			cout << endl;
		}
		cout << endl << endl;
	}

	void generateVehicles(){
		for (int i = 1; i <= this->noOfVehicles; i++) {
			vehicles.push_back(Vehicle(i));
		}
	}

	void printVehicles()
	{
		cout << "Vehicles" << endl;

		//Print Vehicles
		for (int i = 0; i < this->noOfVehicles; i++) {
			cout << "Vehicle ID:" << vehicles[i].vehicleId
				<< "\tCapacity:" << vehicles[i].capacity
				<< "\tLoad:" << vehicles[i].load
				<< "\tCurrent location:" << vehicles[i].currentLocation
				<< endl;
		}
		cout << endl << endl;
	}

	void GreedySolutionForVRP()
	{
		double CandidateCost, EndCost;
		int vehicleIndex = 0;
		//Loop if there is a customer node which has not assigned yes
		while (CheckUnassignedCustomerExists()) {
			int CustomerIndex = 0;
			Node CandCustomer; // Candidate Customer Node

			double minimumCost = (float)DBL_MAX; //Minimum Cost

			//If route of vehicle is empty, then add first node
			if (vehicles[vehicleIndex].nodes.empty())
			{
				vehicles[vehicleIndex].addNode(nodes[0]);
			}

			for (int i = 1; i <= noOfCustomers; i++) {
				//If customer has not a root yet
				if (nodes[i].isRouted == false) {
					//Check whether there exists enough space for that customer in vehicle
					if (vehicles[vehicleIndex].checkCapacity(nodes[i].demand)) {
						//Find the distance between vehicle's cureent prosition and customer 
						CandidateCost = this->distanceMatrix[vehicles[vehicleIndex].currentLocation][i];
						//If that distance is smaller than found minCost found so far
						//then set this cost as new min cost
						if (minimumCost > CandidateCost) {
							minimumCost = CandidateCost;
							CustomerIndex = i;
							CandCustomer = nodes[i];
						}

					}
				}
			}
			//If CandidateCustomer is null
			if (!CandCustomer.isValid)
			{
				//That customer does not fit
				if (vehicleIndex + 1 < sizeof(vehicles)) //We can assign that customer to other cancidate vehicles
				{
					if (vehicles[vehicleIndex].currentLocation != 0) {//Finish the route for that vehicle
						EndCost = this->distanceMatrix[vehicles[vehicleIndex].currentLocation][0]; // Find the distance of that vehicle to depot
						vehicles[vehicleIndex].addNode(nodes[0]); // Add depot to its route
						this->cost += EndCost; //update total cost of greedy solution
					}
					vehicleIndex = vehicleIndex + 1; //Choose next vehicle
				}
				else //We DO NOT have any more vehicle to assign. The problem is unsolved under these parameters
				{
					cout << "\n The customers which do not have a route can not be assigned" << endl <<
						"Under these conditions, the problem is unsolvable" << endl;
					exit(0);
				}
			}
			else
			{
				vehicles[vehicleIndex].addNode(CandCustomer);//Add this candidate customer to the vehicle's path
				nodes[CustomerIndex].isRouted = true;
				this->cost += minimumCost; //ncrease the total cost for this greedySolution
			}

			// Find the distance of that vehicle to depot
			EndCost = this->distanceMatrix[vehicles[vehicleIndex].currentLocation][0];
			// Add depot to its route
			vehicles[vehicleIndex].addNode(nodes[0]);
			//update total cost of greedy solution
			this->cost += EndCost;

		}

	}

	bool CheckUnassignedCustomerExists()
	{
		for (int i = 1; i < sizeof(nodes); i++)
		{
			if (!nodes[i].isRouted)
				return true;
		}
		return false;
	}

	void PrintSolution(string name)//Print Solution In console
	{
		cout << "=========================================================" << endl;
		printf("Solution name is : %s\n", name);

		for (int j = 0; j < noOfVehicles; j++)
		{
			if (!vehicles[j].nodes.empty())
			{
				cout << "Vehicle " << j << ":";
				int routeSize = vehicles[j].nodes.size();
				for (int e = 0; e < routeSize; e++) {
					if (e == routeSize - 1)
					{
						cout << vehicles[j].nodes.at(e).nodeId;
					}
					else
					{
						cout << vehicles[j].nodes.at(e).nodeId << "->";
					}
				}
				cout << endl;
			}
		}
		cout << "\nSolution Cost " << this->cost << endl;
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	cout << "Vehicle routing optimization" << endl;

	Init init(NO_OF_CUSTOMER, NO_OF_VEHICLES, VEHICLE_CAPACITY);
	init.generateNodes();
	init.printNodes();
	init.calculateDistanceMatrix();
	init.printDistanceMatrix();

	init.generateVehicles();
	init.printVehicles();

	init.GreedySolutionForVRP();
	init.PrintSolution("Greedy Solution");



	return 0;
}


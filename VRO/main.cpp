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
#include <fstream>
#include <time.h>       /* time */
#include <omp.h>

#define NO_OF_CUSTOMER 50
#define NO_OF_VEHICLES 10
#define VEHICLE_CAPACITY 80
#define TABU_COUNT 10

using namespace std;

class Node {
public:
	int nodeId, nodeX, nodeY, demand;
	bool isRouted, isDepot, isValid;

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
	vector<Vehicle> vehiclesForBestSolution;
	double distanceMatrix[NO_OF_CUSTOMER + 1][NO_OF_CUSTOMER + 1];// = { { 0 } };
	int noOfCustomers;
	int noOfVehicles;
	int vehicleCap;
	int tabuCount;
	double cost;
	double bestSolutionCost;
	vector<double> pastSolutions;

public:
	Init(int noOfCustomers, int noOfVehicles, int vehicleCap, int tabuCount)
	{
		this->noOfCustomers = noOfCustomers;
		this->noOfVehicles = noOfVehicles;
		this->vehicleCap = vehicleCap;
		this->cost = 0;
		this->tabuCount = tabuCount;
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
			vehiclesForBestSolution.push_back(Vehicle(i));
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
		double CandidateCost, EndCost = 0;
		int vehicleIndex = 0;
		//Loop if there is a customer node which has not assigned yes
		while (CheckUnassignedCustomerExists()) {
			int CustomerIndex = 0;
			Node CandCustomer; // Candidate Customer Node

			double minimumCost = (float)DBL_MAX; //Minimum Cost

			//If route of vehicle is empty, then add first node
			if (this->vehicles[vehicleIndex].nodes.empty())
			{
				this->vehicles[vehicleIndex].addNode(nodes[0]);
			}
//#pragma omp parallel for
			for (int i = 1; i <= noOfCustomers; i++) {
				//If customer has not a root yet
				if (this->nodes[i].isRouted == false) {
					//Check whether there exists enough space for that customer in vehicle
					if (this->vehicles[vehicleIndex].checkCapacity(nodes[i].demand)) {
						//Find the distance between vehicle's cureent prosition and customer 
						CandidateCost = this->distanceMatrix[vehicles[vehicleIndex].currentLocation][i];
						//If that distance is smaller than found minCost found so far
						//then set this cost as new min cost
						if (minimumCost > CandidateCost) {
							minimumCost = CandidateCost;
							CustomerIndex = i;
							CandCustomer = nodes[i];
							CandCustomer.isValid = true;
						}

					}
				}
			}
			//If CandidateCustomer is null
			if (!CandCustomer.isValid)
			{
				//That customer does not fit
				if (vehicleIndex + 1 < this->noOfVehicles) //We can assign that customer to other cancidate vehicles
				{
					if (this->vehicles[vehicleIndex].currentLocation != 0) {//Finish the route for that vehicle
						EndCost = this->distanceMatrix[this->vehicles[vehicleIndex].currentLocation][0]; // Find the distance of that vehicle to depot
						this->vehicles[vehicleIndex].addNode(nodes[0]); // Add depot to its route
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
				this->vehicles[vehicleIndex].addNode(CandCustomer);//Add this candidate customer to the vehicle's path
				nodes[CustomerIndex].isRouted = true;
				this->cost += minimumCost; //ncrease the total cost for this greedySolution
			}



		}

		// Find the distance of that vehicle to depot
		EndCost = this->distanceMatrix[this->vehicles[vehicleIndex].currentLocation][0];
		// Add depot to its route
		vehicles[vehicleIndex].addNode(nodes[0]);
		//update total cost of greedy solution
		this->cost += EndCost;

	}

	bool CheckUnassignedCustomerExists()
	{
		for (int i = 1; i < this->noOfCustomers; i++)
		{
			if (!nodes[i].isRouted)
				return true;
		}
		return false;
	}

	void TabuSearchSolutionForVRP()
	{
		std::vector<Node> previousRoute;
		std::vector<Node> destinationRoute;

		int nodeDemand = 0;

		int vehicleIndexSource, vehicleIndexDestination;

		double BestNCost, NeighboorCost;

		int SwapIndexA = -1, SwapIndexB = -1, SwapRouteFrom = -1, SwapRouteTo = -1;

		int MAX_ITERATIONS = 200;
		int iteration_number = 0;

		int DimensionCustomer = NO_OF_CUSTOMER + 1;
		vector<vector<int> > TABU_Matrix;

		// Set up sizes. (HEIGHT x WIDTH)
		TABU_Matrix.resize(DimensionCustomer + 1);
		for (int i = 0; i < DimensionCustomer + 1; ++i)
			TABU_Matrix[i].resize(DimensionCustomer + 1);

		this->bestSolutionCost = this->cost; //Initial Solution cost

		bool Termination = false;

		while (!Termination) {
			iteration_number++;
			BestNCost = DBL_MAX;

			for (vehicleIndexSource = 0; vehicleIndexSource < NO_OF_VEHICLES; vehicleIndexSource++) {
				previousRoute = this->vehicles[vehicleIndexSource].nodes;

				int RouteFromLength = previousRoute.size();
				for (int i = 1; i < RouteFromLength - 1; i++) { //Not possible to move depot!
					for (vehicleIndexDestination = 0; vehicleIndexDestination < NO_OF_VEHICLES; vehicleIndexDestination++) {
						destinationRoute = this->vehicles[vehicleIndexDestination].nodes;
						int RouteTolength = destinationRoute.size();
						for (int j = 0; (j < RouteTolength - 1); j++) { //Not possible to move after last Depot!

							nodeDemand = previousRoute[i].demand;

							if ((vehicleIndexSource == vehicleIndexDestination) || this->vehicles[vehicleIndexDestination].checkCapacity(nodeDemand)) {
								//If we assign to a different route check capacity constrains
								//if in the new route is the same no need to check for capacity

								if (((vehicleIndexSource == vehicleIndexDestination) && ((j == i) || (j == i - 1))) == false) // Not a move that Changes solution cost
								{

									double MinusCost1 = this->distanceMatrix[previousRoute[i - 1].nodeId][previousRoute[i].nodeId];
									double MinusCost2 = this->distanceMatrix[previousRoute[i].nodeId][previousRoute[i + 1].nodeId];
									double MinusCost3 = this->distanceMatrix[destinationRoute[j].nodeId][destinationRoute[j + 1].nodeId];

									double AddedCost1 = this->distanceMatrix[previousRoute[i - 1].nodeId][previousRoute[i + 1].nodeId];
									double AddedCost2 = this->distanceMatrix[destinationRoute[j].nodeId][previousRoute[i].nodeId];
									double AddedCost3 = this->distanceMatrix[previousRoute[i].nodeId][destinationRoute[j + 1].nodeId];

									//Check if the move is a Tabu! - If it is Tabu break
									if ((TABU_Matrix[previousRoute[i - 1].nodeId][previousRoute[i + 1].nodeId] != 0)
										|| (TABU_Matrix[destinationRoute[j].nodeId][previousRoute[i].nodeId] != 0)
										|| (TABU_Matrix[previousRoute[i].nodeId][destinationRoute[j + 1].nodeId] != 0)) {
										break;
									}

									NeighboorCost = AddedCost1 + AddedCost2 + AddedCost3
										- MinusCost1 - MinusCost2 - MinusCost3;

									if (NeighboorCost < BestNCost) {
										BestNCost = NeighboorCost;
										SwapIndexA = i;
										SwapIndexB = j;
										SwapRouteFrom = vehicleIndexSource;
										SwapRouteTo = vehicleIndexDestination;
									}
								}
							}
						}
					}
				}
			}

			for (int o = 0; o < NO_OF_CUSTOMER + 2; o++) {
				for (int p = 0; p < NO_OF_CUSTOMER + 2; p++) {
					if (TABU_Matrix[o][p] > 0)
					{
						TABU_Matrix[o][p]--;
					}
				}
			}

			previousRoute = this->vehicles[SwapRouteFrom].nodes;
			destinationRoute = this->vehicles[SwapRouteTo].nodes;
			this->vehicles[SwapRouteFrom].nodes.clear();
			this->vehicles[SwapRouteTo].nodes.clear();

			Node SwapNode = previousRoute[SwapIndexA];

			int NodeIDBefore = previousRoute[SwapIndexA - 1].nodeId;
			int NodeIDAfter = previousRoute[SwapIndexA + 1].nodeId;
			int NodeID_F = destinationRoute[SwapIndexB].nodeId;
			int NodeID_G = destinationRoute[SwapIndexB + 1].nodeId;


			/* initialize random seed: */
			srand(time(NULL));


			int RendomDelay1 = rand() % 5;
			int RendomDelay2 = rand() % 5;
			int RendomDelay3 = rand() % 5;

			TABU_Matrix[NodeIDBefore][SwapNode.nodeId] = this->tabuCount + RendomDelay1;
			TABU_Matrix[SwapNode.nodeId][NodeIDAfter] = this->tabuCount + RendomDelay2;
			TABU_Matrix[NodeID_F][NodeID_G] = this->tabuCount + RendomDelay3;

			previousRoute.erase(previousRoute.begin() + SwapIndexA - 1);

			if (SwapRouteFrom == SwapRouteTo) {
				if (SwapIndexA < SwapIndexB) {
					destinationRoute.insert(destinationRoute.begin() + SwapIndexB - 1, SwapNode);
				}
				else {
					destinationRoute.insert(destinationRoute.begin() + SwapIndexB, SwapNode);
				}
			}
			else
			{
				destinationRoute.insert(destinationRoute.begin() + SwapIndexB, SwapNode);
			}

			this->vehicles[SwapRouteFrom].nodes = previousRoute;
			this->vehicles[SwapRouteFrom].load -= nodeDemand;

			this->vehicles[SwapRouteTo].nodes = destinationRoute;
			this->vehicles[SwapRouteTo].load += nodeDemand;


			pastSolutions.push_back(this->cost);

			this->cost += BestNCost;

			if (this->cost <   bestSolutionCost)
			{
				SaveBestSolution();
			}

			if (iteration_number == MAX_ITERATIONS)
			{
				Termination = true;
			}

		}
		for (int i = 0; i < this->noOfVehicles; i++)
		{
			vehicles[i] = vehiclesForBestSolution[i];
		}

		this->cost = bestSolutionCost;


		try{
			ofstream myfile;
			myfile.open("PastSol.txt");
			for (int i = 0; i< pastSolutions.size(); i++){
				myfile << pastSolutions[i] << "\t";
			}
			myfile.close();
		}
		catch (...) {
			cout << "Eception occured";
		}
	}

	void IntraRouteLocalSearch() {

		//We use 1-0 exchange move
		vector<Node> rt;
		double BestNCost, NeigthboorCost;

		int SwapIndexA = -1, SwapIndexB = -1, SwapRoute = -1;

		int MAX_ITERATIONS = 20000;
		int iteration_number = 0;

		bool Termination = false;

		while (!Termination)
		{
			iteration_number++;
			BestNCost = 9999999999999999;

			for (int VehIndex = 0; VehIndex < NO_OF_VEHICLES; VehIndex++) {
				rt = this->vehicles[VehIndex].nodes;
				if (rt.size() == 0)
					break;
				int RoutLength = rt.size();

				for (int i = 1; i < RoutLength - 1; i++) { //Not possible to move depot!

					for (int j = 0; (j < RoutLength - 1); j++) {//Not possible to move after last Depot!

						if ((j != i) && (j != i - 1)) { // Not a move that cHanges solution cost


							double MinusCost1 = this->distanceMatrix[rt[i - 1].nodeId][rt[i].nodeId];
							double MinusCost2 = this->distanceMatrix[rt[i].nodeId][rt[i + 1].nodeId];
							double MinusCost3 = this->distanceMatrix[rt[j].nodeId][rt[j + 1].nodeId];

							double AddedCost1 = this->distanceMatrix[rt[i - 1].nodeId][rt[i + 1].nodeId];
							double AddedCost2 = this->distanceMatrix[rt[j].nodeId][rt[i].nodeId];
							double AddedCost3 = this->distanceMatrix[rt[i].nodeId][rt[j + 1].nodeId];


							NeigthboorCost = AddedCost1 + AddedCost2 + AddedCost3
								- MinusCost1 - MinusCost2 - MinusCost3;

							if (NeigthboorCost < BestNCost) {
								BestNCost = NeigthboorCost;
								SwapIndexA = i;
								SwapIndexB = j;
								SwapRoute = VehIndex;

							}
						}
					}
				}
			}

			if (BestNCost < 0) {

				cout << SwapRoute ;
				rt = this->vehicles[SwapRoute].nodes;

				Node SwapNode = rt[SwapIndexA];

				rt.erase(rt.begin() + SwapIndexA - 1);

				if (SwapIndexA < SwapIndexB)
				{
					rt.push_back(SwapNode);
				}
				else
				{
					rt.push_back(SwapNode);
				}

				pastSolutions.push_back(this->cost);
				this->cost += BestNCost;
			}
			else{
				Termination = true;
			}

			if (iteration_number == MAX_ITERATIONS)
			{
				Termination = true;
			}
		}
		pastSolutions.push_back(this->cost);

		try{
			ofstream myfile;
			myfile.open("PastSol.txt");
			for (int i = 0; i< pastSolutions.size(); i++){
				myfile << pastSolutions[i] << "\t";
			}
			myfile.close();
		}
		catch (...) {
			cout << "Eception occured";
		}
	}


	void InterRouteLocalSearch() {

		//We use 1-0 exchange move
		vector<Node> previousRoute;
		vector<Node> destinationRoute;

		int MovingNodeDemand = 0;

		int VehIndexFrom, VehIndexTo;
		double BestNCost, NeigthboorCost;

		int SwapIndexA = -1, SwapIndexB = -1, SwapRouteFrom = -1, SwapRouteTo = -1;

		int MAX_ITERATIONS = 20000;
		int iteration_number = 0;

		bool Termination = false;

		while (!Termination)
		{
			iteration_number++;
			BestNCost = DBL_MAX;

			for (VehIndexFrom = 0; VehIndexFrom < NO_OF_VEHICLES; VehIndexFrom++) {
				previousRoute = this->vehicles[VehIndexFrom].nodes;
				int RoutFromLength = previousRoute.size();
				for (int i = 1; i < RoutFromLength - 1; i++) { //Not possible to move depot!

					for (VehIndexTo = 0; VehIndexTo < NO_OF_VEHICLES; VehIndexTo++) {
						destinationRoute = this->vehicles[VehIndexTo].nodes;
						int RouteTolength = destinationRoute.size();
						for (int j = 0; (j < RouteTolength - 1); j++) {//Not possible to move after last Depot!

							MovingNodeDemand = previousRoute[i].demand;
							if ((VehIndexFrom == VehIndexTo) || this->vehicles[VehIndexTo].checkCapacity(MovingNodeDemand))
							{
								if (((VehIndexFrom == VehIndexTo) && ((j == i) || (j == i - 1))) == false)  // Not a move that Changes solution cost
								{
									double MinusCost1 = this->distanceMatrix[previousRoute[i - 1].nodeId][previousRoute[i].nodeId];
									double MinusCost2 = this->distanceMatrix[previousRoute[i].nodeId][previousRoute[i + 1].nodeId];
									double MinusCost3 = this->distanceMatrix[destinationRoute[j].nodeId][destinationRoute[j + 1].nodeId];

									double AddedCost1 = this->distanceMatrix[previousRoute[i - 1].nodeId][previousRoute[i + 1].nodeId];
									double AddedCost2 = this->distanceMatrix[destinationRoute[j].nodeId][previousRoute[i].nodeId];
									double AddedCost3 = this->distanceMatrix[previousRoute[i].nodeId][destinationRoute[j + 1].nodeId];


									NeigthboorCost = AddedCost1 + AddedCost2 + AddedCost3
										- MinusCost1 - MinusCost2 - MinusCost3;

									if (NeigthboorCost < BestNCost) {
										BestNCost = NeigthboorCost;
										SwapIndexA = i;
										SwapIndexB = j;
										SwapRouteFrom = VehIndexFrom;
										SwapRouteTo = VehIndexTo;

									}
								}
							}
						}
					}
				}
			}

			if (BestNCost < 0) {// If Best Neightboor Cost is better than the current

				previousRoute = this->vehicles[SwapRouteFrom].nodes;
				destinationRoute = this->vehicles[SwapRouteTo].nodes;
				this->vehicles[SwapRouteFrom].nodes.clear();
				this->vehicles[SwapRouteTo].nodes.clear();

				Node SwapNode = previousRoute[SwapIndexA];

				previousRoute.erase(previousRoute.begin() + SwapIndexA - 1);

				if (SwapRouteFrom == SwapRouteTo) {
					if (SwapIndexA < SwapIndexB) {
						destinationRoute.insert(destinationRoute.begin() + SwapIndexB - 1, SwapNode);
					}
					else {
						destinationRoute.insert(destinationRoute.begin() + SwapIndexB, SwapNode);
					}
				}
				else
				{
					destinationRoute.insert(destinationRoute.begin() + SwapIndexB, SwapNode);
				}

				this->vehicles[SwapRouteFrom].nodes = previousRoute;
				this->vehicles[SwapRouteFrom].load -= MovingNodeDemand;

				this->vehicles[SwapRouteTo].nodes = destinationRoute;
				this->vehicles[SwapRouteTo].load += MovingNodeDemand;

				pastSolutions.push_back(this->cost);
				this->cost += BestNCost;
			}
			else{
				Termination = true;
			}

			if (iteration_number == MAX_ITERATIONS)
			{
				Termination = true;
			}
		}
		pastSolutions.push_back(this->cost);

		try{
			ofstream myfile;
			myfile.open("PastSol.txt");
			for (int i = 0; i< pastSolutions.size(); i++){
				myfile << pastSolutions[i] << "\t";
			}
			myfile.close();
		}
		catch (...) {
			cout << "Eception occured";
		}
	}



	void SaveBestSolution()
	{
		bestSolutionCost = this->cost;
		for (int j = 0; j < NO_OF_VEHICLES; j++)
		{
			vehiclesForBestSolution[j].nodes.clear();
			if (!this->vehicles[j].nodes.empty())
			{
				int RouteSize = this->vehicles[j].nodes.size();
				for (int k = 0; k < RouteSize; k++) {
					Node n = this->vehicles[j].nodes[k];
					vehiclesForBestSolution[j].nodes.push_back(n);
				}
			}
		}
	};


	void PrintSolution(char* name)//Print Solution In console
	{
		cout << "=========================================================" << endl;
		cout << "Solution is : ";
		std::cout << name << endl;

		for (int j = 0; j < noOfVehicles; j++)
		{
			if (!this->vehicles[j].nodes.empty())
			{
				cout << "Vehicle " << j << ":";
				int routeSize = this->vehicles[j].nodes.size();
				for (int e = 0; e < routeSize; e++) {
					if (e == routeSize - 1)
					{
						cout << this->vehicles[j].nodes.at(e).nodeId;
					}
					else
					{
						cout << this->vehicles[j].nodes.at(e).nodeId << "->";
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

	Init init(NO_OF_CUSTOMER, NO_OF_VEHICLES, VEHICLE_CAPACITY, TABU_COUNT);
	init.generateNodes();
	init.printNodes();
	init.calculateDistanceMatrix();
	init.printDistanceMatrix();

	init.generateVehicles();
	init.printVehicles();

	init.GreedySolutionForVRP();
	init.PrintSolution("Greedy Solution");

	init.IntraRouteLocalSearch();
	init.PrintSolution("Solution after Intra-Route Heuristic Neighborhood Search");

	init.GreedySolutionForVRP();
	init.InterRouteLocalSearch();
	init.PrintSolution("Solution after Inter-Route Heuristic Neighborhood Search");

	init.GreedySolutionForVRP();
	init.TabuSearchSolutionForVRP();
	init.PrintSolution("Tabu Search Solution");

	return 0;
}



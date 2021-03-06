/************************************
Name: Celine Fong (1580124) 
	  Claire Martin (1571140)
CMPUT 275, Winter Semester
Major Assignment 2, Part 1
************************************/

#include "wdigraph.h"
#include "dijkstra.h"
#include "serialport.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

struct Point {
long long lat; // latitude of the point
long long lon; // longitude of the point
};

long long manhattan(const Point& pt1, const Point& pt2) {
/* 
	Calculate the manhattan distance between two points

	PARAMETERS:
	pt1, pt2: Call-by-reference to two Point objects, each
	with a latitute/longitude that is used to calculate
	distance.
*/

	long long dist = abs(pt1.lat - pt2.lat) + abs(pt1.lon - pt2.lon);
	return dist;
}

void readGraph(string filename, WDigraph& graph, unordered_map<int, Point>& points) {
/*
	Read the Edmonton map data from the provided file
	and load it into the given WDigraph object.
	Store vertex coordinates in Point struct and map
	each vertex to its corresponding Point struct.
	PARAMETERS:
	filename: name of the file describing a road network
	graph: an instance of the weighted directed graph (WDigraph) class
	points: a mapping between vertex identifiers and their coordinates
*/
	// graph is already instantiated
	// initialize ifstream object
	ifstream file;

	// open the file
	file.open(filename);

	// initialize line that getline is reading
	string line;

	// declare variables
	int IDend, latEnd, lonEnd;
	int ID1end, ID2end;
	int ID, ID1, ID2;
	double latIn, lonIn;
	long long newLat, newLon;

	// check if file is opened correctly
	if (file.is_open()) {
		while (getline(file, line)) {
			if (line.find("V") == 0) {
				// the line specifies a vertex
				// find position of all commas
				IDend = line.find(',', 2);
				latEnd = line.find(',', IDend+1);
				lonEnd = line.length();

				// get ID as int
				ID = stoi(line.substr(2, IDend-2));

				// get lat and lon as double
				latIn = stod(line.substr(IDend+1, latEnd-IDend));
				lonIn = stod(line.substr(latEnd+1, lonEnd-latEnd));

				// store the coordinates to 100000th degree
				newLat = static_cast <long long> (latIn*100000);
				newLon = static_cast <long long> (lonIn*100000);

				// add vertex by ID to graph
				graph.addVertex(ID);

				// temporary Point struct to add to map
				Point tempPoint;
				tempPoint.lat = newLat;
				tempPoint.lon = newLon;

				// store Point struct in points map, paired w/ vertex ID
				pair<int, Point> tempPair (ID, tempPoint);
				points.insert(tempPair);
			} else if (line.find("E") == 0) {
				// the line specifies an edge
				// find position of all commas
				ID1end = line.find(',', 2);
				ID2end = line.find(',', ID1end+1);

				// get both vertex IDs as integers
				ID1 = stoi(line.substr(2, ID1end - 2));
				ID2 = stoi(line.substr(ID1end+1, ID2end - ID1end - 1));

				// add edge one direction only to make directed graph
				long long cost = manhattan(points.at(ID1), points.at(ID2));
				graph.addEdge(ID1, ID2, cost);
			}		
		}
	}

	// close file
	file.close();
}

int main() {

	// instantiate weighted digraph
	WDigraph graph;
	unordered_map<int, Point> points;
	unordered_map<int, PIL> tree;

	// read and create the graph
	readGraph("edmonton-roads-2.0.1.txt", graph, points);

	// instantiate SerialPort object
    SerialPort Serial("/dev/ttyACM0");

    // finite state machine
	enum {WAIT_FOR_CLIENT, SEND_POINTS} curr_mode = WAIT_FOR_CLIENT;

	// declare input string
	string input;

	long long shortestToStart;
	long long shortestToEnd;
	int startID, endID;
	int	currPointIndex = 1;
	
	// initialize vector to keep track of waypoints
	vector <Point> waypoints;

	// initialize node count
	int nodeCount = 1;

	// set up loop to read requests and send points
	while (true) {
		if (curr_mode == WAIT_FOR_CLIENT) {
			// if the server is currently waiting for a request	or acknowledgement
			// read in current line
			input = Serial.readline();
			cout << "This was what the client said: " << input << " IIIIIII" << endl;
			if (input[0] == 'R') {
				// if the first letter of the string is R
				// then it is the request and must read in request coordinates

				// get locations of all spaces
				int firstSpace = input.find(" ");
				int secondSpace = input.find(" ", firstSpace + 1);
				int thirdSpace = input.find(" ", secondSpace + 1);
				int fourthSpace = input.find(" ", thirdSpace + 1);

				// read in start point
				Point start;
				start.lat = stoll(input.substr(firstSpace + 1, secondSpace));
				start.lon = stoll(input.substr(secondSpace + 1, thirdSpace));

				// read in end point
				Point end;
				end.lat = stoll(input.substr(thirdSpace + 1, fourthSpace));
				end.lon = stoll(input.substr(fourthSpace + 1, input.length()));

				cout << start.lat << " " << start.lon << " " << end.lat << " " << end.lon << endl;
				// find closest vertex to start point
				for (auto iter = points.begin(); iter != points.end(); ++iter) {
					long long distStart = manhattan(start, iter->second);
					if (iter == points.begin()) {
						shortestToStart = distStart;
					}
					if (distStart < shortestToStart) {
						startID = iter->first;
						shortestToStart = distStart;
					}
				}

				// find closest vertex to end point
				for (auto iter = points.begin(); iter != points.end(); ++iter) {
					long long distEnd = manhattan(end, iter->second);
					if (iter == points.begin()) {
						shortestToEnd = distEnd;
					}
					if (distEnd < shortestToEnd) {
						endID = iter->first;
						shortestToEnd = distEnd;
					}
				}

				// run dijkstra's from start vertex
				dijkstra(graph, startID, tree);

				// start tracking back waypoints from the endpoint
				int ID = endID;

				// keep reading waypoint IDs until we reach the start point
				while (ID != startID) {
					// add new waypoint to the vector
					waypoints.push_back(points[ID]);
					ID = tree[ID].first;
					nodeCount++;
				}

				// add start point to waypoint vector
				waypoints.push_back(points[startID]);

				// print number of waypoints to serial
				Serial.writeline(nodeCount + "\n");

				cout << nodeCount << endl;
			} else if (input[0] == 'A') {
				// then we have received acknowledgement
				// proceed to sending points
				curr_mode = SEND_POINTS;
			}
		} else {
			// then the server is sending points
			// we must have received acknowledgment to have reached this state

			// make sure we are not retriving waypoints outside of the array
			if (currPointIndex <= nodeCount) {
				// retrive the waypoint we are currently on
				Point currPoint = waypoints[nodeCount - currPointIndex];

				// increment index
				currPointIndex++;

				// print waypoint to Serial
				Serial.writeline("W ");
				Serial.writeline(to_string(currPoint.lat));
				Serial.writeline(" ");
				Serial.writeline(to_string(currPoint.lon));
				Serial.writeline("\n");
				cout << "W " << currPoint.lat << " " << currPoint.lon << endl;

				if (currPointIndex == nodeCount + 1) {
					// we have gone outside the bounds of the waypoint array
					// this indicates the end of the waypoints
					Serial.writeline("E\n");
					cout << "E" << endl;
				}
			}
			curr_mode = WAIT_FOR_CLIENT;
		}
	}

	return 0;
}
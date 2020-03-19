#include "wdigraph.h"
#include "dijkstra.h"
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
	// Return the Manhattan distance between the two given points
	// dist = |x1-x2| + |y1-y1|
	// x = lat, y = lon

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

	// read in input from stdin
	char startInput;
	cin >> startInput; // should be R
	Point start;
	cin >> start.lat >> start.lon; // read in first point
	Point end;
	cin >> end.lat >> end.lon; // read in second point

	long long shortestToStart;
	long long shortestToEnd;
	int startID, endID;

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

	// initialize vector to keep track of waypoints
	vector <Point> waypoints;

	// initailize node count
	int nodeCount = 1;
	int ID = endID;
	while (ID != startID) {
		waypoints.push_back(points[ID]);
		ID = tree[ID].first;
		nodeCount++;
	}
	waypoints.push_back(points[startID]);
	cout << "N " << nodeCount << endl;
	for (int i = nodeCount - 1; i >= 0; i--) {
		cin >> startInput; // should be A
		Point currPoint = waypoints[i];
		cout << "W " << currPoint.lat << " " << currPoint.lon << endl;
	}
	cout << "E" << endl;
	return 0;
}
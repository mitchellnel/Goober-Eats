#include "provided.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>

// MARK: REMOVE
using namespace std;

bool loadDeliveryRequests(string deliveriesFile, GeoCoord& depot, vector<DeliveryRequest>& v);
bool parseDelivery(string line, string& lat, string& lon, string& item);

// MARK: REMOVE
int smTest();
int pTpRTest();
int DPTest();

// MARK: REMOVE
int main2() {
    cerr << "Building successful." << endl;
    smTest();
//    pTpRTest();
//    DPTest();
    
    return 0;
}

// MARK: REMOVE
int smTest() {
    StreetMap sm2;
    assert(sm2.load("/Users/mmclinton/Google Drive/UCLA/@Winter 20/COM SCI 32/Programming Assignments/Assignment 4/Goober Eats/Goober Eats/shortmapdata.txt"));
    vector <StreetSegment> someMoreSegs;
    sm2.getSegmentsThatStartWith(GeoCoord("9", "10"), someMoreSegs);
    for (auto itr = someMoreSegs.begin(); itr != someMoreSegs.end(); itr++) {
        cerr << "(" << (*itr).start.latitudeText << ", " << (*itr).start.longitudeText << ")" << ", " << "(" << (*itr).end.latitudeText << ", " << (*itr).end.longitudeText << ")" << ", " << (*itr).name << endl;
    }
    sm2.getSegmentsThatStartWith(GeoCoord("11", "12"), someMoreSegs);
    for (auto itr = someMoreSegs.begin(); itr != someMoreSegs.end(); itr++) {
        cerr << "(" << (*itr).start.latitudeText << ", " << (*itr).start.longitudeText << ")" << ", " << "(" << (*itr).end.latitudeText << ", " << (*itr).end.longitudeText << ")" << ", " << (*itr).name << endl;
    }
    
    StreetMap sm;
    assert(sm.load("/Users/mmclinton/Google Drive/UCLA/@Winter 20/COM SCI 32/Programming Assignments/Assignment 4/Goober Eats/Goober Eats/mapdata.txt"));
    GeoCoord someStreet("34.0732851", "-118.4931016");
    vector<StreetSegment> someSegs;
    sm.getSegmentsThatStartWith(someStreet, someSegs);
    
    for (auto itr = someSegs.begin(); itr != someSegs.end(); itr++) {
        cerr << "(" << (*itr).start.latitudeText << ", " << (*itr).start.longitudeText << ")" << ", " << "(" << (*itr).end.latitudeText << ", " << (*itr).end.longitudeText << ")" << ", " << (*itr).name << endl;
    }
    
    GeoCoord someGC("34.0687275", "-118.4483620");
    sm.getSegmentsThatStartWith(someGC, someSegs);
    
    for (auto itr = someSegs.begin(); itr != someSegs.end(); itr++) {
        cerr << "(" << (*itr).start.latitudeText << ", " << (*itr).start.longitudeText << ")" << ", " << "(" << (*itr).end.latitudeText << ", " << (*itr).end.longitudeText << ")" << ", " << (*itr).name << endl;
    }
    
    return 0;
}

// MARK: REMOVE
int pTpRTest() {
    StreetMap sm;
    assert(sm.load("/Users/mmclinton/Google Drive/UCLA/@Winter 20/COM SCI 32/Programming Assignments/Assignment 4/Goober Eats/Goober Eats/shortmapdata.txt"));
    PointToPointRouter ptpr(&sm);
    list<StreetSegment> route;
    double distanceTravelled;
    
    assert(ptpr.generatePointToPointRoute(GeoCoord("1", "2"), GeoCoord("17", "18"), route, distanceTravelled) == DELIVERY_SUCCESS);
    cerr << "shortmapdata.txt routing 1 succeeded." << endl;
    
    assert(ptpr.generatePointToPointRoute(GeoCoord("34.0420561", "-118.5011699"), GeoCoord("34.0290818", "-118.4859930"), route, distanceTravelled) == DELIVERY_SUCCESS);
    cerr << "shortmapdata.txt routing 2 succeeded." << endl;
    
    assert(ptpr.generatePointToPointRoute(GeoCoord("34.0547000", "-118.4794734"), GeoCoord("34.0567492", "-118.4805550"), route, distanceTravelled) == DELIVERY_SUCCESS);
    cerr << "shortmapdata.txt routing 3 succeeded." << endl;
    
    StreetMap sm2;
    assert(sm2.load("/Users/mmclinton/Google Drive/UCLA/@Winter 20/COM SCI 32/Programming Assignments/Assignment 4/Goober Eats/Goober Eats/mapdata.txt"));
    PointToPointRouter ptpr2(&sm2);
    
    assert(ptpr2.generatePointToPointRoute(GeoCoord("34.0625329", "-118.4470263"), GeoCoord("34.0685657", "-118.4489289"), route, distanceTravelled) == DELIVERY_SUCCESS);
    cerr << "mapdata.txt routing 1 succeeded." << endl;
    
    assert(ptpr2.generatePointToPointRoute(GeoCoord("34.0625329", "-118.4470263"), GeoCoord("34.0712323", "-118.4505969"), route, distanceTravelled) == DELIVERY_SUCCESS);
    cerr << "mapdata.txt routing 2.0 succeeded." << endl;
    
    assert(ptpr2.generatePointToPointRoute(GeoCoord("34.0712323", "-118.4505969"), GeoCoord("34.0687443", "-118.4449195"), route, distanceTravelled) == DELIVERY_SUCCESS);
    cerr << "mapdata.txt routing 2.1 succeeded." << endl;
    
    assert(ptpr2.generatePointToPointRoute(GeoCoord("34.0687443", "-118.4449195"), GeoCoord("34.0685657", "-118.4489289"), route, distanceTravelled) == DELIVERY_SUCCESS);
    cerr << "mapdata.txt routing 2.2 succeeded." << endl;
    
    assert(ptpr2.generatePointToPointRoute(GeoCoord("34.0685657", "-118.4489289"), GeoCoord("34.0625329", "-118.4470263"), route, distanceTravelled) == DELIVERY_SUCCESS);
    cerr << "mapdata.txt routing 2.3 succeeded." << endl;
    
    assert(ptpr2.generatePointToPointRoute(GeoCoord("34.0625329", "-118.4470263"), GeoCoord("34.0625329", "-118.4470263"), route, distanceTravelled) == DELIVERY_SUCCESS);
    assert(route.empty());
    cerr << "mapdata.txt routing 3 succeeded." << endl;
    
    assert(ptpr2.generatePointToPointRoute(GeoCoord("34.0687275", "-118.4483620"), GeoCoord("34.0686684", "-118.4486251"), route, distanceTravelled) == DELIVERY_SUCCESS);
    cerr << "mapdata.txt routing 4 succeeded." << endl;

    return 0;
}

// MARK: REMOVE
int DPTest() {
    StreetMap sm2;
    assert(sm2.load("/Users/mmclinton/Google Drive/UCLA/@Winter 20/COM SCI 32/Programming Assignments/Assignment 4/Goober Eats/Goober Eats/mapdata.txt"));
    
    DeliveryPlanner dp(&sm2);
    
    DeliveryRequest dr1("memes", GeoCoord("34.0625329", "-118.4470263"));
    vector<DeliveryRequest> deliveries;
    deliveries.push_back(dr1);
    
    vector<DeliveryCommand> commands;
    double dist;
    
        // Deliver to depot
    dp.generateDeliveryPlan(GeoCoord("34.0625329", "-118.4470263"), deliveries, commands, dist);
    
    for (auto itr = commands.begin(); itr != commands.end(); itr++) {
        cerr << (*itr).description() << endl;
    }
    
    deliveries.clear();
    commands.clear();
    
    DeliveryRequest dr2("dreams", GeoCoord("34.0701186", "-118.4682217"));
    deliveries.push_back(dr2);
    
        // Deliver in line along Acari Dr
    dp.generateDeliveryPlan(GeoCoord("34.0684297", "-118.4662551"), deliveries, commands, dist);
    
    for (auto itr = commands.begin(); itr != commands.end(); itr++) {
        cerr << (*itr).description() << endl;
    }
    
    deliveries.clear();
    commands.clear();
    
    DeliveryRequest dr3("waaahhhrder", GeoCoord("34.0704222", "-118.4686314"));
    deliveries.push_back(dr3);
    
        // Deliver from end of Acari Dr to somewhere on S Gunstone Dr
    dp.generateDeliveryPlan(GeoCoord("34.0684297", "-118.4662551"), deliveries, commands, dist);
    
    for (auto itr = commands.begin(); itr != commands.end(); itr++) {
        cerr << (*itr).description() << endl;
    }
    
    deliveries.clear();
    commands.clear();
    
    DeliveryRequest dr4("ice cream", GeoCoord("34.0685657", "-118.4489289"));
    deliveries.push_back(dr4);
    
        // Deliver from end of Acari Dr to somewhere on S Gunstone Dr
    dp.generateDeliveryPlan(GeoCoord("34.0625329", "-118.4470263"), deliveries, commands, dist);
    
    for (auto itr = commands.begin(); itr != commands.end(); itr++) {
        cerr << (*itr).description() << endl;
    }
    
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc == 3)  // MARK: was !=, changed to ==
    {
        cout << "Usage: " << argv[0] << " mapdata.txt deliveries.txt" << endl;
        return 1;
    }

    StreetMap sm;
        
    if (!sm.load("/Users/mmclinton/Google Drive/UCLA/@Winter 20/COM SCI 32/Programming Assignments/Assignment 4/Goober Eats/Goober Eats/mapdata.txt")) // MARK:
    {
        cout << "Unable to load map data file " << argv[1] << endl;
        return 1;
    }

    GeoCoord depot;
    vector<DeliveryRequest> deliveries;
    if (!loadDeliveryRequests("/Users/mmclinton/Google Drive/UCLA/@Winter 20/COM SCI 32/Programming Assignments/Assignment 4/Goober Eats/Goober Eats/deliveries.txt", depot, deliveries))   // MARK:
    {
        cout << "Unable to load delivery request file " << argv[2] << endl;
        return 1;
    }

    cout << "Generating route...\n\n";

    DeliveryPlanner dp(&sm);
    vector<DeliveryCommand> dcs;
    double totalMiles;
    DeliveryResult result = dp.generateDeliveryPlan(depot, deliveries, dcs, totalMiles);
    if (result == BAD_COORD)
    {
        cout << "One or more depot or delivery coordinates are invalid." << endl;
        return 1;
    }
    if (result == NO_ROUTE)
    {
        cout << "No route can be found to deliver all items." << endl;
        return 1;
    }
    cout << "Starting at the depot...\n";
    for (const auto& dc : dcs)
        cout << dc.description() << endl;
    cout << "You are back at the depot and your deliveries are done!\n";
    cout.setf(ios::fixed);
    cout.precision(2);
    cout << totalMiles << " miles travelled for all deliveries." << endl;
    
    // MARK: Remove
    return 0;
}

bool loadDeliveryRequests(string deliveriesFile, GeoCoord& depot, vector<DeliveryRequest>& v)
{
    ifstream inf(deliveriesFile);
    if (!inf)
        return false;
    string lat;
    string lon;
    inf >> lat >> lon;
    inf.ignore(10000, '\n');
    depot = GeoCoord(lat, lon);
    string line;
    while (getline(inf, line))
    {
        string item;
        if (parseDelivery(line, lat, lon, item))
            v.push_back(DeliveryRequest(item, GeoCoord(lat, lon)));
    }
    return true;
}

bool parseDelivery(string line, string& lat, string& lon, string& item)
{
    const size_t colon = line.find(':');
    if (colon == string::npos)
    {
        cout << "Missing colon in deliveries file line: " << line << endl;
        return false;
    }
    istringstream iss(line.substr(0, colon));
    if (!(iss >> lat >> lon))
    {
        cout << "Bad format in deliveries file line: " << line << endl;
        return false;
    }
    item = line.substr(colon + 1);
    if (item.empty())
    {
        cout << "Missing item in deliveries file line: " << line << endl;
        return false;
    }
    return true;
}

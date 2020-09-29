#include "provided.h"
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cctype>

#include "ExpandableHashMap.h"
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
    
private:
    ExpandableHashMap<GeoCoord, vector<StreetSegment>> streetMapData;
    
        // Auxiliary Functions
    void addStreetSeg(const GeoCoord& gc, const StreetSegment& ss);
    void getGeoCoordData(istream& is, string& startLat, string& startLon, string& endLat, string& endLon);
};

StreetMapImpl::StreetMapImpl()
{}

StreetMapImpl::~StreetMapImpl()
{}

bool StreetMapImpl::load(string mapFile)
{
    // Scan each coord line
    //      Take first GeoCoord and push_back the exact line (the StreetSegment Sn) to the vector associated with it
    //      Take the second GeoCoord and push_back the line with the coords reversed (the StreetSegment S(n-1)) to the vector associated with it
    ifstream mapData(mapFile);
    if (!mapData) {
        cerr << "Cannot open Map Data file!" << endl;
        return false;
    }

    
    while (mapData.peek() != EOF) {
            // We know a street name always comes first
        string streetName;
        getline(mapData, streetName);
        
            // Then the number of street segments comes next
        int nSegs;
        mapData >> nSegs;
        mapData.ignore(10'000, '\n');
        
            // Add the GeoCoord associations
            // We'll go line by line, adding Sn and Rev(Sn) to the hashmap
        for (int n = 1; n <= nSegs; n++) {
            string startLat, startLon, endLat, endLon;
            getGeoCoordData(mapData, startLat, startLon, endLat, endLon);
            
                // Add Sn
            addStreetSeg(GeoCoord(startLat, startLon), StreetSegment(GeoCoord(startLat, startLon), GeoCoord(endLat, endLon), streetName));
                // Add Rev(Sn)
            addStreetSeg(GeoCoord(endLat, endLon), StreetSegment(GeoCoord(endLat, endLon), GeoCoord(startLat, startLon), streetName));
        }
    }
    
    return true;    // loading successful
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    const vector<StreetSegment>* segsOfGc = streetMapData.find(gc);
    
    if (segsOfGc == nullptr) {
        return false;   // gc not found in streetMapData
    }
    
    segs = *segsOfGc;
    return true;
}

/////////////////////////////////////////////////
// Auxiliary Functions
/////////////////////////////////////////////////
    // Retrieves the startLat, startLon, endLat, and endLon functions from the next line of a passed in istream
void StreetMapImpl::getGeoCoordData(istream& is, string& startLat, string& startLon, string& endLat, string& endLon) {
    string geoCoordLine;
    getline(is, geoCoordLine);
    istringstream iss(geoCoordLine);
    
    iss >> startLat >> startLon >> endLat >> endLon;
}

    // Adds the passed in StreetSegment to the vector of StreetSegments associated with each GeoCoord in streetMapData
void StreetMapImpl::addStreetSeg(const GeoCoord& gc, const StreetSegment& ss) {
    vector<StreetSegment>* streetSegVector = streetMapData.find(gc);
    
    if (streetSegVector == nullptr) {
        vector<StreetSegment> additions;
        additions.push_back(ss);
        streetMapData.associate(gc, additions);
        return;
    }
    
    streetSegVector->push_back(ss);
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}

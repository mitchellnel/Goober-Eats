#include "provided.h"
#include <vector>
#include <algorithm>
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_streetMap;
    
    string dirToWords(const double& dir) const;
    bool proceedAlongStreet(
        list<StreetSegment>& currRoute,
        const StreetSegment& currSS,
        list<StreetSegment>::iterator& currSSItr,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
    
    double getDistanceAlongStreet(
        const list<StreetSegment>::iterator startSSItr,
        const list<StreetSegment>::iterator endSSItr) const;
    
    void turnOntoStreet(
        const StreetSegment& prevSS,
        const StreetSegment& currSS,
        vector<DeliveryCommand>& commands) const;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm) : m_streetMap(sm)
{}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
        // Reorder delivery requests to make optimal
    DeliveryOptimizer deliveryOpt(m_streetMap);
    double oldCrowDist = 0;
    double newCrowDist = 0;
    vector<DeliveryRequest> orderedDeliveries = deliveries;
    deliveryOpt.optimizeDeliveryOrder(depot, orderedDeliveries, oldCrowDist, newCrowDist);
    
        // Generate point-to-point routes between the depot to each of the successive delivery points
    vector<list<StreetSegment>> completeRoute;
    vector<string> deliveryItems;
    
    GeoCoord prevLoc = depot;
    
    PointToPointRouter ptpr(m_streetMap);
    
    for (vector<DeliveryRequest>::iterator itr = orderedDeliveries.begin(); itr != orderedDeliveries.end(); itr++) {
        deliveryItems.push_back((*itr).item);
        list<StreetSegment> currRoute;
        
        DeliveryResult dr = ptpr.generatePointToPointRoute(prevLoc, (*itr).location, currRoute, totalDistanceTravelled);
        
        if (dr == NO_ROUTE) {
            return NO_ROUTE;
        } else if (dr == BAD_COORD) {
            return BAD_COORD;
        }
        
        completeRoute.push_back(currRoute);
        prevLoc = (*itr).location;
    }

        // Then find route back to the depot
    list<StreetSegment> routeBackToDepot;
    if (ptpr.generatePointToPointRoute(prevLoc, depot, routeBackToDepot, totalDistanceTravelled) == NO_ROUTE) {
        return NO_ROUTE;
    }
    completeRoute.push_back(routeBackToDepot);
    
        // Deliver all the items, generating DeliveryCommands
    DeliveryCommand dc;
    for (int i = 0; i < completeRoute.size() - 1; i++) {
            // Checking for the case that the delivery of food is actually at the depot, in which case our first command is to deliver
        if (completeRoute[i].empty()) {
            dc.initAsDeliverCommand(deliveryItems[i]);
            commands.push_back(dc);
            continue;
        }
        
        list<StreetSegment> currRoute = completeRoute[i];      // just makes the code easier to read
    
            // If we do not deliver first, then our actual first command is to proceed down a street
        list<StreetSegment>::iterator startSSItr = currRoute.begin();
        StreetSegment startSS = *startSSItr;
        
        if (proceedAlongStreet(currRoute, startSS, startSSItr, commands, totalDistanceTravelled)) {
            dc.initAsDeliverCommand(deliveryItems[i]);
            commands.push_back(dc);
            continue;
        }
        
            // The next DC will either be a turn, or a proceed down a new street
        auto currSSItr = startSSItr;
        for (auto itr = currSSItr; itr != currRoute.end(); ) {
            StreetSegment currSS = *itr;
            auto prevSSItr = itr;
            prevSSItr--;
            StreetSegment prevSS = *prevSSItr;
            
            turnOntoStreet(prevSS, currSS, commands);
            
            if (proceedAlongStreet(currRoute, currSS, itr, commands, totalDistanceTravelled)) {
                dc.initAsDeliverCommand(deliveryItems[i]);
                commands.push_back(dc);
                break;
            }
        }
    }
    
        // Now we'll return to the depot
    list<StreetSegment> currRoute = completeRoute[completeRoute.size()-1];
        // First command is to proceed down a street
    list<StreetSegment>::iterator startSSItr = currRoute.begin();
    StreetSegment startSS = *startSSItr;
    
    if (proceedAlongStreet(currRoute, startSS, startSSItr, commands, totalDistanceTravelled)) {
        return DELIVERY_SUCCESS;
    }
    
        // The next DC will either be a turn, or a proceed down a new street
    auto currSSItr = startSSItr;
    for (auto itr = currSSItr; itr != currRoute.end(); ) {
        StreetSegment currSS = *itr;
        auto prevSSItr = itr;
        prevSSItr--;
        StreetSegment prevSS = *prevSSItr;
        
        turnOntoStreet(prevSS, currSS, commands);
        
        if (proceedAlongStreet(currRoute, currSS, itr, commands, totalDistanceTravelled)) {
            return DELIVERY_SUCCESS;
        }
    }
    
    
    
    return DELIVERY_SUCCESS;
}

/////////////////////////////////////////////////
// Auxiliary Functions
/////////////////////////////////////////////////
    // Proceeds along a passed in current Street Segment until that Street ends
bool DeliveryPlannerImpl::proceedAlongStreet(
    list<StreetSegment>& currRoute,
    const StreetSegment& currSS,
    list<StreetSegment>::iterator& currSSItr,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
        // Find when we reach a Street Segment with a different name to our currStreet
        // That indicates to us when to stop proceeding
    list<StreetSegment>::iterator nextStreetItr = find_if(currSSItr, currRoute.end(), [currSS] (const StreetSegment& ss) { return ss.name != currSS.name; } );
    
        // nextStreetItr points to the Street Segment after the end of the currStreet
        // Get the distance to travel along the road (sum of the lengths of each StreetSegment)
    double dist = getDistanceAlongStreet(currSSItr, nextStreetItr);
        // And then the direction to proceed
    string dir = dirToWords(angleOfLine(currSS));
    
        // Now init a proceed command
    DeliveryCommand dc;
    dc.initAsProceedCommand(dir, currSS.name, dist);
    commands.push_back(dc);
    
        // Update currSSItr for use in caller
    currSSItr = nextStreetItr;
    
        // Did we deliver the item? (Or alternatively are we back at the depot?)
        //      We return true if we did to inform the caller
    if (nextStreetItr == currRoute.end()) {
        return true;
    } else {
        return false;
    }
    

    
}

/**
* Works out the cumulative distance across all StreetSegments (SS) on a street
* @param startSSItr An iterator pointing to the starting SS of the road we're proceeding down
* @param endSSItr An iterator pointing to the SS following the last SS on the road we're proceeding down (it's used as a stopping condition)
* @return The distance to proceed along the road
*/
double DeliveryPlannerImpl::getDistanceAlongStreet(
    const list<StreetSegment>::iterator startSSItr,
    const list<StreetSegment>::iterator endSSItr) const
{
        // prev due to use in loop
    auto ssItr = startSSItr;
    
    double distance = 0;
    
    while (ssItr != endSSItr) {
            // Work out length of each SS we travel down
        distance += distanceEarthMiles((*ssItr).start, (*ssItr).end);
        ssItr++;
    }

    return distance;
}

    // Turns onto a new Street from a prev Street, also working out whether that turn is a left or a right
void DeliveryPlannerImpl::turnOntoStreet(
    const StreetSegment &prevSS,
    const StreetSegment &currSS,
    vector<DeliveryCommand> &commands) const
{
    double turnDirDeg = angleBetween2Lines(prevSS, currSS);
    DeliveryCommand dc;
    
    if (turnDirDeg >= 1 && turnDirDeg < 180) {
            // Turn left
        dc.initAsTurnCommand("left", currSS.name);
        commands.push_back(dc);
    } else if (turnDirDeg >= 180 && turnDirDeg <= 359) {
            // Turn right
        dc.initAsTurnCommand("right", currSS.name);
        commands.push_back(dc);
    } else {    // turnDirDeg < 1 || turnDirDeg > 359
            // Do not generate a turn command
            // Instead proceed down a new street
    }
}


    // Converts a direction in degrees to something in words
string DeliveryPlannerImpl::dirToWords(const double& dir) const {
    if (0 <= dir && dir < 22.5) {
        return "east";
    } else if (22.5 <= dir && dir < 67.5) {
        return "northeast";
    } else if (67.5 <= dir && dir < 112.5) {
        return "north";
    } else if (112.5 <= dir && dir < 157.5) {
        return "northwest";
    } else if (157.5 <= dir && dir < 202.5) {
        return "west";
    } else if (202.5 <= dir && dir < 247.5) {
        return "southwest";
    } else if (247.5 <= dir && dir < 292.5) {
        return "south";
    } else if (292.5 <= dir && dir < 337.5) {
        return "southeast";
    } else {    // dir >= 337.5
        return "east";
    }
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}

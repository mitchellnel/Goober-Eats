#include "provided.h"
#include <vector>
#include <algorithm>
using namespace std;

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
        // Our model is to go to the furthest point from the depot, and then work our way backwards by
        //      visiting the next closest delivery location, and then heading back to the depot.
    vector<DeliveryRequest> copyOfDeliveries = deliveries;
    vector<DeliveryRequest> reorderedDeliveries;
    
        // First get total distance as the crow flies in the given order
    GeoCoord prevLoc = depot;
    for (vector<DeliveryRequest>::iterator itr = deliveries.begin(); itr != deliveries.end(); itr++) {
        oldCrowDistance += distanceEarthMiles(prevLoc, (*itr).location);
        prevLoc = (*itr).location;
    }
        // Then include return to depot
    oldCrowDistance += distanceEarthMiles(prevLoc, depot);
    
        // Now we do our model
        // First we'll find the furthest delivery point from the depot; this will be the first point we visit
    vector<DeliveryRequest>::iterator furthestLocItr = max_element(copyOfDeliveries.begin(),
                                                                   copyOfDeliveries.end(),
                                                                   [depot] (const DeliveryRequest& d1, const DeliveryRequest& d2)
                                                                        { return distanceEarthMiles(depot, d1.location) < distanceEarthMiles(depot, d2.location); } );
    reorderedDeliveries.push_back(*furthestLocItr);
    newCrowDistance += distanceEarthMiles(depot, furthestLocItr->location);
    
    prevLoc = furthestLocItr->location;
    
    furthestLocItr = copyOfDeliveries.erase(furthestLocItr);
    
        // Now find the next closest delivery point from the each point we're visiting; this is the second point we visit
    for (int i = 1; i < deliveries.size() - 1; i++) {
        vector<DeliveryRequest>::iterator nextClosestItr = min_element(copyOfDeliveries.begin(),
                                                                       copyOfDeliveries.end(),
                                                                       [prevLoc] (const DeliveryRequest& d1, const DeliveryRequest& d2)
                                                                       { return distanceEarthMiles(prevLoc, d1.location) < distanceEarthMiles(prevLoc, d2.location); } );
        reorderedDeliveries.push_back(*nextClosestItr);
        newCrowDistance += distanceEarthMiles(prevLoc, nextClosestItr->location);
        
        prevLoc = nextClosestItr->location;
        
        nextClosestItr = copyOfDeliveries.erase(nextClosestItr);
    }
    
        // Now push_back the last delivery point remaining in the copyOfDeliveries vector (if there is more than 1)
    if (deliveries.size() > 1) {
        vector<DeliveryRequest>::iterator lastDelReq = copyOfDeliveries.begin();
        reorderedDeliveries.push_back(*lastDelReq);
        newCrowDistance += distanceEarthMiles(prevLoc, lastDelReq->location);
    }
    
    if (newCrowDistance < oldCrowDistance) {
        deliveries = reorderedDeliveries;
    }
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}

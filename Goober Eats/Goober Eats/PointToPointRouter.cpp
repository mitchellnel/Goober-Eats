#include "provided.h"
#include <set>
#include <list>
#include <algorithm>
using namespace std;

struct AStarNode;
struct ASN_comparator;

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_streetMap;
    
    list<AStarNode> getChildren(AStarNode* asn) const;
    bool AStarAlgorithm(const GeoCoord& start, const GeoCoord& end, list<StreetSegment>& route) const;
    void cleanUpAStar(set<AStarNode*, ASN_comparator> openList, list<AStarNode*> closedList) const;
    list<StreetSegment> reverseNodeRoute(AStarNode* asn) const;
};

    // We construct an AStarNode struct for use in our A* Pathfinding algorithm.
    // An AStarNode is essentially a possible movement from one GeoCoord to
    //      another, with consideration of movement costs and distances from a target
    //      and/or ending GeoCoord
    // The node must:
    //   - have a parent AStarNode data member that indicates where we previously moved from
    //   - have a GeoCoord data member that stores the position a movement originates from
    //   - have a GeoCoord data member that stores the eventual target it is trying to reach
    //   - have a gCost relative to the cost of moving from the parent ASN GeoCoord to the curr GeoCoord
    //   - have an hCost relative to the distance of the current GeoCoord to the end GeoCoord
    //   - have a defined operator< so it can be stored in a set of ascending fCosts
struct AStarNode {
        // AStarNode
    AStarNode(AStarNode* parentASN, const GeoCoord& curr, const GeoCoord& target) : parent(parentASN), gc(curr), tar(target) {
        if (parent == nullptr) {        // for the starting asn, leave gCost as 0
            gCost = 0;
        } else {
            gCost = parent->gCost + distanceEarthMiles(parent->gc, curr);
        }
        hCost = distanceEarthMiles(curr, target);
    }
    
    AStarNode* parent;
    GeoCoord gc;
    GeoCoord tar;
    double gCost;
    double hCost;
    
    double fCost() const {
        return gCost + hCost;
    }
};
    
    // AStarNode comparator to help order set in AStarAlgorithm
struct ASN_comparator {
    bool operator() (const AStarNode* lhs, const AStarNode* rhs) const {
        if (lhs == rhs)
            return false;
            
        return lhs->fCost() < rhs->fCost();
    }
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm) : m_streetMap(sm)
{}

PointToPointRouterImpl::~PointToPointRouterImpl()
{}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
        // Check if start and end are GeoCoords in m_streetMap
    vector<StreetSegment> startSegs;
    vector<StreetSegment> endSegs;
    
    if (!(m_streetMap->getSegmentsThatStartWith(start, startSegs) && m_streetMap->getSegmentsThatStartWith(end, endSegs))) {
        return BAD_COORD;
    }
    
        // Check if end is where we already are, in which case the routing is  successful
    if (start == end) {
        route.clear();
        totalDistanceTravelled = 0;
        return DELIVERY_SUCCESS;
    }
    
        // Find a path using the AStarAlgorithm
    if (!AStarAlgorithm(start, end, route)) {
        return NO_ROUTE;
    }
    
        // Call to AStarAlgorithm has already worked out the real route
        // Now compute the distance travelled
    for (StreetSegment ss : route) {
        totalDistanceTravelled += distanceEarthMiles(ss.start, ss.end);
    }
    
    return DELIVERY_SUCCESS;
}

/////////////////////////////////////////////////
// Auxiliary Functions
/////////////////////////////////////////////////
/**
* Implementation of the A* Search Algorithm to find a path between a starting and destination GeoCoord
* Based on and adapted from the pseudocode on https://www.geeksforgeeks.org/a-search-algorithm/
* @param start The starting GeoCoord of the route
* @param end The destination/ending GeoCoord of the route
* @param route A list that will store the route taken from end to start (for now)
* @return true or false dependent on whether a route is found
*/
bool PointToPointRouterImpl::AStarAlgorithm(const GeoCoord& start, const GeoCoord& end, list<StreetSegment>& route) const {
    // Initialise open list and closed list
    set<AStarNode*, ASN_comparator> openList;        // this will hold the nodes to be analysed; we always analyse the lowest f cost node first, so we use a set with ASN_comparator
    list<AStarNode*> closedList;                     // this will hold the nodes that have already been analysed; no need to maintain an order, list is fine
    
    // Put the starting node onto the openList
    AStarNode* startNode = new AStarNode(nullptr, start, end);
    openList.insert(startNode);
    
    while (!openList.empty()) {
        // Get the node with the lowest fCost on the openList, which is just the first element
        AStarNode* currNode = *openList.begin();
        openList.erase(openList.begin());
        
        // We're analysing currNode, so add this to the closedList (we do this here as it makes cleaning up easier)
        closedList.push_back(currNode);
        
        // Generate possible children of currNode (adjacent nodes)
        // it turns out that the cleaning up of all the dynamically allocated memory is much easier if we don't use pointers for this vector
        list<AStarNode> children = getChildren(currNode);
       
        for (AStarNode child : children) {
            // Have we reached the destination?
            if (child.gc == end) {
                route = reverseNodeRoute(&child);
                cleanUpAStar(openList, closedList);
                return true;
            }
            
            // If a node with the same gc as the child is in the openList and has a lower fCost than the child, skip this child
            bool skipChild = false;
            for (AStarNode* node : openList) {
                if (node->gc == child.gc && node->fCost() <= child.fCost()) {
                    skipChild = true;
                    break;
                }
            }
            
            // If a node with the same gc as the child is in the closedList and has a lower fCost than the child, skip this child
            for (AStarNode* node : closedList) {
                if (node->gc == child.gc && node->fCost() <= child.fCost()) {
                    skipChild = true;
                    break;
                }
            }
            
            if (skipChild) {
                continue;
            } else {
                // If we aren't skipping the child, add the child to the openList for analysis
                openList.insert(new AStarNode(child.parent, child.gc, child.tar));
            }
        }
    }
    // If the openList is empty, we cannot find a path
    cleanUpAStar(openList, closedList);
    return false;
}

    // Cleans up all dynamically allocated memory in the AStarAlgorithm
void PointToPointRouterImpl::cleanUpAStar(set<AStarNode*, ASN_comparator> openList, list<AStarNode*> closedList) const {
    for (set<AStarNode*, ASN_comparator>::iterator itr = openList.begin(); itr != openList.end(); itr++) {
        delete *itr;
    }
    
    for (list<AStarNode*>::iterator itr = closedList.begin(); itr != closedList.end(); itr++) {
        delete *itr;
    }
}

    // Get the "children" of the passed in asn, which are its adjacent asns
list<AStarNode> PointToPointRouterImpl::getChildren(AStarNode* asn) const {
    list<AStarNode> children;
    
    vector<StreetSegment> adjStreetSegs;
    m_streetMap->getSegmentsThatStartWith(asn->gc, adjStreetSegs);
    
    for (StreetSegment ss : adjStreetSegs) {
        AStarNode toPush(asn, ss.end, asn->tar);
        children.push_back(toPush);
    }
    
    return children;
}

    // Using asn.parent we can trace back a route from an end node to a start node
list<StreetSegment> PointToPointRouterImpl::reverseNodeRoute(AStarNode* asn) const {
    list<StreetSegment> route;
    
        // If the passed in asn->parent is a nullptr, then 
    
    while (asn->parent != nullptr) {
        vector<StreetSegment> parentAdjStreetSegs;
        
        m_streetMap->getSegmentsThatStartWith(asn->parent->gc, parentAdjStreetSegs);
        
        for (StreetSegment ss : parentAdjStreetSegs) {
            if (ss.end == asn->gc && ss.start == asn->parent->gc) {
                route.push_front(ss);
                asn = asn->parent;
            }
        }
    }
    
    return route;
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}

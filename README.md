# Goober-Eats
The core concept of this project was using a pathfinding algorithm to generate a route based on which food deliveries could be carried out.

#### Concepts:
* Time complexity (Big-O notation)
* Pathfinding algorithms
* Data structures

The full, 39-page specification can be found [here](https://github.com/mitchellnel/Kontagion/files/5317158/Spec.pdf)

The pathfinding algorithm used in GeneratePointToPointRouter is the A* Search Algorithm. The code in this project has been adapted from the pseudocode found [here](https://www.geeksforgeeks.org/a-search-algorithm/)

## Time Complexities
### StreetMap
#### load()
If the mapdata.txt file has N lines of data, then load() is O(N)

#### getSegmentsThatstartWith()
getSegmentsThatStartWith() utilises ExpandableHashMap::find(), which is O(1), so it is O(1).

### PointToPointRouter
#### generatePointToPointRoute()
I used the A* pathfinding algorithm in the implementation of my generatePointToPointRoute() method (where the algorithm itself is contained within its own method).

In the A* search itself, I utilised the following data structures:
*	std::set: This set held pointers to the AStarNodes that I had yet to run the “A* analysis” on. I chose to use a set with a custom comparator (defined ASN_Comparator in the code) so as to keep my AStarNodes constantly ordered in ascending order of their fCost, which was key in allowing me to always analyse the node that had the lowest fCost first.
*	std::list: I used two lists, one to hold pointers to the AStarNodes that I had already run an “A* analysis” for, the other held AStarNode objects (to make cleaning up easier) that were the children of each node I was analysing. I chose to use a list as I did not require random access, and did not want the processor to bother with reallocation memory every time I pushed_back to a vector.

The generatePointToPointRoute() function itself was O(S), where S is the number of Street Segments in the A* resultant route (as I calculated totalDistanceTravelled).

### DeliveryOptimiser
#### optimiseDeliveryOrder()
I used a simplistic model where I visited the furthest location from the depot, and then worked my way through the rest of the delivery locations by then visiting the next closest delivery location.

We work out the ‘oldCrowDistance’, which is the distance to each point in the order of the deliveries vector “as the crow flies”, and then compare it to the ‘newCrowDistance’, which is the distance of our reordered delivery list that has been reordered by the method above.

So, if the parameter vector ‘deliveries’ holds N delivery requests (which means N delivery locations), then this method is O(N).

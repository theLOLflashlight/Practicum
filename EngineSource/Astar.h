#pragma once

#include "Dictionary.h"
#include "HashMap.h"
#include "Util.h"

#include <vector>
#include <memory>

// Forgive this slightly awkward implementation.

template<
    typename NodeType,
    typename DistanceType,
    template< class, class, class... > typename MapType,
    typename... MapArgs
>
class AStar
{
public:

    using Node = NodeType;
    using NodeRef = std::reference_wrapper< Node >;
    using NodeArray = std::vector< NodeRef >;

    using DistType = DistanceType;

    template< typename T >
    using NodeMap = MapType< NodeRef, T, MapArgs... >;

    // Attempts to find the shortest path between two nodes.
    // Returns a list of nodes along the path if a path is found.
    // Returns an empty list otherwise.
    template<
        typename EstimateFn,
        typename NodeCostFn,
        typename NeighborsFn
    >
    static NodeArray find_path(
        Node&         start,
        Node&         goal,
        EstimateFn&&  fnEstimate,
        NodeCostFn&&  fnNodeCost,
        NeighborsFn&& fnNeighbors,
        int           cutoff,
        const int     initial_capacity )
    {
        // Fair initial capacity reduces reallocations for map.
        //const int initial_capacity = std::sqrt( sizeHint );

        NodeArray closed_set;
        NodeArray open_set = { start };
        NodeMap< Node* > came_from( initial_capacity * 4 );

        // For each node, the cost of getting from the start node to that node.
        // The cost of going from start to start is zero.
        NodeMap< DistType > g_score( initial_capacity );
        g_score[ start ] = DistType {}; // Init to zero.

        // For each node, the total cost of getting from the start node to the goal
        // by passing by that node. That value is partly known, partly heuristic.
        // For the first node, that value is completely heuristic.
        NodeMap< DistType > f_score( initial_capacity );
        f_score[ start ] = fnEstimate( start, goal );

        while ( !open_set.empty() && cutoff-- > 0 )
        {
            Node& current = _find_cheapest( open_set, f_score );

            if ( &current == &goal )
                return _reconstruct_path( came_from, goal ); // Success

            // Remove current node from open set.
            for ( auto itr = open_set.begin(); itr < open_set.end(); ++itr )
                if ( &itr->get() == &current ) {
                    open_set.erase( itr );
                    break;
                }

            closed_set.push_back( current );

            for ( Node& neighbor : fnNeighbors( current ) )
            {
                if ( _contains( closed_set, neighbor ) )
                    continue; // Ignore nodes in the closed set.

                DistType tmp_g_score = g_score[ current ] + fnNodeCost( neighbor );

                if ( !_contains( open_set, neighbor ) || tmp_g_score < g_score[ neighbor ] )
                {
                    came_from[ neighbor ] = &current;
                    g_score[ neighbor ] = tmp_g_score;
                    f_score[ neighbor ] = tmp_g_score + fnEstimate( neighbor, goal );

                    if ( !_contains( open_set, neighbor ) )
                        open_set.push_back( neighbor );
                }
            }
        }

        return {}; // Failure
    }

private:

    static bool _contains( NodeArray& lst, Node& node )
    {
        for ( Node& n : lst )
            if ( &n == &node )
                return true;
        return false;
    }

    // Finds the cheapest node in the open set
    static Node& _find_cheapest(
        NodeArray&           open_set,
        NodeMap< DistType >& f_score )
    {
        NodeRef cheapest = open_set[ 0 ];

        for ( Node& node : open_set )
            if ( auto pNode = f_score.search( node ) )
                if ( auto pCheap = f_score.search( cheapest ) )
                    if ( *pNode < *pCheap )
                        cheapest = node;
        return cheapest;
    }

    // Reconstructs the path from the current node in reverse.
    // Returns the path in the forwards order.
    static NodeArray _reconstruct_path(
        NodeMap< Node* >& came_from,
        NodeRef           current )
    {
        NodeArray total_path = { current };

        while ( auto itr = came_from.search( current ) )
            total_path.push_back( current = **itr );

        return total_path;//{ total_path.rbegin(), total_path.rend() };
    }

};

template<
    typename Node,
    typename EstimateFn,
    typename NodeCostFn,
    typename NeighborsFn
>
auto find_path(
    Node&         start,
    Node&         goal,
    EstimateFn&&  fnEstimate,
    NodeCostFn&&  fnNodeCost,
    NeighborsFn&& fnNeighbors,
    int           cuttoff = INT_MAX,
    int           sizeHint = 10 )
{
    using DistType = decltype( fnEstimate( start, goal ) );
    return AStar< Node, DistType, HashMap >::find_path(
        start, goal,
        forward< EstimateFn >( fnEstimate ),
        forward< NodeCostFn >( fnNodeCost ),
        forward< NeighborsFn >( fnNeighbors ),
        cuttoff, sizeHint );
}

/**
 * \file graph.h
 * \brief Generic adjacency list graph data structure implementation.
 *
 * This file provides a template-based graph implementation that can store
 * arbitrary data at nodes and edges.
 */

#ifndef EVGET_EVENT_GRAPH_H
#define EVGET_EVENT_GRAPH_H

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace evget {

/**
 * \brief Empty struct used as default template parameter when no data is needed.
 */
struct [[maybe_unused]] Nothing {};

/**
 * \brief An adjacency list graph data structure.
 *
 * \tparam N Data stored at a node.
 * \tparam E Data stored at an edge.
 */
template <typename N = Nothing, typename E = Nothing>
class Graph {
public:
    /**
     * \brief Add a node if it does not already exist. Adds node data even if the node
     *        already exists.
     *
     * \param name name of the node
     * \param node_data optional node data to include
     */
    constexpr void AddNode(std::string name, N node_data);

    /**
     * \brief Add an edge if it does not already exist. This will create nodes `from` and `to` if they
     *        do not already exist. Adds edge data even if edge already exists.
     *
     * \param from_edge from edge
     * \param to_edge to edge
     * \param edge_data optional edge data to include
     */
    constexpr void AddEdge(std::string from_edge, std::string to_edge, E edge_data);

    /**
     * \brief Add a node if it does not already exist.
     * \param name Name of the node to add
     */
    constexpr void AddNode(std::string name);

    /**
     * \brief Add an edge if it does not already exist. This will create nodes `from` and `to` if they
     *        do not already exist.
     * \param from_edge Source node name
     * \param to_edge Target node name
     */
    constexpr void AddEdge(std::string from_edge, std::string to_edge);

    /**
     * \brief Get an adjacency list representation of the graph.
     * \return Map from node names to lists of connected node names.
     */
    constexpr std::map<std::string, std::vector<std::string>> GetAdjacencyList();

    /**
     * \brief Get the nodes of the graph.
     * \return Reference to the node map
     */
    constexpr std::map<std::string, std::vector<N>>& GetNodes();

    /**
     * \brief Get the edges of the graph.
     * \return Reference to the edge map
     */
    constexpr std::map<std::string, std::map<std::string, std::vector<E>>>& GetEdges();

    /**
     * \brief Check if both the edges and the nodes are empty.
     * \return True if the graph is empty, false otherwise
     */
    constexpr bool Empty();

private:
    template <typename T>
    constexpr void SetGraphData(std::string name, auto& graph, auto data);

    template <typename T>
    static constexpr auto SetEmptyGraphData(std::string name, auto& graph);

    std::map<std::string, std::map<std::string, std::vector<E>>> edges_;
    std::map<std::string, std::vector<N>> nodes_;
};

template <typename N, typename E>
template <typename T>
constexpr void Graph<N, E>::SetGraphData(std::string name, auto& graph, auto data) {
    auto& link = SetEmptyGraphData<T>(std::move(name), graph).first->second;
    link.emplace_back(std::move(data));
}

template <typename N, typename E>
template <typename T>
constexpr auto Graph<N, E>::SetEmptyGraphData(std::string name, auto& graph) {
    return graph.try_emplace(std::move(name), T{});
}

template <typename N, typename E>
constexpr void Graph<N, E>::AddNode(std::string name, N node_data) {
    SetEmptyGraphData<std::map<std::string, std::vector<E>>>(name, edges_);
    SetGraphData<std::vector<N>>(std::move(name), nodes_, std::move(node_data));
}

template <typename N, typename E>
constexpr void Graph<N, E>::AddEdge(std::string from_edge, std::string to_edge, E edge_data) {
    auto& link = SetEmptyGraphData<std::map<std::string, std::vector<E>>>(from_edge, edges_).first->second;
    SetGraphData<std::vector<E>>(to_edge, link, edge_data);

    SetEmptyGraphData<std::vector<N>>(std::move(from_edge), nodes_);
    SetEmptyGraphData<std::vector<N>>(std::move(to_edge), nodes_);
}

template <typename N, typename E>
constexpr void Graph<N, E>::AddNode(std::string name) {
    SetEmptyGraphData<std::map<std::string, std::vector<E>>>(name, edges_);
    SetEmptyGraphData<std::vector<N>>(std::move(name), nodes_);
}

template <typename N, typename E>
constexpr void Graph<N, E>::AddEdge(std::string from_edge, std::string to_edge) {
    auto& link = SetEmptyGraphData<std::map<std::string, std::vector<E>>>(from_edge, edges_).first->second;
    SetEmptyGraphData<std::vector<E>>(to_edge, link);

    SetEmptyGraphData<std::vector<N>>(std::move(from_edge), nodes_);
    SetEmptyGraphData<std::vector<N>>(std::move(to_edge), nodes_);
}

template <typename N, typename E>
constexpr std::map<std::string, std::vector<std::string>> Graph<N, E>::GetAdjacencyList() {
    std::map<std::string, std::vector<std::string>> adjacency_list{};
    for (const auto& value : edges_) {
        std::vector<std::string> list{};
        for (const auto& list_value : value.second) {
            list.push_back(list_value.first);
        }

        adjacency_list.try_emplace(value.first, std::move(list));
    }

    return adjacency_list;
}

template <typename N, typename E>
constexpr std::map<std::string, std::vector<N>>& Graph<N, E>::GetNodes() {
    return nodes_;
}

template <typename N, typename E>
constexpr std::map<std::string, std::map<std::string, std::vector<E>>>& Graph<N, E>::GetEdges() {
    return edges_;
}

template <typename N, typename E>
constexpr bool Graph<N, E>::Empty() {
    return nodes_.empty() && edges_.empty();
}
} // namespace evget

#endif

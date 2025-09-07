#ifndef EVGET_EVENT_GRAPH_H
#define EVGET_EVENT_GRAPH_H

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace evget {

struct [[maybe_unused]] Nothing {};

/**
 * An adjacency list graph data structure.
 *
 * @tparam N Data stored at a node.
 * @tparam E Data stored at an edge.
 */
template <typename N = Nothing, typename E = Nothing>
class Graph {
public:
    /**
     * Add a node if it does not already exist. Adds node data even if node
     * already exists.
     *
     * @param name name of the node.
     * @param nodeData optional node data to include.
     */
    constexpr void AddNode(std::string name, N node_data);

    /**
     * Add an edge if it does not already exist. This will create nodes `from` and `to` if they
     * do not already exist. Adds edge data even if edge already exists.
     *
     * @param from_edge from edge.
     * @param to_edge to edge.
     * @param edgeData optional edge data to include.
     */
    constexpr void AddEdge(std::string from_edge, std::string to_edge, E edge_data);

    /**
     * Add a node if it does not already exist.
     */
    constexpr void AddNode(std::string name);

    /**
     * Add an edge if it does not already exist. This will create nodes `from` and `to` if they
     * do not already exist.
     */
    constexpr void AddEdge(std::string from_edge, std::string to_edge);

    /**
     * Get an adjacency list representation of the graph.
     */
    constexpr std::map<std::string, std::vector<std::string>> GetAdjacencyList();

    /**
     * Get the nodes of the graph.
     */
    constexpr std::map<std::string, std::vector<N>>& GetNodes();

    /**
     * Get the edges of the graph.
     */
    constexpr std::map<std::string, std::map<std::string, std::vector<E>>>& GetEdges();

    /**
     * Check if both the edges and the nodes are empty.
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
    auto& link = setEmptyGraphData<T>(name, graph).first->second;
    link.emplace_back(std::move(data));
}

template <typename N, typename E>
template <typename T>
constexpr auto Graph<N, E>::SetEmptyGraphData(std::string name, auto& graph) {
    return graph.try_emplace(name, T{});
}

template <typename N, typename E>
constexpr void Graph<N, E>::AddNode(std::string name, N node_data) {
    setEmptyGraphData<std::map<std::string, std::vector<E>>>(name, edges_);
    setGraphData<std::vector<N>>(name, nodes_, node_data);
}

template <typename N, typename E>
constexpr void Graph<N, E>::AddEdge(std::string from_edge, std::string to_edge, E edge_data) {
    auto& link = setEmptyGraphData<std::map<std::string, std::vector<E>>>(from_edge, edges_).first->second;
    setGraphData<std::vector<E>>(to_edge, link, edge_data);

    setEmptyGraphData<std::vector<N>>(from_edge, nodes_);
    setEmptyGraphData<std::vector<N>>(to_edge, nodes_);
}

template <typename N, typename E>
constexpr void Graph<N, E>::AddNode(std::string name) {
    setEmptyGraphData<std::map<std::string, std::vector<E>>>(name, edges_);
    setEmptyGraphData<std::vector<N>>(name, nodes_);
}

template <typename N, typename E>
constexpr void Graph<N, E>::AddEdge(std::string from_edge, std::string to_edge) {
    auto& link = setEmptyGraphData<std::map<std::string, std::vector<E>>>(from_edge, edges_).first->second;
    setEmptyGraphData<std::vector<E>>(to_edge, link);

    setEmptyGraphData<std::vector<N>>(from_edge, nodes_);
    setEmptyGraphData<std::vector<N>>(to_edge, nodes_);
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

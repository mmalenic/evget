// MIT License
//
// Copyright (c) 2021 Marko Malenic
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef EVGET_GRAPH_H
#define EVGET_GRAPH_H

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace evget::Event {

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
    constexpr void addNode(std::string name, N nodeData);

    /**
     * Add an edge if it does not already exist. This will create nodes `from` and `to` if they
     * do not already exist. Adds edge data even if edge already exists.
     *
     * @param from_edge from edge.
     * @param to_edge to edge.
     * @param edgeData optional edge data to include.
     */
    constexpr void addEdge(std::string from_edge, std::string to_edge, E edgeData);

    /**
     * Add a node if it does not already exist.
     */
    constexpr void addNode(std::string name);

    /**
     * Add an edge if it does not already exist. This will create nodes `from` and `to` if they
     * do not already exist.
     */
    constexpr void addEdge(std::string from_edge, std::string to_edge);

    /**
     * Get an adjacency list representation of the graph.
     */
    constexpr std::map<std::string, std::vector<std::string>> getAdjacencyList();

    /**
     * Get the nodes of the graph.
     */
    constexpr std::map<std::string, std::vector<N>>& getNodes();

    /**
     * Get the edges of the graph.
     */
    constexpr std::map<std::string, std::map<std::string, std::vector<E>>>& getEdges();

    /**
     * Check if both the edges and the nodes are empty.
     */
    constexpr bool empty();

private:
    template <typename T>
    constexpr void setGraphData(std::string name, auto& graph, auto data);

    template <typename T>
    static constexpr auto setEmptyGraphData(std::string name, auto& graph);

    std::map<std::string, std::map<std::string, std::vector<E>>> edges;
    std::map<std::string, std::vector<N>> nodes;
};

template <typename N, typename E>
template <typename T>
constexpr void Graph<N, E>::setGraphData(std::string name, auto& graph, auto data) {
    auto& link = setEmptyGraphData<T>(name, graph).first->second;
    link.emplace_back(std::move(data));
}

template <typename N, typename E>
template <typename T>
constexpr auto Graph<N, E>::setEmptyGraphData(std::string name, auto& graph) {
    return graph.try_emplace(name, T{});
}

template <typename N, typename E>
constexpr void Graph<N, E>::addNode(std::string name, N nodeData) {
    setEmptyGraphData<std::map<std::string, std::vector<E>>>(name, edges);
    setGraphData<std::vector<N>>(name, nodes, nodeData);
}

template <typename N, typename E>
constexpr void Graph<N, E>::addEdge(std::string from_edge, std::string to_edge, E edgeData) {
    auto& link = setEmptyGraphData<std::map<std::string, std::vector<E>>>(from_edge, edges).first->second;
    setGraphData<std::vector<E>>(to_edge, link, edgeData);

    setEmptyGraphData<std::vector<N>>(from_edge, nodes);
    setEmptyGraphData<std::vector<N>>(to_edge, nodes);
}

template <typename N, typename E>
constexpr void Graph<N, E>::addNode(std::string name) {
    setEmptyGraphData<std::map<std::string, std::vector<E>>>(name, edges);
    setEmptyGraphData<std::vector<N>>(name, nodes);
}

template <typename N, typename E>
constexpr void Graph<N, E>::addEdge(std::string from_edge, std::string to_edge) {
    auto& link = setEmptyGraphData<std::map<std::string, std::vector<E>>>(from_edge, edges).first->second;
    setEmptyGraphData<std::vector<E>>(to_edge, link);

    setEmptyGraphData<std::vector<N>>(from_edge, nodes);
    setEmptyGraphData<std::vector<N>>(to_edge, nodes);
}

template <typename N, typename E>
constexpr std::map<std::string, std::vector<std::string>> Graph<N, E>::getAdjacencyList() {
    std::map<std::string, std::vector<std::string>> adjacencyList{};
    for (const auto& value : edges) {
        std::vector<std::string> list{};
        for (const auto& listValue : value.second) {
            list.push_back(listValue.first);
        }

        adjacencyList.try_emplace(value.first, std::move(list));
    }

    return adjacencyList;
}

template <typename N, typename E>
constexpr std::map<std::string, std::vector<N>>& Graph<N, E>::getNodes() {
    return nodes;
}

template <typename N, typename E>
constexpr std::map<std::string, std::map<std::string, std::vector<E>>>& Graph<N, E>::getEdges() {
    return edges;
}

template <typename N, typename E>
constexpr bool Graph<N, E>::empty() {
    return nodes.empty() && edges.empty();
}
}  // namespace evget::Event

#endif  // EVGET_GRAPH_H

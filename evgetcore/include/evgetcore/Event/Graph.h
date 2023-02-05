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
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace EvgetCore::Event {

/**
 * A node in a graph.
 *
 * @tparam Edge represents data of the edge linking to this node.
 * @tparam Data represents the data of the node.
 */

/**
 * An adjacency list graph data structure.
 *
 * @tparam N Data stored at a node.
 * @tparam E Data stored at an edge.
 */
template <typename N, typename E>
class Graph {
public:
    /**
     * Add a node if it does not already exist. Adds node data even if node
     * already exists.
     *
     * @param nodeData optional node data to include.
     */
    void addNode(std::string name, std::optional<N> nodeData);

    /**
     * Add an edge if it does not already exist and `from` and `to` are valid nodes.
     * Adds edge data even if edge already exists.
     *
     * @param edgeData optional edge data to include.
     */
    void addEdge(std::string from, std::string to, std::optional<N> edgeData);

private:
    using EdgeLink = std::pair<std::string, std::vector<E>>;

    std::map<std::string, std::vector<EdgeLink>> graph;
    std::unordered_map<std::string, std::vector<N>> data;
};

template <typename N, typename E>
void Graph<N, E>::addNode(std::string name, std::optional<N> nodeData) {
    graph.try_emplace(name, std::vector<EdgeLink>{});
    auto element = data.try_emplace(name, std::vector<N>{});

    if (nodeData.has_value()) {
        element.first->second.emplace_back(std::move(*nodeData));
    }
}

}  // namespace EvgetCore::Event

#endif  // EVGET_GRAPH_H

#include "GraphGenerator.hpp"

#include <random>
#include <set>
#include <utility>
#include <stdexcept>
#include <algorithm>

using namespace std;

Graph GraphGenerator::generateConnectedSparseGraph(
    int vertexCount,
    int edgeCount,
    int minWeight,
    int maxWeight,
    unsigned int randomSeed
) {
    if (vertexCount <= 0) {
        throw invalid_argument("vertexCount must be positive");
    }

    if (edgeCount < vertexCount - 1) {
        throw invalid_argument("edgeCount must be at least vertexCount - 1 for connected graph");
    }

    int maxPossibleEdges = vertexCount * (vertexCount - 1) / 2;

    if (edgeCount > maxPossibleEdges) {
        throw invalid_argument("Too many edges for undirected graph");
    }

    if (minWeight <= 0 || maxWeight < minWeight) {
        throw invalid_argument("Incorrect edge weight bounds");
    }

    Graph graph(vertexCount);

    mt19937 generator(randomSeed);

    uniform_int_distribution<int> weightDistribution(minWeight, maxWeight);

    set<pair<int, int>> usedEdges;

    for (int vertex = 0; vertex < vertexCount - 1; ++vertex) {
        int from = vertex;
        int to = vertex + 1;

        int weight = weightDistribution(generator);

        graph.addEdge(from, to, weight, true);

        usedEdges.insert({ from, to });
    }

    uniform_int_distribution<int> vertexDistribution(0, vertexCount - 1);

    while (static_cast<int>(usedEdges.size()) < edgeCount) {
        int from = vertexDistribution(generator);
        int to = vertexDistribution(generator);

        if (from == to) {
            continue;
        }

        if (from > to) {
            swap(from, to);
        }

        pair<int, int> edge = { from, to };

        if (usedEdges.find(edge) != usedEdges.end()) {
            continue;
        }

        usedEdges.insert(edge);

        int weight = weightDistribution(generator);

        graph.addEdge(from, to, weight, true);
    }

    return graph;
}
#include "Graph.hpp"

#include <fstream>
#include <stdexcept>
using namespace std;

Graph::Graph()
    : vertexCount(0), edgeCount(0) {}

Graph::Graph(int vertices)
    : vertexCount(vertices), edgeCount(0), adjacencyList(vertices) {}

void Graph::addEdge(int from, int to, double weight, bool undirected) {
    if (from < 0 || from >= vertexCount || to < 0 || to >= vertexCount) {
        throw out_of_range("Vertex index is out of range");
    }

    if (weight < 0.0) {
        throw invalid_argument("Dijkstra algorithm requires non-negative edge weights");
    }

    adjacencyList[from].push_back({ to, weight });

    if (undirected) {
        adjacencyList[to].push_back({ from, weight });
    }

    ++edgeCount;
}

Graph Graph::loadFromFile(const string& filename, bool undirected) {
    ifstream input(filename);

    if (!input.is_open()) {
        throw runtime_error("Cannot open graph file: " + filename);
    }

    int n;
    int m;

    input >> n >> m;

    if (!input) {
        throw runtime_error("Incorrect graph file format");
    }

    Graph graph(n);

    for (int i = 0; i < m; ++i) {
        int from;
        int to;
        double weight;

        input >> from >> to >> weight;

        if (!input) {
            throw runtime_error("Incorrect edge format in graph file");
        }

        graph.addEdge(from, to, weight, undirected);
    }

    return graph;
}

int Graph::size() const {
    return vertexCount;
}

int Graph::edges() const {
    return edgeCount;
}

const vector<Edge>& Graph::neighbors(int vertex) const {
    if (vertex < 0 || vertex >= vertexCount) {
        throw out_of_range("Vertex index is out of range");
    }

    return adjacencyList[vertex];
}

int Graph::degree(int vertex) const {
    if (vertex < 0 || vertex >= vertexCount) {
        throw out_of_range("Vertex index is out of range");
    }

    return static_cast<int>(adjacencyList[vertex].size());
}

void Graph::saveToFile(const string& filename, bool undirected) const {
    ofstream output(filename);

    if (!output.is_open()) {
        throw runtime_error("Cannot open output graph file: " + filename);
    }

    output << vertexCount << " " << edgeCount << "\n";

    for (int from = 0; from < vertexCount; ++from) {
        for (const Edge& edge : adjacencyList[from]) {
            int to = edge.to;

            if (undirected && from > to) {
                continue;
            }

            output << from << " " << to << " " << edge.weight << "\n";
        }
    }
}
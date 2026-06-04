#include "LandmarkOracle.hpp"
#include "Dijkstra.hpp"

#include <limits>
#include <algorithm>
#include <utility>
#include <stdexcept>

using namespace std;

string strategyToString(LandmarkSelectionStrategy strategy) {
    switch (strategy) {
    case LandmarkSelectionStrategy::SEQUENTIAL_INDEX:
        return "SEQUENTIAL_INDEX";

    case LandmarkSelectionStrategy::DEGREE_CENTRALITY:
        return "DEGREE_CENTRALITY";

    default:
        return "UNKNOWN";
    }
}

void LandmarkOracle::selectSequentialIndexLandmarks(
    const Graph& graph,
    int landmarkCount
) {
    landmarks.clear();

    int n = graph.size();

    for (int i = 0; i < landmarkCount && i < n; ++i) {
        landmarks.push_back(i);
    }
}

void LandmarkOracle::selectDegreeCentralityLandmarks(
    const Graph& graph,
    int landmarkCount
) {
    landmarks.clear();

    int n = graph.size();

    vector<pair<int, int>> degreeVertexPairs;
    degreeVertexPairs.reserve(n);

    for (int vertex = 0; vertex < n; ++vertex) {
        int degree = graph.degree(vertex);
        degreeVertexPairs.push_back({ degree, vertex });
    }

    sort(
        degreeVertexPairs.begin(),
        degreeVertexPairs.end(),
        [](const pair<int, int>& a, const pair<int, int>& b) {
            if (a.first != b.first) {
                return a.first > b.first;
            }

            return a.second < b.second;
        }
    );

    for (int i = 0; i < landmarkCount && i < n; ++i) {
        landmarks.push_back(degreeVertexPairs[i].second);
    }
}

LandmarkOracle::LandmarkOracle(
    const Graph& graph,
    int landmarkCount,
    LandmarkSelectionStrategy strategy
) {
    int n = graph.size();

    if (landmarkCount < 0) {
        throw invalid_argument("landmarkCount must be non-negative");
    }

    if (landmarkCount > n) {
        landmarkCount = n;
    }

    if (strategy == LandmarkSelectionStrategy::SEQUENTIAL_INDEX) {
        selectSequentialIndexLandmarks(graph, landmarkCount);
    }
    else if (strategy == LandmarkSelectionStrategy::DEGREE_CENTRALITY) {
        selectDegreeCentralityLandmarks(graph, landmarkCount);
    }
    else {
        throw invalid_argument("Unknown landmark selection strategy");
    }

    distancesFromLandmarks.reserve(landmarks.size());

    for (int landmark : landmarks) {
        vector<double> distances = dijkstra(graph, landmark);
        distancesFromLandmarks.push_back(distances);
    }
}

double LandmarkOracle::query(int from, int to) const {
    double best = numeric_limits<double>::infinity();

    for (const vector<double>& dist : distancesFromLandmarks) {
        double candidate = dist[from] + dist[to];

        if (candidate < best) {
            best = candidate;
        }
    }

    return best;
}

size_t LandmarkOracle::memoryUsageBytes() const {
    size_t result = 0;

    result += landmarks.size() * sizeof(int);

    for (const vector<double>& distances : distancesFromLandmarks) {
        result += distances.size() * sizeof(double);
    }

    return result;
}

int LandmarkOracle::landmarkCount() const {
    return static_cast<int>(landmarks.size());
}

const vector<int>& LandmarkOracle::getLandmarks() const {
    return landmarks;
}
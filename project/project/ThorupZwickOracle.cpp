#include "ThorupZwickOracle.hpp"
#include "Dijkstra.hpp"

#include <random>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <algorithm>

using namespace std;

string thorupZwickStrategyToString(ThorupZwickSamplingStrategy strategy) {
    switch (strategy) {
    case ThorupZwickSamplingStrategy::RANDOM:
        return "THORUP_ZWICK_RANDOM";

    case ThorupZwickSamplingStrategy::DEGREE_BIASED:
        return "THORUP_ZWICK_DEGREE_BIASED";

    default:
        return "THORUP_ZWICK_UNKNOWN";
    }
}

ThorupZwickOracle::ThorupZwickOracle(
    const Graph& graph,
    int k,
    ThorupZwickSamplingStrategy strategy,
    unsigned int randomSeed
)
    : vertexCount(graph.size()),
    parameterK(k),
    samplingStrategy(strategy) {
    if (vertexCount <= 0) {
        throw invalid_argument("Graph must contain at least one vertex");
    }

    if (parameterK <= 0) {
        throw invalid_argument("Parameter k must be positive");
    }

    buildLevels(graph, randomSeed);

    vector<vector<double>> allDistances;
    allDistances.reserve(vertexCount);

    for (int source = 0; source < vertexCount; ++source) {
        allDistances.push_back(dijkstra(graph, source));
    }

    buildPivots(allDistances);
    buildBunches(allDistances);
}

double ThorupZwickOracle::averageDegree(const Graph& graph) const {
    double totalDegree = 0.0;

    for (int vertex = 0; vertex < graph.size(); ++vertex) {
        totalDegree += graph.degree(vertex);
    }

    if (graph.size() == 0) {
        return 0.0;
    }

    return totalDegree / static_cast<double>(graph.size());
}

void ThorupZwickOracle::buildLevels(
    const Graph& graph,
    unsigned int randomSeed
) {
    levels.clear();
    levels.resize(parameterK + 1);

    for (int vertex = 0; vertex < vertexCount; ++vertex) {
        levels[0].push_back(vertex);
    }

    mt19937 generator(randomSeed);

    double baseProbability = pow(
        static_cast<double>(vertexCount),
        -1.0 / static_cast<double>(parameterK)
    );

    double avgDegree = averageDegree(graph);

    uniform_real_distribution<double> distribution(0.0, 1.0);

    for (int level = 1; level < parameterK; ++level) {
        for (int vertex : levels[level - 1]) {
            double probability = baseProbability;

            if (samplingStrategy == ThorupZwickSamplingStrategy::DEGREE_BIASED) {
                double degreeFactor = 1.0;

                if (avgDegree > 0.0) {
                    degreeFactor =
                        static_cast<double>(graph.degree(vertex)) / avgDegree;
                }

                probability = baseProbability * degreeFactor;

                if (probability > 1.0) {
                    probability = 1.0;
                }

                if (probability < 0.0) {
                    probability = 0.0;
                }
            }

            double value = distribution(generator);

            if (value < probability) {
                levels[level].push_back(vertex);
            }
        }

        /*
         * Чтобы уровень не оказался пустым слишком рано,
         * добавляем одну случайную вершину с предыдущего уровня.
         */
        if (levels[level].empty() && !levels[level - 1].empty()) {
            uniform_int_distribution<int> indexDistribution(
                0,
                static_cast<int>(levels[level - 1].size()) - 1
            );

            int randomIndex = indexDistribution(generator);
            levels[level].push_back(levels[level - 1][randomIndex]);
        }
    }

    levels[parameterK].clear();
}

void ThorupZwickOracle::buildPivots(
    const vector<vector<double>>& allDistances
) {
    const double INF = numeric_limits<double>::infinity();

    pivot.assign(
        parameterK,
        vector<int>(vertexCount, -1)
    );

    pivotDistance.assign(
        parameterK,
        vector<double>(vertexCount, INF)
    );

    for (int level = 0; level < parameterK; ++level) {
        for (int vertex = 0; vertex < vertexCount; ++vertex) {
            double bestDistance = INF;
            int bestPivot = -1;

            for (int candidate : levels[level]) {
                double distance = allDistances[vertex][candidate];

                if (distance < bestDistance) {
                    bestDistance = distance;
                    bestPivot = candidate;
                }
            }

            pivot[level][vertex] = bestPivot;
            pivotDistance[level][vertex] = bestDistance;
        }
    }
}

void ThorupZwickOracle::buildBunches(
    const vector<vector<double>>& allDistances
) {
    const double INF = numeric_limits<double>::infinity();

    bunchDistances.clear();
    bunchDistances.resize(vertexCount);

    for (int vertex = 0; vertex < vertexCount; ++vertex) {
        for (int level = 0; level < parameterK; ++level) {
            double threshold = INF;

            if (level + 1 < parameterK) {
                threshold = pivotDistance[level + 1][vertex];
            }

            for (int candidate : levels[level]) {
                if (containsInLevel(level + 1, candidate)) {
                    continue;
                }

                double distance = allDistances[vertex][candidate];

                if (distance < threshold) {
                    bunchDistances[vertex][candidate] = distance;
                }
            }
        }
    }
}

bool ThorupZwickOracle::containsInLevel(int levelIndex, int vertex) const {
    if (levelIndex < 0 || levelIndex >= static_cast<int>(levels.size())) {
        return false;
    }

    for (int current : levels[levelIndex]) {
        if (current == vertex) {
            return true;
        }
    }

    return false;
}

double ThorupZwickOracle::query(int source, int target) const {
    if (source < 0 || source >= vertexCount || target < 0 || target >= vertexCount) {
        throw out_of_range("Query vertex is out of range");
    }

    if (source == target) {
        return 0.0;
    }

    int u = source;
    int v = target;

    for (int level = 0; level < parameterK; ++level) {
        int w = pivot[level][u];

        if (w == -1) {
            continue;
        }

        auto iterator = bunchDistances[v].find(w);

        if (iterator != bunchDistances[v].end()) {
            return pivotDistance[level][u] + iterator->second;
        }

        swap(u, v);
    }

    return numeric_limits<double>::infinity();
}

int ThorupZwickOracle::getK() const {
    return parameterK;
}

int ThorupZwickOracle::stretch() const {
    return 2 * parameterK - 1;
}

size_t ThorupZwickOracle::memoryUsageBytes() const {
    size_t result = 0;

    for (const vector<int>& level : levels) {
        result += level.size() * sizeof(int);
    }

    for (const vector<int>& levelPivot : pivot) {
        result += levelPivot.size() * sizeof(int);
    }

    for (const vector<double>& levelDistances : pivotDistance) {
        result += levelDistances.size() * sizeof(double);
    }

    for (const unordered_map<int, double>& bunch : bunchDistances) {
        result += bunch.size() * (sizeof(int) + sizeof(double));
    }

    return result;
}

size_t ThorupZwickOracle::totalBunchSize() const {
    size_t result = 0;

    for (const unordered_map<int, double>& bunch : bunchDistances) {
        result += bunch.size();
    }

    return result;
}
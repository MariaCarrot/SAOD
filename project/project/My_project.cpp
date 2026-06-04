#include "Graph.hpp"
#include "Dijkstra.hpp"
#include "LandmarkOracle.hpp"
#include "ThorupZwickOracle.hpp"
#include "Timer.hpp"
#include "DatasetGenerator.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <limits>
#include <string>
#include <stdexcept>
#include <utility>

using namespace std;

/**
 * @brief Результат одного запроса расстояния.
 */
struct QueryResult {
    string datasetName;
    int vertices;
    int edges;

    string methodName;
    string parameterName;
    int parameterValue;

    int queryIndex;
    int source;
    int target;

    double exactDistance;
    double approximateDistance;
    double absoluteError;
    double relativeError;

    double dijkstraTimeMs;
    double oracleTimeMs;
    double oracleBuildTimeMs;

    size_t oracleMemoryBytes;

    int theoreticalStretch;
    size_t totalBunchSize;
};

/**
 * @brief Сводный результат эксперимента для одного графа, метода и параметра.
 */
struct SummaryResult {
    string datasetName;
    int vertices;
    int edges;

    string methodName;
    string parameterName;
    int parameterValue;

    double oracleBuildTimeMs;
    size_t oracleMemoryBytes;

    double avgDijkstraTimeMs;
    double avgOracleTimeMs;

    double avgAbsoluteError;
    double avgRelativeError;

    double maxAbsoluteError;
    double maxRelativeError;

    int theoreticalStretch;
    size_t totalBunchSize;

    int validQueries;
};

/**
 * @brief Возвращает имя файла без папки.
 *
 * @param path Путь к файлу.
 * @return Имя файла.
 */
string getFileName(const string& path) {
    size_t pos1 = path.find_last_of('/');
    size_t pos2 = path.find_last_of('\\');

    size_t pos = string::npos;

    if (pos1 != string::npos && pos2 != string::npos) {
        pos = (pos1 > pos2) ? pos1 : pos2;
    }
    else if (pos1 != string::npos) {
        pos = pos1;
    }
    else if (pos2 != string::npos) {
        pos = pos2;
    }

    if (pos == string::npos) {
        return path;
    }

    return path.substr(pos + 1);
}

/**
 * @brief Читает список датасетов из текстового файла.
 *
 * @param filename Имя файла со списком графов.
 * @return Список путей к графам.
 */
vector<string> readDatasetList(const string& filename) {
    ifstream input(filename);

    if (!input.is_open()) {
        throw runtime_error("Cannot open dataset list file: " + filename);
    }

    vector<string> datasets;
    string line;

    while (getline(input, line)) {
        if (line.empty()) {
            continue;
        }

        if (line[0] == '#') {
            continue;
        }

        datasets.push_back(line);
    }

    return datasets;
}

/**
 * @brief Генерирует случайные запросы между парами вершин.
 *
 * @param vertexCount Количество вершин.
 * @param queryCount Количество запросов.
 * @param seed Seed генератора.
 * @return Список пар вершин.
 */
vector<pair<int, int>> generateQueries(
    int vertexCount,
    int queryCount,
    unsigned int seed
) {
    vector<pair<int, int>> queries;
    queries.reserve(queryCount);

    mt19937 generator(seed);
    uniform_int_distribution<int> vertexDistribution(0, vertexCount - 1);

    while (static_cast<int>(queries.size()) < queryCount) {
        int source = vertexDistribution(generator);
        int target = vertexDistribution(generator);

        if (source == target) {
            continue;
        }

        queries.push_back(make_pair(source, target));
    }

    return queries;
}

/**
 * @brief Создаёт подробный результат одного запроса.
 */
QueryResult makeQueryResult(
    const string& datasetName,
    int vertices,
    int edges,
    const string& methodName,
    const string& parameterName,
    int parameterValue,
    int queryIndex,
    int source,
    int target,
    double exactDistance,
    double approximateDistance,
    double dijkstraTimeMs,
    double oracleTimeMs,
    double oracleBuildTimeMs,
    size_t oracleMemoryBytes,
    int theoreticalStretch,
    size_t totalBunchSize
) {
    QueryResult result;

    result.datasetName = datasetName;
    result.vertices = vertices;
    result.edges = edges;

    result.methodName = methodName;
    result.parameterName = parameterName;
    result.parameterValue = parameterValue;

    result.queryIndex = queryIndex;
    result.source = source;
    result.target = target;

    result.exactDistance = exactDistance;
    result.approximateDistance = approximateDistance;

    result.absoluteError = approximateDistance - exactDistance;

    if (exactDistance > 0.0) {
        result.relativeError = result.absoluteError / exactDistance;
    }
    else {
        result.relativeError = 0.0;
    }

    result.dijkstraTimeMs = dijkstraTimeMs;
    result.oracleTimeMs = oracleTimeMs;
    result.oracleBuildTimeMs = oracleBuildTimeMs;

    result.oracleMemoryBytes = oracleMemoryBytes;

    result.theoreticalStretch = theoreticalStretch;
    result.totalBunchSize = totalBunchSize;

    return result;
}

/**
 * @brief Считает сводную статистику.
 */
SummaryResult makeSummaryResult(
    const string& datasetName,
    int vertices,
    int edges,
    const string& methodName,
    const string& parameterName,
    int parameterValue,
    double oracleBuildTimeMs,
    size_t oracleMemoryBytes,
    int theoreticalStretch,
    size_t totalBunchSize,
    const vector<QueryResult>& localResults
) {
    SummaryResult summary;

    summary.datasetName = datasetName;
    summary.vertices = vertices;
    summary.edges = edges;

    summary.methodName = methodName;
    summary.parameterName = parameterName;
    summary.parameterValue = parameterValue;

    summary.oracleBuildTimeMs = oracleBuildTimeMs;
    summary.oracleMemoryBytes = oracleMemoryBytes;

    summary.theoreticalStretch = theoreticalStretch;
    summary.totalBunchSize = totalBunchSize;
    summary.validQueries = static_cast<int>(localResults.size());

    double totalDijkstraTime = 0.0;
    double totalOracleTime = 0.0;
    double totalAbsoluteError = 0.0;
    double totalRelativeError = 0.0;

    double maxAbsoluteError = 0.0;
    double maxRelativeError = 0.0;

    for (size_t i = 0; i < localResults.size(); ++i) {
        const QueryResult& result = localResults[i];

        totalDijkstraTime += result.dijkstraTimeMs;
        totalOracleTime += result.oracleTimeMs;
        totalAbsoluteError += result.absoluteError;
        totalRelativeError += result.relativeError;

        if (result.absoluteError > maxAbsoluteError) {
            maxAbsoluteError = result.absoluteError;
        }

        if (result.relativeError > maxRelativeError) {
            maxRelativeError = result.relativeError;
        }
    }

    if (!localResults.empty()) {
        double count = static_cast<double>(localResults.size());

        summary.avgDijkstraTimeMs = totalDijkstraTime / count;
        summary.avgOracleTimeMs = totalOracleTime / count;
        summary.avgAbsoluteError = totalAbsoluteError / count;
        summary.avgRelativeError = totalRelativeError / count;
        summary.maxAbsoluteError = maxAbsoluteError;
        summary.maxRelativeError = maxRelativeError;
    }
    else {
        summary.avgDijkstraTimeMs = 0.0;
        summary.avgOracleTimeMs = 0.0;
        summary.avgAbsoluteError = 0.0;
        summary.avgRelativeError = 0.0;
        summary.maxAbsoluteError = 0.0;
        summary.maxRelativeError = 0.0;
    }

    return summary;
}

/**
 * @brief Эксперимент для landmark-оракула.
 */
SummaryResult runLandmarkExperiment(
    const Graph& graph,
    const string& datasetName,
    const vector<pair<int, int>>& queries,
    int landmarkCount,
    LandmarkSelectionStrategy strategy,
    vector<QueryResult>& allQueryResults
) {
    string methodName = strategyToString(strategy);
    string parameterName = "landmarks";

    Timer buildTimer;
    LandmarkOracle oracle(graph, landmarkCount, strategy);
    double buildTimeMs = buildTimer.elapsedMilliseconds();

    size_t memoryBytes = oracle.memoryUsageBytes();

    vector<QueryResult> localResults;

    for (int i = 0; i < static_cast<int>(queries.size()); ++i) {
        int source = queries[i].first;
        int target = queries[i].second;

        Timer dijkstraTimer;
        vector<double> exactDistances = dijkstra(graph, source);
        double dijkstraTimeMs = dijkstraTimer.elapsedMilliseconds();

        double exactDistance = exactDistances[target];

        if (exactDistance == numeric_limits<double>::infinity()) {
            continue;
        }

        Timer oracleTimer;
        double approximateDistance = oracle.query(source, target);
        double oracleTimeMs = oracleTimer.elapsedMilliseconds();

        if (approximateDistance == numeric_limits<double>::infinity()) {
            continue;
        }

        QueryResult queryResult = makeQueryResult(
            datasetName,
            graph.size(),
            graph.edges(),
            methodName,
            parameterName,
            landmarkCount,
            i,
            source,
            target,
            exactDistance,
            approximateDistance,
            dijkstraTimeMs,
            oracleTimeMs,
            buildTimeMs,
            memoryBytes,
            0,
            0
        );

        localResults.push_back(queryResult);
        allQueryResults.push_back(queryResult);
    }

    return makeSummaryResult(
        datasetName,
        graph.size(),
        graph.edges(),
        methodName,
        parameterName,
        landmarkCount,
        buildTimeMs,
        memoryBytes,
        0,
        0,
        localResults
    );
}

/**
 * @brief Эксперимент для Thorup-Zwick.
 */
SummaryResult runThorupZwickExperiment(
    const Graph& graph,
    const string& datasetName,
    const vector<pair<int, int>>& queries,
    int k,
    ThorupZwickSamplingStrategy samplingStrategy,
    vector<QueryResult>& allQueryResults
) {
    string methodName = thorupZwickStrategyToString(samplingStrategy);
    string parameterName = "k";

    Timer buildTimer;
    ThorupZwickOracle oracle(graph, k, samplingStrategy, 42);
    double buildTimeMs = buildTimer.elapsedMilliseconds();

    size_t memoryBytes = oracle.memoryUsageBytes();
    int theoreticalStretch = oracle.stretch();
    size_t totalBunchSize = oracle.totalBunchSize();

    vector<QueryResult> localResults;

    for (int i = 0; i < static_cast<int>(queries.size()); ++i) {
        int source = queries[i].first;
        int target = queries[i].second;

        Timer dijkstraTimer;
        vector<double> exactDistances = dijkstra(graph, source);
        double dijkstraTimeMs = dijkstraTimer.elapsedMilliseconds();

        double exactDistance = exactDistances[target];

        if (exactDistance == numeric_limits<double>::infinity()) {
            continue;
        }

        Timer oracleTimer;
        double approximateDistance = oracle.query(source, target);
        double oracleTimeMs = oracleTimer.elapsedMilliseconds();

        if (approximateDistance == numeric_limits<double>::infinity()) {
            continue;
        }

        QueryResult queryResult = makeQueryResult(
            datasetName,
            graph.size(),
            graph.edges(),
            methodName,
            parameterName,
            k,
            i,
            source,
            target,
            exactDistance,
            approximateDistance,
            dijkstraTimeMs,
            oracleTimeMs,
            buildTimeMs,
            memoryBytes,
            theoreticalStretch,
            totalBunchSize
        );

        localResults.push_back(queryResult);
        allQueryResults.push_back(queryResult);
    }

    return makeSummaryResult(
        datasetName,
        graph.size(),
        graph.edges(),
        methodName,
        parameterName,
        k,
        buildTimeMs,
        memoryBytes,
        theoreticalStretch,
        totalBunchSize,
        localResults
    );
}

/**
 * @brief Сохраняет подробные результаты.
 */
void saveQueryResultsToCsv(
    const string& filename,
    const vector<QueryResult>& results
) {
    ofstream output(filename);

    if (!output.is_open()) {
        throw runtime_error("Cannot open query results CSV file");
    }

    output << "dataset,vertices,edges,"
        << "method,parameter_name,parameter_value,"
        << "query_index,source,target,"
        << "exact_distance,approximate_distance,"
        << "absolute_error,relative_error,"
        << "dijkstra_time_ms,oracle_time_ms,"
        << "oracle_build_time_ms,oracle_memory_bytes,"
        << "theoretical_stretch,total_bunch_size\n";

    for (size_t i = 0; i < results.size(); ++i) {
        const QueryResult& result = results[i];

        output << result.datasetName << ","
            << result.vertices << ","
            << result.edges << ","
            << result.methodName << ","
            << result.parameterName << ","
            << result.parameterValue << ","
            << result.queryIndex << ","
            << result.source << ","
            << result.target << ","
            << result.exactDistance << ","
            << result.approximateDistance << ","
            << result.absoluteError << ","
            << result.relativeError << ","
            << result.dijkstraTimeMs << ","
            << result.oracleTimeMs << ","
            << result.oracleBuildTimeMs << ","
            << result.oracleMemoryBytes << ","
            << result.theoreticalStretch << ","
            << result.totalBunchSize << "\n";
    }
}

/**
 * @brief Сохраняет сводные результаты.
 */
void saveSummaryResultsToCsv(
    const string& filename,
    const vector<SummaryResult>& results
) {
    ofstream output(filename);

    if (!output.is_open()) {
        throw runtime_error("Cannot open summary results CSV file");
    }

    output << "dataset,vertices,edges,"
        << "method,parameter_name,parameter_value,"
        << "oracle_build_time_ms,oracle_memory_bytes,"
        << "avg_dijkstra_time_ms,avg_oracle_time_ms,"
        << "avg_absolute_error,avg_relative_error,"
        << "max_absolute_error,max_relative_error,"
        << "theoretical_stretch,total_bunch_size,"
        << "valid_queries\n";

    for (size_t i = 0; i < results.size(); ++i) {
        const SummaryResult& result = results[i];

        output << result.datasetName << ","
            << result.vertices << ","
            << result.edges << ","
            << result.methodName << ","
            << result.parameterName << ","
            << result.parameterValue << ","
            << result.oracleBuildTimeMs << ","
            << result.oracleMemoryBytes << ","
            << result.avgDijkstraTimeMs << ","
            << result.avgOracleTimeMs << ","
            << result.avgAbsoluteError << ","
            << result.avgRelativeError << ","
            << result.maxAbsoluteError << ","
            << result.maxRelativeError << ","
            << result.theoreticalStretch << ","
            << result.totalBunchSize << ","
            << result.validQueries << "\n";
    }
}

int main() {
    try {
        /*DatasetGenerator::generateRandomGraphFiles("data");
        return 0;*/
        int queryCount = 1000;

        vector<string> datasetFiles = readDatasetList("data/datasets.txt");

        vector<int> landmarkCounts;
        landmarkCounts.push_back(4);
        landmarkCounts.push_back(8);
        landmarkCounts.push_back(16);

        vector<int> thorupZwickKValues;
        thorupZwickKValues.push_back(2);
        thorupZwickKValues.push_back(3);

        vector<LandmarkSelectionStrategy> landmarkStrategies;
        landmarkStrategies.push_back(LandmarkSelectionStrategy::SEQUENTIAL_INDEX);
        landmarkStrategies.push_back(LandmarkSelectionStrategy::DEGREE_CENTRALITY);

        vector<ThorupZwickSamplingStrategy> thorupZwickStrategies;
        thorupZwickStrategies.push_back(ThorupZwickSamplingStrategy::RANDOM);
        thorupZwickStrategies.push_back(ThorupZwickSamplingStrategy::DEGREE_BIASED);

        vector<QueryResult> queryResults;
        vector<SummaryResult> summaryResults;

        for (size_t datasetIndex = 0; datasetIndex < datasetFiles.size(); ++datasetIndex) {
            string datasetPath = datasetFiles[datasetIndex];
            string datasetName = getFileName(datasetPath);

            cout << "\n========================================\n";
            cout << "Loading dataset: " << datasetPath << "\n";

            Graph graph = Graph::loadFromFile(datasetPath, true);

            cout << "Loaded: " << datasetName << "\n";
            cout << "Vertices: " << graph.size() << "\n";
            cout << "Edges: " << graph.edges() << "\n";

            vector<pair<int, int>> queries = generateQueries(
                graph.size(),
                queryCount,
                static_cast<unsigned int>(1000 + datasetIndex)
            );

            for (size_t s = 0; s < landmarkStrategies.size(); ++s) {
                for (size_t l = 0; l < landmarkCounts.size(); ++l) {
                    int landmarkCount = landmarkCounts[l];

                    if (landmarkCount > graph.size()) {
                        continue;
                    }

                    cout << "\nLandmark method: "
                        << strategyToString(landmarkStrategies[s])
                        << ", landmarks = " << landmarkCount << "\n";

                    SummaryResult summary = runLandmarkExperiment(
                        graph,
                        datasetName,
                        queries,
                        landmarkCount,
                        landmarkStrategies[s],
                        queryResults
                    );

                    summaryResults.push_back(summary);

                    cout << "Build time: " << summary.oracleBuildTimeMs << " ms\n";
                    cout << "Memory:     " << summary.oracleMemoryBytes << " bytes\n";
                    cout << "Avg error:  " << summary.avgRelativeError << "\n";
                }
            }

            for (size_t s = 0; s < thorupZwickStrategies.size(); ++s) {
                for (size_t kIndex = 0; kIndex < thorupZwickKValues.size(); ++kIndex) {
                    int k = thorupZwickKValues[kIndex];

                    cout << "\nThorup-Zwick method: "
                        << thorupZwickStrategyToString(thorupZwickStrategies[s])
                        << ", k = " << k << "\n";

                    SummaryResult summary = runThorupZwickExperiment(
                        graph,
                        datasetName,
                        queries,
                        k,
                        thorupZwickStrategies[s],
                        queryResults
                    );

                    summaryResults.push_back(summary);

                    cout << "Build time: " << summary.oracleBuildTimeMs << " ms\n";
                    cout << "Memory:     " << summary.oracleMemoryBytes << " bytes\n";
                    cout << "Avg error:  " << summary.avgRelativeError << "\n";
                }
            }
        }

        saveQueryResultsToCsv("all_query_results.csv", queryResults);
        saveSummaryResultsToCsv("all_summary_results.csv", summaryResults);

        cout << "\n========================================\n";
        cout << "All experiments finished\n";
        cout << "Saved: all_query_results.csv\n";
        cout << "Saved: all_summary_results.csv\n";
    }
    catch (const exception& exception) {
        cerr << "Error: " << exception.what() << "\n";
        return 1;
    }

    return 0;
}
#include "Graph.hpp"
#include "Dijkstra.hpp"
#include "LandmarkOracle.hpp"
#include "ThorupZwickOracle.hpp"
#include "GraphGenerator.hpp"

#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
using namespace std;

/**
 * @brief Проверяет условие теста.
 *
 * @param condition Условие.
 * @param message Сообщение об ошибке.
 */
void assertTrue(bool condition, const string& message) {
    if (!condition) {
        throw runtime_error("Test failed: " + message);
    }
}

/**
 * @brief Проверяет равенство double с небольшой погрешностью.
 *
 * @param actual Полученное значение.
 * @param expected Ожидаемое значение.
 * @param message Сообщение об ошибке.
 */
void assertDoubleEqual(double actual, double expected, const string& message) {
    const double EPS = 1e-9;

    if (fabs(actual - expected) > EPS) {
        throw runtime_error(
            "Test failed: " + message +
            ". Expected: " + to_string(expected) +
            ", actual: " + to_string(actual)
        );
    }
}

/**
 * @brief Создаёт маленький тестовый граф с заранее известными расстояниями.
 *
 * Кратчайшие расстояния от 0:
 * до 0 = 0
 * до 1 = 3, путь 0 -> 2 -> 1
 * до 2 = 2
 * до 3 = 8, путь 0 -> 2 -> 1 -> 3
 * до 4 = 12, путь 0 -> 2 -> 4
 * до 5 = 14, путь 0 -> 2 -> 1 -> 3 -> 5
 *
 * @return Тестовый граф.
 */
Graph createSmallTestGraph() {
    Graph graph(6);

    graph.addEdge(0, 1, 4.0, true);
    graph.addEdge(0, 2, 2.0, true);
    graph.addEdge(1, 2, 1.0, true);
    graph.addEdge(1, 3, 5.0, true);
    graph.addEdge(2, 3, 8.0, true);
    graph.addEdge(2, 4, 10.0, true);
    graph.addEdge(3, 5, 6.0, true);

    return graph;
}

/**
 * @brief Тестирует создание графа, рёбра и степени вершин.
 */
void testGraphBasics() {
    Graph graph(4);

    assertTrue(graph.size() == 4, "Graph must contain 4 vertices");
    assertTrue(graph.edges() == 0, "New graph must contain 0 edges");

    graph.addEdge(0, 1, 3.0, true);
    graph.addEdge(1, 2, 5.0, true);

    assertTrue(graph.edges() == 2, "Graph must contain 2 logical edges");
    assertTrue(graph.degree(0) == 1, "Vertex 0 degree must be 1");
    assertTrue(graph.degree(1) == 2, "Vertex 1 degree must be 2");
    assertTrue(graph.degree(2) == 1, "Vertex 2 degree must be 1");

    const vector<Edge>& neighbors = graph.neighbors(0);

    assertTrue(neighbors.size() == 1, "Vertex 0 must have one neighbor");
    assertTrue(neighbors[0].to == 1, "Vertex 0 must be connected to vertex 1");
    assertDoubleEqual(neighbors[0].weight, 3.0, "Edge weight must be 3");

    cout << "testGraphBasics passed\n";
}

/**
 * @brief Тестирует граничные случаи при добавлении рёбер.
 */
void testInvalidEdges() {
    Graph graph(3);

    bool caughtOutOfRange = false;

    try {
        graph.addEdge(0, 5, 1.0, true);
    }
    catch (const out_of_range&) {
        caughtOutOfRange = true;
    }

    assertTrue(caughtOutOfRange, "Wrong vertex index must throw out_of_range");

    bool caughtNegativeWeight = false;

    try {
        graph.addEdge(0, 1, -2.0, true);
    }
    catch (const invalid_argument&) {
        caughtNegativeWeight = true;
    }

    assertTrue(caughtNegativeWeight, "Negative edge weight must throw invalid_argument");

    cout << "testInvalidEdges passed\n";
}

/**
 * @brief Тестирует алгоритм Дейкстры.
 */
void testDijkstra() {
    Graph graph = createSmallTestGraph();

    vector<double> distances = dijkstra(graph, 0);

    assertDoubleEqual(distances[0], 0.0, "Distance 0 -> 0 must be 0");
    assertDoubleEqual(distances[1], 3.0, "Distance 0 -> 1 must be 3");
    assertDoubleEqual(distances[2], 2.0, "Distance 0 -> 2 must be 2");
    assertDoubleEqual(distances[3], 8.0, "Distance 0 -> 3 must be 8");
    assertDoubleEqual(distances[4], 12.0, "Distance 0 -> 4 must be 12");
    assertDoubleEqual(distances[5], 14.0, "Distance 0 -> 5 must be 14");

    cout << "testDijkstra passed\n";
}

/**
 * @brief Тестирует LandmarkOracle.
 */
void testLandmarkOracle() {
    Graph graph = createSmallTestGraph();

    LandmarkOracle oracle(
        graph,
        2,
        LandmarkSelectionStrategy::SEQUENTIAL_INDEX
    );

    assertTrue(oracle.landmarkCount() == 2, "Oracle must contain 2 landmarks");
    assertTrue(oracle.memoryUsageBytes() > 0, "Oracle memory usage must be positive");

    vector<double> exactDistances = dijkstra(graph, 0);

    double exact = exactDistances[5];
    double approximate = oracle.query(0, 5);

    assertTrue(
        approximate >= exact,
        "Landmark approximate distance must not be less than exact distance"
    );

    assertDoubleEqual(approximate, 14.0, "Landmark distance 0 -> 5 must be 14");

    cout << "testLandmarkOracle passed\n";
}

/**
 * @brief Проверяет, что при увеличении числа landmarks результат не ухудшается.
 */
void testMoreLandmarksNotWorse() {
    Graph graph = createSmallTestGraph();

    LandmarkOracle oracleOne(
        graph,
        1,
        LandmarkSelectionStrategy::SEQUENTIAL_INDEX
    );

    LandmarkOracle oracleFour(
        graph,
        4,
        LandmarkSelectionStrategy::SEQUENTIAL_INDEX
    );

    double distanceOne = oracleOne.query(2, 5);
    double distanceFour = oracleFour.query(2, 5);

    assertTrue(
        distanceFour <= distanceOne,
        "More landmarks must not give worse result for the same strategy"
    );

    cout << "testMoreLandmarksNotWorse passed\n";
}

/**
 * @brief Тестирует оптимизированную стратегию выбора landmarks по степени.
 */
void testDegreeCentralityStrategy() {
    Graph graph = createSmallTestGraph();

    LandmarkOracle oracle(
        graph,
        1,
        LandmarkSelectionStrategy::DEGREE_CENTRALITY
    );

    const vector<int>& landmarks = oracle.getLandmarks();

    assertTrue(landmarks.size() == 1, "Oracle must contain exactly one landmark");

    int chosenVertex = landmarks[0];

    assertTrue(
        graph.degree(chosenVertex) >= 2,
        "Degree centrality must choose a vertex with high degree"
    );

    cout << "testDegreeCentralityStrategy passed\n";
}

/**
 * @brief Тестирует Thorup-Zwick oracle.
 */
void testThorupZwickOracle() {
    Graph graph = createSmallTestGraph();

    ThorupZwickOracle oracle(
        graph,
        2,
        ThorupZwickSamplingStrategy::RANDOM,
        42
    );

    assertTrue(oracle.getK() == 2, "Thorup-Zwick k must be 2");
    assertTrue(oracle.stretch() == 3, "Thorup-Zwick stretch for k=2 must be 3");
    assertTrue(oracle.memoryUsageBytes() > 0, "Thorup-Zwick memory must be positive");
    assertTrue(oracle.totalBunchSize() > 0, "Thorup-Zwick bunch size must be positive");

    vector<double> exactDistances = dijkstra(graph, 0);

    double exact = exactDistances[5];
    double approximate = oracle.query(0, 5);

    assertTrue(
        approximate != numeric_limits<double>::infinity(),
        "Thorup-Zwick query must return finite distance"
    );

    assertTrue(
        approximate >= exact,
        "Thorup-Zwick approximate distance must not be less than exact distance"
    );

    assertTrue(
        approximate <= oracle.stretch() * exact,
        "Thorup-Zwick approximate distance must satisfy stretch bound on test graph"
    );

    cout << "testThorupZwickOracle passed\n";
}

/**
 * @brief Тестирует оптимизированный Thorup-Zwick с degree-biased sampling.
 */
void testThorupZwickDegreeBiased() {
    Graph graph = createSmallTestGraph();

    ThorupZwickOracle oracle(
        graph,
        2,
        ThorupZwickSamplingStrategy::DEGREE_BIASED,
        42
    );

    double approximate = oracle.query(0, 5);

    assertTrue(
        approximate != numeric_limits<double>::infinity(),
        "Degree-biased Thorup-Zwick query must return finite distance"
    );

    assertTrue(
        oracle.memoryUsageBytes() > 0,
        "Degree-biased Thorup-Zwick memory must be positive"
    );

    cout << "testThorupZwickDegreeBiased passed\n";
}

/**
 * @brief Тестирует генератор связного случайного графа.
 */
void testGraphGenerator() {
    int vertexCount = 20;
    int edgeCount = 40;

    Graph graph = GraphGenerator::generateConnectedSparseGraph(
        vertexCount,
        edgeCount,
        1,
        10,
        123
    );

    assertTrue(graph.size() == vertexCount, "Generated graph must have correct vertex count");
    assertTrue(graph.edges() == edgeCount, "Generated graph must have correct edge count");

    vector<double> distances = dijkstra(graph, 0);

    for (int vertex = 0; vertex < graph.size(); ++vertex) {
        assertTrue(
            distances[vertex] != numeric_limits<double>::infinity(),
            "Generated graph must be connected"
        );
    }

    cout << "testGraphGenerator passed\n";
}

/**
 * @brief Точка входа для запуска базовых тестов.
 */
int main() {
    try {
        cout << "Running basic tests...\n\n";

        testGraphBasics();
        testInvalidEdges();
        testDijkstra();
        testLandmarkOracle();
        testMoreLandmarksNotWorse();
        testDegreeCentralityStrategy();
        testThorupZwickOracle();
        testThorupZwickDegreeBiased();
        testGraphGenerator();

        cout << "\nAll tests passed successfully!\n";
    }
    catch (const exception& exception) {
        cerr << "\n" << exception.what() << "\n";
        return 1;
    }

    return 0;
}

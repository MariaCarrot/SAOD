#ifndef THORUP_ZWICK_ORACLE_HPP
#define THORUP_ZWICK_ORACLE_HPP

#include "Graph.hpp"

#include <vector>
#include <unordered_map>
#include <cstddef>
#include <string>

/**
 * @brief Стратегия построения уровней Thorup-Zwick.
 */
enum class ThorupZwickSamplingStrategy {
    RANDOM,
    DEGREE_BIASED
};

/**
 * @brief Возвращает строковое название стратегии Thorup-Zwick.
 *
 * @param strategy Стратегия построения уровней.
 * @return Название стратегии.
 */
std::string thorupZwickStrategyToString(ThorupZwickSamplingStrategy strategy);

/**
 * @brief Приближённый оракул расстояний Thorup-Zwick.
 *
 * Классическая схема строит случайную иерархию множеств A0, A1, ..., Ak.
 * В оптимизированной версии DEGREE_BIASED вероятность попадания вершины
 * на следующий уровень зависит от степени вершины.
 */
class ThorupZwickOracle {
private:
    int vertexCount;
    int parameterK;
    ThorupZwickSamplingStrategy samplingStrategy;

    /**
     * @brief levels[i] хранит множество A_i.
     */
    std::vector<std::vector<int>> levels;

    /**
     * @brief pivot[i][v] хранит ближайшую вершину из A_i к вершине v.
     */
    std::vector<std::vector<int>> pivot;

    /**
     * @brief pivotDistance[i][v] хранит расстояние от v до pivot[i][v].
     */
    std::vector<std::vector<double>> pivotDistance;

    /**
     * @brief bunchDistances[v] хранит расстояния от v до вершин из bunch(v).
     */
    std::vector<std::unordered_map<int, double>> bunchDistances;

    /**
     * @brief Строит иерархию множеств A0, A1, ..., Ak.
     *
     * @param graph Граф.
     * @param randomSeed Seed для генератора случайных чисел.
     */
    void buildLevels(const Graph& graph, unsigned int randomSeed);

    /**
     * @brief Вычисляет ближайшие pivot-вершины для каждого уровня.
     *
     * @param allDistances Матрица кратчайших расстояний между всеми вершинами.
     */
    void buildPivots(const std::vector<std::vector<double>>& allDistances);

    /**
     * @brief Строит bunch(v) для каждой вершины v.
     *
     * @param allDistances Матрица кратчайших расстояний между всеми вершинами.
     */
    void buildBunches(const std::vector<std::vector<double>>& allDistances);

    /**
     * @brief Проверяет, принадлежит ли вершина заданному уровню.
     *
     * @param levelIndex Номер уровня.
     * @param vertex Вершина.
     * @return true, если вершина принадлежит уровню.
     */
    bool containsInLevel(int levelIndex, int vertex) const;

    /**
     * @brief Вычисляет среднюю степень вершины в графе.
     *
     * @param graph Граф.
     * @return Средняя степень.
     */
    double averageDegree(const Graph& graph) const;

public:
    /**
     * @brief Строит оракул Thorup-Zwick.
     *
     * @param graph Граф.
     * @param k Параметр оракула. Для классической версии stretch равен 2k - 1.
     * @param strategy Стратегия построения уровней.
     * @param randomSeed Seed для воспроизводимого случайного выбора множеств.
     */
    ThorupZwickOracle(
        const Graph& graph,
        int k,
        ThorupZwickSamplingStrategy strategy = ThorupZwickSamplingStrategy::RANDOM,
        unsigned int randomSeed = 42
    );

    /**
     * @brief Возвращает приближённое расстояние между двумя вершинами.
     *
     * @param source Начальная вершина.
     * @param target Конечная вершина.
     * @return Приближённое расстояние.
     */
    double query(int source, int target) const;

    /**
     * @brief Возвращает параметр k.
     *
     * @return Значение k.
     */
    int getK() const;

    /**
     * @brief Возвращает теоретический stretch классической схемы Thorup-Zwick.
     *
     * Для DEGREE_BIASED это значение используется как ориентир сравнения,
     * так как модифицированная выборка является эвристической.
     *
     * @return Stretch, равный 2k - 1.
     */
    int stretch() const;

    /**
     * @brief Возвращает примерный объём памяти, используемый оракулом.
     *
     * @return Количество байт.
     */
    std::size_t memoryUsageBytes() const;

    /**
     * @brief Возвращает суммарное количество элементов bunch.
     *
     * @return Суммарное количество элементов bunch.
     */
    std::size_t totalBunchSize() const;
};

#endif
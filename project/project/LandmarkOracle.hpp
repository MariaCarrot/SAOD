#ifndef LANDMARK_ORACLE_HPP
#define LANDMARK_ORACLE_HPP

#include "Graph.hpp"

#include <vector>
#include <cstddef>
#include <string>

/**
 * @brief Стратегия выбора landmark-вершин.
 */
enum class LandmarkSelectionStrategy {
    SEQUENTIAL_INDEX,
    DEGREE_CENTRALITY
};

/**
 * @brief Возвращает строковое название стратегии выбора landmark-вершин.
 *
 * @param strategy Стратегия выбора landmark-вершин.
 * @return Название стратегии.
 */
std::string strategyToString(LandmarkSelectionStrategy strategy);

/**
 * @brief Приближённый оракул расстояний на основе landmark-вершин.
 *
 * Оракул заранее считает расстояния от выбранных landmark-вершин
 * до всех остальных вершин. Запрос расстояния между двумя вершинами
 * оценивается как минимальный путь через одну из landmark-вершин.
 */
class LandmarkOracle {
private:
    std::vector<int> landmarks;
    std::vector<std::vector<double>> distancesFromLandmarks;

    /**
     * @brief Выбирает landmark-вершины по порядку индексов.
     *
     * @param graph Граф.
     * @param landmarkCount Количество landmark-вершин.
     */
    void selectSequentialIndexLandmarks(const Graph& graph, int landmarkCount);

    /**
     * @brief Выбирает landmark-вершины по наибольшей степени.
     *
     * @param graph Граф.
     * @param landmarkCount Количество landmark-вершин.
     */
    void selectDegreeCentralityLandmarks(const Graph& graph, int landmarkCount);

public:
    /**
     * @brief Строит оракул расстояний.
     *
     * @param graph Граф.
     * @param landmarkCount Количество landmark-вершин.
     * @param strategy Стратегия выбора landmark-вершин.
     */
    LandmarkOracle(
        const Graph& graph,
        int landmarkCount,
        LandmarkSelectionStrategy strategy
    );

    /**
     * @brief Возвращает приближённое расстояние между двумя вершинами.
     *
     * @param from Начальная вершина.
     * @param to Конечная вершина.
     * @return Приближённое расстояние.
     */
    double query(int from, int to) const;

    /**
     * @brief Возвращает примерный объём памяти, используемый оракулом.
     *
     * @return Количество байт.
     */
    std::size_t memoryUsageBytes() const;

    /**
     * @brief Возвращает количество landmark-вершин.
     *
     * @return Количество landmark-вершин.
     */
    int landmarkCount() const;

    /**
     * @brief Возвращает список выбранных landmark-вершин.
     *
     * @return Список landmark-вершин.
     */
    const std::vector<int>& getLandmarks() const;
};

#endif
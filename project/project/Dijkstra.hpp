#pragma once
#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include "Graph.hpp"

#include <vector>

/**
 * @brief Запускает алгоритм Дейкстры из одной стартовой вершины.
 *
 * @param graph Граф.
 * @param source Стартовая вершина.
 * @return Массив расстояний от source до всех вершин.
 */
std::vector<double> dijkstra(const Graph& graph, int source);

#endif

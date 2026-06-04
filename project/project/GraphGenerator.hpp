#ifndef GRAPH_GENERATOR_HPP
#define GRAPH_GENERATOR_HPP

#include "Graph.hpp"

/**
 * @brief Генератор тестовых графов.
 */
class GraphGenerator {
public:
    /**
     * @brief Генерирует случайный связный неориентированный взвешенный разреженный граф.
     *
     * Сначала создаётся цепочка из всех вершин, чтобы граф был связным.
     * Затем добавляются случайные дополнительные рёбра.
     *
     * @param vertexCount Количество вершин.
     * @param edgeCount Количество рёбер.
     * @param minWeight Минимальный вес ребра.
     * @param maxWeight Максимальный вес ребра.
     * @param randomSeed Seed генератора случайных чисел.
     * @return Случайный связный граф.
     */
    static Graph generateConnectedSparseGraph(
        int vertexCount,
        int edgeCount,
        int minWeight = 1,
        int maxWeight = 20,
        unsigned int randomSeed = 42
    );
};

#endif
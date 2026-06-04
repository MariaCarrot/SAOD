#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <string>

/**
 * @brief Ребро взвешенного графа.
 */
struct Edge {
    int to;
    double weight;
};

/**
 * @brief Класс для хранения взвешенного разреженного графа.
 *
 * Граф хранится в виде списка смежности.
 */
class Graph {
private:
    int vertexCount;
    int edgeCount;
    std::vector<std::vector<Edge>> adjacencyList;

public:
    /**
     * @brief Создаёт пустой граф.
     */
    Graph();

    /**
     * @brief Создаёт граф с заданным количеством вершин.
     *
     * @param vertices Количество вершин.
     */
    explicit Graph(int vertices);

    /**
     * @brief Добавляет ребро в граф.
     *
     * @param from Начальная вершина.
     * @param to Конечная вершина.
     * @param weight Вес ребра.
     * @param undirected Если true, ребро добавляется в обе стороны.
     */
    void addEdge(int from, int to, double weight, bool undirected = true);

    /**
     * @brief Загружает граф из файла.
     *
     * @param filename Имя файла.
     * @param undirected Если true, граф считается неориентированным.
     * @return Загруженный граф.
     */
    static Graph loadFromFile(const std::string& filename, bool undirected = true);

    /**
     * @brief Возвращает количество вершин.
     *
     * @return Количество вершин.
     */
    int size() const;

    /**
     * @brief Возвращает количество рёбер.
     *
     * @return Количество рёбер.
     */
    int edges() const;

    /**
     * @brief Возвращает список соседей вершины.
     *
     * @param vertex Номер вершины.
     * @return Список рёбер из данной вершины.
     */
    const std::vector<Edge>& neighbors(int vertex) const;

    /**
     * @brief Возвращает степень вершины.
     *
     * @param vertex Номер вершины.
     * @return Количество соседей вершины.
     */
    int degree(int vertex) const;

    /**
     * @brief Сохраняет граф в файл.
     *
     * Формат файла:
     * n m
     * from to weight
     *
     * @param filename Имя выходного файла.
     * @param undirected Если true, для неориентированного графа сохраняется только одно направление ребра.
     */
    void saveToFile(const std::string& filename, bool undirected = true) const;
};

#endif
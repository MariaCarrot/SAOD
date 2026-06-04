#include "DatasetGenerator.hpp"
#include "GraphGenerator.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <stdexcept>
using namespace std;

/**
 * @brief Проверяет, можно ли открыть папку через пробное создание файла.
 *
 * В C++14 нет filesystem, поэтому папку data нужно создать вручную.
 *
 * @param outputDirectory Папка для сохранения.
 */
static void checkOutputDirectory(const string& outputDirectory) {
    string testPath = outputDirectory + "/test_write.tmp";

    ofstream testFile(testPath);

    if (!testFile.is_open()) {
        throw runtime_error(
            "Cannot write to directory: " + outputDirectory +
            ". Create this directory manually."
        );
    }

    testFile.close();

    remove(testPath.c_str());
}

/**
 * @brief Генерирует один граф и сохраняет его в файл.
 *
 * @param outputDirectory Папка для сохранения.
 * @param filename Имя файла.
 * @param vertexCount Количество вершин.
 * @param edgeCount Количество рёбер.
 * @param seed Seed генератора.
 */
static void generateAndSaveGraph(
    const string& outputDirectory,
    const string& filename,
    int vertexCount,
    int edgeCount,
    unsigned int seed
) {
    Graph graph = GraphGenerator::generateConnectedSparseGraph(
        vertexCount,
        edgeCount,
        1,
        20,
        seed
    );

    string fullPath = outputDirectory + "/" + filename;

    graph.saveToFile(fullPath, true);

    double density = 0.0;

    if (vertexCount > 1) {
        int maxEdges = vertexCount * (vertexCount - 1) / 2;
        density = static_cast<double>(edgeCount) / static_cast<double>(maxEdges);
    }

    cout << "Saved: " << fullPath << "\n";
    cout << "  vertices: " << vertexCount << "\n";
    cout << "  edges:    " << edgeCount << "\n";
    cout << "  density:  " << density << "\n";
}

/**
 * @brief Генерирует несколько случайных разреженных графов.
 */
void DatasetGenerator::generateRandomGraphFiles(const string& outputDirectory) {
    checkOutputDirectory(outputDirectory);

    generateAndSaveGraph(
        outputDirectory,
        "random_1000_3000.txt",
        1000,
        3000,
        202
    );

    generateAndSaveGraph(
        outputDirectory,
        "random_2000_6000.txt",
        2000,
        6000,
        303
    );

    generateAndSaveGraph(
        outputDirectory,
        "random_3000_9000.txt",
        3000,
        9000,
        504
    );

    generateAndSaveGraph(
        outputDirectory,
        "random_5000_15000.txt",
        5000,
        15000,
        505
    );
}
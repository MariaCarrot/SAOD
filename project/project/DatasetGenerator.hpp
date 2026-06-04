#pragma once
#ifndef DATASET_GENERATOR_HPP
#define DATASET_GENERATOR_HPP

#include <string>

/**
 * @brief Генератор набора файлов со случайными разреженными графами.
 */
class DatasetGenerator {
public:
    /**
     * @brief Генерирует несколько случайных графов разных размеров и плотности
     * и сохраняет их в указанную папку.
     *
     * @param outputDirectory Папка для сохранения графов.
     */
    static void generateRandomGraphFiles(const std::string& outputDirectory);
};

#endif

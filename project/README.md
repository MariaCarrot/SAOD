# Distance Oracles for Sparse Graphs

## Структура каталогов

Примерная структура проекта:

```text
project/
│
├── project/
│   │ 	
│   ├── data/
│   │	│
│   │   ├── datasets.txt
│   │   ├── random_1000_3000.txt
│   │   ├── facebook_1000_3000.txt
│   │   ├── twitter_220_3000.txt
│   │   ├── roadNet-PA_1000_1303.txt
│   │   └── ...
│   │
│   ├── My_project.cpp
│   ├── Graph.hpp
│   ├── Graph.cpp
│   ├── Dijkstra.hpp
│   ├── Dijkstra.cpp
│   ├── LandmarkOracle.hpp
│   ├── LandmarkOracle.cpp
│   ├── ThorupZwickOracle.hpp
│   ├── ThorupZwickOracle.cpp
│   ├── GraphGenerator.hpp
│   ├── GraphGenerator.cpp
│   ├── DatasetGenerator.hpp
│   ├── DatasetGenerator.cpp
│   ├── Timer.hpp
│   ├── all_query_results.csv
│   └── all_summary_results.csv
│
├── convert_snap.py
├── plot_analysis.py
├── TestOracle.cpp
└── README.md
```

Назначение основных файлов:

* `My_project.cpp` — основной файл программы, запускающий эксперименты;
* `Graph.hpp`, `Graph.cpp` — структура графа;
* `Dijkstra.hpp`, `Dijkstra.cpp` — реализация алгоритма Дейкстры;
* `LandmarkOracle.hpp`, `LandmarkOracle.cpp` — реализация landmark-оракулов;
* `ThorupZwickOracle.hpp`, `ThorupZwickOracle.cpp` — реализация оракула Thorup–Zwick;
* `GraphGenerator.hpp`, `GraphGenerator.cpp` — генерация случайных разреженных графов;
* `DatasetGenerator.hpp`, `DatasetGenerator.cpp` — генерация файлов со случайными графами;
* `Timer.hpp` — измерение времени выполнения;
* `convert_snap.py` — преобразование графов SNAP в формат проекта;
* `plot_analysis.py` — построение графиков по результатам экспериментов;
* `data/datasets.txt` — список графов, на которых запускаются эксперименты.

В решении также может присутствовать отдельный проект для тестирования:

```text
TestOracle/
```

Он содержит базовые тесты корректности реализации.

## Инструкция по сборке

### Сборка в Visual Studio

1. Открыть решение проекта в Visual Studio.
2. Выбрать конфигурацию `Debug` или `Release`.
3. Убедиться, что используется стандарт C++14.
4. Назначить проект `My_project` запускаемым проектом.
5. Выполнить сборку.

Проект использует только стандартную библиотеку C++.

## Инструкция по запуску программы

### Запуск основного эксперимента

1. Убедиться, что в папке `data` находятся входные графы.
2. Убедиться, что файл `data/datasets.txt` содержит список графов для эксперимента.
3. Назначить проект `My_project` запускаемым.
4. Запустить программу.

После завершения работы создаются файлы:

```text
all_summary_results.csv
all_query_results.csv
```

Файл `all_summary_results.csv` содержит сводные результаты экспериментов.

Файл `all_query_results.csv` содержит подробные результаты по каждому запросу расстояния.

### Запуск тестов

Для запуска тестов нужно:

1. Назначить проект `TestOracle` запускаемым.
2. Выполнить сборку.
3. Запустить проект.

При успешном прохождении тестов программа выводит сообщение:

```text
All tests passed successfully!
```

### Построение графиков

После выполнения основного эксперимента можно построить графики по файлу:

```text
all_summary_results.csv
```

Для этого используется Python-скрипт:

```text
plot_analysis.py
```

Скрипт строит графики для анализа времени построения, времени ответа, ошибки и использования памяти.

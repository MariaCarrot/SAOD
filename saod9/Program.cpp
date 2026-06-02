#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <random>
#include <vector>
#include <functional>
#include <numeric>
#include <mutex>
#include <queue>
#include <atomic>
#include <string>
#include <windows.h>
using namespace std;
typedef unsigned long long int bigint;

bigint fibonacci(bigint n)
{
    return n < 2 ? n : fibonacci(n - 1) + fibonacci(n - 2);
}

bool special(int n)
{
    bigint sum = 0;

    for (int i = 0; i != n; i++)
        sum += fibonacci(i);

    return sum % 2 == 0;
}

size_t single(const vector<int>& v)
{
    return count_if(v.begin(), v.end(), [](const auto& el)
        {
            return special(el);
        });
}

// Блочная реализация: каждый поток заранее получает свой диапазон элементов
size_t block(const vector<int>& v, size_t n_threads)
{
    vector<size_t> results(n_threads, 0);
    auto lambda = [&v, &results](size_t a, size_t b, size_t thread_id)
    {
        size_t sum = count_if(v.begin() + a, v.begin() + b, [](const auto& el)
            {return special(el);});

        results[thread_id] = sum;
    };

    vector<thread> threads(n_threads);
    size_t part_size = v.size() / n_threads;
    size_t a = 0;
    size_t b = 0;

    for (size_t t = 0; t != n_threads; t++, a = b)
    {
        b = (t == n_threads - 1) ? v.size() : a + part_size;
        threads[t] = thread(lambda, a, b, t);
    }

    for (auto& t : threads)
        t.join();

    return accumulate(results.begin(), results.end(), 0ULL);
}

// Динамическая очередь + mutex: все элементы лежат в общей очереди. Поток берёт одно число, считает special(), потом берёт следующее
size_t this_is_the_way(const vector<int>& v, size_t n_threads)
{
    queue<int> tasks;
    for (int x : v)
        tasks.push(x);

    mutex mtx;
    vector<size_t> results(n_threads, 0);

    auto worker = [&tasks, &mtx, &results](size_t thread_id)
    {
        size_t local_count = 0;
        while (true)
        {
            int value;
            {
                lock_guard<mutex> lock(mtx);
                if (tasks.empty())
                    break;

                value = tasks.front();
                tasks.pop();
            }
            if (special(value))
                local_count++;
        }
        results[thread_id] = local_count;
    };

    vector<thread> threads;
    for (size_t i = 0; i < n_threads; i++)
        threads.emplace_back(worker, i);

    for (auto& t : threads)
        t.join();

    return accumulate(results.begin(), results.end(), 0ULL);
}

// Динамическое распределение через atomic: есть общий индекс next_index. Каждый поток атомарно получает номер следующего элемента
size_t is_this_the_way(const vector<int>& v, size_t n_threads)
{
    atomic<size_t> next_index(0);
    vector<size_t> results(n_threads, 0);

    auto worker = [&v, &next_index, &results](size_t thread_id)
    {
        size_t local_count = 0;
        while (true)
        {
            size_t index = next_index.fetch_add(1);
            if (index >= v.size())
                break;

            if (special(v[index]))
                local_count++;
        }
        results[thread_id] = local_count;
    };

    vector<thread> threads;
    for (size_t i = 0; i < n_threads; i++)
        threads.emplace_back(worker, i);

    for (auto& t : threads)
        t.join();

    return accumulate(results.begin(), results.end(), 0ULL);
}
long long measure_time(const function<size_t(const vector<int>&, size_t)>& func, const vector<int>& v, size_t n_threads, size_t& result)
{
    auto start = chrono::steady_clock::now();
    result = func(v, n_threads);
    auto end = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}
//long long measure_time(const function<size_t(const vector<int>&, size_t)>& func, const vector<int>& v, size_t n_threads, size_t& result)
//{
//    clock_t start = clock();
//    result = func(v, n_threads);
//    clock_t end = clock();
//
//    double total_cpu_time = static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000.0;
//    return static_cast<long long>(total_cpu_time / n_threads);
//}

int main()
{
    DWORD_PTR mask = 0x0F; // 00001111 — разрешаем только первые 4 логических процессора
    if (!SetProcessAffinityMask(GetCurrentProcess(), mask))
    {
        cerr << "Failed to set process affinity" << endl;
        return 1;
    }
    vector<int> v(50);
    mt19937_64 gen;
    gen.seed(1);
    poisson_distribution<> pd(4);

    cout << "# Array: ";

    for (auto& item : v)
    {
        int value = 40 + pd(gen);
        item = value < 53 ? value : 53;
        cout << item << ' ';
    }

    cout << endl << endl;
    vector<function<size_t(const vector<int>&, size_t)>> functions =
    {
        block, this_is_the_way, is_this_the_way
    };

    vector<string> names =
    {
        "block", "mutex_queue", "atomic_index"
    };

    auto start_single = chrono::steady_clock::now();
    size_t single_result = single(v);
    auto end_single = chrono::steady_clock::now();

    long long single_time = chrono::duration_cast<chrono::milliseconds>(
        end_single - start_single
        ).count();

    cout << "Single result: " << single_result << endl;
    cout << "Single time: " << single_time << " ms" << endl << endl;

    cout << "threads\tmethod\t\ttime(ms)\tresult\tspeedup\t\tefficiency" << endl;

    for (size_t th_number : { 2, 3, 4})
    {
        for (size_t i = 0; i < functions.size(); i++)
        {
            size_t result = 0;

            long long time = measure_time(functions[i], v, th_number, result);

            double speedup = static_cast<double>(single_time) / time;
            double efficiency = speedup / th_number;

            cout << th_number << '\t'
                << names[i] << "\t";

            if (names[i].size() < 8)
                cout << '\t';

            cout << time << "\t\t"
                << result << "\t"
                << speedup << "\t\t"
                << efficiency
                << endl;
        }
    }
    return 0;
}

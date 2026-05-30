#include <iostream>
#include <string>
#include <string_view>
#include <chrono>
#include <fstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <array>

using namespace std;

const size_t NOT_FOUND = string::npos;

// наивный поиск
size_t naive_search(const string& text, const string& pattern)
{
    size_t n = text.size();
    size_t m = pattern.size();

    if (m == 0) return 0;
    if (m > n) return NOT_FOUND;

    for (size_t i = 0; i + m <= n; i++)
    {
        size_t j = 0;

        while (j < m && text[i + j] == pattern[j])
            j++;

        if (j == m)
            return i;
    }

    return NOT_FOUND;
}

// КМП
vector<size_t> build_lps(const string& pattern)
{
    size_t m = pattern.size();
    vector<size_t> lps(m, 0);

    size_t len = 0;
    size_t i = 1;

    while (i < m)
    {
        if (pattern[i] == pattern[len])
        {
            len++;
            lps[i] = len;
            i++;
        }
        else
        {
            if (len != 0)
                len = lps[len - 1];
            else
            {
                lps[i] = 0;
                i++;
            }
        }
    }
    return lps;
}

size_t kmp_search(const string& text, const string& pattern, const vector<size_t>& lps)
{
    size_t n = text.size();
    size_t m = pattern.size();

    if (m == 0) return 0;
    if (m > n) return NOT_FOUND;

    size_t i = 0; // индекс в text
    size_t j = 0; // индекс в pattern

    while (i < n)
    {
        if (text[i] == pattern[j])
        {
            i++;
            j++;

            if (j == m)
                return i - j;
        }
        else
        {
            if (j != 0)
                j = lps[j - 1];
            else
                i++;
        }
    }
    return NOT_FOUND;
}

// БОЙЕР-МУР
array<int, 256> build_bad_char(const string& pattern)
{
    array<int, 256> bad;

    for (int i = 0; i < 256; i++)
        bad[i] = -1;

    for (int i = 0; i < (int)pattern.size(); i++)
    {
        unsigned char c = (unsigned char)pattern[i];
        bad[c] = i;
    }
    return bad;
}

void preprocess_good_suffix(
    const string& pattern,
    vector<int>& shift,
    vector<int>& border
)
{
    int m = (int)pattern.size();

    int i = m;
    int j = m + 1;

    border[i] = j;

    while (i > 0)
    {
        while (j <= m && pattern[i - 1] != pattern[j - 1])
        {
            if (shift[j] == 0) shift[j] = j - i;

            j = border[j];
        }

        i--;
        j--;

        border[i] = j;
    }
}

void preprocess_case2(vector<int>& shift, vector<int>& border, int m)
{
    int j = border[0];

    for (int i = 0; i <= m; i++)
    {
        if (shift[i] == 0) shift[i] = j;

        if (i == j) j = border[j];
    }
}

vector<int> build_good_suffix(const string& pattern)
{
    int m = (int)pattern.size();

    vector<int> shift(m + 1, 0);
    vector<int> border(m + 1, 0);

    preprocess_good_suffix(pattern, shift, border);
    preprocess_case2(shift, border, m);

    return shift;
}

size_t bm_search(
    const string& text,
    const string& pattern,
    const array<int, 256>& bad,
    const vector<int>& good_suffix
)
{
    int n = (int)text.size();
    int m = (int)pattern.size();

    if (m == 0) return 0;
    if (m > n) return NOT_FOUND;

    int s = 0;

    while (s <= n - m)
    {
        int j = m - 1;

        while (j >= 0 && pattern[j] == text[s + j])
            j--;

        if (j < 0) return s;
        else
        {
            unsigned char c = (unsigned char)text[s + j];

            int bad_shift = j - bad[c];
            int good_shift = good_suffix[j + 1];

            int shift = max(bad_shift, good_shift);

            if (shift < 1) shift = 1;

            s += shift;
        }
    }

    return NOT_FOUND;
}

// сбор статистики 
double average(const vector<double>& v)
{
    double sum = accumulate(v.begin(), v.end(), 0.0);
    return sum / v.size();
}

double median(vector<double> v)
{
    sort(v.begin(), v.end());

    size_t n = v.size();

    if (n % 2 == 1)
        return v[n / 2];

    return (v[n / 2 - 1] + v[n / 2]) / 2.0;
}

void print_stats(const string& name, const vector<double>& times)
{
    double avg = average(times);
    double med = median(times);

    auto min_it = min_element(times.begin(), times.end());
    auto max_it = max_element(times.begin(), times.end());

    cout << "\n" << name << endl;
    cout << "average: " << avg << " us" << endl;
    cout << "min:     " << *min_it << " us" << endl;
    cout << "max:     " << *max_it << " us" << endl;
    cout << "median:  " << med << " us" << endl;
}

void write_csv(
    const vector<double>& naive_times,
    const vector<double>& kmp_times,
    const vector<double>& bm_times
)
{
    ofstream fout("times.csv");

    fout << "algorithm,time_us\n";

    for (double t : naive_times)
        fout << "Naive," << t << "\n";

    for (double t : kmp_times)
        fout << "KMP," << t << "\n";

    for (double t : bm_times)
        fout << "BM," << t << "\n";
}

// время
template <typename Func>
vector<double> measure_algorithm(
    const string& name,
    Func func,
    size_t runs,
    vector<size_t>& indexes
)
{
    vector<double> times(runs);
    cout << "\n" << name << "\n";
    volatile size_t control_sum = 0;

    for (size_t i = 0; i < runs; i++)
    {
        auto time_start = chrono::steady_clock::now();
        size_t index = func();
        auto time_end = chrono::steady_clock::now();
        double time_us = chrono::duration<double, micro>(
            time_end - time_start
            ).count();

        indexes[i] = index;
        times[i] = time_us;

        if (index != NOT_FOUND)
            control_sum += index;

        cout << i + 1 << ") index = ";

        if (index == NOT_FOUND)
            cout << "not found";
        else
            cout << index;

        cout << ", time = " << time_us << " us\n";
    }

    cout << "control sum: " << control_sum << endl;
    return times;
}


int main()
{
    string filename = "D:/прога/САОД/ConsoleApplication2_find/simplewiki-20260201.txt";

    string text;
    string pattern = "and is the second single from their greatest hits";

    ifstream fin(filename, ios::binary);

    if (!fin.is_open())
    {
        cout << "File not open: " << filename << endl;
        return 0;
    }

    text.assign(
        istreambuf_iterator<char>(fin),
        istreambuf_iterator<char>()
    );

    cout << "File loaded: " << filename << endl;
    cout << "Text size: " << text.size() << " bytes" << endl;
    cout << "Pattern: " << pattern << endl;
    cout << "Pattern size: " << pattern.size() << endl;

    const size_t runs = 25;

    vector<size_t> naive_indexes(runs);
    vector<size_t> kmp_indexes(runs);
    vector<size_t> bm_indexes(runs);

    // таблицы для КМП и БМ.
    vector<size_t> lps = build_lps(pattern);

    array<int, 256> bad_char = build_bad_char(pattern);
    vector<int> good_suffix = build_good_suffix(pattern);

    vector<double> naive_times = measure_algorithm(
        "Naive",
        [&]()
        {
            return naive_search(text, pattern);
        },
        runs,
            naive_indexes
            );

    vector<double> kmp_times = measure_algorithm(
        "KMP",
        [&]()
        {
            return kmp_search(text, pattern, lps);
        },
        runs,
            kmp_indexes
            );

    vector<double> bm_times = measure_algorithm(
        "Boyer-Moore",
        [&]()
        {
            return bm_search(text, pattern, bad_char, good_suffix);
        },
        runs,
            bm_indexes
            );

    print_stats("Naive stats", naive_times);
    print_stats("KMP stats", kmp_times);
    print_stats("Boyer-Moore stats", bm_times);

    write_csv(naive_times, kmp_times, bm_times);

    cout << "\nResults were saved to times.csv" << endl;

    return 0;
}

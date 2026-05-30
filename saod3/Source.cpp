#include <array>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

struct ITrie {
    virtual ~ITrie() {}
    virtual void insert(const string& str) = 0;
    virtual size_t get(const string& str) const = 0;
    virtual size_t nodes() const = 0;
    virtual size_t size() const = 0;
};

class ArrayTrie : public ITrie {
private:
    static const uint32_t NO_CHILD = 0;
    static const size_t ALPHABET = 54;

    struct Node {
        uint32_t next[ALPHABET];
        uint32_t word_count;

        Node() : word_count(0) {
            memset(next, 0, sizeof(next));
        }
    };

    vector<Node> tree;
    size_t unique_words;
    uint32_t current_node;
    bool in_word;

    // Таблица быстрых индексов символов
    // Для несловарного символа значение -1
    static signed char char_to_index[256];
    static bool table_ready;

    static void prepare_table() {
        if (table_ready) return;

        for (int i = 0; i < 256; ++i)
            char_to_index[i] = -1;

        for (int c = 'a'; c <= 'z'; ++c)
            char_to_index[c] = static_cast<signed char>(c - 'a');

        for (int c = 'A'; c <= 'Z'; ++c)
            char_to_index[c] = static_cast<signed char>(26 + c - 'A');

        char_to_index[static_cast<unsigned char>('\'')] = 52;
        char_to_index[static_cast<unsigned char>('-')] = 53;
        table_ready = true;
    }

    static int char_index(unsigned char ch) {
        return char_to_index[ch];
    }

public:
    ArrayTrie(size_t reserve_nodes = 0)
        : unique_words(0), current_node(0), in_word(false) {
        prepare_table();

        if (reserve_nodes > 0)
            tree.reserve(reserve_nodes);

        tree.push_back(Node()); // root
    }

    void insert(const string& str) {
        uint32_t v = 0;

        for (size_t i = 0; i < str.size(); ++i) {
            unsigned char ch = static_cast<unsigned char>(str[i]);
            int idx = char_index(ch);
            if (idx < 0) continue;

            uint32_t to = tree[v].next[static_cast<size_t>(idx)];

            if (to == NO_CHILD) {
                uint32_t new_index = static_cast<uint32_t>(tree.size());
                tree[v].next[static_cast<size_t>(idx)] = new_index;
                tree.push_back(Node());
                v = new_index;
            }
            else v = to;
        }

        if (tree[v].word_count == 0) ++unique_words;
        ++tree[v].word_count;
    }

    void build_from_text_fast(const string& text) {
        const char* data = text.data();
        size_t n = text.size();

        uint32_t v = 0;
        bool inside = false;

        for (size_t i = 0; i < n; ++i) {
            unsigned char ch = static_cast<unsigned char>(data[i]);
            int idx = char_to_index[ch];

            if (idx >= 0) {
                uint32_t to = tree[v].next[static_cast<size_t>(idx)];

                if (to == NO_CHILD) {
                    uint32_t new_index = static_cast<uint32_t>(tree.size());
                    tree[v].next[static_cast<size_t>(idx)] = new_index;
                    tree.push_back(Node());
                    v = new_index;
                }
                else v = to;

                inside = true;
            }
            else {
                if (inside) {
                    if (tree[v].word_count == 0) ++unique_words;
                    ++tree[v].word_count;

                    v = 0;
                    inside = false;
                }
            }
        }

        if (inside) {
            if (tree[v].word_count == 0) ++unique_words;
            ++tree[v].word_count;
        }

        current_node = 0;
        in_word = false;
    }

    void process_char(unsigned char ch) {
        int idx = char_index(ch);

        if (idx >= 0) {
            uint32_t to = tree[current_node].next[static_cast<size_t>(idx)];

            if (to == NO_CHILD) {
                uint32_t new_index = static_cast<uint32_t>(tree.size());
                tree[current_node].next[static_cast<size_t>(idx)] = new_index;
                tree.push_back(Node());
                current_node = new_index;
            }
            else current_node = to;

            in_word = true;
        }
        else finish_word();
    }

    void finish_word() {
        if (in_word) {
            if (tree[current_node].word_count == 0) ++unique_words;
            ++tree[current_node].word_count;

            current_node = 0;
            in_word = false;
        }
    }

    size_t get(const string& str) const {
        uint32_t v = 0;

        for (size_t i = 0; i < str.size(); ++i) {
            unsigned char ch = static_cast<unsigned char>(str[i]);
            int idx = char_to_index[ch];

            if (idx < 0) return 0;

            uint32_t to = tree[v].next[static_cast<size_t>(idx)];
            if (to == NO_CHILD) return 0;

            v = to;
        }

        return tree[v].word_count;
    }

    size_t nodes() const {
        return tree.size();
    }

    size_t size() const {
        return unique_words;
    }
};

signed char ArrayTrie::char_to_index[256];
bool ArrayTrie::table_ready = false;

static bool is_word_char(unsigned char ch) {
    return (ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z') ||
        ch == '\'' ||
        ch == '-';
}

static string read_file(const string& path) {
    ifstream fin(path.c_str(), ios::binary);

    if (!fin.is_open()) throw runtime_error("Cannot open file: " + path);

    fin.seekg(0, ios::end);
    streamsize file_size = fin.tellg();
    fin.seekg(0, ios::beg);

    string text;
    text.resize(static_cast<size_t>(file_size));

    if (file_size > 0) fin.read(&text[0], file_size);

    return text;
}

template <class Dict>
static void scan_words_to_dict(const string& text, Dict& dict) {
    string current_word;

    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(text[i]);

        if (is_word_char(ch)) current_word += static_cast<char>(ch);
        else {
            if (!current_word.empty()) {
                dict.insert(current_word);
                current_word.clear();
            }
        }
    }

    if (!current_word.empty()) dict.insert(current_word);
}


static void scan_words_to_trie_fast(const string& text, ArrayTrie& trie) {
    trie.build_from_text_fast(text);
}

class HashDict {
private:
    struct Entry {
        int hashCode;
        int next;
        string key;
        size_t value;

        Entry() : hashCode(-1), next(-1), key(), value(0) {}
        Entry(int h, int n, const string& k, size_t v)
            : hashCode(h), next(n), key(k), value(v) {}
    };

    vector<int> buckets;
    vector<Entry> entries;
    size_t unique_words;

    static int make_hash(const string& word) {
        int hash = 0;

        for (size_t i = 0; i < word.size(); ++i) {
            hash = hash * 31 + static_cast<unsigned char>(word[i]);
        }

        return hash & 0x7FFFFFFF;
    }

    void initialize(size_t reserve_words) {
        size_t bucket_count = reserve_words * 2 + 1;

        if (bucket_count < 17) {
            bucket_count = 17;
        }

        buckets.assign(bucket_count, -1);

        if (reserve_words > 0) {
            entries.reserve(reserve_words);
        }
    }

    void resize_table() {
        size_t new_bucket_count = buckets.size() * 2 + 1;
        vector<int> new_buckets(new_bucket_count, -1);

        for (size_t i = 0; i < entries.size(); ++i) {
            int bucket = entries[i].hashCode % static_cast<int>(new_bucket_count);
            entries[i].next = new_buckets[bucket];
            new_buckets[bucket] = static_cast<int>(i);
        }

        buckets.swap(new_buckets);
    }

public:
    HashDict(size_t reserve_words = 0) : unique_words(0) {
        initialize(reserve_words);
    }

    void insert(const string& word) {
        if (buckets.empty()) {
            initialize(17);
        }

        // Если таблица стала слишком заполненной, увеличиваем число корзин
        if ((entries.size() + 1) * 4 > buckets.size() * 3) {
            resize_table();
        }

        int hashCode = make_hash(word);
        int bucket = hashCode % static_cast<int>(buckets.size());

        for (int i = buckets[bucket]; i >= 0; i = entries[static_cast<size_t>(i)].next) {
            if (entries[static_cast<size_t>(i)].hashCode == hashCode &&
                entries[static_cast<size_t>(i)].key == word) {
                ++entries[static_cast<size_t>(i)].value;
                return;
            }
        }

        Entry new_entry(hashCode, buckets[bucket], word, 1);
        entries.push_back(new_entry);
        buckets[bucket] = static_cast<int>(entries.size() - 1);
        ++unique_words;
    }

    size_t get(const string& word) const {
        if (buckets.empty()) {
            return 0;
        }

        int hashCode = make_hash(word);
        int bucket = hashCode % static_cast<int>(buckets.size());

        for (int i = buckets[bucket]; i >= 0; i = entries[static_cast<size_t>(i)].next) {
            if (entries[static_cast<size_t>(i)].hashCode == hashCode &&
                entries[static_cast<size_t>(i)].key == word) {
                return entries[static_cast<size_t>(i)].value;
            }
        }

        return 0;
    }

    size_t size() const {
        return unique_words;
    }
};

struct RunResult {
    double seconds;
    size_t unique_words;
    size_t nodes;
    size_t word_count;

    RunResult() : seconds(0), unique_words(0), nodes(0), word_count(0) {}
    RunResult(double s, size_t u, size_t n, size_t w)
        : seconds(s), unique_words(u), nodes(n), word_count(w) {}
};

static RunResult build_hash_timed(const string& text, const string& target_word, size_t reserve_words) {
    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

    HashDict dict(reserve_words);
    scan_words_to_dict(text, dict);

    chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
    double seconds = chrono::duration<double>(t2 - t1).count();

    return RunResult(seconds, dict.size(), 0, dict.get(target_word));
}

static RunResult build_trie_timed(const string& text, const string& target_word, size_t reserve_nodes) {
    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

    ArrayTrie trie(reserve_nodes);
    scan_words_to_trie_fast(text, trie);

    chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
    double seconds = chrono::duration<double>(t2 - t1).count();

    return RunResult(seconds, trie.size(), trie.nodes(), trie.get(target_word));
}

static double mean(const vector<double>& values) {
    if (values.empty()) return 0.0;

    double sum = accumulate(values.begin(), values.end(), 0.0);
    return sum / static_cast<double>(values.size());
}

static void print_vector(const string& name, const vector<double>& values) {
    cout << name << ": {";

    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) cout << ", ";
        cout << fixed << setprecision(6) << values[i];
    }

    cout << "}" << endl;
}

static bool check_expected(
    const RunResult& hash_result,
    const RunResult& trie_result,
    size_t expected_word_count,
    size_t expected_unique,
    size_t expected_nodes
) {
    bool ok = true;

    ok = ok && (hash_result.word_count == expected_word_count);
    ok = ok && (trie_result.word_count == expected_word_count);
    ok = ok && (hash_result.unique_words == expected_unique);
    ok = ok && (trie_result.unique_words == expected_unique);
    ok = ok && (trie_result.nodes == expected_nodes);
    ok = ok && (hash_result.unique_words == trie_result.unique_words);
    ok = ok && (hash_result.word_count == trie_result.word_count);

    return ok;
}

static void print_one_check(
    const string& title,
    const string& text,
    const string& word,
    size_t expected_word_count,
    size_t expected_unique,
    size_t expected_nodes,
    size_t reserve_words,
    size_t reserve_nodes
) {
    cout << "========================================" << endl;
    cout << "One run check: " << title << endl;
    cout << "Target word: " << word << endl;

    RunResult hash_result = build_hash_timed(text, word, reserve_words);
    RunResult trie_result = build_trie_timed(text, word, reserve_nodes);

    cout << "hash time: " << fixed << setprecision(6) << hash_result.seconds << " seconds" << endl;
    cout << "trie time: " << fixed << setprecision(6) << trie_result.seconds << " seconds" << endl;
    cout << "hash word count: " << hash_result.word_count << endl;
    cout << "trie word count: " << trie_result.word_count << endl;
    cout << "expected word count: " << expected_word_count << endl;
    cout << "hash unique words: " << hash_result.unique_words << endl;
    cout << "trie unique words: " << trie_result.unique_words << endl;
    cout << "expected unique words: " << expected_unique << endl;
    cout << "trie nodes: " << trie_result.nodes << endl;
    cout << "expected nodes: " << expected_nodes << endl;

    bool ok = check_expected(
        hash_result,
        trie_result,
        expected_word_count,
        expected_unique,
        expected_nodes
    );

    cout << "Check: " << (ok ? "OK" : "FAILED") << endl;
    cout << endl;
}

static void run_benchmark_10_times(
    const string& title,
    const string& text,
    const string& word,
    size_t reserve_words,
    size_t reserve_nodes
) {
    cout << "========================================" << endl;
    cout << "Benchmark: " << title << endl;
    cout << "Runs: 10" << endl;
    cout << "Target word: " << word << endl << endl;

    int runs = 10;
    vector<double> hash_times;
    vector<double> trie_times;

    hash_times.reserve(static_cast<size_t>(runs));
    trie_times.reserve(static_cast<size_t>(runs));

    RunResult last_hash;
    RunResult last_trie;

    for (int i = 1; i <= runs; ++i) {
        last_hash = build_hash_timed(text, word, reserve_words);
        last_trie = build_trie_timed(text, word, reserve_nodes);

        hash_times.push_back(last_hash.seconds);
        trie_times.push_back(last_trie.seconds);

        bool same = (last_hash.unique_words == last_trie.unique_words) &&
            (last_hash.word_count == last_trie.word_count);

        cout << "Run " << i << ": ";
        cout << "hash=" << fixed << setprecision(6) << last_hash.seconds << "s, ";
        cout << "trie=" << fixed << setprecision(6) << last_trie.seconds << "s, ";
        cout << "same_counts=" << same << endl;
    }

    cout << endl;
    print_vector("hash", hash_times);
    print_vector("trie", trie_times);

    double hash_mean = mean(hash_times);
    double trie_mean = mean(trie_times);

    cout << "hash mean: " << fixed << setprecision(6) << hash_mean << " seconds" << endl;
    cout << "trie mean: " << fixed << setprecision(6) << trie_mean << " seconds" << endl;

    if (trie_mean > 0) {
        cout << "Average speedup: " << fixed << setprecision(6)
            << hash_mean / trie_mean << "x" << endl;
    }
    cout << endl;
}

int main() {
    try {
        cout << boolalpha;
        string word = "Dubna";
        string simplewiki_path = "D:/прога/САОД/Project4/simplewiki-20260201.txt";
        string engwiki_path = "D:/прога/САОД/Project4/engwiki-ascii-20260201_1gb.txt";

        string example_text = "are they the most fun and these are a fun.";
        print_one_check("example string", example_text, word, 0, 8, 19, 0, 0);

        cout << "Reading file: " << simplewiki_path << endl;
        string simple_text = read_file(simplewiki_path);
        cout << "File size: " << simple_text.size() << " bytes" << endl << endl;

        print_one_check("simplewiki-20260201.txt", simple_text, word, 17, 610625, 1939049, 610625, 1939049);

        cout << "Reading file: " << engwiki_path << endl;
        string eng_text = read_file(engwiki_path);
        cout << "File size: " << eng_text.size() << " bytes" << endl << endl;

        print_one_check(
            "engwiki-ascii-20260201_1gb.txt", eng_text, word, 119, 1428994, 4836427, 1428994, 4836427);
        run_benchmark_10_times("engwiki-ascii-20260201_1gb.txt", eng_text, word, 1428994, 4836427);
        return 0;
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}

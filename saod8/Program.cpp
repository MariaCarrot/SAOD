#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

static constexpr size_t ALPHABET = 128;
static constexpr const char MAGIC[4] = { 'H', 'U', 'F', '1' }; // для распаковки нужного файла
static constexpr uint8_t VERSION = 1;
static constexpr const char* ARCHIVE_EXT = ".compressed";

// структура для хранения кода символа
struct Code {
    vector<bool> bits;
};

struct HuffNode {
    uint64_t freq{};
    int symbol{ -1 }; // -1 внутренний узел
    shared_ptr<HuffNode> left, right;

    bool leaf() const {
        return !left && !right;
    }
};

struct NodeCmp { // сравнение узлов
    bool operator()(const shared_ptr<HuffNode>& a, const shared_ptr<HuffNode>& b) const {
        if (a->freq != b->freq) return a->freq > b->freq;
        return a->symbol > b->symbol;
    }
};

// заканчивается ли имя файла на .compressed
static bool has_compressed_extension(const string& path) {
    string ext = ARCHIVE_EXT;
    return path.size() >= ext.size() && path.compare(path.size() - ext.size(), ext.size(), ext) == 0;
}

// размер файла в байтах
static uint64_t file_size_of(const string& path) {
    ifstream in(path, ios::binary | ios::ate);
    if (!in) throw runtime_error("cannot open file");
    return static_cast<uint64_t>(in.tellg());
}

// функции для записи 
static void write_u64(ofstream& out, uint64_t x) {
    for (int i = 0; i < 8; ++i)
        out.put(static_cast<char>((x >> (8 * i)) & 0xFF));
}
static void write_u32(ofstream& out, uint32_t x) {
    for (int i = 0; i < 4; ++i)
        out.put(static_cast<char>((x >> (8 * i)) & 0xFF));
}

// функции для чтения
static uint64_t read_u64(ifstream& in) {
    uint64_t x = 0;

    for (int i = 0; i < 8; ++i) {
        int c = in.get();

        if (c == EOF)
            throw runtime_error("unexpected end of archive");

        x |= (uint64_t(static_cast<uint8_t>(c)) << (8 * i));
    }
    return x;
}
static uint32_t read_u32(ifstream& in) {
    uint32_t x = 0;

    for (int i = 0; i < 4; ++i) {
        int c = in.get();

        if (c == EOF)
            throw runtime_error("unexpected end of archive");

        x |= (uint32_t(static_cast<uint8_t>(c)) << (8 * i));
    }
    return x;
}

// CRC32 нужен не для сжатия, а для проверки повреждения архива
static uint32_t crc32_update(uint32_t crc, uint8_t byte) {
    crc ^= byte;
    for (int k = 0; k < 8; ++k)
        crc = (crc & 1) ? (0xEDB88320u ^ (crc >> 1)) : (crc >> 1);
    return crc;
}
static uint32_t crc32_bytes(const vector<uint8_t>& data) {
    uint32_t crc = 0xFFFFFFFFu;
    for (uint8_t b : data)
        crc = crc32_update(crc, b);
    return crc ^ 0xFFFFFFFFu;
}

static vector<uint8_t> read_ascii_file(const string& path, array<uint64_t, ALPHABET>& freq) {
    freq.fill(0);
    ifstream in(path, ios::binary);

    if (!in)
        throw runtime_error("cannot open input file");

    vector<uint8_t> data;
    char ch;

    while (in.get(ch)) {
        uint8_t b = static_cast<uint8_t>(ch);
        if (b > 127)
            throw runtime_error("not ascii");

        data.push_back(b); // сохраняем байты
        freq[b]++;
    }
    return data;
}

static shared_ptr<HuffNode> build_huffman_tree(const array<uint64_t, ALPHABET>& freq) {
    priority_queue< 
        shared_ptr<HuffNode>,
        vector<shared_ptr<HuffNode>>,
        NodeCmp
    > pq;

    for (int s = 0; s < static_cast<int>(ALPHABET); ++s) {
        if (freq[s] > 0) {
            auto node = make_shared<HuffNode>();
            node->freq = freq[s];
            node->symbol = s;
            pq.push(node);
        }
    }

    if (pq.empty())
        return nullptr;

    while (pq.size() > 1) {
        auto a = pq.top();
        pq.pop();

        auto b = pq.top();
        pq.pop();

        auto parent = make_shared<HuffNode>();
        parent->freq = a->freq + b->freq;
        parent->symbol = min(a->symbol, b->symbol);
        parent->left = a;
        parent->right = b;

        pq.push(parent);
    }
    return pq.top();
}

// получение длин кодов
static void collect_lengths(const shared_ptr<HuffNode>& node, int depth, array<uint8_t, ALPHABET>& lengths) {
    if (!node)
        return;

    if (node->leaf()) {
        lengths[node->symbol] = static_cast<uint8_t>(max(1, depth));
        return;
    }

    collect_lengths(node->left, depth + 1, lengths);
    collect_lengths(node->right, depth + 1, lengths);
}

// массив длин кодов
static array<uint8_t, ALPHABET> make_code_lengths(const array<uint64_t, ALPHABET>& freq) {
    array<uint8_t, ALPHABET> lengths{};
    lengths.fill(0);

    auto root = build_huffman_tree(freq);
    collect_lengths(root, 0, lengths);
    return lengths;
}

// увеличение двоичного кода на 1
static void increment_binary(vector<bool>& code) {
    if (code.empty()) {
        code.push_back(true);
        return;
    }

    for (int i = static_cast<int>(code.size()) - 1; i >= 0; --i) {
        if (!code[i]) {
            code[i] = true;

            for (size_t j = i + 1; j < code.size(); ++j)
                code[j] = false;
            return;
        }
    }
    fill(code.begin(), code.end(), false);
    code.insert(code.begin(), true);
}

// канонические коды
static array<Code, ALPHABET> build_canonical_codes(const array<uint8_t, ALPHABET>& lengths) {
    vector<pair<int, int>> symbols;

    for (int s = 0; s < static_cast<int>(ALPHABET); ++s)
        if (lengths[s] > 0) symbols.push_back({ lengths[s], s });

    sort(symbols.begin(), symbols.end()); // сортируем сначала по длине кода, потом по номеру символ

    array<Code, ALPHABET> codes; // массив кодов для всех символов
    vector<bool> code; // текущий код
    int prev_len = 0; // длина предыдущего кода

    for (const auto& item : symbols) {
        int len = item.first;
        int sym = item.second;

        if (len <= 0 || len > 127) throw runtime_error("unsupported code length");

        int shift = len - prev_len;
        if (shift < 0) throw runtime_error("invalid code lengths order");

        for (int i = 0; i < shift; ++i)
            code.push_back(false);

        if (static_cast<int>(code.size()) != len) throw runtime_error("invalid canonical code table");

        codes[sym].bits = code;
        increment_binary(code);
        prev_len = len;
    }
    return codes; // таблица кодов
}

// чтение и запись битов
struct BitWriter {
    ofstream& out;
    uint8_t buffer{ 0 };
    int used{ 0 };
    uint64_t bit_count{ 0 };

    explicit BitWriter(ofstream& out_) : out(out_) {}

    void write_bit(bool bit) {
        buffer = static_cast<uint8_t>((buffer << 1) | (bit ? 1 : 0));
        ++used;
        ++bit_count;

        if (used == 8)
            flush_byte();
    }

    void write_code(const Code& c) {
        for (bool bit : c.bits)
            write_bit(bit);
    }

    void flush_byte() {
        out.put(static_cast<char>(buffer));
        buffer = 0;
        used = 0;
    }

    void finish() {
        if (used > 0) {
            buffer <<= (8 - used);
            flush_byte();
        }
    }
};
struct BitReader {
    ifstream& in;
    uint8_t buffer{ 0 };
    int left{ 0 };
    uint64_t read_bits{ 0 };
    uint64_t max_bits{ 0 };

    BitReader(ifstream& in_, uint64_t max_bits_)
        : in(in_), max_bits(max_bits_) {}

    bool read_bit(bool& bit) {
        if (read_bits >= max_bits) return false;

        if (left == 0) {
            int c = in.get();

            if (c == EOF) throw runtime_error("unexpected end of encoded data");
            buffer = static_cast<uint8_t>(c);
            left = 8;
        }

        bit = ((buffer >> 7) & 1u) != 0;
        buffer <<= 1;
        --left;
        ++read_bits;
        return true;
    }
};

struct DecodeNode {
    int symbol{ -1 };
    unique_ptr<DecodeNode> zero;
    unique_ptr<DecodeNode> one;
};

static unique_ptr<DecodeNode> build_decode_tree(const array<Code, ALPHABET>& codes) {
    auto root = make_unique<DecodeNode>();

    for (int s = 0; s < static_cast<int>(ALPHABET); ++s) {
        if (codes[s].bits.empty()) continue;

        DecodeNode* cur = root.get();
        for (bool bit : codes[s].bits) {
            auto& next = bit ? cur->one : cur->zero;

            if (!next)  next = make_unique<DecodeNode>();
            cur = next.get();
        }
        if (cur->symbol != -1) throw runtime_error("invalid code table");
        cur->symbol = s;
    }
    return root;
}

// проверка на корректность кодов
static void validate_lengths(const array<uint8_t, ALPHABET>& lengths, uint64_t original_size) {
    int count = 0;
    long double kraft = 0.0L;

    for (uint8_t len : lengths) {
        if (len > 127) throw runtime_error("unsupported code length");

        if (len > 0) {
            ++count;
            kraft += ldexpl(1.0L, -static_cast<int>(len));
        }
    }

    if (original_size == 0) {
        if (count != 0) throw runtime_error("invalid empty archive code table");
    }
    else {
        if (count == 0) throw runtime_error("empty code table");
        if (kraft > 1.0000000000001L) throw runtime_error("invalid huffman code lengths");
    }
}

static void compress_file(const string& input, const string& output) {
    array<uint64_t, ALPHABET> freq{};
    auto data = read_ascii_file(input, freq);

    uint64_t original_size = static_cast<uint64_t>(data.size());
    uint32_t original_crc = crc32_bytes(data);

    auto lengths = make_code_lengths(freq);
    auto codes = build_canonical_codes(lengths);

    uint64_t encoded_bits = 0;
    for (uint8_t b : data)
        encoded_bits += codes[b].bits.size();

    ofstream out(output, ios::binary);
    if (!out) throw runtime_error("cannot open output file");

    out.write(MAGIC, 4);
    out.put(static_cast<char>(VERSION));

    write_u64(out, original_size);
    write_u64(out, encoded_bits);
    write_u32(out, original_crc);

    for (uint8_t len : lengths) 
        out.put(static_cast<char>(len));

    BitWriter bw(out);
    for (uint8_t b : data)
        bw.write_code(codes[b]);

    bw.finish();
    if (!out) throw runtime_error("write error");
}

static void decompress_file(const string& input, const string& output) {
    if (!has_compressed_extension(input)) throw runtime_error("not compressed extension");
    ifstream in(input, ios::binary);
    if (!in) throw runtime_error("cannot open input archive");

    char magic[4];
    in.read(magic, 4);

    if (in.gcount() != 4 || !equal(begin(magic), end(magic), begin(MAGIC))) throw runtime_error("not archive");
    int ver = in.get();
    if (ver == EOF || static_cast<uint8_t>(ver) != VERSION) throw runtime_error("unsupported archive version");

    uint64_t original_size = read_u64(in);
    uint64_t encoded_bits = read_u64(in);
    uint32_t expected_crc = read_u32(in);

    array<uint8_t, ALPHABET> lengths{};
    for (size_t i = 0; i < ALPHABET; ++i) {
        int c = in.get();
        if (c == EOF) throw runtime_error("unexpected end of archive");
        lengths[i] = static_cast<uint8_t>(c);
    }

    validate_lengths(lengths, original_size);
    if (original_size == 0) {
        if (encoded_bits != 0) throw runtime_error("corrupt empty archive");
        ofstream out(output, ios::binary);
        if (!out) throw runtime_error("cannot open output file");
        return;
    }

    auto codes = build_canonical_codes(lengths);
    auto decode_root = build_decode_tree(codes);

    ofstream out(output, ios::binary);
    if (!out) throw runtime_error("cannot open output file");

    BitReader br(in, encoded_bits);

    uint64_t written = 0;
    uint32_t crc = 0xFFFFFFFFu;
    DecodeNode* cur = decode_root.get();

    while (written < original_size) {
        bool bit = false;

        if (!br.read_bit(bit)) throw runtime_error("not enough encoded bits");
        cur = bit ? cur->one.get() : cur->zero.get();
        if (!cur) throw runtime_error("invalid encoded stream");

        if (cur->symbol != -1) {
            uint8_t b = static_cast<uint8_t>(cur->symbol);
            out.put(static_cast<char>(b));

            if (!out) throw runtime_error("write error");

            crc = crc32_update(crc, b);
            ++written;
            cur = decode_root.get();
        }
    }
    uint32_t actual_crc = crc ^ 0xFFFFFFFFu;
    if (actual_crc != expected_crc) throw runtime_error("crc mismatch");
}

// функция для сравнения двух файлов (исходного и разархивируемого)
static bool files_equal(const string& file1, const string& file2) {
    ifstream a(file1, ios::binary);
    ifstream b(file2, ios::binary);

    if (!a || !b) return false;

    char ca, cb;
    while (true) {
        bool ra = static_cast<bool>(a.get(ca));
        bool rb = static_cast<bool>(b.get(cb));

        if (ra != rb) return false; 
        if (!ra && !rb) return true;
        if (ca != cb) return false;
    }
}

static void run_test(const string& input_file) {
    string archive_file = input_file + ".compressed";
    string restored_file = input_file + ".restored";

    cout << "========================================\n";
    cout << "Input file: " << input_file << "\n";
    cout << "Archive file: " << archive_file << "\n";
    cout << "Restored file: " << restored_file << "\n\n";

    try {
        uint64_t original_size = file_size_of(input_file);
        auto compress_start = chrono::steady_clock::now();
        compress_file(input_file, archive_file);
        auto compress_finish = chrono::steady_clock::now();
        uint64_t archive_size = file_size_of(archive_file);

        double compression_time =
            chrono::duration<double>(compress_finish - compress_start).count();

        double ratio = archive_size == 0
            ? 0.0
            : static_cast<double>(original_size) / static_cast<double>(archive_size);

        cout << "Compression done!\n";
        cout << "Original size: " << original_size << " bytes\n";
        cout << "Archive size: " << archive_size << " bytes\n";

        cout << "Compression ratio: "
            << original_size << " bytes / "
            << archive_size << " bytes = "
            << fixed << setprecision(3) << ratio << "\n";

        cout << "Compression time: "
            << fixed << setprecision(3) << compression_time << " sec\n\n";

        auto decompress_start = chrono::steady_clock::now();
        decompress_file(archive_file, restored_file);
        auto decompress_finish = chrono::steady_clock::now();
        double decompression_time =
            chrono::duration<double>(decompress_finish - decompress_start).count();

        cout << "Decompression done!\n";
        cout << "Decompression time: "
            << fixed << setprecision(3) << decompression_time << " sec\n";

        uint64_t restored_size = file_size_of(restored_file);

        cout << "Restored size: " << restored_size << " bytes\n";
        if (original_size == restored_size)
            cout << "Size check: OK\n";
        else
            cout << "Size check: FAIL\n";
        
        if (files_equal(input_file, restored_file))
            cout << "File check: OK! Restored file is identical to original.\n";
        else
            cout << "File check: FAIL! Restored file is different from original.\n";

        cout << "========================================\n\n";
    }
    catch (const runtime_error& e) {
        string msg = e.what();

        if (msg == "not ascii")
            cerr << "Fail! The input file must be ASCII with 128 characters only!\n";
        else if (msg == "not compressed extension" || msg == "not archive")
            cerr << "Fail! It is not a .compressed archive!\n";
        else 
            cerr << "Fail! " << msg << "!\n";

        cerr << "File: " << input_file << "\n";
        cerr << "========================================\n\n";
    }
}

int main() {
    cout << "This is Huffman coding compressor for ASCII text files only.\n\n";
    run_test("D:/прога/САОД/ConsoleApplication1/simplewiki-20260201.txt");
    run_test("D:/прога/САОД/ConsoleApplication1/1brs-traj1.pdb");
    run_test("D:/прога/САОД/ConsoleApplication1/400mb.fa");
    return 0;
}

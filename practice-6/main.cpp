#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>

using namespace std;

static vector<int> build_sa(const string& s) {
  const int n = static_cast<int>(s.size());
  vector<int> sa(n);
  for (int i = 0; i < n; ++i) sa[i] = i;
  sort(sa.begin(), sa.end(), [&](int a, int b){
    for (int k = 0; k < n; ++k) {
      unsigned char ca = static_cast<unsigned char>(s[(a + k) % n]);
      unsigned char cb = static_cast<unsigned char>(s[(b + k) % n]);
      if (ca != cb) return ca < cb;
    }
    return false;
  });
  return sa;
}

static pair<string, uint32_t> bwt_encode(const string &input) {
  const size_t n = input.size();
  if (n == 0) return {"", 0};

  vector<int> sa = build_sa(input);
  string L(n, '\0');
  uint32_t primary = 0;

  for (size_t i = 0; i < n; ++i) {
    int si = sa[i];
    L[i] = input[(si + n - 1) % n];
    if (si == 0) primary = static_cast<uint32_t>(i);
  }
  return {L, primary};
}

static string bwt_decode(const string &L, uint32_t primary_index) {
  const size_t n = L.size();
  if (n == 0) return {};

  vector<int> count(256, 0);
  for (unsigned char c : L) ++count[c];

  vector<int> C(256, 0);
  for (int c = 1; c < 256; ++c) C[c] = C[c - 1] + count[c - 1];

  vector<int> occ(256, 0);
  vector<int> T(n);
  for (size_t i = 0; i < n; ++i) {
    unsigned char c = static_cast<unsigned char>(L[i]);
    T[i] = C[c] + occ[c]++;
  }

  string out(n, '\0');
  size_t p = primary_index;
  for (size_t i = n; i-- > 0; ) {
    out[i] = L[p];
    p = static_cast<size_t>(T[p]);
  }

  return out;
}

static string mtf_encode(const string &input) {
  vector<unsigned char> table(256);
  for (int i = 0; i < 256; ++i) table[i] = static_cast<unsigned char>(i);

  string output;
  output.reserve(input.size());
  for (unsigned char c : input) {
    int index = 0;
    while (table[index] != c) ++index;
    output.push_back(static_cast<unsigned char>(index));
    table.erase(table.begin() + index);
    table.insert(table.begin(), c);
  }
  return output;
}

static string mtf_decode(const string &input) {
  vector<unsigned char> table(256);
  for (int i = 0; i < 256; ++i) table[i] = static_cast<unsigned char>(i);

  string output;
  output.reserve(input.size());
  for (unsigned char idx : input) {
    unsigned char c = table[idx];
    output.push_back(c);
    table.erase(table.begin() + idx);
    table.insert(table.begin(), c);
  }
  return output;
}

static string rle_encode(const string &input) {
    string output;
    output.reserve(input.size());

    int zero_run = 0;
    for (unsigned char c : input) {
        if (c == 0) {
            ++zero_run;
            if (zero_run == 255) {
                output.push_back(0);
                output.push_back(static_cast<unsigned char>(zero_run));
                zero_run = 0;
            }
        } else {
            if (zero_run > 0) {
                output.push_back(0);
                output.push_back(static_cast<unsigned char>(zero_run));
                zero_run = 0;
            }
            output.push_back(c);
        }
    }
    if (zero_run > 0) {
        output.push_back(0);
        output.push_back(static_cast<unsigned char>(zero_run));
    }
    return output;
}

static string rle_decode(const string &input) {
    string output;
    output.reserve(input.size());

    for (size_t i = 0; i < input.size();) {
        unsigned char c = input[i++];
        if (c == 0 && i < input.size()) {
            unsigned char run_len = input[i++];
            output.append(run_len, '\0');
        } else {
            output.push_back(c);
        }
    }
    return output;
}

queue<char> bit_buffer;

string calculate_bits(uint64_t data, uint64_t num_bits) {
  string result;
  result.reserve(num_bits);
  for (uint64_t i = 0; i < num_bits; ++i)
    result += (((data >> (num_bits - i - 1)) & 0x01) + '0');
  return result;
}

void write_to_file(const string &bits, ofstream &encoded_file) {
  for (char bit : bits)
    bit_buffer.push(bit);

  while (bit_buffer.size() > 8) {
    char byte = 0;
    for (int i = 0; i < 8; ++i) {
      bool bit = bit_buffer.front() - '0';
      bit_buffer.pop();
      byte |= (bit << (7 - i));
    }
    encoded_file.put(byte);
  }
}

uint64_t calculate_match(string &data, uint64_t num_bits) {
  uint64_t match = 0;
  for (uint64_t i = 0; i < num_bits; i++) {
    match <<= 1;
    match |= (data[i] - '0');
  }
  return match;
}

void encode_to_file(const string &input_data, const string &output_path) {
  ofstream encoded_file(output_path, ios::binary);
  if (!encoded_file.is_open()) {
    cerr << "Error: could not open output\n";
    return;
  }

  unordered_map<string, uint64_t> dictionary;
  for (uint16_t i = 0; i < 256; ++i)
    dictionary[string(1, static_cast<char>(i))] = i;

  uint64_t total_bits = 0;
  uint64_t num_bits = 8;
  uint64_t next_code = 256;
  string current_match;
  encoded_file.put(0xFF);

  for (unsigned char symbol : input_data) {
    string new_match = current_match + static_cast<char>(symbol);
    if (dictionary.find(new_match) != dictionary.end()) {
      current_match = new_match;
    } else {
      uint64_t code = dictionary[current_match];
      string bits = calculate_bits(code, num_bits);
      total_bits += bits.size();
      write_to_file(bits, encoded_file);
      dictionary[new_match] = next_code++;
      if (dictionary.size() > ((1ULL << num_bits) - 1)) ++num_bits;
      current_match = string(1, symbol);
    }
  }

  if (!current_match.empty()) {
    uint64_t code = dictionary[current_match];
    string bits = calculate_bits(code, num_bits);
    total_bits += bits.size();
    write_to_file(bits, encoded_file);
  }

  char last_byte = 0;
  for (int i = 0; !bit_buffer.empty(); ++i) {
    bool bit = bit_buffer.front() - '0';
    bit_buffer.pop();
    last_byte |= (bit << (7 - i));
  }
  encoded_file.put(last_byte);

  uint8_t padding_bits = (8 - (total_bits % 8)) % 8;
  char padding_header = 0;
  for (int i = 0; i < 3; ++i) {
    padding_header <<= 1;
    padding_header |= (padding_bits >> (2 - i));
  }
  encoded_file.seekp(0, ios::beg);
  encoded_file.put(padding_header);
  encoded_file.close();
}

string decode_from_file(const string &input_path) {
  ifstream encoded_file(input_path, ios::binary);
  if (!encoded_file.is_open()) {
    cerr << "Error: could not open encoded file\n";
    return {};
  }

  int padding_bits = 0;
  char padding_header;
  encoded_file.get(padding_header);
  for (int i = 0; i < 3; i++) {
    padding_bits <<= 1;
    padding_bits |= ((padding_header >> (2 - i)) & 0x01);
  }

  unordered_map<uint64_t, string> dictionary;
  for (int i = 0; i < 256; i++)
    dictionary[i] = string(1, static_cast<char>(i));

  char symbol;
  uint64_t num_bits = 8;
  uint64_t next_code = 0x100;
  string bit_stream;
  string result;
  bool change = false;

  while (!encoded_file.eof() || !bit_stream.empty()) {
    while (bit_stream.size() < num_bits && encoded_file.get(symbol)) {
      int bits_to_read = encoded_file.peek() == EOF ? 8 - padding_bits : 8;
      for (int i = 0; i < bits_to_read; ++i)
        bit_stream += ((symbol >> (7 - i)) & 0x01) + '0';
    }
    if (bit_stream.size() < num_bits) break;

    uint64_t code = calculate_match(bit_stream, num_bits);
    bit_stream.erase(0, num_bits);
    if (change) {
      string tmp = dictionary[dictionary.size() - 1];
      tmp[tmp.size() - 1] = dictionary[code][0];
      dictionary[dictionary.size() - 1] = tmp;
    } else {
      change = true;
    }

    string entry = dictionary[code];
    result += entry;

    dictionary[next_code++] = dictionary[code] + "$";
    if (dictionary.size() > ((1ULL << num_bits) - 1)) ++num_bits;
  }
  return result;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        cout << "Usage:\n";
        cout << argv[0] << " -d <input> <output>\n";
        cout << argv[0] << " -e <input> <output>\n";
        cout << argv[0] << " -d <input> <output> -no-rle\n";
        cout << argv[0] << " -e <input> <output> -no-rle\n";
        return 1;
    }

    string mode = argv[1];
    string input_file = argv[2];
    string output_file = argv[3];

    if (mode == "-e") {
        ifstream in(input_file, ios::binary);
        if (!in) {
            cerr << "Error: Cannot open input file " << input_file << "\n";
            return 1;
        }

        auto start_enc = chrono::high_resolution_clock::now();
        string input_data((istreambuf_iterator<char>(in)), {});
        in.close();

        auto [bwt_data, idx] = bwt_encode(input_data);
        string mtf_data = mtf_encode(bwt_data);

        string encoded_data;
        if (argc == 5 && string(argv[4]) == "-no-rle") {
            encoded_data = mtf_data;
        } else {
            encoded_data = rle_encode(mtf_data);
        }

        string packed(sizeof(uint32_t), '\0');
        memcpy(packed.data(), &idx, sizeof(uint32_t));
        packed += encoded_data;

        encode_to_file(packed, output_file);

        auto end_enc = chrono::high_resolution_clock::now();
        double enc_time = chrono::duration<double>(end_enc - start_enc).count();
        cout << "Encoding time: " << enc_time << "s\n";
    }

    else if (mode == "-d") {
        auto start_dec = chrono::high_resolution_clock::now();
        string decoded_bytes = decode_from_file(input_file);

        if (decoded_bytes.size() < sizeof(uint32_t)) {
            cerr << "Error: Corrupted input file\n";
            return 1;
        }

        uint32_t primary;
        memcpy(&primary, decoded_bytes.data(), sizeof(uint32_t));
        string body = decoded_bytes.substr(sizeof(uint32_t));

        string mtf_input;
        if (argc == 5 && string(argv[4]) == "-no-rle") {
            mtf_input = body;
        } else {
            mtf_input = rle_decode(body);
        }

        string mtf_decoded = mtf_decode(mtf_input);
        string restored = bwt_decode(mtf_decoded, primary);

        ofstream out(output_file, ios::binary);
        out.write(restored.data(), restored.size());
        out.close();

        auto end_dec = chrono::high_resolution_clock::now();
        double dec_time = chrono::duration<double>(end_dec - start_dec).count();
        cout << "Decoding time: " << dec_time << "s\n";
    }

    else {
        cout << "Usage:\n";
        cout << argv[0] << " -d <input> <output>\n";
        cout << argv[0] << " -e <input> <output>\n";
        cout << argv[0] << " -d <input> <output> -no-rle\n";
        cout << argv[0] << " -e <input> <output> -no-rle\n";
        return 1;
    }

    return 0;
}


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <string>
#include <climits>
#include <chrono>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

#ifndef ASSERT
#define ASSERT(c, m, ...) \
  if (!(c)) { \
    std::fprintf(stderr, "ERROR: " m "\n", ##__VA_ARGS__); \
    std::exit(1); \
  }
#endif

struct Node {
  double lat;
  double lon;
};

struct Edge {
  unsigned from;
  unsigned to;
  unsigned time;
};

struct Interest {
  unsigned node;
  unsigned code;
  std::string name;
};

struct FoundInterest {
  Interest point;
  int dist;
};

using Adj = std::vector<std::vector<std::pair<int,int>>>;

std::vector<Node> read_nodes(const std::string &filename) {
  std::ifstream f(filename);
  ASSERT(f, "Cannot open %s", filename.c_str());

  unsigned count;
  f >> count;

  std::vector<Node> nodes(count);
  unsigned id;
  for (unsigned i = 0; i < count; i++) {
    f >> id >> nodes[i].lat >> nodes[i].lon;
  }
  return nodes;
}

std::vector<Edge> read_edges(const std::string &filename) {
  std::ifstream f(filename);
  ASSERT(f, "Cannot open %s", filename.c_str());

  unsigned count;
  f >> count;

  std::vector<Edge> edges;
  edges.reserve(count);

  Edge e;
  unsigned dist_dummy, speed_dummy;
  while (f >> e.from >> e.to >> e.time >> dist_dummy >> speed_dummy) {
    edges.push_back(e);
  }
  return edges;
}

void read_interest(const std::string &filename,
                   unsigned node_count,
                   std::vector<std::vector<Interest>> &by_node) {
  std::ifstream f(filename);
  ASSERT(f, "Cannot open %s", filename.c_str());

  by_node.assign(node_count, {});

  std::string line;
  while (std::getline(f, line)) {
    if (line.empty()) continue;

    std::stringstream ss(line);
    Interest ip;
    ss >> ip.node >> ip.code;

    std::string name;
    std::getline(ss, name);

    std::size_t q1 = name.find('"');
    std::size_t q2 = name.rfind('"');
    if (q1 != std::string::npos && q2 != std::string::npos && q2 > q1) {
      ip.name = name.substr(q1 + 1, q2 - q1 - 1);
    } else {
      ip.name = "UNKNOWN";
    }

    if (ip.node < node_count) {
      by_node[ip.node].push_back(ip);
    }
  }
}

std::string format_time(int ticks) {
  int ms  = ticks * 10;
  int H   = ms / (1000 * 60 * 60);
  int M   = (ms / (1000 * 60)) % 60;
  int S   = (ms / 1000) % 60;
  int HS  = (ms / 10) % 100;

  char buf[32];
  std::snprintf(buf, sizeof(buf), "%d:%02d:%02d,%02d", H, M, S, HS);
  return std::string(buf);
}

void choose_landmarks(const std::vector<Node> &nodes,
                      std::vector<int> &landmarks) {
  int n = (int)nodes.size();
  ASSERT(n > 0, "No nodes");

  int minLat = 0, maxLat = 0, minLon = 0, maxLon = 0;

  for (int i = 1; i < n; i++) {
    if (nodes[i].lat < nodes[minLat].lat) minLat = i;
    if (nodes[i].lat > nodes[maxLat].lat) maxLat = i;
    if (nodes[i].lon < nodes[minLon].lon) minLon = i;
    if (nodes[i].lon > nodes[maxLon].lon) maxLon = i;
  }

  landmarks.clear();
  landmarks.push_back(minLat);
  if (maxLat != minLat) landmarks.push_back(maxLat);
  if (minLon != minLat && minLon != maxLat) landmarks.push_back(minLon);
  if (maxLon != minLat && maxLon != maxLat && maxLon != minLon)
    landmarks.push_back(maxLon);

  while (landmarks.size() < 4)
    landmarks.push_back(landmarks.back());
}

std::vector<int> dijkstra_all(const Adj &adj, int src) {
  int V = (int)adj.size();
  std::vector<int> dist(V, INT_MAX);

  std::priority_queue<
    std::pair<int,int>,
    std::vector<std::pair<int,int>>,
    std::greater<std::pair<int,int>>
  > pq;

  dist[src] = 0;
  pq.emplace(0, src);

  while (!pq.empty()) {
    auto [d, u] = pq.top();
    pq.pop();
    if (d > dist[u]) continue;

    for (auto &[v, w] : adj[u]) {
      if (dist[u] + w < dist[v]) {
        dist[v] = dist[u] + w;
        pq.emplace(dist[v], v);
      }
    }
  }

  return dist;
}

int dijkstra_single(const Adj &adj,
                    int start, int goal,
                    int &pops,
                    std::vector<int> &parent) {
  int V = (int)adj.size();
  parent.assign(V, -1);

  std::vector<int> dist(V, INT_MAX);
  std::vector<char> used(V, 0);

  std::priority_queue<
    std::pair<int,int>,
    std::vector<std::pair<int,int>>,
    std::greater<std::pair<int,int>>
  > pq;

  dist[start] = 0;
  pq.emplace(0, start);
  pops = 0;

  while (!pq.empty()) {
    auto [d, u] = pq.top();
    pq.pop();
    if (used[u]) continue;
    used[u] = 1;
    pops++;

    if (u == goal) break;

    for (auto &[v, w] : adj[u]) {
      if (dist[u] + w < dist[v]) {
        dist[v] = dist[u] + w;
        parent[v] = u;
        pq.emplace(dist[v], v);
      }
    }
  }

  return dist[goal];
}

void preprocess_alt(const Adj &adj,
                    const Adj &rev_adj,
                    const std::vector<int> &landmarks,
                    std::vector<std::vector<int>> &distFrom,
                    std::vector<std::vector<int>> &distTo) {
  int L = (int)landmarks.size();
  int N = (int)adj.size();

  distFrom.assign(L, std::vector<int>(N, INT_MAX));
  distTo.assign(L,   std::vector<int>(N, INT_MAX));

  for (int i = 0; i < L; i++) {
    int s = landmarks[i];
    distFrom[i] = dijkstra_all(adj, s);
    distTo[i]   = dijkstra_all(rev_adj, s);
  }
}

int alt_estimate(int u, int goal,
                 const std::vector<int> &landmarks,
                 const std::vector<std::vector<int>> &distFrom,
                 const std::vector<std::vector<int>> &distTo) {
  int best = 0;
  int L = (int)landmarks.size();

  for (int i = 0; i < L; i++) {
    int dLg = distFrom[i][goal];
    int dLn = distFrom[i][u];
    if (dLg != INT_MAX && dLn != INT_MAX)
      best = std::max(best, dLg - dLn);

    int dnL = distTo[i][u];
    int dgL = distTo[i][goal];
    if (dnL != INT_MAX && dgL != INT_MAX)
      best = std::max(best, dnL - dgL);
  }

  return best < 0 ? 0 : best;
}

int alt_search(const Adj &adj,
               int start, int goal,
               const std::vector<int> &landmarks,
               const std::vector<std::vector<int>> &distFrom,
               const std::vector<std::vector<int>> &distTo,
               int &pops,
               std::vector<int> &parent) {
  int V = (int)adj.size();
  parent.assign(V, -1);

  std::vector<int> g(V, INT_MAX);
  std::vector<int> h(V, -1);
  std::vector<char> used(V, 0);

  std::priority_queue<
    std::pair<int,int>,
    std::vector<std::pair<int,int>>,
    std::greater<std::pair<int,int>>
  > pq;

  g[start] = 0;
  h[start] = alt_estimate(start, goal, landmarks, distFrom, distTo);
  pq.emplace(g[start] + h[start], start);
  pops = 0;

  while (!pq.empty()) {
    auto [prio, u] = pq.top();
    pq.pop();
    if (used[u]) continue;
    used[u] = 1;
    pops++;

    if (u == goal) break;
    if (g[u] == INT_MAX) continue;

    for (auto &[v, w] : adj[u]) {
      int newg = g[u] + w;
      if (newg < g[v]) {
        g[v] = newg;
        parent[v] = u;
        if (h[v] < 0)
          h[v] = alt_estimate(v, goal, landmarks, distFrom, distTo);
        pq.emplace(g[v] + h[v], v);
      }
    }
  }

  return g[goal];
}

std::vector<int> reconstruct_path(int start, int goal,
                                  const std::vector<int> &parent) {
  std::vector<int> path;
  int cur = goal;

  while (cur != -1) {
    path.push_back(cur);
    if (cur == start) break;
    cur = parent[cur];
  }

  if (cur == -1) {
    path.clear();
    return path;
  }

  std::reverse(path.begin(), path.end());
  return path;
}

std::vector<FoundInterest> find_nearest_interest(
    const Adj &adj,
    const std::vector<std::vector<Interest>> &by_node,
    int start,
    unsigned mask,
    int max_found) {
  int V = (int)adj.size();

  std::priority_queue<
    std::pair<int,int>,
    std::vector<std::pair<int,int>>,
    std::greater<std::pair<int,int>>
  > pq;

  std::vector<int> dist(V, INT_MAX);
  std::vector<char> used(V, 0);
  std::vector<FoundInterest> found;

  dist[start] = 0;
  pq.emplace(0, start);

  while (!pq.empty() && (int)found.size() < max_found) {
    auto [d, u] = pq.top();
    pq.pop();

    if (used[u]) continue;
    used[u] = 1;

    for (const auto &ip : by_node[u]) {
      if (ip.code & mask) {
        found.push_back({ip, d});
        if ((int)found.size() >= max_found)
          break;
      }
    }

    if ((int)found.size() >= max_found)
      break;

    for (auto &[v, w] : adj[u]) {
      if (dist[u] + w < dist[v]) {
        dist[v] = dist[u] + w;
        pq.emplace(dist[v], v);
      }
    }
  }

  return found;
}

void write_path(const std::vector<int> &path,
                        const std::vector<Node> &nodes,
                        const std::string &filename) {
  std::ofstream out(filename);
  ASSERT(out, "Cannot open output file %s", filename.c_str());

  for (int node : path) {
    out << nodes[node].lat << "," << nodes[node].lon << "\n";
  }
  std::cout << "Wrote route to " << filename << "\n";
}

void write_poi(const std::vector<FoundInterest> &pois,
                       const std::vector<Node> &nodes,
                       const std::string &filename) {
  std::ofstream out(filename);
  ASSERT(out, "Cannot open output file %s", filename.c_str());

  for (const auto &fi : pois) {
    unsigned node = fi.point.node;
    ASSERT(node < nodes.size(), "POI node index out of range");
    out << nodes[node].lat << "," << nodes[node].lon << "\n";
  }
  std::cout << "Wrote POIs to " << filename << "\n";
}

void run_route(const Adj &adj,
               const Adj &rev_adj,
               const std::vector<int> &landmarks,
               const std::vector<std::vector<int>> &distFrom,
               const std::vector<std::vector<int>> &distTo,
               const std::vector<Node> &nodes,
               int start, int goal,
               const std::string &route_name) {
    std::cout << "\nROUTE: " << route_name << "\n";
    std::cout << "From node " << start << " → to node " << goal << "\n";

    std::vector<int> parent_dij, parent_alt;
    int pops_dij = 0, pops_alt = 0;

    auto t_d1 = std::chrono::high_resolution_clock::now();
    int dist_dij = dijkstra_single(adj, start, goal, pops_dij, parent_dij);
    auto t_d2 = std::chrono::high_resolution_clock::now();

    auto t_a1 = std::chrono::high_resolution_clock::now();
    int dist_alt = alt_search(adj, start, goal,
                              landmarks, distFrom, distTo,
                              pops_alt, parent_alt);
    auto t_a2 = std::chrono::high_resolution_clock::now();

    ASSERT(dist_dij != INT_MAX, "Dijkstra: No path");
    ASSERT(dist_alt != INT_MAX, "ALT: No path");
    ASSERT(dist_dij == dist_alt, "Mismatch in Dijkstra vs ALT distance");

    double dij_secs = std::chrono::duration<double>(t_d2 - t_d1).count();
    double alt_secs = std::chrono::duration<double>(t_a2 - t_a1).count();

    auto path_dij = reconstruct_path(start, goal, parent_dij);
    auto path_alt = reconstruct_path(start, goal, parent_alt);

    std::cout << "\nDijkstra\n";
    std::cout << "Distance:   " << dist_dij << " (" << format_time(dist_dij) << ")\n";
    std::cout << "Path nodes: " << path_dij.size() << "\n";
    std::cout << "Pops:       " << pops_dij << "\n";
    std::cout << "Runtime:    " << dij_secs << " s\n";

    std::cout << "\nALT\n";
    std::cout << "Distance:   " << dist_alt << " (" << format_time(dist_alt) << ")\n";
    std::cout << "Path nodes: " << path_alt.size() << "\n";
    std::cout << "Pops:       " << pops_alt << "\n";
    std::cout << "Runtime:    " << alt_secs << " s\n";

    std::string filename_dij = "route_" + route_name + "_dij.txt";
    write_path(path_dij, nodes, filename_dij);

    std::string filename_alt = "route_" + route_name + "_alt.txt";
    write_path(path_alt, nodes, filename_alt);
}

void run_poi_search(const Adj &adj,
                    const std::vector<std::vector<Interest>> &interest_by_node,
                    const std::vector<Node> &nodes,
                    int start_node,
                    unsigned mask,
                    int max_found,
                    const std::string &poi_name,
                    const std::string &output_filename) {
    std::cout << "\nNearest " << poi_name
              << " from node " << start_node << "\n";

    auto pois = find_nearest_interest(adj, interest_by_node,
                                      start_node, mask, max_found);

    for (size_t i = 0; i < pois.size(); i++) {
        const auto &fi = pois[i];
        unsigned n = fi.point.node;
        ASSERT(n < nodes.size(), "POI node out of range");

        std::cout << "  " << (i + 1) << ". " << fi.point.name
                  << " (node " << n << ")  dist=" << fi.dist
                  << " (" << format_time(fi.dist) << ")  "
                  << "lat=" << nodes[n].lat
                  << ", lon=" << nodes[n].lon << "\n";
    }

    write_poi(pois, nodes, output_filename);
}

void save_alt_data(const std::string &filename,
                   const std::vector<int> &landmarks,
                   const std::vector<std::vector<int>> &distFrom,
                   const std::vector<std::vector<int>> &distTo) {
    std::ofstream f(filename, std::ios::binary);
    ASSERT(f, "Cannot write ALT cache file %s", filename.c_str());

    uint32_t L = landmarks.size();
    uint32_t N = distFrom[0].size();

    f.write((char*)&L, sizeof(L));
    f.write((char*)&N, sizeof(N));

    f.write((char*)landmarks.data(), L * sizeof(int));

    for (uint32_t i = 0; i < L; i++)
        f.write((char*)distFrom[i].data(), N * sizeof(int));

    for (uint32_t i = 0; i < L; i++)
        f.write((char*)distTo[i].data(), N * sizeof(int));

    std::cout << "Saved ALT preprocessing to " << filename << "\n";
}

bool load_alt_data(const std::string &filename,
                   std::vector<int> &landmarks,
                   std::vector<std::vector<int>> &distFrom,
                   std::vector<std::vector<int>> &distTo) {
    std::ifstream f(filename, std::ios::binary);
    if (!f) return false;

    uint32_t L, N;
    f.read((char*)&L, sizeof(L));
    f.read((char*)&N, sizeof(N));

    landmarks.resize(L);
    f.read((char*)landmarks.data(), L * sizeof(int));

    distFrom.assign(L, std::vector<int>(N));
    distTo.assign(L, std::vector<int>(N));

    for (uint32_t i = 0; i < L; i++)
        f.read((char*)distFrom[i].data(), N * sizeof(int));

    for (uint32_t i = 0; i < L; i++)
        f.read((char*)distTo[i].data(), N * sizeof(int));

    std::cout << "Loaded ALT preprocessing from " << filename << "\n";
    return true;
}


int main() {
  const int NODE_TYHOLT = 2374446;
  const int NODE_ALVDAL = 3414169;

  const std::string ROUTE_FILE = "route_tyholt_alvdal.txt";
  const std::string POI_ALVDAL_FILE = "poi_ladestasjon_alvdal.txt";
  const std::string POI_TYHOLT_FILE = "poi_spise_drikk_tyholt.txt";

  auto nodes = read_nodes("noder.txt");
  auto edges = read_edges("kanter.txt");

  ASSERT(NODE_TYHOLT >= 0 && (size_t)NODE_TYHOLT < nodes.size(), "Invalid Tyholt node");
  ASSERT(NODE_ALVDAL >= 0 && (size_t)NODE_ALVDAL < nodes.size(), "Invalid Alvdal node");

  Adj adj(nodes.size()), rev_adj(nodes.size());
  for (const auto &e : edges) {
    adj[e.from].push_back({(int)e.to, (int)e.time});
    rev_adj[e.to].push_back({(int)e.from, (int)e.time});
  }

  std::vector<std::vector<Interest>> interest_by_node;
  read_interest("interessepkt.txt", (unsigned)nodes.size(), interest_by_node);

  std::vector<int> landmarks;
  std::vector<std::vector<int>> distFrom, distTo;

  const std::string ALT_CACHE = "alt_cache.bin";

  if (!load_alt_data(ALT_CACHE, landmarks, distFrom, distTo)) {
      std::cout << "ALT cache not found – computing...\n";

      choose_landmarks(nodes, landmarks);

      auto t1 = std::chrono::high_resolution_clock::now();
      preprocess_alt(adj, rev_adj, landmarks, distFrom, distTo);
      auto t2 = std::chrono::high_resolution_clock::now();

      double secs = std::chrono::duration<double>(t2 - t1).count();
      std::cout << "ALT preprocessing completed in " << secs << " s\n";

      save_alt_data(ALT_CACHE, landmarks, distFrom, distTo);
  }

  run_route(adj, rev_adj,
            landmarks, distFrom, distTo,
            nodes,
            2001238,   // Gløshaugen
            1987066,   // Otilienborg
            "gloshaugen_otilienborg");

  run_route(adj, rev_adj,
            landmarks, distFrom, distTo,
            nodes,
            2486870,   // Fosnavåg
            5394165,   // Espoo
            "fosnavag_espoo");

  run_poi_search(adj,
                interest_by_node,
                nodes,
                NODE_ALVDAL,
                4,                      // ladestasjon bit
                5,                      // find 5
                "ladestasjoner",
                POI_ALVDAL_FILE);

  run_poi_search(adj,
               interest_by_node,
               nodes,
               NODE_TYHOLT,
               8 | 16,                 // spise + drikke bits
               5,
               "spise/drikkesteder",
               POI_TYHOLT_FILE);

  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct {
  int *to;
  int size;
} node_t;

typedef struct {
  node_t *nodes;
  int size;
  const char *path;
} graph_t;

graph_t *file_to_graph(const char *path) {
  FILE *f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "Fopen: '%s': %s\n", path, strerror(errno));
    return NULL;
  }

  int node_size, edge_size;
  if (fscanf(f, "%d %d", &node_size, &edge_size) != 2) {
    fprintf(stderr, "File content format wrong\n");
    fclose(f);
    return NULL;
  }

  graph_t *graph = calloc(1, sizeof(graph_t));
  graph->nodes = calloc(node_size, sizeof(node_t));
  graph->size = node_size;
  graph->path = path;

  int from, to;
  while (fscanf(f, "%d %d", &from, &to) == 2) {
    node_t *node = &graph->nodes[from];
    int *new_to = realloc(node->to, (node->size + 1) * sizeof(int));
    node->to = new_to;
    node->to[node->size] = to;
    node->size++;
  }

  fclose(f);
  return graph;
}

void bfs(graph_t *graph, int start) {
  int *visited = calloc(graph->size, sizeof(int));
  int *queue = calloc(graph->size, sizeof(int));
  int *distance = malloc(graph->size * sizeof(int));
  int *predecessor = malloc(graph->size * sizeof(int));

  for (int i = 0; i < graph->size; i++) {
    distance[i] = -1;
    predecessor[i] = -1;
  }

  int head = 0, tail = 0;
  visited[start] = 1;
  distance[start] = 0;
  queue[tail++] = start;

  while (head < tail) {
    int node = queue[head++];
    node_t *n = &graph->nodes[node];
    for (int i = 0; i < n->size; i++) {
      int neighbor = n->to[i];
      if (!visited[neighbor]) {
        visited[neighbor] = 1;
        distance[neighbor] = distance[node] + 1;
        predecessor[neighbor] = node;
        queue[tail++] = neighbor;
      }
    }
  }

  printf("node\tpre\tdist\t for %s\n", graph->path);
  for (int i = 0; i < graph->size; i++) {
    printf("%d\t", i);
    if (predecessor[i] != -1) printf("%d\t", predecessor[i]);
    if (distance[i] != -1) printf("%d\t", distance[i]);
    printf("\n");
  }

  free(visited);
  free(queue);
  free(distance);
  free(predecessor);
}

void topological_sort(graph_t *graph) {
  int *in_degree = calloc(graph->size, sizeof(int));
  int *queue = calloc(graph->size, sizeof(int));
  int head = 0, tail = 0;

  for (int i = 0; i < graph->size; i++) {
    node_t *n = &graph->nodes[i];
    for (int j = 0; j < n->size; j++) in_degree[n->to[j]]++;
  }

  for (int i = 0; i < graph->size; i++) {
    if (in_degree[i] == 0) queue[tail++] = i;
  }

  int count = 0;
  int *topo_order = malloc(graph->size * sizeof(int));

  while (head < tail) {
    int node = queue[head++];
    topo_order[count++] = node;
    node_t *n = &graph->nodes[node];
    for (int i = 0; i < n->size; i++) {
      int neighbor = n->to[i];
      in_degree[neighbor]--;
      if (in_degree[neighbor] == 0) 
        queue[tail++] = neighbor;
    }
  }

  if (count != graph->size) {
    printf("Graph is not a DAG\n");
    return;
  }

  printf("topological sort for %s\n", graph->path);
  for (int i = 0; i < graph->size; i++) 
    printf("%d ", topo_order[i]);
  printf("\n");

  free(in_degree);
  free(queue);
  free(topo_order);
}

void graph_free(graph_t *graph) {
  if (!graph) return;

  for (int i = 0; i < graph->size; i++) {
    free(graph->nodes[i].to);
  }

  free(graph->nodes);
  free(graph);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("Usage: %s <file> <start>\n", argv[0]);
    return 1;
  }

  graph_t *graph = file_to_graph(argv[1]);
  if (!graph) {
      fprintf(stderr, "Failed to load graph from file\n");
      return 1;
  }

  int start = atoi(argv[2]);
  bfs(graph, start);
  graph_free(graph);

  graph_t *topo1 = file_to_graph("ø5g5.txt");
  graph_t *topo2 = file_to_graph("ø5g7.txt");
  topological_sort(topo1);
  topological_sort(topo2);
  graph_free(topo1);
  graph_free(topo2);
  return 0;
}


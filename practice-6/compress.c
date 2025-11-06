#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct huffman_node {
  unsigned char data;
  unsigned freq;
  struct huffman_node *left, *right;
};

struct huffman_heap {
  struct huffman_node **nodes;
  unsigned size;
  unsigned capacity;
};

struct huffman_code {
  char data;
  unsigned code;
  unsigned size;
};

struct huffman_codes {
  struct huffman_code *codes;
  unsigned size;
};

struct huffman_node *new_huffman_node(unsigned char data, 
                                      unsigned freq) {
  struct huffman_node *node = malloc(sizeof(struct huffman_node));
  if (!node) {
    perror("Error malloc");
    return NULL;
  }

  node->data = data;
  node->freq = freq;
  node->left = node->right = NULL;
  return node;
}

struct huffman_heap* new_huffman_heap(unsigned capacity) {
  struct huffman_heap *heap = malloc(sizeof(struct huffman_heap));
  if (!heap) {
    perror("Error malloc");
    return NULL;
  }

  heap->nodes = calloc(capacity, 
                       sizeof(struct huffman_node *));
  if (!heap->nodes) {
    perror("Error calloc");
    return NULL;
  }

  heap->size = 0;
  heap->capacity = capacity;
  return heap;
} 

void swap_huffman_nodes(struct huffman_node **a, 
                        struct huffman_node **b) {
  struct huffman_node *t = *a;
  *a = *b;
  *b = t;
}

void huffman_heapify_min(struct huffman_heap *heap, int index) {
  int smallest = index;
  int left = 2 * index + 1;
  int right = 2 * index + 2;

  if (left < heap->size && heap->nodes[left]->freq 
    < heap->nodes[smallest]->freq) {
    smallest = left;
  }

  if (right < heap->size && heap->nodes[right]->freq 
    < heap->nodes[smallest]->freq) {
    smallest = right;
  }

  if (smallest != index) {
    swap_huffman_nodes(&heap->nodes[index], &heap->nodes[smallest]);
    huffman_heapify_min(heap, smallest);
  }
}

struct huffman_node *huffman_extract_min(struct huffman_heap *heap) {
  struct huffman_node *node = heap->nodes[0];
  heap->nodes[0] = heap->nodes[heap->size - 1];
  heap->size--;
  huffman_heapify_min(heap, 0);
  return node;
}

void huffman_insert_min(struct huffman_heap *heap, 
                        struct huffman_node *node) {
  heap->size++;
  int i = heap->size - 1;

  while (i && node->freq < heap->nodes[(i - 1) / 2]->freq) {
    heap->nodes[i] = heap->nodes[(i - 1) / 2];
    i = (i - 1) / 2;
  }

  heap->nodes[i] = node;
}

unsigned *frequencies(FILE *in) {
  unsigned *freq = calloc(256, sizeof(unsigned));
  if (!freq) {
    perror("Error calloc");
    return NULL;
  }

  int c;
  while ((c = fgetc(in)) != EOF)
    freq[(unsigned char)c]++;

  return freq;
}

unsigned count_unique(unsigned *freq) {
  unsigned unique = 0;
  for (int i = 0; i < 256; i++)
    if (freq[i] > 0) unique++;

  return unique;
}

struct huffman_node *build_huffman_tree(unsigned *freq, 
                                        unsigned unique) {
  struct huffman_heap *heap = new_huffman_heap(unique);
  for (int i = 0; i < 256; i++)
    if (freq[i] > 0)
      heap->nodes[heap->size++] = new_huffman_node((unsigned char)i, freq[i]);

  for (int i = (heap->size - 1) / 2; i >= 0; i--)
    huffman_heapify_min(heap, i);

  while (heap->size > 1) {
    struct huffman_node *left = huffman_extract_min(heap);
    struct huffman_node *right = huffman_extract_min(heap);
    struct huffman_node *parent = new_huffman_node(0, left->freq + right->freq);

    parent->left = left;
    parent->right = right;
    huffman_insert_min(heap, parent);
  }

  struct huffman_node *root = huffman_extract_min(heap);
  free(heap->nodes);
  free(heap);
  return root;
}

struct huffman_codes *new_huffman_codes() {
  struct huffman_codes *codes = malloc(sizeof(struct huffman_codes));
  if (!codes) {
    perror("Error malloc");
    return NULL;
  }

  codes->codes = calloc(256, sizeof(struct huffman_code));
  if (!codes->codes) {
    perror("Error calloc");
    return NULL;
  }

  codes->size = 0;
  return codes;
}

void generate_huffman_codes(struct huffman_node *root, struct huffman_codes *codes,
                            unsigned code, unsigned size) {
  if (!root)
    return;

  if (!root->left && !root->right) {
    unsigned char data = root->data;
    codes->codes[data].data = data;
    codes->codes[data].code = code;
    codes->codes[data].size = size;
    codes->size++;
    return;
  }

  generate_huffman_codes(root->left, codes, code << 1, size + 1);
  generate_huffman_codes(root->right, codes, (code << 1) | 1, size + 1);
}

int compress(const char *input, const char *output) {
  FILE *in = fopen(input, "rb");
  FILE *out = fopen(output, "wb");
  if (!in || !out) {
    perror("Error file");
    return 1;
  }

  unsigned *freq = frequencies(in);
  unsigned unique = count_unique(freq);

  struct huffman_node *root = build_huffman_tree(freq, unique);
  struct huffman_codes *codes = new_huffman_codes();
  generate_huffman_codes(root, codes, 0, 0);
  fwrite(freq, sizeof(unsigned), 256, out);

  unsigned char byte = 0;
  int bits = 0;

  rewind(in);
  int c;
  while ((c = fgetc(in)) != EOF) {
    unsigned code = codes->codes[c].code;
    unsigned len  = codes->codes[c].size;
    for (int i = len - 1; i >= 0; i--) {
      byte = (byte << 1) | ((code >> i) & 1);
      bits++;
      if (bits == 8) {
        fputc(byte, out);
        bits = 0;
        byte = 0;
      }
    }
  }

  if (bits > 0) {
    byte <<= (8 - bits);
    fputc(byte, out);
  }

  fclose(in);
  fclose(out);
  return 0;
}

int decompress(const char *input, const char *output) {
  FILE *in = fopen(input, "rb");
  FILE *out = fopen(output, "wb");
  if (!in || !out) {
    perror("Error file");
    return 1;
  }

  fclose(in);
  fclose(out);
  return 0;
}

int main(const int argc, const char **argv) {
  if (argc < 4 )
    goto badexit;

  const char *mode = argv[1];
  const char *input = argv[2];
  const char *output = argv[3];

  int ret = -1;
  if (strcmp(mode, "-c") == 0)
    ret = compress(input, output);
  else if (strcmp(mode, "-d") == 0)
    ret = decompress(input, output);
  else
    goto badexit;

  return ret;

badexit:
  printf("usage: %s -[c/d] [input] [output]\n", argv[0]);
  return 1;
}

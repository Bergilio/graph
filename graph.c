#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024
#define MAX_CODE_SIZE 64
#define NO_NODE ((size_t)-1)

// Macro to dynamically allocate memory to an array, and to append items to that
// same array, growing as needed
#define da_append(xs, x)                                                       \
    do {                                                                       \
        if (xs->count >= xs->capacity) {                                       \
            if (xs->capacity == 0)                                             \
                xs->capacity = 256;                                            \
            xs->capacity *= 2;                                                 \
            xs->items = realloc(xs->items, xs->capacity * sizeof(*xs->items)); \
        }                                                                      \
        xs->items[xs->count++] = x;                                            \
    } while (0)

// Struct to abstract complexity from c strings and their immutability
typedef struct {
    const char *str;
    size_t size;
} String_View;

String_View str_to_sv(const char *str) {
    return (String_View){str, strlen(str)};
}

void trim_left(String_View *sv) {
    while (sv->size > 0 && isspace((unsigned char)*(sv->str))) {
        sv->str += 1;
        sv->size -= 1;
    }
}

void trim_right(String_View *sv) {
    while (sv->size > 0 && isspace((unsigned char)sv->str[sv->size - 1])) {
        sv->size -= 1;
    }
}

void trim(String_View *sv) {
    trim_left(sv);
    trim_right(sv);
}

// Function to check if string is just made of whiteapces
bool sv_is_empty(String_View *sv) {
    for (size_t i = 0; i < sv->size; ++i) {
        if (!isspace((unsigned char)sv->str[i])) {
            return false;
        }
    }
    return true;
}

// Function to check if string contains another
bool sv_contains(String_View *sv, const char *s, size_t n) {
    if (n == 0) {
        return true;
    }
    for (size_t i = 0; i + n <= sv->size; ++i) {
        if (memcmp(sv->str + i, s, n) == 0) {
            return true;
        }
    }
    return false;
}

// Function to tokenize String_View given a delimiter
bool sv_split(String_View *sv, String_View *r, const char *del,
              size_t del_size) {
    if (del_size > 0) {
        for (size_t i = 0; i + del_size <= sv->size; ++i) {
            if (memcmp(sv->str + i, del, del_size) == 0) {
                r->str = sv->str + i + del_size;
                r->size = sv->size - i - del_size;
                sv->size = i;
                return true;
            }
        }
    }
    r->str = sv->str + sv->size;
    r->size = 0;
    return false;
}

int sv_to_int(String_View *sv) {
    trim(sv);
    return strtol(sv->str, NULL, 10);
}

typedef struct Node Node;

// Edge structure, only holds the destination
typedef struct {
    size_t dest;
    int weight;
} Edge;

// Node structure, implemented with adjancy list
struct Node {
    size_t id;
    char code[MAX_CODE_SIZE];
    int dist;
    Edge *items;
    size_t count;
    size_t capacity;
};

// Graph structure, simply store the nodes
typedef struct {
    Node *items;
    size_t count;
    size_t capacity;
} Graph;

// List to store node ids, used just for bfs
typedef struct {
    size_t *items;
    size_t count;
    size_t capacity;
} List;

// Function to get node from code
Node *get_node_from_code(Graph *graph, const char *code) {
    for (size_t i = 0; i < graph->count; ++i) {
        if (strcmp(code, graph->items[i].code) == 0) {
            return &graph->items[i];
        }
    }
    return NULL;
}

// Function that traverses the graph in a breath first approach
void bfs(Graph *graph) {
    if (graph->count == 0) {
        return;
    }
    bool *visited = calloc(graph->count, sizeof(bool));
    Node *cur = NULL;

    for (size_t s = 0; s < graph->count; ++s) {
        if (visited[s]) {
            continue;
        }
        List list = {0};
        da_append((&list), graph->items[s].id);
        visited[s] = true;

        size_t i = 0;
        while (i < list.count) {
            cur = &graph->items[list.items[i]];
            for (int j = 0; j < cur->count; ++j) {
                size_t d = cur->items[j].dest;
                if (!visited[d]) {
                    visited[d] = true;
                    da_append((&list), d);
                }
            }
            printf(" %s", cur->code);
            ++i;
        }

        free(list.items);
    }
    free(visited);
    printf("\n");
}

// Frame strucutre, holds the stack call of dfs algorithm at each step
typedef struct {
    size_t node_id;
    size_t next_child;
} Frame;

// Stack struct, used to mimic a stack that holds frames, needed for the dfs
// algorithm
typedef struct {
    Frame *items;
    size_t count;
    size_t capacity;
} Stack;

// Function that traverses the graph in a depth first approach
void dfs(Graph *graph) {
    if (graph->count == 0) {
        return;
    }
    bool *visited = calloc(graph->count, sizeof(bool));

    for (size_t s = 0; s < graph->count; ++s) {
        if (visited[s]) {
            continue;
        }
        Stack stack = {0};
        visited[s] = true;
        Frame f0 = {graph->items[s].id, 0};
        da_append((&stack), f0);
        printf(" %s", graph->items[s].code);

        while (stack.count > 0) {
            Frame *cur = &stack.items[stack.count - 1];
            Node *node = &graph->items[cur->node_id];
            if (cur->next_child >= graph->items[cur->node_id].count) {
                stack.count -= 1;
                continue;
            }
            size_t dest = node->items[cur->next_child++].dest;
            if (!visited[dest]) {
                visited[dest] = true;
                printf(" %s", graph->items[dest].code);
                f0 = (Frame){dest, 0};
                da_append((&stack), f0);
            }
        }

        free(stack.items);
    }
    free(visited);
    printf("\n");
}

// Function that resets distances to be used before performing path algorithms
void reset_distances(Graph *graph) {
    for (size_t i = 0; i < graph->count; ++i) {
        graph->items[i].dist = -1;
    }
}

// Struct to be used in the Priorety_Queue collection
// It holds a node id and it's current dist, which serves as the factor to
// calculate the priority
typedef struct {
    size_t node_id;
    int dist;
} Queue_Node;

// Struct to represent a priority queue, holds Queue_Nodes
// Field begin serves to indicate where the actual inital idx is, as the pop
// function only shifts it (no reallocation actually needed)
typedef struct {
    Queue_Node *items;
    size_t count;
    size_t capacity;
    size_t begin;
} Priority_Queue;

// Rework of the da_append macro, only for priority queues
// The reallocation process is the same, but the appending process finds the
// index to where this new node should be added, according to the priority
// policy, and it moves all items from that index to the end to the next
// position in the collection. Notice that begin is used a lot to offset the
// nodes that were popped but are still in the collection, the 0 index is not
// necessarily the front of the queue in the state we want to represent
void pq_append(Priority_Queue *pq, Queue_Node n) {
    if (pq->count + pq->begin >= pq->capacity) {
        if (pq->capacity == 0) {
            pq->capacity = 256;
        }
        pq->capacity *= 2;
        pq->items = realloc(pq->items, pq->capacity * sizeof(*pq->items));
    }
    if (pq->count == 0) {
        pq->items[pq->begin] = n;
        ++pq->count;
        return;
    }
    size_t n_pos = 0;
    while (n_pos < pq->count && n.dist > pq->items[n_pos + pq->begin].dist) {
        ++n_pos;
    }
    for (size_t i = pq->count; i > n_pos; --i) {
        pq->items[i + pq->begin] = pq->items[i - 1 + pq->begin];
    }
    pq->items[n_pos + pq->begin] = n;
    ++pq->count;
}

// Simple pop function without reallocation in memory, shifts the begin offset
// that represents the actual front of the queue
void pq_pop(Priority_Queue *pq) {
    if (pq->count != 0) {
        --pq->count;
        ++pq->begin;
    }
}

// Function that computes the shortest path to every node, starting from a given
// node Dijkstra's algorithm
void shortest_path(Graph *graph, const char *s) {
    reset_distances(graph);
    Node *cur = get_node_from_code(graph, s);
    if (cur == NULL) {
        return;
    }
    bool *visited = calloc(graph->count, sizeof(bool));
    Priority_Queue pq = {0};
    cur->dist = 0;
    pq_append(&pq, (Queue_Node){cur->id, cur->dist});

    while (pq.count > 0) {
        cur = &graph->items[pq.items[pq.begin].node_id];
        if (!visited[cur->id]) {
            for (size_t j = 0; j < graph->items[cur->id].count; ++j) {
                Edge *e = &cur->items[j];
                Node *dest = &graph->items[e->dest];
                if (dest->dist < 0 || dest->dist > cur->dist + e->weight) {
                    dest->dist = cur->dist + e->weight;
                    pq_append(&pq, (Queue_Node){dest->id, dest->dist});
                }
            }
        }
        visited[cur->id] = true;
        pq_pop(&pq);
    }

    free(pq.items);
    free(visited);
}

// Deallocate all memory from a graph
void free_graph(Graph *graph) {
    for (size_t i = 0; i < graph->count; ++i) {
        Node *node = &graph->items[i];
        free(node->items);
    }
    free(graph->items);
    graph->items = NULL;
    graph->count = 0;
    graph->capacity = 0;
}

// Function to parse the weight (if it exists) from the node, default is 1
int parse_weight(String_View *edge) {
    String_View w_sv;
    if (sv_split(edge, &w_sv, ":", 1)) {
        return sv_to_int(&w_sv);
    }
    return 1;
}

// Function to create and add a node from its code, returns it's id
// If found to be a duplicate, return original's node id
size_t add_node(Graph *graph, String_View *node_sv) {
    bool found_node = false;
    size_t i = 0;
    while (i < graph->count) {
        if (strlen(graph->items[i].code) == node_sv->size &&
            strncmp(graph->items[i].code, node_sv->str, node_sv->size) == 0) {
            found_node = true;
            break;
        }
        ++i;
    }
    if (!found_node) {
        Node new_node = {graph->count, "", -1, NULL, 0, 0};
        memcpy(new_node.code, node_sv->str, node_sv->size);
        new_node.code[node_sv->size] = '\0';
        da_append(graph, new_node);
        i = new_node.id;
    }
    return i;
}

// Function to add an edge with destination to child_id and weight to the
// adjacency list of parent id
void add_edge(Graph *graph, size_t parent_id, size_t child_id, int weight) {
    bool found_edge = false;
    Node *parent_node = &graph->items[parent_id];
    for (size_t i = 0; i < parent_node->count; ++i) {
        if (parent_node->items[i].dest == child_id) {
            parent_node->items[i].weight = weight;
            found_edge = true;
            break;
        }
    }
    if (!found_edge) {
        Edge new_edge = {child_id, weight};
        da_append(parent_node, new_edge);
    }
}

// Function to parse a file with handmade notation representing a directed graph
bool parse_graph_file(const char *path, Graph *graph) {
    FILE *fp = fopen(path, "r");
    if (fp != NULL) {
        char buffer[BUF_SIZE];
        while (fgets(buffer, BUF_SIZE, fp)) {
            String_View node = str_to_sv(buffer);
            if (sv_is_empty(&node)) {
                continue;
            }
            String_View adjacents;
            bool found_split = sv_split(&node, &adjacents, "->", 2);
            trim(&node);
            if (sv_is_empty(&node) || sv_contains(&node, "|", 1) ||
                sv_contains(&node, ":", 1) || node.size >= MAX_CODE_SIZE) {
                printf("found an error at: %s\n", buffer);
                continue;
            }
            size_t parent_id = add_node(graph, &node);
            if (found_split) {
                String_View rest;
                while (!sv_is_empty(&adjacents)) {
                    found_split = sv_split(&adjacents, &rest, "|", 1);
                    if (sv_is_empty(&adjacents) ||
                        sv_contains(&adjacents, "->", 2)) {
                        printf("found an error at: %s\n", buffer);
                        break;
                    }
                    int weight = parse_weight(&adjacents);
                    trim(&adjacents);
                    if (adjacents.size >= MAX_CODE_SIZE ||
                        sv_is_empty(&adjacents)) {
                        printf("found an error at: %s\n", buffer);
                        adjacents = rest;
                        continue;
                    }
                    size_t child_id = add_node(graph, &adjacents);
                    add_edge(graph, parent_id, child_id, weight);
                    adjacents = rest;
                }
            }
        }
    } else {
        return false;
    }
    fclose(fp);
    return true;
}

// Function that prints each node and its edges and respective weights
void print_graph(Graph *graph) {
    for (size_t i = 0; i < graph->count; ++i) {
        Node *n = &graph->items[i];
        printf("Node: %s | %d edges\n", n->code, (int)n->count);
        for (int j = 0; j < n->count; ++j) {
            Edge *e = &n->items[j];
            printf("Edge: %d | Dest: %s | Weight: %d\n", j,
                   graph->items[e->dest].code, e->weight);
        }
        printf("--------------\n");
    }
}

int main(void) {
    const char *path = "graphs/first.gph";
    Graph graph = {0};
    if (!parse_graph_file(path, &graph)) {
        printf("Found an error\n");
        return 1;
    }
    print_graph(&graph);
    free_graph(&graph);
    return 0;
}

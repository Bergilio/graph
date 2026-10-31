# Graph Implementation

Implementation of a graph in c, using handmade file parsing with notation as rules below will specify.

Just a play project.

---

## Graph characteristics

- Represents directed graphs. Any need to represent an undirected graph will have to simply translate it to a directed graph.
- `Graph` holds an array of Nodes, the array's size, and the adjacency matrix;
- `Node` holds an id, a code (as parsed from the file), dist, and an array with the adjacent edges (and it's size);
- `Edge` holds the destination Node id, and the weight of the edge;
- `dist` serves the purpose of helping with algorithm implementations.

---

## File parsing

It allows to construct a graph structure from a special file, that follows a certain notation.

The parsing logic can be found entirely in `parse_graph_file()`, `parse_weight()`, `add_node()` and `add_edge()`.

To be noted that `String_View` was created to facilitate and organize the parsing logic, and it's related functions are used a lot through the parsing function.

### Notation

Notation (n -> a1:10 | a2:12 | a3 ...):

1. Every line represents an adjacency list
    
2. To the left of the '->' you find the code(char*) of the node that owns the list
    
3. To the right of the '->' you find one or more codes(char*) of the nodes that can be found in the list, separated by '|'
    
4. Nodes with empty lists can be defines as 'e ->' or 'e'. In case they have been referenced in other lists, this is optional representation
    
5. '-> a1 | a2', 'a1 | a2' are not accepted, the owner of the edges has to be specified
    
6. Leading and trailing white spaces are ignored, i.e., ' n -> a1 | a2 ' = 'n->a1|a2'
    
7. Nodes can be named with multiple chars, that might include white spaces.
' Hello World -> Lore ipsum ' will result in a node with code='Hello World' and one node in its adjacency list where code='Lore ipsum'
 
8. '|', ':' and '->' are not allowed in node names
    
9. A single trailing '|' is valid, as long as it is specified at least one adjacent node, e.g. 'n -> a1 |'
    
10. After the name of a node given after '->' (as an adjacent node), the weight of the edge can be specified, following ':'
    
11. Once again, any amount of leading or trailing white spaces can be existent between the code of the node or the wight and ':''
    
12. The weights can only be integers (for now)    

---

## Graph Algorithms

The implementation includes robust, iterative graph traversal and path finding algorithms that safely handle disconnected graph components.

- `bfs(Graph *graph)`: Traverses the graph in a Breadth-First Search approach. It uses a dynamic `List` queue and an outer loop to ensure all disconnected components of the graph are visited.
- `dfs(Graph *graph)`: Traverses the graph in a Depth-First Search approach. It uses an explicit `Stack` of `Frame` structures (which track the current node and the `next_child` index) to mimic the system call stack. This avoids recursion depth limits and prevents duplicate node processing, while also handling disconnected components.
- `shortest_path(Graph *graph, char *s)`: Computes the shortest path from a given starting node `s` to all other reachable nodes using **Dijkstra's algorithm**. It utilizes a custom `Priority_Queue` to efficiently extract the unvisited node with the minimum distance, combined with a lazy-deletion strategy to handle stale queue entries.
- `reset_distances(Graph *graph)`: A helper function that resets the `dist` field of all nodes to `-1` (representing infinity/unreachable) before executing path finding algorithms.
- `get_node_from_code(Graph *graph, char *code)`: A helper function that safely searches the graph for a node matching the given code string, returning a pointer to the node or `NULL` if not found.

---

## String_View

Wrapper around simple c strings (char*). The structure simply holds the pointer to the beginning of the string, and it's size, with this last one being the true delimiter of the end of the string (not '\0').

- `str_to_sv(char*)`: takes normal c string and constructs String_View, which is returned;
- `trim_left(String_View*)`: Cleans leading white spaces;
- `trim_right(String_View*)`: Same as trim_left, but with trailing white spaces;
- `trim(String_View*)`: Makes use of trim_left and trim_right to clean up the string from leading and trailing white spaces;
- `sv_is_empty(String_View*)`: Checks if a String_View is empty or just make of white spaces;
- `sv_contains(String_View*, const char*, size_t)`: Checks if String_View contains a given c string of specified size;
- `sv_split(String_View*, String_View*, const char*, size_t)`: Splits the first String_View argument in the first delimiter of specified size found. After call, the first sv keeps the first part of the string, while the second sv keeps the second half. The delimiter is thrown away. Returns true on success.
- `sv_to_int()` attempts to implement a parser of string to int using String_View

---

## Dynamic Arrays

To facilitate the allocation of new nodes to the graph and new edges to the adjacency array, the structures `Graph` and `Node` make use of an in house implementation of dynamic arrays.

For this, both structures follow a default representation of dynamic arrays, having a field, `items`, for the array, `count` to store the amount of items currently stored, and `capacity`, to store the current capacity.

In the code, the arrays are usually initialized to NULL, and the macro `da_append(xs, x)` is responsible for the allocation of memory in the heap. `xs` is a pointer to the array, `x` is a copy of an element.

### Memory Management

Proper manual memory management is enforced to prevent memory leaks.

- `free_graph(Graph *graph)`: Safely deallocates all dynamically allocated memory associated with the graph. It iterates through every node to free its individual `items` (edges) array, and finally frees the main `items` (nodes) array of the graph, resetting all counters to zero.

---

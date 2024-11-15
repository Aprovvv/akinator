#ifndef TREE_H
#define TREE_H

#define STRLEN 256

struct tree_node_t
{
    char str[STRLEN];
    tree_node_t* yes;
    tree_node_t* no;
};

tree_node_t* new_node(char str[STRLEN]);
void node_insert(tree_node_t* node, int val);
void branch_delete(tree_node_t* node);
void tree_print(FILE* fp, tree_node_t* node);
void tree_graph_dump(tree_node_t* node);

#endif

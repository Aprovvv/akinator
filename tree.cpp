#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

static void add_dot_node(FILE* fp,
                         tree_node_t* node,
                         tree_node_t* the_node,
                         size_t code);

tree_node_t* new_node(char str[STRLEN])
{
    tree_node_t* p = (tree_node_t*)calloc(sizeof(tree_node_t), 1);
    if (p)
        strcpy(p->str, str);
    return p;
}

void branch_delete(tree_node_t* node)
{
    if (node->yes)
        branch_delete(node->yes);
    if (node->no)
        branch_delete(node->no);
    free(node);
}

tree_node_t* tree_search_by_name (tree_node_t* node,
                                  char* str, list_t* list)
{
    tree_node_t* ptr = NULL;
    if (node->yes && node->no)
    {
        list_push_back(list, 1);
        if (ptr = tree_search_by_name(node->yes, str, list))
            return ptr;
        list_pop_back(list);

        list_push_back(list, 0);
        if (ptr = tree_search_by_name(node->no, str, list))
            return ptr;
        list_pop_back(list);

        return NULL;
    }
    if (strcmp(node->str, str) == 0)
        return node;

    return NULL;
}

void tree_print(FILE* fp, tree_node_t* node)
{
    if (node == NULL)
        return;
    fprintf(fp, "(");
    fprintf(fp, "%s", node->str);
    if (node->yes)
        tree_print(fp, node->yes);
    if (node->no)
        tree_print(fp, node->no);
    fprintf(fp, ")");
}

void tree_graph_dump(tree_node_t* node, tree_node_t* the_node)
{
    static int numb = 0;
    numb++;
    if (numb == 1)
    {
        system("rm -rf pngs/*.png");
        system("rm -rf dots/*.dot");
    }
    char dot_name[20];
    char png_name[20];
    sprintf(dot_name, "dots/%d.dot", numb);
    sprintf(png_name, "pngs/%d.png", numb);
    FILE* fp = fopen(dot_name, "w");
    if (!fp)
        return;

    fprintf(fp, "digraph G{\n"
                "\tnode[shape=ellipse]\n"
                "\t{\n");
    add_dot_node(fp, node, the_node, 1);

    fprintf(fp, "\t}\n}");
    fclose(fp);
    char command[55] = "";
    sprintf(command, "dot -Tpng %s -o %s", dot_name, png_name);
    system(command);
}

static void add_dot_node(FILE* fp, tree_node_t* node, tree_node_t* the_node, size_t code)
{
    if (node == the_node)
        fprintf(fp, "%zu [label = \"%s\"; color=\"red\"];\n", code, node->str);
    else
        fprintf(fp, "%zu [label = \"%s\"];\n", code, node->str);
    if (node->yes)
    {
        fprintf(fp, "%zu -> %zu [label = да];\n", code, code*2 + 0);
        add_dot_node(fp, node->yes, the_node, code*2 + 0);
    }
    if (node->no)
    {
        fprintf(fp, "%zu -> %zu [label = нет]\n", code, code*2 + 1);
        add_dot_node(fp, node->no, the_node, code*2 + 1);
    }
}

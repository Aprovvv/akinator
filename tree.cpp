#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

static void add_dot_node(FILE* fp, tree_node_t* node, size_t code);

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

void tree_graph_dump(tree_node_t* node)
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
    add_dot_node(fp, node, 1);

    fprintf(fp, "\t}\n}");
    fclose(fp);
    char command[55] = "";
    sprintf(command, "dot -Tpng %s -o %s", dot_name, png_name);
    system(command);
}

static void add_dot_node(FILE* fp, tree_node_t* node, size_t code)
{
    fprintf(fp, "%zu [label = \"%s\"];\n", code, node->str);
    if (node->yes)
    {
        fprintf(fp, "%zu -> %zu [label = да];\n", code, code*2 + 0);
        add_dot_node(fp, node->yes, code*2 + 0);
    }
    if (node->no)
    {
        fprintf(fp, "%zu -> %zu [label = нет]\n", code, code*2 + 1);
        add_dot_node(fp, node->no, code*2 + 1);
    }
}

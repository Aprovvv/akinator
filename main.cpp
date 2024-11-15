#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "tree.h"
#include "list/list.h"

#define CMD_SIZE 64
#define QUOTES(a) #a

const int ADD_VAL = 228;

static tree_node_t* tree_from_file (FILE* data);
static int run_akinator (FILE* data, tree_node_t* root);
static int add_object(tree_node_t* node);
static int fgetstr (FILE* fp, char* str, int n);
static int ask_node (tree_node_t* node, list_t* list);
static int print_node (FILE* data, tree_node_t* node, int code);
static int tree_to_file (FILE* data, tree_node_t* root);

int main()
{
    FILE* data = fopen("data.txt", "r+");
    tree_node_t* root = tree_from_file(data);
    rewind(data);
    assert(root != NULL);
    putchar('\n');
    tree_print(stderr, root);
    tree_graph_dump(root);
    char cmd[CMD_SIZE] = "";
    putchar('\n');
    while (1)
    {
        printf("Введите start для начала, end для выхода\n");
        scanf("%s", cmd);//TODO: ограничение по размеру
        if (strcmp(cmd, "start") == 0)
            run_akinator(data, root);
        if (strcmp(cmd, "end") == 0)
            break;
    }
    tree_graph_dump(root);
    fclose(data);
    branch_delete(root);
}

static int run_akinator (FILE* data, tree_node_t* root)
{
    list_t* list = list_init(8);
    if (ask_node(root, list) == ADD_VAL)
    {
        tree_to_file(data, root);
    }
    list_destroy(list);
    return 0;
}

static int ask_node (tree_node_t* node, list_t* list)
{
    int ch = 0;
    printf("Он(а) %s? [y/n] ", node->str);
    char str[STRLEN] = "";
    getchar();
    ch = getchar();
    if (ch == 'y')
    {
        if (node->yes)
        {
            list_push_back(list, 1);
            return ask_node(node->yes, list);
        }
        else
        {
            printf("Я так и знал!\n");
            return 0;
        }
    }
    if (ch == 'n')
    {
        if (node->no)
        {
            list_push_back(list, 0);
            return ask_node(node->no, list);
        }
        else
        {
            add_object(node);
            return ADD_VAL;
        }
    }
    return 0;
}

static int add_object (tree_node_t* node)
{
    int ch = 0;
    char str[STRLEN] = "";
    printf("Упс! Я таких не знаю :(\n"
           "А кто это?\n");
    fgetstr(stdin, str, STRLEN);
    node->yes = new_node(str);
    node->no = new_node(node->str);
    printf("Чем отличается %s от %s?\n",
           node->yes->str, node->no->str);
    fgetstr(stdin, node->str, STRLEN);
    printf("%s %s, а %s нет? [y/n]\n",
           node->yes->str, node->str, node->no->str);
    if ((ch = getchar()) == 'n')
    {
        tree_node_t* temp = node->yes;
        node->yes = node->no;
        node->no = temp;
        printf("%s %s, а %s нет? [y/n]\n",
           node->yes->str, node->str, node->no->str);
        getchar();
        if ((ch = getchar()) == 'y')
        {
            return 0;
        }
        else
        {
            fprintf(stderr, "бля ты че сделал нахуй\n");
            abort();
        }
    }
    return 0;
}

static int tree_to_file (FILE* data, tree_node_t* root)
{
    rewind(data);
    print_node(data, root, 0);
    return 0;
}

static int print_node (FILE* data, tree_node_t* node, int code)
{
    for (int i = 0; i < code; i++)
        fprintf(data, "\t");
    fprintf(data, "{\n");
    for (int i = 0; i < code; i++)
        fprintf(data, "\t");
    fprintf(data, "\"%s\"\n", node->str);

    if (node->no && node->yes)
    {
        print_node(data, node->yes, code + 1);
        print_node(data, node->no, code + 1);
    }
    for (int i = 0; i < code; i++)
        fprintf(data, "\t");
    fprintf(data, "}\n");
    return 0;
}

static tree_node_t* tree_from_file (FILE* data)
{
    int ch = 0;
    char str[STRLEN] = "";
    while((ch = fgetc(data)) != '{')
    {
        //fprintf(stderr, "%d ", ch);
        if (ch == EOF)
            return NULL;
    }
    while((ch = fgetc(data)) != '\"')
    {
        if (ch == EOF)
            return NULL;
    }
    fscanf(data, "%[^\"]", str);
    //fprintf(stderr, "[%s]", str);
    tree_node_t* node = new_node(str);
    ch = fgetc(data);
    while (ch != '{' && ch != '}')
        ch = fgetc(data);
    if (ch == '}')
    {
        node->no = NULL;
        node->yes = NULL;
        fprintf(stderr, "}");
    }
    else
    {
        ungetc(ch, data);
        node->yes = tree_from_file(data);
        node->no = tree_from_file(data);
    }
    return node;
}

static int fgetstr (FILE* fp, char* str, int n)
{
    int ch = 32, i = 0;
    while (isspace(ch))
    {
        ch = fgetc(fp);
    }
    while (ch != '\n' && ch != '\0' && ch != EOF && i < n)
    {
        str[i++] = (char)ch;
        ch = fgetc(fp);
    }
    str[i] = 0;
    return i;
}

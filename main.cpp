#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "tree.h"

#define CMD_SIZE 64
#define QUOTES(a) #a

const int ADD_VAL = 228;

static tree_node_t* tree_from_file (FILE* data);
static int run_akinator (FILE* data, tree_node_t* root);
static int add_object(tree_node_t* node);
static int fgetstr (FILE* fp, char* str, int n);
static int ask_node (tree_node_t* node);
static int print_node (FILE* data, tree_node_t* node, int code);
static int tree_to_file (FILE* data, tree_node_t* root);

//FIXME: обработка EOF
//TODO: int на size_t

int main()
{
    FILE* data = fopen("data.txt", "r+");
    if (data == NULL)
    {
        fprintf(stderr, "Ошибка открытия файла\n");
        return EXIT_FAILURE;
    }
    tree_node_t* root = tree_from_file(data);
    if (root == NULL)
    {
        fprintf(stderr, "Ошибка чтения дерева из файла\n");
        return EXIT_FAILURE;
    }
    rewind(data);
    tree_graph_dump(root);
    char cmd[CMD_SIZE+1] = "";
    while (1)
    {
        printf("Введите старт (start) для начала, выход (end) для выхода\n");
        scanf("%s", cmd);//TODO: ограничение по размеру
        if (strcmp(cmd, "старт") == 0 || strcmp(cmd, "start") == 0)
            run_akinator(data, root);
        if (strcmp(cmd, "выход") == 0 || strcmp(cmd, "end") == 0)
            break;
    }
    tree_graph_dump(root);
    fclose(data);
    branch_delete(root);
}

static int run_akinator (FILE* data, tree_node_t* root)
{
    if (ask_node(root) == ADD_VAL)
        tree_to_file(data, root);
    return 0;
}

static int ask_node (tree_node_t* node)
{
    int ch = 0;
    while (1)
    {
        printf("Он(а) %s? [y/n]\n", node->str);
        getchar();
        ch = getchar();
        switch(ch)
        {
        case 'y':
        case 'д':
            if (node->yes)
            {
                return ask_node(node->yes);
            }
            else
            {
                printf("Я так и знал!\n");
                return 0;
            }
            break;
        case 'n':
        case 'н':
            if (node->no)
            {
                return ask_node(node->no);
            }
            else
            {
                add_object(node);
                return ADD_VAL;
            }
            break;
        case EOF:
            return 0;
        default:
            printf("Неопознанная команда:(\n");
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
        while(1)
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
                fprintf(stderr, "Ошибка! %s должен относиться к хоть какой-то "
                        "категории. Попробуйте еще раз!\n", node->no->str);
            }
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
        if (ch == EOF)
            return NULL;
    }
    while((ch = fgetc(data)) != '\"')
    {
        if (ch == EOF)
            return NULL;
    }
    fscanf(data, "%[^\"]", str);
    tree_node_t* node = new_node(str);
    ch = fgetc(data);
    while (ch != '{' && ch != '}')
        ch = fgetc(data);
    if (ch == '}')
    {
        node->no = NULL;
        node->yes = NULL;
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

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
const int ERROR_VAL = 1;

static tree_node_t* tree_from_file (FILE* data);
static int run_akinator (FILE* data, tree_node_t* root);
static int add_object(tree_node_t* node);
static int fgetstr (FILE* fp, char* str, int n);
static int ask_node (tree_node_t* node);
static int print_node (FILE* data, tree_node_t* node, int code);
static int tree_to_file (FILE* data, tree_node_t* root);
static int read_word (FILE* fp, char* str, int n);
static int give_definition (tree_node_t* root, char* str);
static int search_node (tree_node_t* root, tree_node_t* node, char* str, list_t* list);

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
    tree_graph_dump(root, 0);
    char cmd[CMD_SIZE+1] = "";
    while (1)
    {
        printf("Введите старт (start) для начала, выход (end) для выхода\n");
        if (read_word(stdin, cmd, CMD_SIZE) == EOF)
            break;
        if (strcmp(cmd, "старт") == 0 || strcmp(cmd, "start") == 0)
            run_akinator(data, root);
        if (strcmp(cmd, "выход") == 0 || strcmp(cmd, "end") == 0)
            break;
        if (strcmp(cmd, "определение") == 0 || strcmp(cmd, "definition") == 0)
        {
            if (read_word(stdin, cmd, CMD_SIZE) == EOF)
                break;
            //printf("%s\n", cmd);
            give_definition(root, cmd);
        }
    }
    tree_graph_dump(root, 0);
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
        //getchar();
        ch = getchar();
        getchar();
        //printf("ch = %d\n", ch);
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
                return ask_node(node->no);
            else
                return add_object(node);
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
    if (fgetstr(stdin, str, STRLEN) == EOF)
        return ERROR_VAL;

    node->yes = new_node(str);
    node->no = new_node(node->str);
    if (node->yes == NULL || node->no == NULL)
        return ERROR_VAL;

    printf("Чем отличается %s от %s?\n",
           node->yes->str, node->no->str);
    if (fgetstr(stdin, node->str, STRLEN) == EOF)
        return ERROR_VAL;

    printf("%s %s, а %s нет? [y/n]\n",
           node->yes->str, node->str, node->no->str);
    while (1)
        switch (ch = getchar())
        {
        case 'y':
            return ADD_VAL;
        case EOF:
            return ERROR_VAL;
        case 'n':
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
                    return ADD_VAL;
                }
                else
                {
                    if (ch == EOF)
                        return ERROR_VAL;
                    fprintf(stderr, "Ошибка! %s должен относиться к хоть какой-то "
                            "категории. Попробуйте еще раз!\n", node->no->str);
                }
            }
        }
        default:
            printf("Неопознанная команда:(\n");
        }
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

static int give_definition (tree_node_t* root, char* str)
{
    list_t* list = list_init(8);
    search_node(root, root, str, list);
    //graph_dump(list);
    int size = list_size(list);
    if (size == 0)
    {
        printf("Объект не найден\n");
        return 0;
    }
    else
    {
        tree_node_t* tnode = root;
        list_node* lnode = get_start(list);
        printf("%s: ", str);
        for (int i = 0; i < size; i++)
        {
            if (get_val(lnode))
            {
                printf("%s, ", tnode->str);
                tnode = tnode->yes;
            }
            else
            {
                printf("не %s, ", tnode->str);
                tnode = tnode->no;
            }
            lnode = get_next(list, lnode);
        }
        printf("\n");
    }
    list_destroy(list);
    return 0;
}

static int search_node (tree_node_t* root, tree_node_t* node, char* str, list_t* list)
{
    //tree_graph_dump(root, node);
    if (node->yes && node->no)
    {
        list_push_back(list, 1);
        if (search_node(root, node->yes, str, list))
            return 1;
        list_pop_back(list);

        list_push_back(list, 0);
        if (search_node(root, node->no, str, list))
            return 1;
        list_pop_back(list);

        return 0;
    }
    if (strcmp(node->str, str) == 0)
        return 1;

    return 0;
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
    if (ch == EOF)
        return EOF;
    return i;
}

static int read_word (FILE* fp, char* str, int n)
{
    int ch = 32, i = 0;
    while (isspace(ch))
        ch = getc(fp);
    while (i < n && !isspace(ch) && ch != EOF)
    {
        str[i++] = (char)ch;
        ch = getc(fp);
    }
    str[i] = 0;
    if (ch == EOF)
        return EOF;
    return i;
}

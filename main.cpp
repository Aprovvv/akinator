#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tree.h"
#include "list/list.h"

#define CMD_SIZE 64
#define QUOTES(a) #a

const int ADD_VAL = 228;
const int ERROR_VAL = 1;

static int run_akinator (FILE* data, tree_node_t* root);
static int ask_node (tree_node_t* node);
static int add_object(tree_node_t* node);

static tree_node_t* tree_from_file (FILE* data);
static int tree_to_file (FILE* data, tree_node_t* root);
static int print_node (FILE* data, tree_node_t* node, int code);

static int give_definition (tree_node_t* root, char* str);
static int compare (tree_node_t* root, char* str1, char* str2);

static int fgetstr (FILE* fp, char* str, int n);
static int read_word (FILE* fp, char* str, int n);

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
    char cmd[CMD_SIZE] = "";
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
        if (strcmp(cmd, "сравни") == 0 || strcmp(cmd, "compare") == 0)
        {
            char str1[CMD_SIZE] = "";
            char str2[CMD_SIZE] = "";

            if (read_word(stdin, str1, CMD_SIZE) == EOF)
                break;
            if (read_word(stdin, str2, CMD_SIZE) == EOF)
                break;
            //printf("%s\n", cmd);
            compare(root, str1, str2);
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
    char cmd[CMD_SIZE];
    while (1)
    {
        printf("Он(а) %s? [y/n]\n", node->str);
        if (read_word(stdin, cmd, CMD_SIZE) == EOF)
            return ERROR_VAL;
        ch = cmd[0];
        if (ch == -48)
            ch = 208*256 + 256 + cmd[1];
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
    char cmd[CMD_SIZE];
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
    {
        if (read_word(stdin, cmd, CMD_SIZE) == EOF)
            return ERROR_VAL;
        ch = cmd[0];
        if (ch == -48)
            ch = 208*256 + 256 + cmd[1];
        switch (ch)
        {
        case 'д':
        case 'y':
            return ADD_VAL;
        case EOF:
            return ERROR_VAL;
        case 'н':
        case 'n':
        {
            while(1)
            {
                tree_node_t* temp = node->yes;
                node->yes = node->no;
                node->no = temp;
                printf("%s %s, а %s нет? [y/n]\n",
                        node->yes->str, node->str, node->no->str);
                if (read_word(stdin, cmd, CMD_SIZE) == EOF)
                    return ERROR_VAL;
                fprintf(stderr, "%d\n", ch);
                if (ch == -48)
                    ch = 208*256 + 256 + cmd[1];
                if (ch == 'y' || ch == 'д')
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
    tree_search_by_name(root, str, list);
    //graph_dump(list);
    size_t size = list_size(list);
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
        for (size_t i = 0; i < size; i++)
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

static int compare (tree_node_t* root, char* str1, char* str2)
{
    list_t* list1 = list_init(8);
    list_t* list2 = list_init(8);
    if (tree_search_by_name(root, str1, list1) == NULL)
    {
        printf("%s не найден:(\n", str1);
        return ERROR_VAL;
    }
    if (tree_search_by_name(root, str2, list2) == NULL)
    {
        printf("%s не найден:(\n", str2);
        return ERROR_VAL;
    }
    list_node* lnode1 = get_start(list1);
    list_node* lnode2 = get_start(list2);
    tree_node_t* tnode = root;
    size_t size1 = list_size(list1);
    size_t size2 = list_size(list2);
    size_t size = size1 > size2 ? size2 : size1;
    if (get_val(lnode1) == get_val(lnode2))
        printf("%s и %s объединяет то, что они ", str1, str2);
    else
        printf("У %s и %s нет ничего общего :(", str1, str2);

    for (size_t i = 0; i < size; i++)
    {
        if (get_val(lnode1) != get_val(lnode2))
            break;
        if (get_val(lnode1) && get_val(lnode2))
            printf("%s, ", tnode->str);
        else
            printf("не %s, ", tnode->str);
        //tree_graph_dump(root, tnode);
        if (get_val(lnode1))
            tnode = tnode->yes;
        else
            tnode = tnode->no;
        lnode1 = get_next(list1, lnode1);
        lnode2 = get_next(list2, lnode2);
    }
    printf("\n");
    if (get_val(lnode1))
        printf("Их различает то, что %s %s, а %s нет\n",
                str1, tnode->str, str2);
    if (get_val(lnode2))
        printf("Их различает то, что %s %s, а %s нет\n",
                str2, tnode->str, str1);
    list_destroy(list1);
    list_destroy(list2);
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

#include <stdio.h>
#include <stdlib.h>
#include "Header.h"

int main()
{
    struct Node* list;

    create_list(&list);

    push_back(&list, 'c');
    push_back(&list, 'd');
    push_back(&list, 'e');
    push_back(&list, 'f');
    printf("size : %d\n", size(list));
    print_list(list);
    push_front(&list, 'b');
    push_front(&list, 'a');

    printf("size : %d\n", size(list));
    print_list(list);
    pop_back(&list);

    printf("size : %d\n", size(list));
    print_list(list);

    pop_front(&list);

    printf("size : %d\n", size(list));
    print_list(list);

    insert_node(&list, 2, 'x');

    printf("size : %d\n", size(list));
    print_list(list);

    remove_node(&list, 2);

    printf("size : %d\n", size(list));
    print_list(list);

    remove_list(&list);
    return 0;
}

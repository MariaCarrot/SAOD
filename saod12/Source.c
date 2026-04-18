#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Header.h"
#include <string.h>


int main()
{
    clock_t start, end;
    double elapsed_time;
    struct Node* list = NULL;
    struct Metrics metrics;

    int sizes[] = { 1024, 2048, 4096, 8192, 16384, 32768 };
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    const char* gen_names[] = { "Random", "Sorted", "Reverse", "Almost Sorted" };
    int num_gens = sizeof(gen_names) / sizeof(gen_names[0]);

    const char* sort_names[] = { "Insertion Sort", "Merge Sort" };
    int num_sorts = sizeof(sort_names) / sizeof(sort_names[0]);

    int runs = 10;

    FILE* file = NULL;
    errno_t err = fopen_s(&file, "results_2.csv", "w");
    if (err != 0 || file == NULL) {
        printf("Ошибка: не удалось открыть файл results.csv для записи.\n");
        return 1;
    }

    /* Заголовок CSV */
    fprintf(file, "run,size,generator,sort,time,comparisons,pointer_changes\n");

    srand((unsigned)time(NULL));

    for (int run = 1; run <= runs; run++) {
        printf("=========== RUN %d ===========\n", run);

        for (int i = 0; i < num_sizes; i++) {
            int n = sizes[i];
            printf("================ SIZE: %d ================\n", n);

            for (int j = 0; j < num_gens; j++) {
                for (int k = 0; k < num_sorts; k++) {
                    list = NULL;
                    create_list(&list);

                    metrics.comparisons = 0;
                    metrics.pointer_changes = 0;

                    if (strcmp(gen_names[j], "Random") == 0)
                        generate_random(&list, n, 0, 100);
                    else if (strcmp(gen_names[j], "Sorted") == 0)
                        generate_sorted(&list, n);
                    else if (strcmp(gen_names[j], "Reverse") == 0)
                        generate_reverse_sorted(&list, n);
                    else if (strcmp(gen_names[j], "Almost Sorted") == 0)
                        generate_almost_sorted(&list, n);

                    printf("--- Generator: %s | Sort: %s ---\n", gen_names[j], sort_names[k]);

                    start = clock();

                    if (strcmp(sort_names[k], "Insertion Sort") == 0) {
                        insertion_sort(list, &metrics);
                    }
                    else {
                        metrics = merge_sort(&list);
                    }

                    end = clock();
                    elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;

                    printf("Time: %.6f | Comp: %ld | Ptr: %ld\n",
                        elapsed_time, metrics.comparisons, metrics.pointer_changes);

                    /* Запись в CSV */
                    fprintf(file, "%d,%d,%s,%s,%.6f,%ld,%ld\n",
                        run,
                        n,
                        gen_names[j],
                        sort_names[k],
                        elapsed_time,
                        metrics.comparisons,
                        metrics.pointer_changes);

                    clear(&list);
                }
            }
        }
    }

    fclose(file);
    printf("\nДанные успешно записаны в results.csv\n");

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

#define TASK_SIZE 100
#define TAM_VET 10000000
#define MAX 5000

int k = 0;

int partition(int *arr, long int low_index, long int high_index)
{
    long int i, j, temp, key;
    key = arr[low_index];
    i = low_index + 1;
    j = high_index;
    while (1)
    {
        while (i < high_index && key >= arr[i])
            i++;
        while (key < arr[j])
            j--;
        if (i < j)
        {
            temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
        else
        {
            temp = arr[low_index];
            arr[low_index] = arr[j];
            arr[j] = temp;
            return (j);
        }
    }
}

void quicksort(int *a, long int p, long int r)
{
    int div;
    if (p < r)
    {
        div = partition(a, p, r);
#pragma omp task shared(a) if (r - p > TASK_SIZE)
        quicksort(a, p, div - 1);
#pragma omp task shared(a) if (r - p > TASK_SIZE)
        quicksort(a, div + 1, r);
    }
}
void quicksortSerial(int *arr, long int low_index, long int high_index)
{
    int j;

    if (low_index < high_index)
    {
        j = partition(arr, low_index, high_index);

        k = k + 1;
        quicksort(arr, low_index, j - 1);

        k = k + 1;
        quicksort(arr, j + 1, high_index);
    }
}

int main()
{

    time_t t;
    srand(time(NULL));
    double inicioTempo, fimTempo, TempoSerial, TempoParalelo;
    int *vetorA = malloc(sizeof(int) * TAM_VET);
    int *vetorB = malloc(sizeof(int) * TAM_VET);
    int num;
    for (long int i = 0; i < TAM_VET; i++)
    {
        num = (rand() % (MAX + 1));
        vetorA[i] = num;
        vetorB[i] = num;
    }
    inicioTempo = omp_get_wtime();
    quicksortSerial(vetorA, 0, TAM_VET - 1);
    fimTempo = omp_get_wtime();
    TempoSerial = fimTempo - inicioTempo;
    inicioTempo = omp_get_wtime();
#pragma omp parallel
    {
#pragma omp single
        quicksort(vetorB, 0, TAM_VET);
    }
    fimTempo = omp_get_wtime();

    TempoParalelo = fimTempo - inicioTempo;
    printf("Tempo serial: %.3f\n", TempoSerial);
    printf("Tempo paralelo: %.3f\n", TempoParalelo);
    printf("Speed up: %.3f\n", TempoSerial / TempoParalelo);
}
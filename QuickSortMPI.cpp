#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
using namespace std;

void swap(int *arr, int i, int j)
{
    int t = arr[i];
    arr[i] = arr[j];
    arr[j] = t;
}

void quicksort(int *arr, int start, int end)
{
    int pivot, index;

    if (end <= 1)
        return;

    // Pick pivot and swap with first
    // element Pivot is middle element
    pivot = arr[start + end / 2];
    swap(arr, start, start + end / 2);

    // Partitioning Steps
    index = start;

    // Iterate over the range [start, end]
    for (int i = start + 1; i < start + end; i++)
    {

        // Swap if the element is less
        // than the pivot element
        if (arr[i] < pivot)
        {
            index++;
            swap(arr, i, index);
        }
    }

    // Swap the pivot into place
    swap(arr, start, index);

    // Recursive Call for sorting
    // of quick sort function
    quicksort(arr, start, index - start);
    quicksort(arr, index + 1, start + end - index - 1);
}

int *merge(int *arr1, int n1, int *arr2, int n2)
{
    int *result = (int *)malloc((n1 + n2) * sizeof(int));
    int i = 0;
    int j = 0;
    int k;

    for (k = 0; k < n1 + n2; k++)
    {
        if (i >= n1)
        {
            result[k] = arr2[j];
            j++;
        }
        else if (j >= n2)
        {
            result[k] = arr1[i];
            i++;
        }

        // Indices in bounds as i < n1
        // && j < n2
        else if (arr1[i] < arr2[j])
        {
            result[k] = arr1[i];
            i++;
        }

        // v2[j] <= v1[i]
        else
        {
            result[k] = arr2[j];
            j++;
        }
    }
    return result;
}

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

#define TAM 10000000
#define max 1000

int main()
{
    int number_of_elements = TAM;
    int *dataA = NULL;
    int *dataB = (int *)malloc(sizeof(int) * TAM);
    int chunk_size, own_chunk_size;
    int *chunk;
    int r, i;
    char numero[20], tamanhoVetor[20];
    double time_taken, time_serial, time_paralelo;

    time_t t;
    srand((unsigned)time(&t));
    for (i = 0; i < TAM; i++)
    {
        int num = (rand() % (max + 1));
        dataB[i] = num;
    }

    MPI_Status status;

    int number_of_process, rank_of_process;
    int rc = MPI_Init(NULL, NULL);

    // SERIAL
    time_taken -= MPI_Wtime();
    quicksort(dataB, 0, TAM);
    time_taken += MPI_Wtime();
    time_serial = time_taken;
    printf("Tempo em Serial: %f \n", time_taken);
    time_taken = 0;

    // PARALELIZADO
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_process);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_of_process);

    if (rank_of_process == 0)
    {

        chunk_size = (number_of_elements %
                          number_of_process ==
                      0)
                         ? (number_of_elements /
                            number_of_process)
                         : (number_of_elements /
                            (number_of_process - 1));

        dataA = (int *)malloc(number_of_process *
                              chunk_size *
                              sizeof(int));

        for (i = 0; i < TAM; i++)
        {
            dataA[i] = dataB[i];
        }

        for (int i = number_of_elements;
             i < number_of_process *
                     chunk_size;
             i++)
        {
            dataA[i] = 0;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    time_taken -= MPI_Wtime();

    MPI_Bcast(&number_of_elements, 1, MPI_INT, 0,
              MPI_COMM_WORLD);

    chunk_size = (number_of_elements %
                      number_of_process ==
                  0)
                     ? (number_of_elements /
                        number_of_process)
                     : (number_of_elements /
                        (number_of_process - 1));

    chunk = (int *)malloc(chunk_size *
                          sizeof(int));

    MPI_Scatter(dataA, chunk_size, MPI_INT, chunk,
                chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    free(dataA);
    dataA = NULL;

    own_chunk_size = (number_of_elements >=
                      chunk_size * (rank_of_process + 1))
                         ? chunk_size
                         : (number_of_elements -
                            chunk_size * rank_of_process);

    quicksort(chunk, 0, own_chunk_size);

    for (int step = 1; step < number_of_process; step = 2 * step)
    {
        if (rank_of_process % (2 * step) != 0)
        {
            MPI_Send(chunk, own_chunk_size, MPI_INT,
                     rank_of_process - step, 0,
                     MPI_COMM_WORLD);
            break;
        }

        if (rank_of_process + step < number_of_process)
        {
            int received_chunk_size = (number_of_elements >= chunk_size * (rank_of_process + 2 * step))
                                          ? (chunk_size * step)
                                          : (number_of_elements - chunk_size * (rank_of_process + step));
            int *chunk_received;
            chunk_received = (int *)malloc(
                received_chunk_size * sizeof(int));
            MPI_Recv(chunk_received, received_chunk_size,
                     MPI_INT, rank_of_process + step, 0,
                     MPI_COMM_WORLD, &status);

            dataA = merge(chunk, own_chunk_size,
                          chunk_received,
                          received_chunk_size);

            free(chunk);
            free(chunk_received);
            chunk = dataA;
            own_chunk_size = own_chunk_size + received_chunk_size;
        }
    }

    time_taken += MPI_Wtime();

    if (rank_of_process == 0)
    {
        time_paralelo = time_taken;
        printf(
            "\n\nQuicksort %d ints on %d procs: %f secs\n",
            number_of_elements, number_of_process,
            time_taken);
    }
    MPI_Finalize();

    double speedup = time_serial / time_paralelo;

    printf("Speed up: %f \n", speedup);
    return 0;
}
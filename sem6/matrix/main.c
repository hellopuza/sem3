
/* Compiled with O3, run using wsl
threads num |   1      2      3      4      5      6      7      8      9     10     11     12    
  400 time: | 0.033  0.022  0.029  0.014  0.023  0.023  0.027  0.050  0.025  0.050  0.046  0.020 
     boost: | 1.000  1.509  1.164  2.333  1.455  1.422  1.253  0.671  1.348  0.671  0.733  1.629 
  500 time: | 0.067  0.068  0.077  0.047  0.074  0.090  0.075  0.090  0.127  0.080  0.109  0.111 
     boost: | 1.000  0.982  0.867  1.423  0.903  0.749  0.899  0.745  0.527  0.844  0.615  0.603 
  600 time: | 0.153  0.152  0.176  0.135  0.186  0.164  0.191  0.212  0.210  0.221  0.229  0.192 
     boost: | 1.000  1.006  0.865  1.134  0.822  0.928  0.799  0.718  0.728  0.689  0.668  0.794 
  700 time: | 0.245  0.259  0.247  0.243  0.283  0.285  0.332  0.372  0.334  0.366  0.394  0.420 
     boost: | 1.000  0.947  0.991  1.008  0.865  0.861  0.739  0.659  0.735  0.670  0.622  0.584 
  800 time: | 0.393  0.380  0.357  0.364  0.372  0.446  0.467  0.499  0.539  0.548  0.627  0.673 
     boost: | 1.000  1.034  1.099  1.079  1.056  0.880  0.841  0.787  0.728  0.717  0.626  0.583 
  900 time: | 0.537  0.537  0.552  0.667  0.751  0.804  0.969  1.313  1.125  1.423  1.764  2.571 
     boost: | 1.000  1.001  0.974  0.806  0.715  0.669  0.554  0.409  0.478  0.378  0.305  0.209 
 1000 time: | 0.728  0.740  0.862  1.383  1.124  1.541  1.266  1.735  1.860  2.437  3.005  4.745 
     boost: | 1.000  0.984  0.845  0.526  0.648  0.472  0.575  0.419  0.391  0.299  0.242  0.153 
 1100 time: | 0.981  1.018  1.161  1.135  1.022  2.053  2.070  3.445  2.461  3.364  5.267 12.506 
     boost: | 1.000  0.964  0.845  0.864  0.960  0.478  0.474  0.285  0.399  0.292  0.186  0.078 
 1200 time: | 1.340  1.370  1.663  1.938  2.164  2.742  2.373  3.274  3.487  5.275  7.656 15.220 
     boost: | 1.000  0.978  0.806  0.691  0.619  0.489  0.565  0.409  0.384  0.254  0.175  0.088 
 1300 time: | 2.249  2.066  4.891  2.281  3.874  4.184  3.597  4.621  6.067  7.753  9.458 20.963 
     boost: | 1.000  1.088  0.460  0.986  0.580  0.537  0.625  0.487  0.371  0.290  0.238  0.107 
 1400 time: | 2.950  3.050  2.975  4.861  4.681  6.321  5.930  5.674  8.829  9.860 14.571 27.397 
     boost: | 1.000  0.967  0.992  0.607  0.630  0.467  0.497  0.520  0.334  0.299  0.202  0.108 
*/

#include <sys/sysinfo.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

struct matrix
{
    int* data;
    size_t n;
};

struct thread_data
{
    struct matrix* matr1;
    struct matrix* matr2;
    struct matrix* result;
    size_t begin;
    size_t end;
    pthread_t id;
};

struct matrix multMatrix (struct matrix matr1, struct matrix matr2, size_t threads_num);
void fillRandomMatrix (struct matrix* matr, size_t n);
void printMatrix (struct matrix matr);
void* multThread (void* arg);

int main ()
{
    printf("threads num | ");
    for (size_t threads_num = 1; threads_num <= get_nprocs(); threads_num++)
    {
        printf(" %2lu    ", threads_num);
    }
    printf("\n");

    for (size_t size = 400; size < 1500; size += 100)
    {
        printf("%5lu time: |", size);
        float* exec_time = (float*)calloc(get_nprocs(), sizeof(float));
        for (size_t threads_num = 1; threads_num <= get_nprocs(); threads_num++)
        {
            struct matrix m1;
            struct matrix m2;
            fillRandomMatrix(&m1, size);
            fillRandomMatrix(&m2, size);

            clock_t begin = clock();
            struct matrix res = multMatrix(m1, m2, threads_num);
            clock_t dt = clock() - begin;
            exec_time[threads_num] = (float)dt / CLOCKS_PER_SEC;
            printf("%6.3f ", exec_time[threads_num]);

            free(m1.data);
            free(m2.data);
            free(res.data);
        }
        printf("\n");

        printf("     boost: |");
        for (size_t threads_num = 1; threads_num <= get_nprocs(); threads_num++)
        {
            printf("%6.3f ", exec_time[1] / exec_time[threads_num]);
        }
        free(exec_time);
        printf("\n");
    }

    return 0;
}

void fillRandomMatrix (struct matrix* matr, size_t n)
{
    matr->n = n;
    matr->data = (int*)calloc(n * n, sizeof(int));

    for (size_t i = 0; i < n * n; i++)
    {
        matr->data[i] = rand() % 100;
    }
}

void* multThread (void* arg)
{
    struct thread_data* tdata = (struct thread_data*)arg;
    size_t n = tdata->matr1->n;

    for (size_t row = tdata->begin; row < tdata->end; row++)
    {
        for (size_t col = 0; col < n; col++)
        {
            tdata->result->data[row * n + col] = 0;
            for (size_t k = 0; k < n; k++)
            {
                tdata->result->data[row * n + col] += tdata->matr1->data[row * n + k] * tdata->matr2->data[k * n + col];
            }
        }
    }
}

struct matrix multMatrix (struct matrix matr1, struct matrix matr2, size_t threads_num)
{
    struct matrix result;
    result.n = matr1.n;
    result.data = (int*)calloc(matr1.n * matr1.n, sizeof(int));

    size_t segment = matr1.n / threads_num;

    struct thread_data* tdata = (struct thread_data*)calloc(threads_num, sizeof(struct thread_data));
    size_t i = 0;
    for (size_t begin = 0; begin < matr1.n; begin += segment)
    {
        tdata[i].matr1 = &matr1;
        tdata[i].matr2 = &matr2;
        tdata[i].result = &result;
        tdata[i].begin = begin;
        tdata[i].end = begin + segment;

        if (matr1.n - tdata[i].end < segment)
            tdata[i].end = begin = matr1.n;

        pthread_create(&tdata[i].id, NULL, multThread, &tdata[i]);
        i++;
    }

    for (int i = 0; i < threads_num; i++)
    {
        pthread_join(tdata[i].id, NULL);
    }

    return result;
}

void printMatrix (struct matrix matr)
{
    for (size_t row = 0; row < matr.n; row++)
    {
        for (size_t col = 0; col < matr.n; col++)
        {
            printf("%d ", matr.data[row * matr.n + col]);
        }
        printf("\n");
    }
}
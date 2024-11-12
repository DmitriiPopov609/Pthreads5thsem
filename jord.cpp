#include <algorithm>
#include <pthread.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <iomanip>

#include "jord.hpp"


void synchronize(int total_threads)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static pthread_cond_t condvar_in = PTHREAD_COND_INITIALIZER;
    static pthread_cond_t condvar_out = PTHREAD_COND_INITIALIZER;
    static int threads_in = 0;
    static int threads_out = 0;
    
    pthread_mutex_lock(&mutex);
    
    threads_in++;
    if (threads_in >= total_threads)
    {
        threads_out = 0;
        pthread_cond_broadcast(&condvar_in);
    }
    else
    {
        while (threads_in < total_threads)
            pthread_cond_wait(&condvar_in, &mutex);
    }
    
    threads_out++;
    if (threads_out >= total_threads)
    {
        threads_in = 0;
        pthread_cond_broadcast(&condvar_out);
    }
    else
    {
        while (threads_out < total_threads)
            pthread_cond_wait(&condvar_out, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

void swap_rows(double* a, int n, int row1, int row2)
{
    for (int j = 0; j < n; j++)
    {
        std::swap(a[row1 * n + j], a[row2 * n + j]);
    }
}

int SolveSystem(int n, double *a, double *b, double *x, int *index, int my_rank, int total_threads)
{
    int i, k1;
    double tmp;

    for (i = 0; i < n; i++)
        index[i] = i;

     int cols_per_thread =n/ total_threads;
     int remainder = n % total_threads;
     int first_col = my_rank * cols_per_thread + std::min(my_rank, remainder);
     int last_col = first_col + cols_per_thread - 1 + (my_rank < remainder ? 1 : 0);


    for (i = first_col; i <= last_col; i++)
    {
        if(i==n-1)
        break;
    {
        k1=i;
            for (int j1 = i+1; j1 < n; j1++)
            {
                    if (fabs(a[k1 * n + i]) < fabs(a[j1 * n + i]))
                    {
                        k1 = j1;
                    }
            }

            swap_rows(a, n, i, k1);

            std::swap(b[i], b[k1]);
            }
    }
    synchronize(total_threads);
    
    if (my_rank==0)
    {
        for(int i1=0; i1<n; i1++)
        {
            for (int j = 0; j <n; j++)
            {
                if(j!=i1)
                {
                    tmp=1/a[i1*n+i1];
                    tmp *=a[(j) * n + i1];
                    for (int k = i1; k <n; k++)
                    {
                        a[(j) * n + k] -= tmp* a[i1* n + k];
                    }
                    b[j] -= tmp * b[i1];
                }
 
            }
        }
    }
    if (my_rank == 0)
    {

        for (i = 0; i < n; i++)
        {
            x[index[i]] = b[i]/a[i*n+i];
        }
    }
    return 0;
}

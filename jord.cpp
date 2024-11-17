#include <algorithm>
#include <pthread.h>
#include <math.h>
#include<cstdio>
#include<iostream>


#include "jord.hpp"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


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
        double tmp = a[row1 * n + j];
        a[row1 * n + j] = a[row2 * n + j];
        a[row2 * n + j] = tmp;
    }
}

void swap_cols(double* a, int n, int col1, int col2)
{
    for (int j = 0; j < n; j++)
    {
        double tmp = a[j * n + col1];
        a[j * n + col1] = a[j * n + col2];
        a[j * n + col2] = tmp;
    }
}


int SolveSystem(int n, double *a, double *b, double *x, int *index, int my_rank, int total_threads)
{
    int i, j, k;
    int k3, k4;
    double tmp;

    int rows_per_thread =n/total_threads;
    int remainder = n % total_threads;
    int first_row = my_rank * rows_per_thread + std::min(my_rank, remainder);
    int last_row = first_row + rows_per_thread - 1 + (my_rank < remainder ? 1 : 0);

    for (i = 0; i < n; i++)
        index[i] = i;


    for (i = 0; i < n; i++)
    {
        k3=k4=i;
        double local_max_value = -1.0;
        int local_max_row = -1, local_max_col = -1;

        for (int j1 = first_row; j1 <= last_row; j1++)
        {
            for (int j2 = i+1; j2 < n; j2++) 
            { 
                if (fabs(a[j1 * n + j2]) > local_max_value) 
                {
                    local_max_value = fabs(a[j1 * n + j2]);
                    local_max_row = j1;
                    local_max_col = j2;
                }
            }
        }

    static double global_max_value = -1.0;
    static int global_max_row = -1, global_max_col = -1;

    pthread_mutex_lock(&mutex);
    if (local_max_value > global_max_value) 
    {
        global_max_value = local_max_value;
        global_max_row = local_max_row;
        global_max_col = local_max_col;
    }
    pthread_mutex_unlock(&mutex);
    synchronize(total_threads);
    
    if(my_rank==0)
    {
        k3= global_max_row;
        k4= global_max_col;
        
        std::swap(index[i], index[k4]);
        
        swap_rows(a, n, i, k3);
        swap_cols(a, n, i, k4);

        std::swap(b[i], b[k3]);

        tmp = 1.0 / a[i * n + i];
        for (j = i; j < n; j++)
            a[i * n + j] *= tmp;
        b[i] *= tmp;
    }
    synchronize(total_threads);

        for (j = first_row; j <= last_row; j++)
        {
            if(j!=i)
            {
                tmp = a[j * n + i];
                for (k = i; k < n; k++)
                    a[j * n + k] -= tmp * a[i * n + k];
                b[j] -= tmp * b[i];
            }
        }
         synchronize(total_threads);
    }


    for (i = first_row; i <= last_row; i++)
        x[index[i]] = b[i];

    return 0;
}

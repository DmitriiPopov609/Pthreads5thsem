#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <errno.h>
#include <string>

#include "matrix.hpp"
#include "jord.hpp"

typedef struct
{
	int n;
	double *a;
	double *b;
	double *x;
	int *index;
	int my_rank;
	int total_threads;
} ARGS;

long int thread_time = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *Solution(void *p_arg)
{
	ARGS *arg = (ARGS*)p_arg;
	long int t1;

	t1 = get_time();
	SolveSystem(arg->n, arg->a, arg->b, arg->x, arg->index, arg->my_rank, arg->total_threads);
	t1 = get_time() - t1;

	pthread_mutex_lock(&mutex);
	thread_time += t1;
	pthread_mutex_unlock(&mutex);

	return NULL;
}

int main(int argc, char *argv[])
{
	if(argc!=5)
	{
		printf("Wrong input!\n");
		printf("Usage: prog InputMode (1 for file from matrix, 2 for creating random matrix) file name/size of matrix  size of minor you want to see total threads\n");
		return -1;
	}
	else
	{
	int i;
	int n;
	double *a;
	double *b;
	double *x;
	int *index;
	long int t_full;
	int total_threads;
	pthread_t *threads;
	ARGS *args;
	FILE *fin = NULL;
	int inputMode, m, rezult;

	inputMode = std::stoi(argv[1]);

	switch (inputMode)
	{
	case 1:
		fin = fopen(argv[2], "r");

		if (!fin)
		{
			printf("Can't open file!\n");

			return -1;
		}

		if (fscanf(fin, "%d", &n) != 1)
		{
			printf("Error in reading from file!\n");

			fclose(fin);

			return -2;
		}

		m=std::stoi(argv[3]);
		if(m>n)
			m=n;
		if(m<=0)
		{
			printf("size of minor must be positive\n");
			return -2;
		}

		break;
	case 2:
		n=std::stoi(argv[2]);


		break;

	default:
		printf("Incorrect mode!\n");
		return -3;
	}

	if (n < 1)
	{
		printf("Incorrect N!\n");

		if (inputMode == 1)
			fclose(fin);

		return -4;
	}
	else
	{
		m=std::stoi(argv[3]);
		if(m<=0)
		{
			printf("size of minor must be positive\n");
			return -2;
		}
		if(m>n)
			m=n;

	}

	total_threads=std::stoi(argv[4]);

	index = (int*)malloc(n * sizeof(int));
	a = (double*)malloc(n * n * sizeof(double));
	b = (double*)malloc(n * sizeof(double));
	x = (double*)malloc(n * sizeof(double));
	threads = (pthread_t*)malloc(total_threads * sizeof(pthread_t));
	args = (ARGS*)malloc(total_threads * sizeof(ARGS));

	if (!(index && a && b && x && threads && args))
	{
		printf("Not enough memory!\n");

		if (index)
			free(index);
		if (a)
			free(a);
		if (b)
			free(b);
		if (x)
			free(x);
		if (threads)
			free(threads);
		if (args)
			free(args);

		if (inputMode == 1)
			fclose(fin);

		return -5;
	}

	rezult = InputMatrix(n, a, b, inputMode, fin);

	if (inputMode == 1)
		fclose(fin);

	if (rezult == -1)
	{
		printf("Error in reading from file!\n");

		free(index);
		free(a);
		free(b);
		free(x);
		free(threads);
		free(args);


		return -6;
	}

	printf("\nMatrix A:\n");
	PrintMatrix(n, m, a, b);
	printf("\n");

	printf("Calculating...\n");

	for (i = 0; i < total_threads; i++)
	{
		args[i].n = n;
		args[i].a = a;
		args[i].b = b;
		args[i].x = x;
		args[i].index = index;
		args[i].my_rank = i;
		args[i].total_threads = total_threads;
	}

	t_full = get_full_time();

	for (i = 0; i < total_threads; i++)
		if (pthread_create(threads + i, 0, Solution, args + i))
		{
			printf("Cannot create thread %d!\n", i);

			if (a) free(a);
			if (b) free(b);
			if (x) free(x);
			if (index) free(index);
			if (threads) free(threads);
			if (args) free(args);

			return -7;
		}

	for (i = 0; i < total_threads; i++)
		if (pthread_join(threads[i], 0))
		{
			printf("Cannot wait thread %d!\n", i);

			if (a) free(a);
			if (b) free(b);
			if (x) free(x);
			if (index) free(index);
			if (threads) free(threads);
			if (args) free(args);

			return -8;
		}

	t_full = get_full_time() - t_full;

	if (t_full == 0)
		t_full = 1;

	switch (rezult)
	{
	case -1:
		printf("Can't solve - matrix is deteriorated.\n");

		break;
	case 0:
		printf("\nRezulting vector:");
		PrintVector(n, m, x);
		printf("\n");

		printf("\n\nSolution time = %.3f seconds\nTotal threads time = %.3f seconds"\
		" (%.2f%%)\nPer thread = %.3f seconds\n",
		(double)t_full/100, (double)thread_time/100, (double)thread_time * 100/(double)t_full,
		(double)(thread_time)/(100*total_threads));

		if (inputMode == 1)
		{
			fin = fopen(argv[2], "r");
			fscanf(fin, "%d", &n);
		}

		InputMatrix(n, a, b, inputMode, fin);

		if (inputMode == 1)
			fclose(fin);

		printf("Solution error ||Ax - b|| = %e\n", SolutionError(n, a, b, x));

		break;
	default:
		printf("Unknown error!\n");

		break;
	}

	free(index);
	free(a);
	free(b);
	free(x);
	free(threads);
	free(args);
	}

	return 0;
}

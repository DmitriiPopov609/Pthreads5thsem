#include <sys/resource.h>
#include <sys/time.h>
#include <stdio.h>
#include <math.h>

#include "matrix.hpp"

static double f(int i, int j)
{
	return pow(j+10, 2)+5%(i+1);
}

int InputMatrix(int n, double* a, double* b, int inputMode, FILE* fin)
{
	int i;
	int j;
	double tmp=0;

	if (inputMode == 1)
	{
		for (i = 0; i < n; ++i)
		{
			for (j = 0; j < n; ++j)
				if (fscanf(fin, "%lf", &a[i * n + j]) != 1)
					return -1;

			if (fscanf(fin, "%lf", &b[i]) != 1)
				return -1;
		}
	}
	else
	{
		for (i = 0; i < n; ++i)
		{
			for (j = 0; j < n; ++j)
			{
				a[i * n + j] = f(i, j)+(i==j ? 7 :0);
					tmp += a[i * n + j];
			}
			b[i]=tmp;
			tmp=0;
		}
	}

	return 0;
}

void PrintMatrix(int n, int m, double* a, double* b)
{
	int i;
	int j;
	int nPrint;

	nPrint = (n > m) ? m : n;

	for (i = 0; i < nPrint; ++i)
	{
		printf("| ");
		for (j = 0; j < nPrint; ++j)
			printf("%10.3g ", a[i * n + j]);
		printf("|\t%10.3g |\n", b[i]);
	}
}

void PrintVector(int n, int m, double* x)
{
	int i;
	int nPrint;

	nPrint = (n > m) ? m : n;

	for (i = 0; i < nPrint; ++i)
	{
		if(i==0)
			printf("(");
		printf("%-.3g", x[i]);
		if(i!=nPrint-1)
			printf(",");
		else
		{
			if(n==m)
				printf(")");
			else
				printf("...");
		}
	}
	printf("\n");
}

double SolutionError(int n, double* a, double* b, double* x)
{
	int i;
	int j;
	double tmp;
	double rezult;

	rezult = 0.0;
	for (i = 0; i < n; ++i)
	{
		tmp = 0.0;
		for (j = 0; j < n; ++j)
			tmp += a[i * n + j] * x[j];
		tmp -= b[i];

		rezult += tmp * tmp;
	}

	return sqrt(rezult);
}

long int get_time(void)
{
	struct rusage buf;

	getrusage(RUSAGE_SELF, &buf);

	return buf.ru_utime.tv_sec * 100 + buf.ru_utime.tv_usec/10000;
}

long int get_full_time(void)
{
	struct timeval buf;

	gettimeofday(&buf, 0);

	return buf.tv_sec * 100 + buf.tv_usec/10000;
}


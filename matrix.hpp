#ifndef __MATRIX_H_INCLUDED__
#define __MATRIX_H_INCLUDED__

int InputMatrix(int n, double* a, double* b, int inputMode, FILE* fin);

void PrintMatrix(int n, int m, double* a, double* b);

void PrintVector(int n, int m, double* x);

double SolutionError(int n, double* a, double* b, double* x);

long int get_time(void);

long int get_full_time(void);

#endif

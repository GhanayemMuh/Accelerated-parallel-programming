/*
 * A parallel implementation of a stencil computation to solve the
 * 2-dimensional heat diffusion problem by Jacobi iteration using TBB.
 *
 * Copyright (c) 2013 Australian National University. All rights reserved.
 */

 /* Updated by Dr. Yariv Aridor, 2022 */

#include <iostream>
#include <algorithm>
#include<mutex>
#include <limits>
#include <chrono>
#include <tbb/tbb.h>

using namespace std;

size_t n_x = 700;
size_t n_y = 700;
int max_iter = 1000;
double t_edge = 100.0;
double converge = 0.01;

// allocate data arrays
double* t_old = new double[n_x * n_y]();
double* t_new = new double[n_x * n_y]();

void delete_heap() {
    delete[] t_old;
    delete[] t_new;
}

void init_heap() {
    size_t i, j;
    // fix boundary values
    j = 0;     for (i = 0; i < n_x; i++) t_new[j * n_x + i] = t_old[j * n_x + i] = t_edge;
    j = n_y - 1; for (i = 0; i < n_x; i++) t_new[j * n_x + i] = t_old[j * n_x + i] = t_edge;
    i = 0;     for (j = 0; j < n_y; j++) t_new[j * n_x + i] = t_old[j * n_x + i] = t_edge;
    i = n_x - 1; for (j = 0; j < n_y; j++) t_new[j * n_x + i] = t_old[j * n_x + i] = t_edge;

    for (j=1; j < n_x-1; j++)
        for (i=1; i < n_y-1; i++)
             t_new[j * n_x + i] = t_old[j * n_x + i] = 0;

}

bool validate_heat (double* matrix) {
    /* Printout the result */
    size_t i, j;
    const size_t kMaxPrint = 5;
    double reference[(kMaxPrint-1)*(kMaxPrint-1)] = {99.8727, 99.7457, 99.6192, 99.4935, 99.7457, 99.492, 99.2392, 98.988, 99.6192, 99.2392, 98.8607, 98.4845, 99.4935, 98.988, 98.4845, 97.984};
    for (j = 1; j < kMaxPrint; j++) {
        for (i = 1; i < kMaxPrint; i++) {
            double diff = fabs(matrix[j * n_x + i]- reference[(j-1) * (kMaxPrint-1) + (i-1)]);
            if (diff>1e-4) return false;
        }
    }
    return true;
}


int heat (int mode) {
    int iter = 0;
    double max_diff;
    int rc=0;

    while (iter < max_iter) {
        iter++;
        max_diff = 0.f;
       
        if (mode == 0) {  // sequential execution 
            for (size_t j = 1; j < n_y - 1; j++) {
                for (size_t i = 1; i < n_x - 1; i++) {
                    t_new[j * n_x + i] = 0.25 * (t_old[j * n_x + i + 1] + t_old[j * n_x + i - 1] +
                        t_old[(j + 1) * n_x + i] + t_old[(j - 1) * n_x + i]);
                    double tdiff = fabs(t_old[j * n_x + i] - t_new[j * n_x + i]);
                    max_diff = max(max_diff, tdiff);
                }
            }
        }

        if (mode == 1) {  // parallel_for (global_diff w/lock) 
        }

        if (mode == 2) {  // parallel_for (local_diff) 
        }


        if (mode == 3) {  // parallel_reduce 
        }

        // swap array pointers
        double* temp = t_new; t_new = t_old; t_old = temp;
        if (max_diff < converge) break;
    }

    cout << "iterations: " << iter << " convergence:" << max_diff << endl;

    return (validate_heat(t_new)==false) ? -1 : 0; 
}

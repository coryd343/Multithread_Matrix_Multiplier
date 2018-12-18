/*
 *  prodcons module
 *  Producer Consumer module
 * 
 *  Implements routines for the producer consumer module based on 
 *  chapter 30, section 2 of Operating Systems: Three Easy Pieces
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 *  Fall 2016
 */

// Include only libraries for this module
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "counter.h"
#include "matrix.h"
#include "pcmatrix.h"
#include "prodcons.h"

#define DEBUG 1
#define QUEUE 1

// Define Locks and Condition variables here
int matrixIndex = 0, writeIndex = 0, readIndex = 0; //Empty: write == read; Full: write = read+1 % MAX
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t can_fill = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_mult = PTHREAD_COND_INITIALIZER;

// Producer consumer data structures
Matrix * bigmatrix[MAX + 1];

#if QUEUE
// Bounded buffer put() get()
int put(Matrix * value)
{
    if((readIndex % MAX) != ((writeIndex + 1) % MAX)) {//If buffer isn't full
        printf("Put successful at index %d\n", writeIndex % MAX);
        return bigmatrix[writeIndex++ % MAX] = value;
    }
    else {
        printf("Put unsuccessful at index %d\n", writeIndex % MAX);
        return 0;
    }
}
Matrix * get() 
{
    if((writeIndex % MAX) != (readIndex % MAX)) {//If buffer isn't empty
        printf("Get successful at index %d\n", writeIndex % MAX);
        return bigmatrix[readIndex++ % MAX];
    }
    else {
        printf("Get unsuccessful at index %d\n", writeIndex % MAX);
        return NULL;
    }
}
#endif

#if !QUEUE
int put(Matrix * value)
{
    if(matrixIndex < MAX)
        return bigmatrix[matrixIndex++] = value;
    else return 0;
}

Matrix * get() 
{
    if(matrixIndex > 0)
        return bigmatrix[--matrixIndex];
    else
        return NULL;
}
#endif

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
    ProdConsStats *proStats = (ProdConsStats *) arg;
    Matrix *m;
    for(int i = 0; i < LOOPS; i++) {
        pthread_mutex_lock(&lock);
        #if DEBUG
        printf("Producer loop %d\n\n", i);
        #endif
        m = GenMatrixRandom();
        //m = GenMatrixBySize(3, 3);
        #if DEBUG
        DisplayMatrix(m, stdout);
        #endif
        #if QUEUE
        while(readIndex == (writeIndex + 1) % MAX)
            pthread_cond_wait(&can_fill, &lock);
        #endif
        #if !QUEUE
        while(matrixIndex == MAX - 1)
            pthread_cond_wait(&can_fill, &lock);
        #endif
        #if DEBUG
        printf("Thread %ld putting matrix into index %d\n", pthread_self(), writeIndex % MAX);
        #endif
        put(m);
        proStats->sumtotal += SumMatrix(m);
        proStats->matrixtotal++;
        pthread_cond_broadcast(&can_mult);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
    ProdConsStats *conStats = (ProdConsStats *) arg;
    Matrix *m1, *m2, *m3;
    for(int i = 0; i < LOOPS; i++) {
        #if DEBUG
        printf("Consumer loop %d\n\n", i);
        #endif
        pthread_mutex_lock(&lock);
        if(m1 == NULL) {
            #if DEBUG
            printf("M1 Selected\n");
            #endif
            #if QUEUE
            while(writeIndex == readIndex)
                pthread_cond_wait(&can_mult, &lock);
            #endif
            #if !QUEUE
            while(matrixIndex == 0)
                pthread_cond_wait(&can_mult, &lock);
            #endif
            #if DEBUG
            printf("Thread %ld getting matrix from index %d\n", pthread_self(), readIndex % MAX);
            #endif
            m1 = get();
            conStats->sumtotal += SumMatrix(m1);
            conStats->matrixtotal++;
        }
        else {
            #if QUEUE
            while(writeIndex == readIndex)
                pthread_cond_wait(&can_mult, &lock);
            #endif
            #if !QUEUE
            while(matrixIndex == 0)
                pthread_cond_wait(&can_mult, &lock);
            #endif
            #if DEBUG
            printf("Thread %ld getting matrix from index %d\n", pthread_self(), readIndex % MAX);
            #endif
            m2 = get();
            conStats->sumtotal += SumMatrix(m2);
            m3 = MatrixMultiply(m1, m2);
            if(m3 != NULL) {
                #if DEBUG
                printf("M2 selected: success: (%dx%d)*(%dx%d)\n", m1->rows, m1->cols, m2->rows, m2->cols);
                #endif
                DisplayMatrix(m1, stdout);
                printf("    X\n");
                DisplayMatrix(m2, stdout);
                printf("    =\n");
                DisplayMatrix(m3, stdout);
                printf("\n\n");
                FreeMatrix(m1);
                m1 = NULL;
                FreeMatrix(m2);
                FreeMatrix(m3);
                conStats->multtotal++;
            }
            else {
                #if DEBUG
                printf("M2 selected: incompatible: (%dx%d)*(%dx%d)\n", m1->rows, m1->cols, m2->rows, m2->cols);
                #endif
                FreeMatrix(m2);
            }
            conStats->matrixtotal++;
        }
        pthread_cond_broadcast(&can_fill);
        pthread_mutex_unlock(&lock);
    }
    #if DEBUG
    printf("!!!!!Exited Consumer main loop!\n");
    #endif
    return NULL;
}



ProdConsStats * initProdConStats() {
    ProdConsStats *stats = malloc(sizeof(ProdConsStats));
    stats->sumtotal = 0;
    stats->multtotal = 0;
    stats->matrixtotal = 0;
    return stats;
}


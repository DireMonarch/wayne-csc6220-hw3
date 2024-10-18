/**
 * Copyright 2024 Jim Haslett
 *
 * This work part of a university assignment.  If you are taking the course
 * this work was assigned for, do the right thing, and solve the assignment
 * yourself!
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/* Define constants */
#define MATRIX_SIZE 256
#define NUM_THREADS 8
#define ROWS_PER_THREAD (MATRIX_SIZE / NUM_THREADS)
#define OUTFILENAME "result.txt"
#define DEBUG 0

/* Forward declare thread function */
void *compute_rows(void *);


void print_matrix(int matrix[MATRIX_SIZE * MATRIX_SIZE]) {
    /**
     * Prints matrix passed to it to the screen.
     *
     * Function must interperet rows and columns, as matricies are stored as
     * flat arrays.
     *
     * @param matrix integer array representing a 2D matrix of size MATRIX_SIZE
     *                  x MATRIX_SIZE
     */

    for (int j=0; j < MATRIX_SIZE; j++) {
        for (int i=0; i < MATRIX_SIZE; i++) {
            printf("%3d ", matrix[j*MATRIX_SIZE + i]);
        }
        printf("\n");
    }
}

/* Declare matricies */
int matrix_a_[MATRIX_SIZE * MATRIX_SIZE];
int matrix_b_[MATRIX_SIZE * MATRIX_SIZE];
int matrix_c_[MATRIX_SIZE * MATRIX_SIZE];

/* Variable used for blocking thread reporting, so each reports complete
    in the proper order */
int reporting_thread_number_;

/* Declare mutexes */
pthread_mutex_t mutex_write_result_lock_;
pthread_cond_t cond_output_write_;

void main() {
    /**
     * main function
     *
     * This is the main function that runs in the main (default) thread
     */

    /* Thread list */
    pthread_t p_threads[NUM_THREADS];

    /* File Pointer for output */
    FILE *outfile;

    /* Needed this to pass individual thread numbers to each thread
        Otherwise, the thread spawning loop would increment the thread number
        variable that each thread refereneces */
    int thread_numbers[NUM_THREADS];

    /* Variable used to capture the return code from spawining threads */
    int rc;

    /* Fill matricies */
    int val;
    for (int i=0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        val = i % MATRIX_SIZE + 1;
        matrix_a_[i] = val;
        matrix_b_[i] = val;
        matrix_c_[i] = 0;
    }

    /* Initialize mutexes */
    pthread_cond_init(&cond_output_write_, NULL);
    pthread_mutex_init(&mutex_write_result_lock_, NULL);

    /* Set the reporting thread number to the last thread */
    reporting_thread_number_ = NUM_THREADS - 1;

    /* Spawn threads */
    for (int tn=0; tn < NUM_THREADS; tn++) {
        thread_numbers[tn] = tn;
        if (DEBUG) printf("MAIN THREAD: Creating threat number %d\n", tn);
        rc = pthread_create(&p_threads[tn], NULL, compute_rows, (void*)&thread_numbers[tn]);
        if (rc) {
            printf("MAIN THREAD: ERROR: Return code from pthread_create on thread number %d is %d\n", tn, rc);
            exit(-1);
        }
    }

    /* Wait for threads to complete */
    for (int i=0; i < NUM_THREADS; i++) {
        pthread_join(p_threads[i], NULL);
    }

    /* Destroy mutexes */
    pthread_mutex_destroy(&mutex_write_result_lock_);
    pthread_cond_destroy(&cond_output_write_);


    /* Output result matrix to file */
    outfile = fopen(OUTFILENAME, "w");
    for (int j=0; j < MATRIX_SIZE; j++) {
        for (int i=0; i < MATRIX_SIZE; i++) {
            fprintf(outfile, "%4d ", matrix_c_[j*MATRIX_SIZE + i]);
        }
        fprintf(outfile, "\n");
    }
    fclose(outfile);

    /* Exit default thread */
    pthread_exit(NULL);
}


void *compute_rows(void *thread_number_pointer) {
    /**
     * This is the function that runs on the worker threads.
     *
     * It calculates ROWS_PER_THREAD rows, performing an elementwise addition
     * for each matrix element within the the rows assigned to it.  Results
     * are written directly to the target matrix.
     *
     * @param thread_number_pointer void* pointer to the thread number for this
     *          thread.  this number is needed to determin which rows will be
     *          calculated by this thread.
     */


    /* type cast and dereference the pointer to a local variable */
    int thread_number = *((int*)thread_number_pointer);

    /* calculate the starting row, based on the thread number */
    int starting_row = thread_number*ROWS_PER_THREAD;

    /* Perform matrix addition on rows assigned to this thread */
    for (int j=starting_row; j < starting_row+ROWS_PER_THREAD; j++) {
        for (int i=0; i < MATRIX_SIZE; i++) {
            matrix_c_[j*MATRIX_SIZE + i] =
                matrix_a_[j*MATRIX_SIZE + i] +
                matrix_b_[j*MATRIX_SIZE + i];
        }
    }

    /* Write thread exit message to the screen */

    /* Lock section with mutex*/
    pthread_mutex_lock(&mutex_write_result_lock_);

    /* Check if it is our turn to write, if not, go into wait state */
    while (reporting_thread_number_ != thread_number) {
        pthread_cond_wait(&cond_output_write_, &mutex_write_result_lock_);
    }
    /* Write our output */
    printf("Thread %d: Done\n", thread_number);
    /* Flush stdout just in case, though it should flush because of the \n */
    fflush(stdout);

    /* Set the condition variable so it is the next thread's turn */
    reporting_thread_number_ = thread_number - 1;

    /* Signal and release the mutex lock */
    pthread_cond_broadcast(&cond_output_write_);
    pthread_mutex_unlock(&mutex_write_result_lock_);

    /* Exit thread */
    pthread_exit(NULL);
}

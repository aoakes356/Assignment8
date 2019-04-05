#include "sort.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>

#define SORT_THRESHOLD      40
#define THREAD_THRESHOLD 20000
typedef struct _sortParams {
    char** array;
    int left;
    int right;
} SortParams;

static int maximumThreads = 1;              /* maximum # of threads to be used */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

/* This is an implementation of insert sort, which although it is */
/* n-squared, is faster at sorting short lists than quick sort,   */
/* due to its lack of recursive procedure call overhead.          */

static void insertSort(char** array, int left, int right) {
    int i, j;
    for (i = left + 1; i <= right; i++) {
        char* pivot = array[i];
        j = i - 1;
        while (j >= left && (strcmp(array[j],pivot) > 0)) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = pivot;
    }
}

int errorHandler(char* message){ // Just a convenient method to have.
    fprintf(stderr,"CSTMERR %s: %s\n", message, strerror(errno));
    printf("%s: %s\n", message, strerror(errno));
    return -1;
}

static void quickSortNoT(void* p) {
    SortParams* params = (SortParams*) p;
    char** array = params->array;
    int left = params->left;
    int right = params->right;
    int i = left, j = right;

    if (j - i > SORT_THRESHOLD) {           /* if the sort range is substantial, use quick sort */

        int m = (i + j) >> 1;               /* pick pivot as median of         */
        char* temp, *pivot;                 /* first, last and middle elements */
        if (strcmp(array[i],array[m]) > 0) {
            temp = array[i]; array[i] = array[m]; array[m] = temp;
        }
        if (strcmp(array[m],array[j]) > 0) {
            temp = array[m]; array[m] = array[j]; array[j] = temp;
            if (strcmp(array[i],array[m]) > 0) {
                temp = array[i]; array[i] = array[m]; array[m] = temp;
            }
        }
        pivot = array[m];

        for (;;) {
            while (strcmp(array[i],pivot) < 0) i++; /* move i down to first element greater than or equal to pivot */
            while (strcmp(array[j],pivot) > 0) j--; /* move j up to first element less than or equal to pivot      */
            if (i < j) {
                char* temp = array[i];      /* if i and j have not passed each other */
                array[i++] = array[j];      /* swap their respective elements and    */
                array[j--] = temp;          /* advance both i and j                  */
            } else if (i == j) {
                i++; j--;
            } else break;                   /* if i > j, this partitioning is done  */
        }

        SortParams first;  first.array = array; first.left = left; first.right = j;
        quickSortNoT(&first);                  /* sort the left partition	*/

        SortParams second; second.array = array; second.left = i; second.right = right;
        quickSortNoT(&second);                 /* sort the right partition */

    } else insertSort(array,i,j);           /* for a small range use insert sort */
}

/* Recursive quick sort, but with a provision to use */
/* insert sort when the range gets small.            */

static void *quickSort(void* p) {
    SortParams* params = (SortParams*) p;
    char** array = params->array;
    int left = params->left;
    int right = params->right;
    int i = left, j = right;
    pthread_t thread1, thread2;    
    int t1=0,t2=0;	
    int diff = j-i;
    if (diff > SORT_THRESHOLD) {           /* if the sort range is substantial, use quick sort */

        int m = (i + j) >> 1;               /* pick pivot as median of         */
        char* temp, *pivot;                 /* first, last and middle elements */
        if (strcmp(array[i],array[m]) > 0) {
            temp = array[i]; array[i] = array[m]; array[m] = temp;
        }
        if (strcmp(array[m],array[j]) > 0) {
            temp = array[m]; array[m] = array[j]; array[j] = temp;
            if (strcmp(array[i],array[m]) > 0) {
                temp = array[i]; array[i] = array[m]; array[m] = temp;
            }
        }
        pivot = array[m];

        for (;;) {
            while (strcmp(array[i],pivot) < 0) i++; /* move i down to first element greater than or equal to pivot */
            while (strcmp(array[j],pivot) > 0) j--; /* move j up to first element less than or equal to pivot      */
            if (i < j) {
                char* temp = array[i];      /* if i and j have not passed each other */
                array[i++] = array[j];      /* swap their respective elements and    */
                array[j--] = temp;          /* advance both i and j                  */
            } else if (i == j) {
                i++; j--;
            } else break;                   /* if i > j, this partitioning is done  */
        }
	

        SortParams first;  first.array = array; first.left = left; first.right = j;
        if(maximumThreads <= 0 || j-left < THREAD_THRESHOLD){
            quickSort(&first);                  /* sort the left partition	*/
        }else{
            pthread_mutex_lock(&mutex);
            if(pthread_create(&thread1,NULL,&quickSort,(void*)(&first)) != 0){
                errorHandler("Failed to create thread");
                return NULL;
            }
            t1 = 1;
            maximumThreads--;
            pthread_mutex_unlock(&mutex);
        }
        SortParams second; second.array = array; second.left = i; second.right = right;
        quickSort(&second);        /* sort the right partition */
    } else insertSort(array,i,j);           /* for a small range use insert sort */
    if(t1){
        pthread_join(thread1,NULL);
        pthread_mutex_lock(&mutex);
        maximumThreads++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}



/* user interface routine to set the number of threads sortT is permitted to use */

void setSortThreads(int count) {
    maximumThreads = count;
}

/* user callable sort procedure, sorts array of count strings, beginning at address array */

void sortThreaded(char** array, unsigned int count) {
    SortParams parameters;
    parameters.array = array; parameters.left = 0; parameters.right = count - 1;
    quickSort(&parameters);
}
void sortNotThreaded(char** array, unsigned int count) {
    SortParams parameters;
    parameters.array = array; parameters.left = 0; parameters.right = count - 1;
    quickSortNoT(&parameters);
}


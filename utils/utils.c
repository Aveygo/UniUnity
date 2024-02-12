/*******************************************************************************
 * utils.c
 * Common/generic functions that are used universally 
*******************************************************************************/

/*******************************************************************************
 * Function prototypes
*******************************************************************************/

#include "utils.h"

/*******************************************************************************
 * Global variables
*******************************************************************************/

extern int DEBUG;

/**
 * "Slow" search through an unsorted array.
 * Works great for one-time finding within a small UNSORTED array
 * Return: int
 *
 * Inputs
 *  - target    the value to search for
 *  - arr       the (sorted) array to search within
 * Outputs
 *  - Index of target in array, or -1
 * 
 */
int simple_search(int target, int* arr, int arr_size) {
    for (int i = 0; i < arr_size; i++) {
        if (arr[i] == target) {
            return i;
        }
    }

    return -1;
}

/**
 * Binary search through a sorted array
 * Splits the array into two halfs, if the target is smaller than the midpoint,
 * continue in the left half, otherwise right.
 * Return: int
 *
 * Inputs
 * - target     the value to search for
 * - arr        the (sorted) array to search within
 * - low        the lower end of the search range
 * - high       the upper end of the search range
 * Outputs
 *  - Index of target in array, or -1
 * 
 */
int binary_search(int target, int* arr, int low, int high) {
    while (low <= high) {
        int mid = low + (high - low) / 2;

        if (arr[mid] == target)
            return mid;
        else if (arr[mid] < target)
            low = mid + 1;
        else
            high = mid - 1;
    }

    return -1;
}

/**
 * Fast search through a sorted array.
 * Splits the array into two halfs, if the target is smaller than the midpoint,
 * continue in the left half, otherwise right.
 * Return: int
 *
 * Inputs
 * - target     the value to search for
 * Outputs
 *  - Index of target in array, or -1
 * 
 */
int fast_search(int target, int* arr, int arr_size) {
    return binary_search(target, arr, 0, arr_size);
}

/**
 * Swap the values of two integer pointers
 * Return: void
 *
 * Inputs
 * - a     first pointer
 * - b     second pointer
 * Outputs
 *  - Swapped pointers
 * 
 */
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * Quicksort partitioning,
 * this "moves" values around until the left half's largest value
 * is smaller than the right half's smallest.
 * eg: [10, 2, 8, 1, 3, 9] -> [2, 1, 3, 10, 8, 9]  
 * Return: int
 *
 * Inputs
 * - arr        the target array
 * - low        the lower end of the current partition
 * - high       the upper end of the current partition
 * Outputs
 *  - new pivot index
 * 
 */
int partition(int arr[], int low, int high) {
    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j <= high; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

/**
 * Quicksort algorithm,
 * Basically splits an array into two halfs where the left contains all the
 * smaller values, then recursively solves each half. 
 * Return: void
 *
 * Inputs
 * - arr        the target array
 * - low        the lower end of the current partition
 * - high       the upper end of the current partition
 * Outputs
 *  - Sorted int array
 * 
 */
void quickSort(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

/**
 * Sort an array of ints
 * Great for fast_search()
 * Return: void
 *
 * Inputs
 * - arr        Array to sort
 * - arr_size   The number of elements in the array
 * Outputs
 *  - Sorted int array
 * 
 */
void sort(int* arr, int arr_size) {
    quickSort(arr, 0, arr_size-1);
}


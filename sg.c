/*
* Program: SacredGeometry
* Author: Cognoscan
* Date: 2015-07-07
* Source: http://www.giantitp.com/forums/archive/index.php/t-426359.html
* Changed by: Temennigru
* Date: 2017-06-14
*
* Brute forces a solution to the Sacred Geometry feat in pathfinder, if a 
* solution exists. Takes in a target level, and a set of dice rolls. It then 
* runs through every unique permutation of the dice rolls and tries all 
* possible operand sequences. If there is a solution, it will find it. May take 
* a while for large dice pools, but honestly, I'm pretty sure it becomes 
* exponentially more likely that a solution exists as dice pool size increases. 
* So don't bother with a full 20 dice unless you need to.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define MAXCOUNT 20
#define MAXBUF 1024

const int goals[9][3] = {
{ 3, 5, 7},
{11, 13, 17},
{19, 23, 29},
{31, 37, 41},
{43, 47, 53},
{59, 61, 67},
{71, 73, 79},
{83, 89, 97},
{101, 103, 107}};

void print_int_array(int array[], int size) {
    int i;
    for (i = 0; i < size; i++) {
        printf("%i ", array[i]);
    }
    printf("\n");
}

void print_char_array(char array[], int size) {
    int i;
    for (i = 0; i < size; i++) {
        printf("\'%c\' ", array[i]);
    }
    printf("\n");
}

void swap(int*, int*);
void reverse(int* array, int length);
int intCompare (const void *a, const void *b);
void printResult (const int dice[], const int ops[], int length, int result);
bool nextPermutation(int data[], int length);
int computeTest(int working, int index, int lvl, const int data[], int ops[], int length);
int roll_d(int num);
void split(const char str[], int dice_array[], int* count);

int main(int argc, char *argv[]) {
    int dice[MAXCOUNT] = {0};
    int lvl = 0;
    int ops[MAXCOUNT] = {0}; // 20 large so we can index into ops[19] without causing an exception
    int count = 0;
    int goal = -1;
    char dice_roll_str[MAXBUF];
    int i;

    srand((unsigned) time(NULL));

    // Grab Spell Level
    printf("Pathfinder - Sacred Geometry\n");
    printf("Feat Solution Finder\n");
    printf("============================\n");
    printf("Spell Level: ");
    while(!scanf("%i", &lvl));
    lvl = (lvl > 9) ? 9 : lvl;
    lvl = (lvl < 1) ? 1 : lvl;
    lvl--;

    // Grab Dice Rolls

    printf("Knowledge (engineering) ranks (-1 to input dice rolls): ");
    while(!scanf("%i", &count));
    if(count > MAXCOUNT) count = MAXCOUNT;
    if(count == -1) {
        printf("Dice Rolls (spaces between): ");
        getchar(); // Removes the newline from stdin, allowing fgets to work
        fgets (dice_roll_str, MAXBUF, stdin);

        split(dice_roll_str, dice, &count);
    } else {
        for(i = 0; i < count; i++) {
            dice[i] = roll_d(6);
        }
    }

    printf("Dice rolls: ");
    print_int_array(dice, count);

    // Permutation algorithm expects sorted array
    qsort(dice, count, sizeof(int), intCompare);

    // Run Recursive brute-force algorithm on each permutation. Stop when goal is 
    // found or when there are no more permutations
    goal = computeTest(dice[0], 1, lvl, dice, ops, count);
    while((goal < 0) && nextPermutation(dice, count)) {
        goal = computeTest(dice[0], 1, lvl, dice, ops, count);
    }

    // If goal value was reached, print the solution
    if (goal >= 0) {
        printResult(dice, ops, count, goals[lvl][goal]);
    } else {
        printf("No valid result found!\n");
    }
    return 0;
}

// Swap two numbers
void swap(int* a, int* b) {
    int temp = *b;
    *b = *a;
    *a = temp;
}

// Reverse a given array of some length
void reverse(int* array, int length) {
    for(int i=0; i<length/2; i++) {
        swap(&array[i], &array[length-1-i]);
    }
}

// Integer compare function for qsort
int intCompare (const void *a, const void *b) {
    if ( *(int*)a < *(int*)b ) return -1;
    if ( *(int*)a > *(int*)b ) return 1;
    return 0; // They are equal
}

// Get next permutation of data array, given an initially ordered array where 
// a[k] <= a[k+1]. Returns false when all permutations have been done. If there
// are repeated values in the data, it handles them by only generating distinct 
// permutations (no repeats due to repeated values).
bool nextPermutation(int data[], int length) {
    int k = -1;
    int l = 0;
    // Find the largest index k such that a[k] < a[k+1]
    for (int i=length-2; i>=0; i--) {
        if (data[i] < data[i+1]) {
            k = i;
            break;
        }
    }
    if (k == -1) return false;
    // Find the largest index l greater than k such that a[k] < a[l].
    for (int i=length-1; i>=k; i--) {
        if (data[k] < data[i]) {
            l = i;
            break;
        }
    }
    if (l <= k) return false; // This should never happen.
    // Swap the value of a[k] with that of a[l].
    swap(&data[k], &data[l]);
    // Reverse the sequence from a[k + 1] up to and including the final element a[n]
    reverse(&data[k+1], length-k-1);
    return true;
}

// Print a Valid solution, with parentheses
void printResult (const int dice[], const int ops[], int length, int result) {

    printf("Valid: %i = ", result);
    // Print out opening parentheses
    for (int i=0; i<length-1; i++) {
        if (ops[i] < 2 && ops[i+1] >= 2) {
            putchar('(');
        }
    }

    printf("%i ", dice[0]);
    for (int i=1; i < length; i++) {
        switch(ops[i-1]) {
            case 0: printf("+ "); break;
            case 1: printf("- "); break;
            case 2: printf("* "); break;
            case 3: printf("/ "); break;
        }
        printf("%i", dice[i]);
        if (ops[i-1] < 2 && ops[i] >= 2) {
            putchar(')');
        }
        putchar(' ');
    }

    printf("\n");
}

/*
* Recursive Testing of all possible operand sequences on a given data set.
* Inputs:
* working - result of last operation. When first called, this should be data[0]
* index - Current index of data[] that is being used to perform next operation
* lvl - For indexing into goals array to see if valid solution was reached.
* data[] - The data being operated on
* ops[] - Array to record the operands used.
* length - Total length of data[] array
* Returns:
* -1 if no valid solution reached, otherwise the index of the goal value reached.
*
* Operand keys are:
* 0 - Addition
* 1 - Subtraction
* 2 - Multiplication
* 3 - Division
*/
int computeTest(int working, int index, int lvl, const int data[], int ops[], int length) {
    int ret = -1;
    // Final value computed. Test against target values
    if (index == length) {
        if (working == goals[lvl][0]) return 0;
        if (working == goals[lvl][1]) return 1;
        if (working == goals[lvl][2]) return 2;
        return -1;
    }

    // Test with the 4 possible arithmetic operations
    for (int i=0; i<4; i++) {
        switch(i) {
            case 0: ret = computeTest(working+data[index], index+1, lvl, data, ops, length); break;
            case 1: ret = computeTest(working-data[index], index+1, lvl, data, ops, length); break;
            case 2: ret = computeTest(working*data[index], index+1, lvl, data, ops, length); break;
            case 3: if (working % data[index] == 0) {
                ret = computeTest(working/data[index], index+1, lvl, data, ops, length); 
            } else {
                ret = -1;
            }
            break;
        }
        // Check to see if we found a solution
        if (ret >= 0) {
            ops[index-1] = i; // Record the operand
            break;
        }
    }
    return ret;
}

int roll_d(int num) {
   return (rand() % num) + 1;
}

void split(const char str[], int dice_array[], int* count) {
    int i = 0, j = 0;
    char temp_buf[33] = {'\0'};
    int tmp_cnt = 0;
    while(tmp_cnt < MAXCOUNT && str[i] != '\0' && i < MAXBUF) {
        if(isdigit(str[i])) {
            if(j < 33) {
                temp_buf[j] = str[i];
                temp_buf[j+1] = '\0';
                j++;
            }
        } else {
            if (strlen(temp_buf) != 0) {
                temp_buf[j] = '\0';
                dice_array[tmp_cnt] = atoi(temp_buf);
                temp_buf[0] = '\0';
                j = 0;
                tmp_cnt++;
            }
        }
        i++;
    }
    *count = tmp_cnt;
}
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "MatrixMultiplication.h"

/*
 * This is the implemented method to read the files and populate the matrices
 * that are to be multiplied together from the files
 *
 * Parameters (fOne, fTwo, mOne, mTwo): The parameters are the files that store
 * the matrices which are passed in from the user in main.
 * The matrices are also passed in so that when the data is read in and stored,
 * it can be used by the calling method
 */
void readFiles(char *fOne, char *fTwo, matrix *mOne, matrix *mTwo)
{
	FILE *fileOne = fopen(fOne, "r");

	// Checks to make sure a valid file is loaded
	if (fileOne == NULL)
	{
		printf("Can't Load File\n");
		exit(EXIT_FAILURE);
	}

	// The first line in the matrix file are number of rows and cols
	fscanf(fileOne, "%d %d", &mOne->row, &mOne->col);

	// Allocates memory for the size of the matrix and creates pointer to it
	mOne->data = malloc(sizeof(int) * mOne->row * mOne->col);

	// Used to iterate through the file and store the values
	for (int i = 0; i < mOne->row; i++)
		for (int j = 0; j < mOne->col; j++)
			fscanf(fileOne, "%d", &mOne->data[i * mOne->col + j]);

	fclose(fileOne);

	FILE *fileTwo = fopen(fTwo, "r");

	// Checks to make sure a valid file is loaded
	if (fileTwo == NULL)
	{
		printf("Can't Load File\n");
		exit(EXIT_FAILURE);
	}

	// The first line in the matrix file are number of rows and cols
	fscanf(fileTwo, "%d %d", &mTwo->row, &mTwo->col);

	// This checks to make sure that the matrices can be multiplied togeter
	if (mOne->col != mTwo->row)
	{
		printf("Matrices can't be multiplied due to size\n");
		exit(EXIT_FAILURE);
	}

	// Allocates memory for the size of the matrix and creates pointer to it
	mTwo->data = malloc(sizeof(int) * mTwo->row * mTwo->col);

	// Used to iterate through the file and store the values
	for (int i = 0; i < mTwo->row; i++)
		for (int j = 0; j < mTwo->col; j++)
			fscanf(fileTwo, "%d", &mTwo->data[i * mTwo->col + j]);

	fclose(fileTwo);
}

/*
 * This is the method which runs the multiply matrix function as a single process
 * and it takes in the name of the two files which contains the matrix as parameters
 */
void singleProcess(char *fOne, char *fTwo)
{
	// Calculating execution time for multiplication function
	struct timespec start, end;

	clock_gettime(CLOCK_REALTIME, &start);

	matrix *mOne = malloc(sizeof(matrix));	   // Space is set aside for matrix one
	matrix *mTwo = malloc(sizeof(matrix));	   // Space is set aside for matrix two
	matrix *mProduct = malloc(sizeof(matrix)); // Space is set aside for matrix res

	// Calls the read method to populate the two passed on matrices
	readFiles(fOne, fTwo, mOne, mTwo);

	// Pointer is created that will store the product matrix
	mProduct->data = malloc(sizeof(int) * mOne->row * mTwo->col);

	int count = 0;
	int temp = 0;
	// Used to multiply the matrix together and store the result
	for (int i = 0; i < mOne->row; i++)
	{
		for (int j = 0; j < mTwo->col; j++)
		{
			for (int k = 0; k < mTwo->row; k++)
				temp += mOne->data[i * mOne->col + k] * mTwo->data[k * mTwo->col + j];

			mProduct->data[count] = temp;

			count++;
			temp = 0;
		}
	}

	// This ends the clock time and computes the duration of the multiply function
	clock_gettime(CLOCK_REALTIME, &end);

	double totalTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

	// This is used to display the resulting matrix
	for (int i = 0; i < mOne->row; i++)
	{
		for (int j = 0; j < mTwo->col; j++)
			printf("%d\t", mProduct->data[i * mTwo->col + j]);
		printf("\n");
	}

	// Prints execution time
	printf("\t\t\tExecution time: %fs\n\n", totalTime);

	// Freeing the malloc
	free(mOne->data);
	free(mTwo->data);
	free(mProduct->data);
	free(mOne);
	free(mTwo);
	free(mProduct);
}

/*
 * This is the child process used for multi-processing which contains the logic
 * for doing matrix multiplication
 *
 * Parameters (toggle, mOne, mTwo, loc, row, col): The parameters here are the
 * toggle function which controls if it is a read or write instruction, the two
 * matrices that were loaded, loc is a count to keep track of the cell that is
 * supposed to be calculated, and row and col are the current row and col so
 * that the calculation can be executed
 */
void child(int *toggle, matrix *mOne, matrix *mTwo, int loc, int row, int col)
{
	childExec result;

	result.val = 0;
	result.loc = loc;

	for (int i = 0; i < mTwo->row; i++)
		result.val += mOne->data[row * mOne->col + i] * mTwo->data[mTwo->col * i + col];

	write(toggle[1], &result, sizeof(result));

	close(toggle[1]); // Close Write
}

/*
 * This is the method which runs the multiply matrix function as multiple processes
 * and it takes in the name of the two files which contains the matrix as parameters
 */
void multipleProcess(char *fOne, char *fTwo)
{
	childExec result;

	matrix *mOne = malloc(sizeof(matrix));	   // Space is set aside for matrix one
	matrix *mTwo = malloc(sizeof(matrix));	   // Space is set aside for matrix two
	matrix *mProduct = malloc(sizeof(matrix)); // Space is set aside for matrix

	// Calls the read method to populate the two passed on matrices
	readFiles(fOne, fTwo, mOne, mTwo);

	// Calculating execution time for multiplication function
	struct timespec start, end;

	clock_gettime(CLOCK_REALTIME, &start);

	// Pointer is created that will store the product matrix
	mProduct->data = malloc(sizeof(int) * mOne->row * mTwo->col);

	// This is used to distinguish between reading and writing
	int toggleRW[2];

	// Pipe failed
	if (pipe(toggleRW) == -1)
	{
		printf("Pipe Error\n");
		exit(EXIT_FAILURE);
	}

	int count = 0;
	for (int i = 0; i < mOne->row; i++)
	{
		for (int j = 0; j < mTwo->col; j++)
		{
			pid_t pid = fork(); // Fork into 2 processes

			if (pid < 0) // Error
			{
				printf("Fork failed, terminating\n");
				exit(EXIT_FAILURE);
			}
			else if (pid == 0) // Child
			{
				close(toggleRW[0]); // Close Read
				child(toggleRW, mOne, mTwo, count, i, j);

				// Freeing child malloc
				free(mOne->data);
				free(mTwo->data);
				free(mProduct->data);
				free(mOne);
				free(mTwo);
				free(mProduct);
				exit(EXIT_SUCCESS);
			}

			count++;
		}
	}

	close(toggleRW[1]); // Close Write

	int complete = 0;
	// Used to load in the values from the child process into the product matrix
	while ((complete = read(toggleRW[0], &result, sizeof(childExec))))
		mProduct->data[result.loc] = result.val;

	// Busy wait, required for proper execution
	for (int i = 0; i < (mOne->row * mTwo->col); i++)
		wait(0);

	// This is used to display the resulting matrix
	for (int i = 0; i < mOne->row; i++)
	{
		for (int j = 0; j < mTwo->col; j++)
			printf("%d\t", mProduct->data[i * mTwo->col + j]);
		printf("\n");
	}

	// This ends the clock time and computes the duration and prints it
	clock_gettime(CLOCK_REALTIME, &end);

	double totalTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
	printf("\t\t\tExecution time: %fs\n", totalTime);

	// Freeing parent malloc
	free(mOne->data);
	free(mTwo->data);
	free(mProduct->data);
	free(mOne);
	free(mTwo);
	free(mProduct);
}
#include "MatrixMultiplication.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

void generateMatrixFile(char *fileName, int rows, int cols)
{
	srand(time(0));

	FILE *fp = fopen(fileName, "w");

	// Checks to make sure a valid file is loaded
	if (fp == NULL)
	{
		printf("Can't Load File\n");
		exit(EXIT_FAILURE);
	}

	// Write the number of rows and cols to the file
	fprintf(fp, "%d %d\n", rows, cols);

	// Write random data as the values of the matrix
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols - 1; j++)
			fprintf(fp, "%d ", rand() % 10);
		fprintf(fp, "%d\n", rand() % 10);
	}

	fclose(fp);
}

int convertToInt(char *argv)
{
	char *p;
	errno = 0; // not 'int errno', because the '#include' already defined it

	long val = strtol(argv, &p, 10);

	if (*p != '\0' || errno != 0)
	{
		printf("Error in converting to int\n");
		exit(EXIT_FAILURE);
	}
	if (val < INT_MIN || val > INT_MAX)
	{
		printf("Error in converting to int\n");
		exit(EXIT_FAILURE);
	}

	// Everything went well, so return the value
	return (int)val;
}

int main(int argc, char *argv[])
{
	// Error check for incorrect number of arguments
	if (argc != 5)
	{
		printf("Syntax: %s <rows of Matrix One> <cols of Matrix One> "
			   "<rows of Matrix Two> <cols of Matrix Two>\n",
			   argv[0]);
		exit(EXIT_FAILURE);
	}

	// Names of the temporary files used for matrix multiplication
	char *fileOne = "matA";
	char *fileTwo = "matB";

	// Generate a random matrix and write it to a file
	generateMatrixFile(fileOne, convertToInt(argv[1]), convertToInt(argv[2]));
	generateMatrixFile(fileTwo, convertToInt(argv[3]), convertToInt(argv[4]));

	printf("\t\t\tSingle Processor Execution\n");
	/*
     * Calls the multiply matrix function to be executed on a single processor
     * and takes the names of the files (paths if files are in a seperate dir as
	 * the program) as parameters and executes the multiplication function
     */
	singleProcess(fileOne, fileTwo);

	printf("====================================================================="
		   "======\n\n\t\t\tMulti-Processor Execution\n");

	/*
     * Calls the multiply matrix function to be executed on multiple processors
     * and takes the names of the files (paths if files are in a seperate dir as
	 * the program) as parameters and executes the multiplication function
     */
	multipleProcess(fileOne, fileTwo);

	// Delete temporary files
	if (remove(fileOne))
		printf("Unable to delete the file %s\n", fileOne);
	if (remove(fileTwo))
		printf("Unable to delete the file %s\n", fileTwo);

	return 0;
}

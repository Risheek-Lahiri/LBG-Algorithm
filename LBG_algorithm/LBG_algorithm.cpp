// LBG_algorithm.cpp : Defines the entry point for the console application.


#include "stdafx.h"
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

//Generates random number between 0 and limit (both included)
int randInt(int limit)
{
	return (rand() % (limit+1));
}

//Converts Universe.csv to a 2D matrix
void universeToMatrix(double** universe, int uSize, int p)
{
	FILE* fp = fopen("Universe.csv","r");

	int i=0, j=0;			//Iterators

	char* csv_line;			//Reads a single value from the row (separated by comma)
	char mystring[1024];	//Records a full row from csv file

	//Loop in each row of Universe.csv, read every line
	while(fgets(mystring, 1024, fp))
	{
		csv_line = strtok(mystring, ",");	//Record single value (token), separated by comma

		j = 0;
		while(csv_line)		//While token is not null
		{
			universe[i][j] = atof(csv_line);	//Save token in 2D array, after converting in double
			j++;
			csv_line = strtok(NULL,",");		//Move to next token
		}
		i++;
	}
}

//Prints codebook
void printCodebook(double** codebook, int k, int p)
{
	int i=0,j=0;
	for(i=0;i<k;i++)
	{
		for(j=0;j<p;j++)
			printf("%f\t",codebook[i][j]);
		printf("\n");
	}
}

//Calculates tokhura distance
double* tokhuraDist(int uIndex, double** universe, int uSize, double** codebook, int k, int p)
{
	double wi[12] = {1.0, 3.0, 7.0, 13.0, 19.0, 22.0, 25.0, 33.0, 42.0, 50.0, 56.0, 61.0};	//Wi values for Tokhura's distance

	double tokhura = 0;
	double tokhuramin[2];	//For calculation of minimum tokhura distance

	tokhuramin[0] = 1000000000000;	//Saves minimum tokhura distance
	tokhuramin[1] = -1;				//Saves region number of min. tokhura distance

	int i=0,j=0;

	for(i=0;i<k;i++)
	{
		for(j=0;j<p;j++)
			tokhura += wi[j]*(pow((universe[uIndex][j]-codebook[i][j]),2));	//Tokhura's distance formula

		//Finding minimum from the distances
		if(tokhura<tokhuramin[0])
		{
			tokhuramin[0] = tokhura;	//Current distance is minimum
			tokhuramin[1] = i;			//Current region is minimum
		}
		tokhura = 0;
	}

	return tokhuramin;
}

double* universeToArray(double** universe, int uSize, int p)
{
	int i=0,j=0;

	double* retarr = new double[p];

	for(i=0;i<p;i++)
		retarr[i] = 0;

	for(i=0;i<uSize;i++)
	{
		for(j=0;j<p;j++)
			retarr[p] += universe[i][p];
	}

	for(i=0;i<p;i++)
		retarr[i] /= uSize;

	return retarr;
}

//Calculate resultant codebook after Kmeans algorithm
double** kmeans(double** universe, int uSize, double** codebook, int k, int p)
{
	int i=0;
	int j=0;

	double delta = 0.00001;

	double prev_distortion = 10000000000;
	double distortion = 0;

	double** region_sum = new double*[p];	//Stores sum of the c1...c12 of a region
	double* region_size = new double[k];	//Stores no. of vectors in a region

	//Initialize region_sum and region_size with zeros
	for(i=0;i<k;i++)
	{
		region_size[i] = 0;

		region_sum[i] = new double[p];	//Also create new rows in region_sum
		for(j=0;j<p;j++)
			region_sum[i][j] = 0;
	}


	double* dist = NULL;	//Stores return of tokhuraDist()
	
	while(delta < abs(prev_distortion - distortion))
	{
		prev_distortion = distortion;	//Update previous distortion
		distortion = 0;
		
		//Initialize region_sum and region_size with zeros
		for(i=0;i<k;i++)
		{
			region_size[i] = 0;

			for(j=0;j<p;j++)
				region_sum[i][j] = 0;
		}

		//Loop in all rows of universe
		for(i=0;i<uSize;i++)
		{
			dist = tokhuraDist(i,universe,uSize,codebook,k,p);

			distortion += dist[0];	//Add min tokhura distance to distortion

			for(j=0;j<p;j++)
			{
				region_sum[(int)dist[1]][j] += universe[i][j];	//Add current universe row to region_sum
			}

			region_size[(int)dist[1]]++;	//Increment region size
		}

		distortion /= uSize;	//Average out distortion to calculate

		for(i=0;i<k;i++)
		{
			for(j=0;j<p;j++)
				codebook[i][j] = (region_sum[i][j] / region_size[i]);	//Averaging out codebook
		}
	}

	printf("Distortion calculated by k-means is------- %f4\n",distortion);

	return codebook;
}

double** LBG(double** universe, int uSize, double* codebook_arr, int k, int p, double epsilon)
{
	int i=0;
	int j=0;

	double delta = 0.00001;

	double **temp_codebook = NULL, **final_codebook = NULL;		//temp_codebook saves saves splitted, final_codebook saves codebook after an iteration
	int codebook_size = 1;

	//Initialize final_codebook of size 1
	final_codebook = new double*[1];
	for(i=0;i<1;i++)
	{
		final_codebook[i] = new double[p];

		for(j=0;j<p;j++)
			final_codebook[i][j] = codebook_arr[j];
	}
	
	//Loop condition
	while(codebook_size <= k)
	{
		temp_codebook = new double*[2*codebook_size];

		//Initialize temp_codebook of double size
		for(i=0;i<(2*codebook_size);i++)
			temp_codebook[i] = new double[p];

		for(i=0;i<codebook_size;i++)
		{
			//Save into splitted codebooks
			for(j=0;j<p;j++)
			{
				temp_codebook[2*i][j] = final_codebook[i][j]*(1+epsilon);
				temp_codebook[(2*i)+1][j] = final_codebook[i][j]*(1-epsilon);
			}
		}

		final_codebook = temp_codebook;	//Merge splitted codebooks
		codebook_size *= 2;

		printf("----For size--- %d we obtained the ",(codebook_size/2));
		final_codebook = kmeans(universe,uSize,final_codebook,codebook_size,p);		//Use kmeans in merged codebook
	}

	return final_codebook;
}




int main()
{
    int universeSize = 6340, codebookSize = 8, dimensionality = 12;  // Initialize quantities

    // Create empty matrices for codebook and universe
    double** codebook = new double*[codebookSize];
    double** universe = new double*[universeSize];

    int i = 0, j = 0;

    // Initialize codebook and universe arrays to zero
    for (i = 0; i < codebookSize; i++)
    {
        codebook[i] = new double[dimensionality];
        for (j = 0; j < dimensionality; j++)
            codebook[i][j] = 0;
    }

    for (i = 0; i < universeSize; i++)
    {
        universe[i] = new double[dimensionality];
        for (j = 0; j < dimensionality; j++)
            universe[i][j] = 0;
    }

    universeToMatrix(universe, universeSize, dimensionality);  // Feed Universe.csv into matrix

    double* codebookArray = universeToArray(universe, universeSize, dimensionality);  // Initialize codebook array of size 1 to start
    double epsilon = 0.03;

    codebook = LBG(universe, universeSize, codebookArray, codebookSize, dimensionality, epsilon);  // Apply LBG

    printf("\n*********************Final codebook through LBG:***************************\n");
    printCodebook(codebook, codebookSize, dimensionality);  // Print result

    system("PAUSE");  // Pause output in console

    return 0;
}


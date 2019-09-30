// Input: DataSetSize, BufferSize, DatasetFilename, OutputFilename
// Output: the file OutputFilename containing the sorted dataset.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

float generateRand(int rmax) {
	//Generate a floating point random number between 0 and rmax
	//initialize the random number generator
	return ( (float)rand()/(float)(RAND_MAX)) * rmax;
}

int createDataset(int datasetSize, char *path) {

	FILE* f = fopen(path, "w");
	if (f != NULL) {
		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		fseek(f, 0, SEEK_SET);

		if (size / sizeof(float) == datasetSize)
			return 0;
	}

	float v[datasetSize];
	for(int i=0; i < datasetSize; i++) {
		v[i] = generateRand(100);
		printf("%.1f ", v[i]);
	}

	fwrite(v, sizeof(float), datasetSize, f);
	fclose(f);
	return 1;
}

float* loadDataset(int dataSetSize, char *path, size_t blockSize) {
	FILE *fp;
	float *v = (float*)malloc(sizeof(float) * dataSetSize);
	fp = fopen(path , "r" );
	size_t fileSize = dataSetSize * sizeof(float);
	double count = (double)fileSize/(double)blockSize;
	count = ceil(count);

	for(int i = 0; i < (int)count; i++) {
		fread(&v[i], blockSize, 1, fp);
	}
	// TIP you can define you own buffer, buffer size and you can read blocks of data of size > 1
	fclose(fp);

	return v;
}

int writeDataset(float* v, int dataSetSize, char *path, size_t blockSize, float avg, float min, float max) {
	FILE *fp;
	fp = fopen(path , "w" );

	fwrite(&avg, sizeof(float), 1, fp);
	fwrite(&min, sizeof(float), 1, fp);
	fwrite(&max, sizeof(float), 1, fp);
	size_t fileSize = dataSetSize * sizeof(float);
	double count = (double)fileSize/(double)blockSize;
	count = ceil(count);
	//double frac = count - (double)((int)count);
	//if (frac > 0.00001)


	for(int i=0; i<(int)count; i++){
		fwrite((void*)(&v[i]), blockSize, 1, fp);
	}
	
	// TIP you can define you own buffer, buffer size and you can write blocks of data of size > 1
	fclose(fp);
	return(0);
}

void swap(float *xp, float *yp) {
	float temp = *xp;
	*xp = *yp;
	*yp = temp;
}

float average(float* dataSet) 
{
	float average = 0;

	int size = sizeof(dataSet) / sizeof(float);
	for(int i = 0; i < size; i++)
		average += dataSet[i];

	
	return average / size;
}

float maxvalue(float* dataSet)
{
	float max = 0.0f;

	int size = sizeof(dataSet) / sizeof(float);
	for(int i = 0; i < size; i++)
		if (max < dataSet[i])
			max = dataSet[i];
	
	return max;
}

float minvalue(float* dataSet)
{
	float min = 0.0f;

	int size = sizeof(dataSet) / sizeof(float);
	for(int i = 0; i < size; i++)
		if (min > dataSet[i])
			min = dataSet[i];
	
	return min;
}

int cmpfunc(const void* a, const void* b) {
	return ( *(float*)a - *(float*)b);
}


int main(int argc, char *argv[]) {
	// Generate random seeds
	srand((unsigned int)time(NULL));

	int dataSetSize = 0;
	int bufferSize = 0;
	char* inputFileName = NULL;
	char* outputFileName = NULL;
	if (argc >= 4) {
		dataSetSize	= atoi(argv[1]);	
		bufferSize	= atoi(argv[2]);	
		inputFileName	= argv[3];	
		outputFileName	= argv[4];	
	}
	else 
		return -1;

	printf("Dataset Size: %d, Buffersize: %d, InputFN: %s, OutputFN: %s \n"
			, dataSetSize, bufferSize, inputFileName, outputFileName);

	createDataset(dataSetSize, inputFileName);
	
	// Timer
	double time_spent = 0.0;
	clock_t begin = clock();

	// load the dateset in the memory area addressed by ds
	float* ds = loadDataset(dataSetSize, inputFileName, (size_t)bufferSize);
	
	// compute the average value of the dataset, i.e. sum_of_dataset_values / num_of_dataset_values
	float avg = average(ds);
	// find the max value in the dataset
	float max = maxvalue(ds);
	// find the min value in the dataset
	float min = minvalue(ds);

	printf("\n\nUnSorted: ");
	for (int i = 0; i < dataSetSize; i++)
		printf("%.1f, ", ds[i]);

	//sort the dataset and copy it into the memory area pointed by sds
	qsort(ds, dataSetSize, sizeof(float), cmpfunc);

	//write the sorted array into a new file plus the valies of the average, min and max as the first three records.
	writeDataset(ds, dataSetSize, outputFileName, (size_t)bufferSize, avg, min, max);
	
	clock_t end = clock();
	time_spent += (double)(end - begin) / CLOCKS_PER_SEC;


	// printf("\n\nSorted: ");
	// for (int i = 0; i < dataSetSize; i++)
	// 	printf("%.1f, ", ds[i]);

	printf("\nTime elap: %f ms\n", time_spent * 1000);
	free(ds);

	return 0; 
}
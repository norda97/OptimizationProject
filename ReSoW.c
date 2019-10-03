// Input: datasetSize, BufferSize, DatasetFilename, OutputFilename
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
	}

	int e = fwrite(v, sizeof(float), datasetSize, f);

	// printf("Created file: ");
	// for (int i = 0; i < datasetSize; i++)
	// 	printf("%.1f, ", v[i]);

	fclose(f);
	return 1;
}

float* loadDataset(int datasetSize, char *path, size_t blockSize) {
	FILE *fp;
	float *v = (float*)malloc(sizeof(float) * datasetSize);
	fp = fopen(path , "r" );
	
	// Calc number of block to retrieve
	size_t fileSize = datasetSize * sizeof(float);
	double count = (double)fileSize/(double)blockSize;
	count = ceil(count);

	int index = 0;
	for(int i = 0; i < (int)count; i++) {
		fread(&v[index], blockSize, 1, fp);
		index += blockSize / sizeof(float);
	}

	// TIP you can define you own buffer, buffer size and you can read blocks of data of size > 1
	fclose(fp);

	return v;
}

int writeDataset(float* v, int datasetSize, char *path, size_t blockSize, float avg, float min, float max) {
	FILE *fp;
	fp = fopen(path , "w" );

	fwrite(&avg, sizeof(float), 1, fp);
	fwrite(&min, sizeof(float), 1, fp);
	fwrite(&max, sizeof(float), 1, fp);

	size_t fileSize = datasetSize * sizeof(float);
	double count = (double)fileSize/(double)blockSize;
	count = ceil(count);

	int index = 0;
	for(int i=0; i<(int)count; i++){
		fwrite(&v[index], blockSize, 1, fp);
		index += blockSize / sizeof(float);
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

void selectionSort(float* arr, int n) {
	int i, j, min_idx;

	for (int i = 0; i < n - 1; i++) {
		// Find the minimum element in unsorted array
		min_idx = i;
		for (int j = i+1; j < n; j++)
			if (arr[j] < arr[min_idx])
				min_idx = j;
		// Swap the found minimum element with the first element
		swap(&arr[min_idx], &arr[i]);
	}
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


int main(int argc, char *argv[]) {
	// Generate random seeds
	srand((unsigned int)time(NULL));

	// Timers for time measurement
	clock_t begin;
	clock_t end;
	double time_spent_writing = 0.0;
	double time_spent_reading = 0.0;
	double time_spent_sorting = 0.0;
	double time_spent_calc_avg = 0.0;
	double time_spent_calc_minmax = 0.0;

	int datasetSize = 0;
	int bufferSize = 0;
	char* inputFileName = NULL;
	char* outputFileName = NULL;
	if (argc >= 4) {
		datasetSize	= atoi(argv[1]);	
		bufferSize	= atoi(argv[2]) * 4;	
		inputFileName	= argv[3];	
		outputFileName	= argv[4];	
	}
	else 
		return -1;

	printf("Dataset Size: %d, Buffersize: %d, InputFN: %s, OutputFN: %s \n"
			, datasetSize, bufferSize, inputFileName, outputFileName);

	createDataset(datasetSize, inputFileName);

	// load the dateset in the memory area addressed by ds
	begin = clock();
	float* ds = loadDataset(datasetSize, inputFileName, (size_t)bufferSize);
	end = clock();
	time_spent_reading = (double)(end - begin) / CLOCKS_PER_SEC;
	
	// compute the average value of the dataset, i.e. sum_of_dataset_values / num_of_dataset_values
	begin = clock();
	float avg = average(ds);
	end = clock();
	time_spent_calc_avg += (double)(end - begin) / CLOCKS_PER_SEC;


	begin = clock();
	// find the max value in the dataset
	float max = maxvalue(ds);
	// find the min value in the dataset
	float min = minvalue(ds);
	end = clock();
	time_spent_calc_minmax += (double)(end - begin) / CLOCKS_PER_SEC;


	// printf("\n\nUnSorted: ");
	// for (int i = 0; i < datasetSize; i++)
	// 	printf("%.1f, ", ds[i]);

	//sort the dataset and copy it into the memory area pointed by sds
	begin = clock();
	selectionSort(ds, datasetSize);
	end = clock();
	time_spent_sorting += (double)(end - begin) / CLOCKS_PER_SEC;
	
	//write the sorted array into a new file plus the valies of the average, min and max as the first three records.
	begin = clock();
	writeDataset(ds, datasetSize, outputFileName, (size_t)bufferSize, avg, min, max);
	end = clock();
	time_spent_writing += (double)(end - begin) / CLOCKS_PER_SEC;

	printf("\n\nTime Read: %f ms\n", time_spent_reading * 1000);
	printf("Time Calc Average: %f ms\n", time_spent_calc_avg * 1000);
	printf("Time Calc MinMax: %f ms\n", time_spent_calc_minmax * 1000);
	printf("Time Sort: %f ms\n", time_spent_sorting * 1000);
	printf("Time Write: %f ms\n", time_spent_writing * 1000);
	free(ds);

	return 0; 
}
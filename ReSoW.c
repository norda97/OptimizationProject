#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

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

	fwrite(v, sizeof(float), datasetSize, f);
	fclose(f);
	return 1;
}

void loadDataset(float* v, long int pos, int dataSetSize, char *path, size_t blockSize) {
	FILE *fp;
	fp = fopen(path , "r" );
	fseek(fp, pos, 0);

	// Calc number of block to retrieve
	size_t fileSize = dataSetSize * sizeof(float);
	double count = (double)fileSize/(double)blockSize;
	count = ceil(count);
	
	int index = pos/sizeof(float);
	for(int i = 0; i < (int)count; i++) {
		size_t t = fread(&v[index], blockSize, 1, fp);
		index += blockSize / sizeof(float);
	}

	fclose(fp);
}

int writeDataset(long int pos, float* v, int dataSetSize, char *path, size_t blockSize, float avg, float min, float max) {
	FILE *fp;
	fp = fopen(path , "w" );
	fseek(fp, pos, 0);

	fwrite(&avg, sizeof(float), 1, fp);
	fwrite(&min, sizeof(float), 1, fp);
	fwrite(&max, sizeof(float), 1, fp);
	
	size_t fileSize = dataSetSize * sizeof(float);
	double count = (double)fileSize/(double)blockSize;
	count = ceil(count);
	
	int index = pos/sizeof(float);
	for(int i=0; i<(int)count; i++){
		fwrite(&v[index], blockSize, 1, fp);
		index += blockSize / sizeof(float);
	}

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

struct timespec time_begin, time_end;
/*
	Start the timer.
*/
void startClock() 
{
	// Reset timer.
	memset(&time_begin, 0, sizeof(time_begin));
	memset(&time_end, 0, sizeof(time_end));

	// Get a starting time.
	clock_gettime(CLOCK_REALTIME, &time_begin);
}

/*
	Get the time elapsed since startClock was called.
	Return the time in milliseconds.
*/
double endClock()
{
	// Get the end time.
	clock_gettime(CLOCK_REALTIME, &time_end);
	// Calculate the time differences.
	time_t sec = time_end.tv_sec - time_begin.tv_sec;
	long nsec = time_end.tv_nsec - time_begin.tv_nsec;

	// Add together and convert to milliseconds.
	return (double)sec * 1000.0 + (double)nsec / 1000000.0;
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

int main(int argc, char *argv[]) {
	// Generate random seeds
	srand((unsigned int)time(NULL));

	// Timers for time measurement
	double time_spent_writing = 0.0;
	double time_spent_reading = 0.0;
	double time_spent_sorting = 0.0;
	double time_spent_calc_avg = 0.0;
	double time_spent_calc_minmax = 0.0;

	int dataSetSize = 0;
	int bufferSize = 0;
	char* inputFileName = NULL;
	char* outputFileName = NULL;
	if (argc >= 4) {
		dataSetSize	= pow(2, atoi(argv[1]));	// Number elements to be tested on given in the exponent of 2
		bufferSize	= atoi(argv[2]);			// Buffer size in bytes
		inputFileName	= argv[3];				// File to read data from and also create data to
		outputFileName	= argv[4];				// File to write data to
	}
	else 
		return -1;

	printf("\nDataset Size: %d, Buffersize: %d, InputFN: %s, OutputFN: %s \n"
			, dataSetSize, bufferSize, inputFileName, outputFileName);

	createDataset(dataSetSize, inputFileName);

	int nrOfTests = 10;
	printf("#Iterations: %d\nProgress: 0%%", nrOfTests);
	for(int i = 0; i < nrOfTests; i++) {
		float* ds = malloc(sizeof(float)*dataSetSize);

		startClock();

		loadDataset(ds, 0, dataSetSize, inputFileName, (size_t)bufferSize);

		time_spent_reading += endClock();

		// compute the average value of the dataset, i.e. sum_of_dataset_values / num_of_dataset_values
		startClock();
		float avg = average(ds);
		time_spent_calc_avg += endClock();

		startClock();
		// find the max value in the dataset
		float max = maxvalue(ds);
		// find the min value in the dataset
		float min = minvalue(ds);
		time_spent_calc_minmax += endClock();

		//sort the dataset and copy it into the memory area pointed by sds
		startClock();
	
		selectionSort(ds, dataSetSize);

		time_spent_sorting += endClock();

		//write the sorted array into a new file plus the valies of the average, min and max as the first three records.
		startClock();
		writeDataset(0, ds, dataSetSize, outputFileName, (size_t)bufferSize, avg, min, max);
		time_spent_writing += endClock();

		free(ds);

		printf("\rProgress: %d%%", (int)(((i+1)/(float)nrOfTests)*100));
		fflush(stdout);
	}
	printf("\n");
	
	#define DATA_SIZE 7
	double data[DATA_SIZE] = {dataSetSize, bufferSize, 0, 0, 0, 0, 0};
	
	data[2] = (time_spent_reading) / (double)nrOfTests;
	data[3] = (time_spent_sorting) / (double)nrOfTests;
	data[4] = (time_spent_writing) / (double)nrOfTests;
	data[5] = (time_spent_calc_avg) / (double)nrOfTests;
	data[6] = (time_spent_calc_minmax) / (double)nrOfTests;

	printf("\nAverage Time\n");
	printf("Time Read: %lf ms\n", data[2]);
	printf("Time Sort: %lf ms\n", data[3]);
	printf("Time Write: %lf ms\n",data[4]);
	printf("Time Avg: %lf ms\n", data[5]);
	printf("Time Min/Max: %lf ms\n", data[6]);

	char path[] = "data.csv";
	char desc[] = "Data_Size,Buffer_size,Read,Sort,Write,Avg,Min/Max\n";
	
	FILE * f = fopen(path, "a");
	if (f) {
		char buf[30];
		for (int i = 0; i < DATA_SIZE; i++) {
			fprintf(f, "%f", data[i]);
			if(i < DATA_SIZE -1)
				fprintf(f, ",");
		}
		fprintf(f, "\n");
		printf("Wrote data to file %s\n", path);
	}
	

	return 0; 
}
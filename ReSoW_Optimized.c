// Input: DataSetSize, BufferSize, DatasetFilename, OutputFilename
// Output: the file OutputFilename containing the sorted dataset.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <string.h>

float generateRand(int rmax) {
	//Generate a floating point random number between 0 and rmax
	//initialize the random number generator
	return ( (float)rand()/(float)(RAND_MAX)) * rmax;
}

int cmpfunc(const void* a, const void* b) {
	float fa = *(float*)a;
	float fb = *(float*)b;
	return (fa > fb) - (fa < fb);	
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
		//if(i < datasetSize/2) v[i] = 1;
		//else v[i] = 2;
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

	//printf("\n\nLoading Data!\n");
	//printf("bytes to read %d, Block size %d, Count of blocks to read %d\n", (int)fileSize, (int)blockSize, (int)count);
	//printf("Position %d\n", (int)pos);

	
	int index = 0;
	for(int i = 0; i < (int)count; i++) {
		size_t t = fread(&v[index], blockSize, 1, fp);
		index += blockSize / sizeof(float);
	}


	// TIP you can define you own buffer, buffer size and you can read blocks of data of size > 1
	fclose(fp);
}

struct ThreadData {
	float* v;
	long int pos;
	char* path;
	int dataSetSize;
	size_t blockSize;
};

void* threadSort(void*data) {
	struct ThreadData* d = data; 
	//printf("Sorting on thread! ");
	qsort(d->v + d->pos, d->dataSetSize, sizeof(float), cmpfunc);
	//printf("----------> Done Sorting on thread!\n");
	free(data);
	return NULL;
}

void startThread(pthread_t* p, int index, int nThreads, float* v, int size, size_t blockSize) {
	int newSize = size / nThreads;
	struct ThreadData* data = malloc(sizeof(struct ThreadData));
	data->dataSetSize = newSize;
	data->v = v;
	data->pos = newSize*index;
	data->path = NULL;
	data->blockSize = blockSize;

	printf("Starting Thread %d, size: %d, newSize: %d, pos: %d\n", index, size, newSize, (int)data->pos);

	if(pthread_create(p, NULL, threadSort, data))
		fprintf(stderr, "Error creating thread %d!\n", index);
}

void* threadLoad(void* data) {
	struct ThreadData* d = data; 

	printf("Start threading!\n");
	loadDataset(d->v, d->pos, d->dataSetSize, d->path, d->blockSize);
	printf("End threading!\n");
	return NULL;
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
	
	int index = 0;
	for(int i=pos/sizeof(float); i<(int)count; i++){
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
	clock_gettime(CLOCK_MONOTONIC, &time_begin);
}

/*
	Get the time elapsed since startClock was called.
	Return the time in milliseconds.
*/
double endClock()
{
	// Get the end time.
	clock_gettime(CLOCK_MONOTONIC, &time_end);
	// Calculate the time differences.
	double sec = time_end.tv_sec - time_begin.tv_sec;
	double nsec = time_end.tv_nsec - time_begin.tv_nsec;
	// Add together and convert to milliseconds.
	return sec * 1000.0 + nsec / 1000000.0;
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
		dataSetSize	= atoi(argv[1]);	
		bufferSize	= atoi(argv[2]) * sizeof(float);	
		inputFileName	= argv[3];	
		outputFileName	= argv[4];	
	}
	else 
		return -1;

	printf("\nDataset Size: %d, Buffersize: %d, InputFN: %s, OutputFN: %s \n"
			, dataSetSize, bufferSize, inputFileName, outputFileName);

	createDataset(dataSetSize, inputFileName);

	int loopCount = 10;
	for(int i = 0; i < loopCount; i++) {
		float* ds = malloc(sizeof(float)*dataSetSize);

		startClock();

		// Thread implementation on load
		#if 0
			pthread_t p2;
			struct ThreadData data;
			data.path = inputFileName;
			data.blockSize = bufferSize;
			data.dataSetSize = (int)(dataSetSize * 0.5f);
			data.v = ds;
			data.pos = 0;
			if(pthread_create(&p2, NULL, threadLoad, &data)) {
				fprintf(stderr, "Error creating!\n");
				return 1;
			}

			printf("Start main thread!\n");
			loadDataset(ds, (int)(dataSetSize*0.5f)*sizeof(float), dataSetSize/2, inputFileName, bufferSize);
			printf("End main thread!\n");

			if(pthread_join(p2, NULL)) {
				fprintf(stderr, "Error join!\n");
				return 1;	
			}

		#else
			loadDataset(ds, 0, dataSetSize, inputFileName, (size_t)bufferSize);
		#endif

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
		#if 1
			pthread_t p1;
			startThread(&p1, 1, 4, ds, dataSetSize, bufferSize);
			pthread_t p2;
			startThread(&p2, 2, 4, ds, dataSetSize, bufferSize);
			pthread_t p3;
			startThread(&p3, 3, 4, ds, dataSetSize, bufferSize);			
			qsort(ds, (dataSetSize / 4), sizeof(float), cmpfunc);

			if(pthread_join(p1, NULL)) {
				fprintf(stderr, "Error join thread %d!\n", 1);
				return 1;	
			}
			if(pthread_join(p2, NULL)) {
				fprintf(stderr, "Error join thread %d!\n", 2);
				return 1;	
			}
			if(pthread_join(p3, NULL)) {
				fprintf(stderr, "Error join thread %d!\n", 3);
				return 1;	
			}

			// TODO: Merge!!
		#else
			qsort(ds, dataSetSize, sizeof(float), cmpfunc);
		#endif

		time_spent_sorting += endClock();

		//printf("\n\nSorted: ");
		//for (int i = 0; i < dataSetSize; i++)
		//	printf("%.1f, ", ds[i]);

		//write the sorted array into a new file plus the valies of the average, min and max as the first three records.
		startClock();
		writeDataset(0, ds, dataSetSize, outputFileName, (size_t)bufferSize, avg, min, max);
		time_spent_writing += endClock();

		free(ds);
	}
	
	#define DATA_SIZE 7
	float data[DATA_SIZE] = {dataSetSize, bufferSize, 0, 0, 0, 0, 0};
	
	data[2] = (time_spent_reading) / (float)loopCount;
	data[3] = (time_spent_sorting) / (float)loopCount;
	data[4] = (time_spent_writing) / (float)loopCount;
	data[5] = (time_spent_calc_avg) / (float)loopCount;
	data[6] = (time_spent_calc_minmax) / (float)loopCount;

	printf("\nAverage Time\n");
	printf("Time Read: %f ms\n", data[2]);
	printf("Time Sort: %f ms\n", data[3]);
	printf("Time Write: %f ms\n",data[4]);
	printf("Time Avg: %f ms\n", data[5]);
	printf("Time Min/Max: %f ms\n", data[6]);

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
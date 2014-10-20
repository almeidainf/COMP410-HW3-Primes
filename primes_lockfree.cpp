#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <queue>
#include <pthread.h>
#include <unistd.h>

#include "almeidamacros.h"

// Bounds
#define LOWER_BOUND 1000000 // 1M
#define UPPER_BOUND 50000000 // 50M

// Is it prime?
#define YES 1
#define NO 0

// For the queues and vector
using namespace std;

// Functions
void create_file(char *file_name);
void process_file(char *file_name);
int is_prime(int number);
void create_threads();
void join_threads();
void *counting_thread(void *id);
void *progress_thread(void *id);
void present_results();

// To acumulate the numbers that need to be checked
vector < queue<int> > numbers;

// Global variables for the threads and primes counting
int nthreads;
pthread_t *threads;
pthread_t prog_thread;
int *thread_ids;
int *nprimes;

// Here we go...
int main(int argc, char **argv){
	
	// How to use it
	if(argc < 3){
		verbose("Usage:\n\t./primes_lockfree --createDataFile [file name]\n\t./primes_lockfree --processDataFile [file name]");
		exit(EXIT_FAILURE);
	}

	// Obtaining input parameters and calling operation functions
	if(!strncmp(argv[1], "--createDataFile", 16))
		create_file(argv[2]);	// Create numbers file
	else if(!strncmp(argv[1], "--processDataFile", 17)){
		process_file(argv[2]);	// Read file and queue numbers
		create_threads();		// Create counting threads
		join_threads();			// Wait for threads to count everything
		present_results();		// Show total number of primes
	}
	else{ // You're not using it right
		error("Operation not identified.");
		verbose("Usage:\n\t./primes_lockfree --createDataFile [file name]\n\t./primes_lockfree --processDataFile [file name]");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

// Creating numbers file
void create_file(char *file_name){

	// Open numbers file for writing
	FILE *file = fopen(file_name, "w");
	if(file == NULL){
		perror("Error opening the file");
		exit(EXIT_FAILURE);
	}

	// Write every number between bounds
	for(int i = LOWER_BOUND; i < UPPER_BOUND; i++)
		fprintf(file, "%d\n", i);

	// Close file
	if(fclose(file) != 0)
		perror("Error closing the file");
}

// Processing numbers file
void process_file(char *file_name){
	
	int number, qindex;
	
	// Defining number os threads
	nthreads = sysconf( _SC_NPROCESSORS_ONLN ) * 2;

	// Create one queue for each thread	
	for(int i = 0; i < nthreads; i++)
		numbers.push_back(queue<int>());


	// Open numbers file for reading
	FILE *file = fopen(file_name, "r");
	if(file == NULL){
		perror("Error opening the file");
		exit(EXIT_FAILURE);
	}

	verbose("Reading numbers from file");

	// Distribute numbers among queues
	qindex = 0;
	while(fscanf(file, "%d\n", &number) != EOF){
		numbers[qindex].push(number);
		if(++qindex == nthreads) qindex = 0;
	}

	verbose("Finished reading file");
}

// Algorithm to verify if a number is prime
int is_prime(int number){
	
	if(number % 2 == 0)
		return NO;

	int divisor, last_check = sqrt(number);
	for(divisor = 3; divisor <= last_check; divisor += 2)
		if(number % divisor == 0)
			return NO;

	return YES;
}

// Creating the counting threads
void create_threads(){
	
	// Allocating arrays
	threads = talloc(pthread_t, nthreads);
	thread_ids = talloc(int, nthreads);
	nprimes = talloc(int, nthreads);
	memset(nprimes, 0, nthreads*sizeof(int));

	// Create all threads
	for(int i = 0; i < nthreads; i++){
		thread_ids[i] = i;
		pthread_create(&threads[i], NULL, counting_thread, (void*)&thread_ids[i]);
	}

	// Create progress monitor thread
	pthread_create(&prog_thread, NULL, progress_thread, NULL);

	verbose("%d counting threads, 1 I/O thread and 1 progress monitor thread running", nthreads);
}

// Wait for all threads to complete
void join_threads(){

	// Wait for counting threads to finish
	for(int i = 0; i < nthreads; i++)
		pthread_join(threads[i], NULL);

	// Cancel progress monitor thread
	pthread_cancel(prog_thread);
}

// Pop a number from the queue and process it
void *counting_thread(void *id){
	
	int thread_id = *(int *)id;

	while(!numbers[thread_id].empty()){
		if(is_prime(numbers[thread_id].front()))
			nprimes[thread_id]++;
		numbers[thread_id].pop();
	}
}

// Calculate and show total number of primes
void present_results(){

	int total = 0;
	for(int i = 0; i < nthreads; i++)
		total += nprimes[i];

	verbose("%d primes found.", total);
}

// Count number of primes and show it every 1 second
void *progress_thread(void *id){

	int primes;

	while(1){
		sleep(1);
		primes = 0;
		for(int i = 0; i < nthreads; i++)
			primes += nprimes[i];
		verbose("%d primes found so far", primes);
	}
}

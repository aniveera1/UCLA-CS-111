/* NAME: Anirudh Veeraragavan
 */

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

// Global Constants
const int SUCCESS_CODE = 0;
const int ERR_CODE = 1;
const int FAIL_CODE = 2;

struct thread_args {
	long long* counter;
	int iterations;
};

// Global Variables
pthread_mutex_t mutexsum;
char* sync_method;
int opt_yield;
int spin_lock = 0;

void add(long long *pointer, long long value)
{
	if (sync_method && *sync_method == 'm')
		pthread_mutex_lock(&mutexsum);

	if (sync_method && *sync_method == 's')
		while(__sync_lock_test_and_set(&spin_lock, 1));

	long long prev;
	long long sum;
	if (sync_method && *sync_method == 'c')
	{
		do
		{
			prev = *pointer;
			sum = prev + value;
			if (opt_yield)
				sched_yield();
		} while(__sync_val_compare_and_swap(pointer, prev, sum) != prev);
		return;
	}

	prev = *pointer;
	sum = *pointer + value;

	if (opt_yield)
		sched_yield();

	*pointer = sum;

	if (sync_method && *sync_method == 's')
		__sync_lock_release(&spin_lock);

	if (sync_method && *sync_method == 'm')
		pthread_mutex_unlock(&mutexsum);
}

// INPUT: Name of sys call that threw error
// Prints reason for error and terminates program
void process_failed_sys_call(const char syscall[])
{
	int err = errno;
	fprintf(stderr, "%s", "An error has occurred.\n");
	fprintf(stderr, "The system call '%s' failed with error code %d\n", syscall, err);
	fprintf(stderr, "This error code means: %s\n", strerror(err));
	exit(ERR_CODE);
}

// INPUT: Info about CL arguments, strings for argument parameters
// Process CL arguments while checking for invalid options
void process_cl_arugments(int argc, char** argv,
						  char** threads, char** iterations, char** sync)
{
	struct option long_options[] =
	{
		{"threads", required_argument, NULL, 't'},
		{"iterations", required_argument, NULL, 'i'},
		{"yield", no_argument, NULL, 'y'},
		{"sync", required_argument, NULL, 's'},
		{0, 0, 0, 0}
	};
	int option_index = 0;

	while (1)
	{
		int arg = getopt_long(argc, argv, "yt:i:s:",
							  long_options, &option_index);

		if (arg == -1)
			return;

		switch(arg)
		{
			case 't':
				*threads = optarg;
				break;
			case 'i':
				*iterations = optarg;
				break;
			case 'y':
				opt_yield = 1;
				break;
			case 's':
				*sync = optarg;
				break;
			case '?':
				fprintf(stderr, "%s\n", "ERROR: Invalid argument.");
				fprintf(stderr, "%s\n", "Usage: lab2a_add [--threads=#] [--iterations=#] [--yield] [--sync=[smc]]");
				exit(ERR_CODE);
		}
	}
}

// INPUT: Information about test that was just run
// Print to STDOUT a CSV record about test
void print_results(char test_name[], int threads, int iterations,
				   long starting_time, long ending_time, long long counter)
{
	long total_operations = threads * iterations * 2;
	long total_run_time = ending_time - starting_time;
	long avg_time_per_operation = total_run_time / total_operations;

	// name of test, # threads, # iterations, # operations, run time, time / operation, counter
	printf("%s%c", test_name, ',');
	printf("%d%c%d%c", threads, ',', iterations, ',');
	printf("%ld%c%ld%c", total_operations, ',', total_run_time, ',');
	printf("%ld%c%lld\n", avg_time_per_operation, ',', counter);
}

// INPUT: Struct containing arguments
// Wrapper function for use in multi-threading
void* add_wrapper(void *args)
{
	struct thread_args *my_args;
	my_args = (struct thread_args *) args;

	int iter = my_args->iterations;

	int i;
	for (i = 0; i < iter; i++)
	{
		add(my_args->counter, 1);
	}

	for (i = 0; i < iter; i++)
	{
		add(my_args->counter, -1);
	}
	pthread_exit(NULL);
}

int main(int argc, char** argv)
{
	// Default both values to 1
	char def_value[2] = {'1'};
	char* str_threads = def_value;
	char* str_iterations = def_value;
	opt_yield = 0;
	sync_method = NULL;
	process_cl_arugments(argc, argv, &str_threads, &str_iterations, &sync_method);

	long long counter = 0;
	int num_threads = atoi(str_threads);
	int num_iterations = atoi(str_iterations);

	// Initialize any synchronization methods
	if (sync_method)
		pthread_mutex_init(&mutexsum, NULL);

	struct timespec starting;
	if (clock_gettime(CLOCK_REALTIME, &starting) == -1)
	{
		process_failed_sys_call("clock_gettime");
	}

	pthread_t* threads = (pthread_t*) malloc(sizeof(pthread_t) * num_threads);

	struct thread_args args;
	args.counter = &counter;
	args.iterations = num_iterations;

	long i;
	for (i = 0; i < num_threads; i++)
	{
		if (pthread_create(&threads[i], NULL, add_wrapper, (void *)&args) != 0)
		{
			process_failed_sys_call("pthread_create");
		}
	}

	void* status;
	for (i = 0; i < num_threads; i++)
	{
		if (pthread_join(threads[i], &status) != 0)
		{
			process_failed_sys_call("pthread_join");
		}
	}

	free(threads);

	struct timespec ending;
	if (clock_gettime(CLOCK_REALTIME, &ending) == -1)
	{
		process_failed_sys_call("clock_gettime");
	}

	if (opt_yield)
	{
		if (sync_method)
		{
			char output_str[12] = {"add-yield-"};
			output_str[10] = *sync_method;

			print_results(output_str, num_threads, num_iterations,
				  	  	  starting.tv_nsec, ending.tv_nsec, counter);
		}
		else
		{
			print_results("add-yield-none", num_threads, num_iterations,
				  	  	  starting.tv_nsec, ending.tv_nsec, counter);
		}
	}
	else
	{
		if (sync_method)
		{
			char output_str[6] = {"add-"};
			output_str[4] = *sync_method;

			print_results(output_str, num_threads, num_iterations,
				  	  	  starting.tv_nsec, ending.tv_nsec, counter);

		}
		else
		{
			print_results("add-none", num_threads, num_iterations,
				  	  	  starting.tv_nsec, ending.tv_nsec, counter);
		}
	}

	pthread_exit(NULL);
	exit(SUCCESS_CODE);
}


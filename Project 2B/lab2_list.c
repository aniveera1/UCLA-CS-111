/* NAME: Anirudh Veeraragavan
 */

#include "SortedList.h"
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

// Global Constants
long SUCCESS_CODE = 0;
long ERR_CODE = 1;
long FAIL_CODE = 2;

struct thread_args {
	int thread_id;
	int iterations;
	long long num_elements;
	long long my_wait;
	SortedListElement_t** l_elements;
};

// Global Variables
int opt_yield;
SortedList_t** head;
char* sync_method = NULL;
pthread_mutex_t* mutex = NULL;
int* lock = NULL;
int num_lists = 1;

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

void signal_handler(int num)
{
	if (num == SIGSEGV)
	{
		fprintf(stderr, "%s\n", "ERROR: A segmentation fault has occurred.");
		exit(FAIL_CODE);
	}
}

// INPUT: Info about CL arguments, strings for argument parameters
// Process CL arguments while checking for invalid options
void process_cl_arugments(int argc, char** argv,
						  char** threads, char** iterations, char** str_yield)
{
	struct option long_options[] =
	{
		{"threads", required_argument, NULL, 't'},
		{"iterations", required_argument, NULL, 'i'},
		{"yield", required_argument, NULL, 'y'},
		{"sync", required_argument, NULL, 's'},
		{"lists", required_argument, NULL, 'l'},
		{0, 0, 0, 0}
	};
	int option_index = 0;

	while (1)
	{
		int arg = getopt_long(argc, argv, "y:t:i:s:",
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
				*str_yield = optarg;
				opt_yield = 1;
				break;
			case 's':
				sync_method = optarg;
				break;
			case 'l':
				num_lists = atoi(optarg);
				break;
			case '?':
				fprintf(stderr, "%s\n", "ERROR: Invalid argument.");
				fprintf(stderr, "%s\n", "Usage: lab2a_add [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=[ms]] [--lists=#]");
				exit(ERR_CODE);
		}
	}
}

// Generate a random 20 char alphanumeric key
char* generate_random_key()
{
	int key_len = 20;
	char* key = (char *) malloc(sizeof(char) * key_len);
	if (key == NULL)
	{
		process_failed_sys_call("malloc");
	}
	char alphanum[] = {"0123456789abcdefghijklmnopqrstuvwxyz"};

	int i;
	for (i = 0; i < key_len; i++)
	{
		key[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	key[key_len] = 0;

	return key;
}

// INPUT: Number of list nodes to allocate
// Return a pointer to a list of pointers to list nodes
SortedListElement_t** generate_list_nodes(long long count)
{
	SortedListElement_t** list_elements = (SortedListElement_t **) malloc(sizeof(SortedList_t*) * count);
	if (list_elements == NULL)
	{
		process_failed_sys_call("malloc");
	}

	long long i;
	for (i = 0; i < count; i++)
	{
		list_elements[i] = (SortedListElement_t *) malloc(sizeof(SortedListElement_t));
		if (list_elements[i] == NULL)
		{
			process_failed_sys_call("malloc");
		}

		list_elements[i]->key = generate_random_key();
		list_elements[i]->prev = NULL;
		list_elements[i]->next = NULL;
	}
	return list_elements;
}

long get_time_dif(struct timespec starting, struct timespec ending)
{
	long run_time = ending.tv_nsec - starting.tv_nsec;
	if (run_time < 0)
	{
		run_time += 1000000000;
	}
	return run_time;
}

void* update_list(void* args)
{
	struct thread_args* my_args;
	my_args = (struct thread_args *) args;

	int my_num = my_args->thread_id;
	SortedListElement_t** list_elements = my_args->l_elements;

	int iter = my_args->iterations;
	long long count = my_args->num_elements;
	long long threads = count / iter;

	struct key_pair {
		const char* key;
		int head_location;
	};

	struct key_pair* keys = (struct key_pair *) malloc(sizeof(struct key_pair) * iter);
	if (keys == NULL)
	{
		process_failed_sys_call("malloc");
	}

	// printf("%s\n", "INSERTING");
	long long i;
	int j;
	for (i = my_num, j = 0; i < count; i += threads, j++)
	{
		int head_num = my_num % num_lists;

		struct timespec starting;
		if (clock_gettime(CLOCK_REALTIME, &starting) == -1)
		{
			process_failed_sys_call("clock_gettime");
		}

		if (sync_method && *sync_method == 'm')
			pthread_mutex_lock(&mutex[head_num]);

		if (sync_method && *sync_method == 's')
			while(__sync_lock_test_and_set(&lock[head_num], 1));

		struct timespec ending;
		if (clock_gettime(CLOCK_REALTIME, &ending) == -1)
		{
			process_failed_sys_call("clock_gettime");
		}

		my_args->my_wait += get_time_dif(starting, ending);

		SortedList_insert(head[head_num], list_elements[i]);

		if (sync_method && *sync_method == 'm')
			pthread_mutex_unlock(&mutex[head_num]);

		if (sync_method && *sync_method == 's')
			__sync_lock_release(&lock[head_num]);

		keys[j].key = list_elements[i]->key;
		keys[j].head_location = head_num;
	}

	// printf("%s\n", "LENGTH");
	int len = 0;
	for (i = 0; i < num_lists; i++)
	{
		len += SortedList_length(head[i]);
	}

	if (len == 0)
	{
		fprintf(stderr, "%s\n", "ERROR: Nodes were not correctly inserted into list.");
		fprintf(stderr, "There should be %d nodes but there are actually %d\n", iter, len);
		pthread_exit((void *)FAIL_CODE);
	}

	// printf("%s\n", "LOOKUP/DELETE");
	for (j = 0; j < iter; j++)
	{
		int head_num = keys[j].head_location;

		struct timespec starting;
		if (clock_gettime(CLOCK_REALTIME, &starting) == -1)
		{
			process_failed_sys_call("clock_gettime");
		}

		if (sync_method && *sync_method == 'm')
			pthread_mutex_lock(&mutex[head_num]);

		if (sync_method && *sync_method == 's')
			while(__sync_lock_test_and_set(&lock[head_num], 1));

		struct timespec ending;
		if (clock_gettime(CLOCK_REALTIME, &ending) == -1)
		{
			process_failed_sys_call("clock_gettime");
		}

		my_args->my_wait += get_time_dif(starting, ending);

		SortedListElement_t* elem = SortedList_lookup(head[head_num], keys[j].key);

		if (sync_method && *sync_method == 'm')
			pthread_mutex_unlock(&mutex[head_num]);

		if (sync_method && *sync_method == 's')
			__sync_lock_release(&lock[head_num]);

		if (elem == NULL)
		{
			fprintf(stderr, "%s\n", "ERROR: Node lookup failed.");
			fprintf(stderr, "There should be a node with key %s but none was found.\n", keys[j].key);
			pthread_exit((void *)FAIL_CODE);
		}

		if (clock_gettime(CLOCK_REALTIME, &starting) == -1)
		{
			process_failed_sys_call("clock_gettime");
		}

		if (sync_method && *sync_method == 'm')
			pthread_mutex_lock(&mutex[head_num]);

		if (sync_method && *sync_method == 's')
			while(__sync_lock_test_and_set(&lock[head_num], 1));

		if (clock_gettime(CLOCK_REALTIME, &ending) == -1)
		{
			process_failed_sys_call("clock_gettime");
		}

		my_args->my_wait += get_time_dif(starting, ending);

		int err = SortedList_delete(elem);

		if (sync_method && *sync_method == 'm')
			pthread_mutex_unlock(&mutex[head_num]);

		if (sync_method && *sync_method == 's')
			__sync_lock_release(&lock[head_num]);

		if (err == 1)
		{
			fprintf(stderr, "%s\n", "ERROR: Node deletion failed.");
			fprintf(stderr, "The node with key %s had corrupted next/prev pointers.", elem->key);
			pthread_exit((void *)FAIL_CODE);
		}
	}

	free(keys);

	pthread_exit(NULL);
}

int main(int argc, char** argv)
{
	// printf("%s\n", "START");
	// Default threads/iterations to 1
	char def_value[2] = {'1'};
	char* str_threads = def_value;
	char* str_iterations = def_value;
	char* str_yield = NULL;
	opt_yield = 0;
	process_cl_arugments(argc, argv, &str_threads, &str_iterations, &str_yield);

	int num_threads = atoi(str_threads);
	int num_iterations = atoi(str_iterations);

	// Split up list
	// printf("%s\n", "HEAD");
	head = (SortedList_t **) malloc(sizeof(SortedList_t*) * num_lists);
	if (head == NULL)
	{
		process_failed_sys_call("malloc");
	}

	int j;
	for (j = 0; j < num_lists; j++)
	{
		head[j] = (SortedList_t *) malloc(sizeof(SortedList_t));
		head[j]->key = NULL;
		head[j]->next = NULL;
		head[j]->prev = NULL;
	}

	// Initialize a 2D array of list nodes
	long long count = num_iterations * num_threads;
	SortedListElement_t** list_elements = generate_list_nodes(count);

	// Register handler
	if (signal(SIGSEGV, signal_handler) == SIG_ERR)
	{
		process_failed_sys_call("signal");
	}

	if (sync_method)
	{
		mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * num_lists);
		int i;
		for (i = 0; i < num_lists; i++)
		{
			pthread_mutex_init(&mutex[i], NULL);
		}

		lock = (int *) malloc(sizeof(int) * num_lists);
		for (i = 0; i < num_lists; i++)
		{
			lock[i] = 0;
		}
	}

	struct timespec starting;
	if (clock_gettime(CLOCK_REALTIME, &starting) == -1)
	{
		process_failed_sys_call("clock_gettime");
	}

	pthread_t* threads = (pthread_t*) malloc(sizeof(pthread_t) * num_threads);
	if (threads == NULL)
	{
		process_failed_sys_call("malloc");
	}

	long long lock_waiting_time = 0;

	struct thread_args** t_args = (struct thread_args **) malloc(sizeof(struct thread_args *) * num_threads);
	long long i;
	for (i = 0; i < num_threads; i++)
	{
		t_args[i] = (struct thread_args *) malloc(sizeof(struct thread_args));
	}

	for (i = 0; i < num_threads; i++)
	{
		t_args[i]->l_elements = list_elements;
		t_args[i]->iterations = num_iterations;
		t_args[i]->num_elements = count;
		t_args[i]->thread_id = i;
		t_args[i]->my_wait = 0;

		if (pthread_create(&threads[i], NULL, update_list, (void *)t_args[i]) != 0)
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
		if ((long)status == 2)
		{
			exit(FAIL_CODE);
		}
	}

	free(threads);

	for (i = 0; i < num_threads; i++)
	{
		lock_waiting_time += t_args[i]->my_wait;
	}

	for (i = num_threads - 1; i >= 0; i--)
	{
		free(t_args[i]);
	}
	free(t_args);

	// printf("%s\n", "ALL LENGTH");
	int len = 0;
	for (i = 0; i < num_lists; i++)
	{
		len += SortedList_length(head[i]);
	}

	if (len != 0)
	{
		fprintf(stderr, "%s\n", "ERROR: The length of the list after the test is not 0.");
		exit(FAIL_CODE);
	}

	struct timespec ending;
	if (clock_gettime(CLOCK_REALTIME, &ending) == -1)
	{
		process_failed_sys_call("clock_gettime");
	}

	printf("%s", "list-");
	if (str_yield == NULL)
		printf("%s", "none-");
	else
		printf("%s-", str_yield);
	if (sync_method == NULL)
		printf("%s", "none,");
	else
		printf("%s,", sync_method);
	printf("%d%s", num_threads, ",");
	printf("%d%s", num_iterations, ",");
	printf("%d%s", num_lists, ",");

	long long num_operations = num_iterations * num_threads * 3;

	printf("%lld%s", num_operations, ",");

	long run_time = ending.tv_nsec - starting.tv_nsec;
	if (run_time < 0)
	{
		run_time += 1000000000;
	}

	printf("%ld%s", run_time, ",");

	long avg_time_per_operation = run_time / num_operations;

	printf("%ld,", avg_time_per_operation);

	printf("%lld\n", lock_waiting_time / num_operations);

	for (i = count - 1; i >= 0; --i)
	{
		free((char *)list_elements[i]->key);
		free(list_elements[i]);
	}
	free(list_elements);
	for (i = num_lists - 1; i >= 0; --i)
	{
		free(head[i]);
	}
	free(head);
	free(mutex);
	exit(SUCCESS_CODE);
}

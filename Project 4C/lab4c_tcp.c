/* NAME: Anirudh Veeraragavan
 * EMAIL: aveeraragavan@g.ucla.edu
 * ID: 004767663
 */

#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <aio.h>
#include <math.h>
#include <poll.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netdb.h>

// Global Constants
long SUCCESS_CODE = 0;
long ERR_CODE = 1;
long FAIL_CODE = 2;

// Global Variables
int socket_fd;

#ifdef DUMMY
// Mock implementations for local dev/test
typedef int mraa_aio_context;
int mraa_aio_init(int input)
{
	input++;
	return 100;
}
int mraa_aio_read(int temp)
{
	temp++;
	return 500;
}

typedef int mraa_gpio_context;
int MRAA_GPIO_IN = 5;
int mraa_gpio_init(int val)
{
	val++;
	return -5;
}
void mraa_gpio_dir(int val, int temp)
{
	val++;
	temp++;
}
int mraa_gpio_read(int* val)
{
	(*val)++;
	return *val;
}
#endif

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
void process_cl_arguments(int argc, char** argv,
						  char** period, char** temp, char** logfile, char** id, char** host, char** port)
{
	int i;
	for (i = 1; i < argc; i++)
	{
		int size = strlen(argv[i]);
		int j;
		for (j = 0; j < size; j++)
		{
			if (!isdigit(argv[i][j]))
			{
				break;
			}
		}
		if (j == size)
		{
			*port = argv[i];
			break;
		}
	}

	struct option long_options[] =
	{
		{"period", required_argument, NULL, 'p'},
		{"scale", required_argument, NULL, 's'},
		{"log", required_argument, NULL, 'l'},
		{"id", required_argument, NULL, 'i'},
		{"host", required_argument, NULL, 'h'},
		{0, 0, 0, 0}
	};
	int option_index = 0;

	*period = "1";
	*temp = "F";

	while (1)
	{
		int arg = getopt_long(argc, argv, "p:s:l:i:h:",
							  long_options, &option_index);

		if (arg == -1)
			return;

		switch (arg)
		{
			case 'p':
				*period = optarg;
				break;
			case 's':
				*temp = optarg;
				break;
			case 'l':
				*logfile = optarg;
				break;
			case 'i':
				*id = optarg;
				break;
			case 'h':
				*host = optarg;
				break;
			case '?':
				fprintf(stderr, "%s\n", "ERROR: Invalid argument.");
				fprintf(stderr, "%s\n", "Usage: lab4c_tcp [--period=#] [--scale=[F,C]] [--log=file] [--id=#] [--host=name] [port #]");
				exit(ERR_CODE);
		}
	}
}

// INPUT: Buffer to write time to
// Get hour:min:sec of current local time
void get_curr_time(char* temp_report)
{
	time_t rawtime;
	time(&rawtime);
	if (rawtime == (time_t)-1)
	{
		process_failed_sys_call("time");
	}

	struct tm* local_time = localtime(&rawtime);
	if (local_time == NULL)
	{
		process_failed_sys_call("localtime");
	}

	//01:34:67
	snprintf(temp_report, 4, "%d:", local_time->tm_hour);
	if (local_time->tm_hour < 10) {
		temp_report[0] = '0';
		snprintf(temp_report+1, 3, "%d:", local_time->tm_hour);
	}

	snprintf(temp_report+3, 4, "%d:", local_time->tm_min);
	if (local_time->tm_min < 10) {
		temp_report[3] = '0';
		snprintf(temp_report+4, 3, "%d:", local_time->tm_min);
	}

	snprintf(temp_report+6, 3, "%d", local_time->tm_sec);
	if (local_time->tm_sec < 10) {
		temp_report[6] = '0';
		snprintf(temp_report+7, 2, "%d", local_time->tm_sec);
	}
}

// INPUT: Analog reading from temperature sensor
// Uses algorithm to convert analog reading to temperature
float convert_analog_to_temp(int analog, char* temp_unit)
{
	float raw_value = (1023.0 / analog - 1.0) * 100000;
	float temperature = 1.0 / (log(raw_value / 100000) / 4275 + 1 / 298.15) - 273.15;

	if (*temp_unit == 'F')
	{
		return ((temperature * 9) / 5) + 32;
	}
	return temperature;
}

// INPUT: Temperature pin, temperature unit, logfile
// Create temperature file according to parameters and report it
void generate_temp_report(int temp_pin, char* scale, char* log_file)
{
	// Sample temperature sensor and convert reading to indicated temperature
	int temp_analog_value = mraa_aio_read(temp_pin);
	if (temp_analog_value == -1)
	{
		fprintf(stderr, "%s\n", "ERROR: mraa_aio_read");
		fprintf(stderr, "%s\n", "There was a problem reading from the temperature sensor.");
		exit(ERR_CODE);
	}
	float temperature = convert_analog_to_temp(temp_analog_value, scale);

	char temp_report[14];
	get_curr_time(temp_report);
	temp_report[8] = ' ';

	snprintf(temp_report+9, 5, "%0.1f", temperature);
	temp_report[13] = '\0';

	//dprintf(0, "%s\n", temp_report);
	dprintf(socket_fd, "%s\n", temp_report);

	// Append report to logfile
	if (log_file)
	{
		FILE* fd = fopen(log_file, "a+");
		if (fd == NULL)
		{
			process_failed_sys_call("fopen");
		}

		fprintf(fd, "%s\n", temp_report);
		fclose(fd);
	}
}

// INPUT: Command, temperature unit, delay time, start/stop flag, logfile
// Process command and modify parameters if necessary
void process_command(char* command, char** scale, int* delay, int* report, const char* log_file)
{
	if (strstr(command, "SCALE") != NULL)
	{
		if (strstr(command, "F") != NULL)
		{
			char* temp_scale = (char *) malloc(sizeof(char) * 2);
			temp_scale[0] = 'F';
			*scale = temp_scale;
		}
		else
		{
			char* temp_scale = (char *) malloc(sizeof(char) * 2);
			temp_scale[0] = 'C';
			*scale = temp_scale;
		}
	}
	else if (strstr(command, "PERIOD") != NULL)
	{
		int i = 0;
		while (command[i] != '=') // TODO: Breaks on multidigit period
		{
			i++;
		}
		*delay = atoi(command + i + 1);
	}
	else if (strstr(command, "STOP") != NULL)
	{
		*report = 0;
	}
	else if (strstr(command, "START") != NULL)
	{
		*report = 1;
	}
	else if (strstr(command, "OFF") != NULL)
	{
		char curr_time[18];
		get_curr_time(curr_time);
		curr_time[8] = ' ';
		curr_time[9] = 'S';
		curr_time[10] = 'H';
		curr_time[11] = 'U';
		curr_time[12] = 'T';
		curr_time[13] = 'D';
		curr_time[14] = 'O';
		curr_time[15] = 'W';
		curr_time[16] = 'N';
		curr_time[17] = '\0';

		char newline[2];
		newline[0] = '\n';
		newline[1] = '\0';

		// dprintf(socket_fd, "%s SHUTDOWN\n\n", curr_time);
		// write(0, curr_time, 18);
		// write(0, newline, 2);
		write(socket_fd, curr_time, 18);
		write(socket_fd, newline, 2);

		if (log_file)
		{
			FILE* fd = fopen(log_file, "a+");
			if (fd == NULL)
			{
				process_failed_sys_call("fopen");
			}

			fprintf(fd, "%s", curr_time);
			fclose(fd);
		}
		exit(SUCCESS_CODE);
	}
	else if (strstr(command, "LOG") != NULL)
	{
		if (log_file)
		{
			FILE* fd = fopen(log_file, "a+");
			if (fd == NULL)
			{
				process_failed_sys_call("fopen");
			}

			fprintf(fd, "%s\n", command + 4);
			fclose(fd);
		}
	}
}

// INPUT: Host name, port number
// Configure client, connect to server, return socker fd
int connect_to_server(const char* host, const char* port)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		process_failed_sys_call("socket");
	}

	struct hostent *server = gethostbyname(host);
	if (server == NULL)
	{
		process_failed_sys_call("gethostbyname");
	}

	struct sockaddr_in serv_addr;
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *)&serv_addr.sin_addr.s_addr,
		   (char *)server->h_addr,
		   server->h_length);
	serv_addr.sin_port = htons(atoi(port));

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		process_failed_sys_call("connect");
	}

	return sockfd;
}

int main(int argc, char** argv)
{
	char* period = NULL;
	char* scale = NULL;
	char* log_file = NULL;
	char* id = NULL;
	char* host = NULL;
	char* port = NULL;

	process_cl_arguments(argc, argv, &period, &scale, &log_file, &id, &host, &port);

	if (!id || !host || !log_file || !port)
	{
		fprintf(stderr, "%s\n", "ERROR: Missing mandatory parameter.");
		fprintf(stderr, "%s\n", "Usage: lab4c_tcp [--period=#] [--scale=[F,C]] [--log=file] [--id=#] [--host=name] [port #]");
		exit(ERR_CODE);
	}

	// Open TCP connection
	socket_fd = connect_to_server(host, port);

	// Send/log ID
	dprintf(socket_fd, "ID=%s\n", id);

	FILE* fd = fopen(log_file, "a+");
	if (fd == NULL)
	{
		process_failed_sys_call("fopen");
	}
	fprintf(fd, "ID=%s\n", id);
	fclose(fd);

	// Connect to temperature sensor
	mraa_aio_context temp_pin = mraa_aio_init(1);
	if (!temp_pin)
	{
		fprintf(stderr, "%s\n", "ERROR: mraa_aio_init");
		fprintf(stderr, "%s\n", "There was a problem with initializing the temperature sensor.");
		exit(ERR_CODE);
	}

	// Set up poll
	struct pollfd fds[1];
	fds[0].fd = socket_fd;
	fds[0].events = POLLIN;
	fds[0].revents = 0;

	// Initial parameters
	int delay = atoi(period);
	int report_enabled = 1;

	char buf[2014];
	memset((char *) &buf, 0, sizeof(buf));
	while (1)
	{
		if (poll(fds, 1, 0) < 0)
		{
			process_failed_sys_call("poll");
		}

		// Produce one temperature report before processing commands
		if (report_enabled)
		{
			generate_temp_report(temp_pin, scale, log_file);
		}

		if (fds[0].revents & POLLIN)
		{
			int bytes_read = read(socket_fd, buf, sizeof(buf));
			if (bytes_read < 0)
			{
				process_failed_sys_call("read");
			}

			// Read may have gotten multiple commands therefore parse all out
			char* command = strtok(buf, "\n");
			while (command != NULL && bytes_read > 0)
			{
				if (log_file)
				{
					FILE* fd = fopen(log_file, "a+");
					if (fd == NULL)
					{
						process_failed_sys_call("fopen");
					}

					fprintf(fd, "%s\n", command);
					fclose(fd);
				}

				process_command(command, &scale, &delay, &report_enabled, log_file);
				
				command = strtok(NULL, "\n");
			}
		}
		// Sampling interval
		sleep(delay);
	}
}



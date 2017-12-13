/*
 * NAME: Anirudh Veeraragavan
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <termios.h>
#include <poll.h>
#include <mcrypt.h>
#include <fcntl.h>

// Global Constants
int ERR_CODE = 1;
int SUCCESS_CODE = 0;
int EOF_CODE = 4;
int CR_CODE = 13;
int LF_CODE = 10;

// Global Variables
struct termios old_term_settings;
int log_file = -1;
int key_size = -1;
MCRYPT crypt_fd;
MCRYPT decrypt_fd;

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

// INPUT: n/a
// Restores normal term env once program exits
void restore_term_env()
{
	if (tcsetattr(0, TCSANOW, &old_term_settings) == -1)
	{
		process_failed_sys_call("tcsetattr");
	}
}

// INPUT: Current term settings
// Edit settings to character at a time, no echo term
void apply_new_term_settings(struct termios settings)
{
	settings.c_iflag = ISTRIP;
	settings.c_oflag = 0;
	settings.c_lflag = 0;

	if (tcsetattr(0, TCSANOW, &settings) == -1)
	{
		process_failed_sys_call("tcsetattr");
	}
}

// INPUT: Name of file that contains key
// Open file, read key, update key length, return key
char* extract_key(const char key_file[])
{
	int key_fd = open(key_file, O_RDONLY);
	struct stat key_stat;
	if (fstat(key_fd, &key_stat) < 0)
	{
		process_failed_sys_call("fstat");
	}

	char* key = (char *)malloc(key_stat.st_size * sizeof(char));
	if (key == NULL)
	{
		process_failed_sys_call("malloc");
	}

	if (read(key_fd, key, key_stat.st_size) < 0)
	{
		process_failed_sys_call("read");
	}

	key_size = key_stat.st_size;
	return key;
}

// INPUT: Info about CLI arguments, strings for argument parameters
// Process CLI arguments while checking for invalid & setting options
int process_cli_arguments(int argc, char** argv,
						  char** port, char** log, char** encrypt)
{
	struct option long_options[] =
	{
		{"port", required_argument, NULL, 'p'},
		{"log", required_argument, NULL, 'l'},
		{"encrypt", required_argument, NULL, 'e'},
		{0, 0, 0, 0}
	};
	int option_index = 0;

	int flag = -1;
	while (1)
	{
		int arg = getopt_long(argc, argv, "p:l:e:",
							  long_options, &option_index);

		if (arg == -1)
			return flag;

		switch(arg)
		{
			case 'p':
				flag = 1;
				*port = optarg;
				break;
			case 'l':
				*log = optarg;
				log_file = creat(optarg, S_IRWXU);
				break;
			case 'e':
				*encrypt = extract_key(optarg);
				break;
			case '?':
				fprintf(stderr, "%s\n", "ERROR: Invalid argument.");
				fprintf(stderr, "%s\n", "Usage: lab1b-client [--port=port#] [--log=filename] [--encrypt=filename]");
				exit(ERR_CODE);
		}
	}
}

// INPUT: Port number
// Configure client settings and connect to server, return socket fd
int get_server_connection(const char port[])
{
	// Use IP, continuous stream, and TCP
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		process_failed_sys_call("socket");
	}

	// Get info about host
	struct hostent *server = gethostbyname("localhost");
	if (server == NULL)
	{
		process_failed_sys_call("gethostbyname");
	}

	// Configure IP address settings
	struct sockaddr_in serv_addr;
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *)server->h_addr, 
		   (char *)&serv_addr.sin_addr.s_addr,
		   server->h_length);
	serv_addr.sin_port = htons(atoi(port));

	// Establish connection to server
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		process_failed_sys_call("connect");
	}

	return sockfd;
}

// INPUT: Fd to write to
// Map newline/carriage return if printing to terminal
void convert_new_line(int writefd)
{
	if (writefd == 1)
	{
		char line_end[] = {'\r', '\n'};
		write(writefd, line_end, sizeof(char) * 2);
	}
}

// INPUT: Message, message type, sizes for both
// Log traffic to/from server to file
void write_to_log_file(const char buf[], const char type[], 
					   int bytes, int type_size)
{
	char log_string[8] = " bytes: ";

	char num[100];
	int len = sprintf(num, "%d", bytes);
	if (len < 0)
	{
		process_failed_sys_call("sprintf");
	}

	write(log_file, type, type_size);
	write(log_file, num, len);
	write(log_file, log_string, 8);

	write(log_file, buf, bytes);

	const char newline = '\n';
	write(log_file, &newline, 1);
}


// INPUT: Read from, write to, quantity
// Read specified bytes and write while encrypting/decrypting based on fd
int process_input(int readfd, int writefd, int bytes)
{
	char buf[256];
	memset((char *) &buf, 0, sizeof(buf));
	int bytes_read = read(readfd, buf, bytes);
	if (bytes_read < 0)
	{
		process_failed_sys_call("read");
	}

	if (bytes_read == 0)
	{
		return -1;
	}

	if (log_file != -1 && readfd != 0)
	{
		char log_string[9] = "RECEIVED ";
		write_to_log_file(buf, log_string, bytes_read, sizeof(log_string));
	}

	int i;
	for (i = 0; i < bytes_read; ++i)
	{
		if (writefd == 1 && key_size != -1)
		{
			if (mdecrypt_generic(decrypt_fd, &buf[i], sizeof(char)) != 0)
			{
				process_failed_sys_call("mdecrypt_generic");
			}
		}  
		/* Map <cr> or <lf> into <cr><lf> */
		if (((int)buf[i] == CR_CODE || (int)buf[i] == LF_CODE) && writefd == 1)
		{
			convert_new_line(writefd);
			continue;
		}
		if (writefd != 1 && key_size != -1)
		{
			if (mcrypt_generic(crypt_fd, &buf[i], sizeof(char)) != 0)
			{
				process_failed_sys_call("mcrypt_generic");
			}
		}
		if (log_file != -1 && writefd != 1)
		{
			char log_string[5] = "SENT ";
			write_to_log_file(&buf[i], log_string, 1, sizeof(log_string));
		}
		 

		write(writefd, &buf[i], sizeof(char));
	}

	return 0;
}

// INPUT: Encryption key
// Set up encryption and decryption fd for future use
void encryption_decryption_init(char* key)
{
	crypt_fd = mcrypt_module_open("rijndael-128", NULL, "cfb", NULL);
	if (crypt_fd == MCRYPT_FAILED)
	{
		process_failed_sys_call("mcrypt_module_open");
	}
	if (mcrypt_generic_init(crypt_fd, key, key_size, "xxxxxxxxxx") < 0)
	{
		process_failed_sys_call("mcrypt_generic_init");
	}

	decrypt_fd = mcrypt_module_open("rijndael-128", NULL, "cfb", NULL);
	if (decrypt_fd == MCRYPT_FAILED)
	{
		process_failed_sys_call("mcrypt_module_open");
	}
	if (mcrypt_generic_init(decrypt_fd, key, key_size, "xxxxxxxxxx") < 0)
	{
		process_failed_sys_call("mcrypt_generic_init");
	}
}

// INPUT: n/a
// Close encryption and decryption
void encryption_decryption_deinit()
{
	mcrypt_generic_deinit(crypt_fd);
	mcrypt_module_close(crypt_fd);

	mcrypt_generic_deinit(decrypt_fd);
	mcrypt_module_close(decrypt_fd);
}

int main(int argc, char *argv[])
{
	char* port_num = NULL;
	char* log_file = NULL;
	char* encrypt_key = NULL;

	int err = process_cli_arguments(argc, argv, 
									&port_num, &log_file, &encrypt_key);

	if (err == -1)
	{
		fprintf(stderr, "%s\n", "ERROR: Invalid usage.");
		fprintf(stderr, "%s\n", "You must specify a port number.");
		fprintf(stderr, "%s\n", "Usage: lab1b-client [--port=port#] [--log=filename] [--encrypt=filename]");
		exit(ERR_CODE);
	}

	if (encrypt_key)
	{
		encryption_decryption_init(encrypt_key);
	}

	// Configure terminal
	if (tcgetattr(0, &old_term_settings) == -1)
	{
		process_failed_sys_call("tcgetattr");
	}
	apply_new_term_settings(old_term_settings);
	atexit(restore_term_env);

	int sockfd = get_server_connection(port_num);

	// Set up poll
	struct pollfd fds[2];
	fds[0].fd = 0;
	fds[0].events = POLLIN;
	fds[0].revents = 0;

	fds[1].fd = sockfd;
	fds[1].events = POLLIN | POLLHUP | POLLERR;
	fds[1].revents = 0;

	while(1)
	{
		if (poll(fds, 2, -1) < 0)
		{
			process_failed_sys_call("poll");
		}

		// Input from keyboard
		if (fds[0].revents & POLLIN)
		{
			process_input(0, sockfd, 1);
		}

		// Input from socket
		if (fds[1].revents & POLLIN)
		{
			if (process_input(sockfd, 1, 256) == -1)
			{
				break;
			}
		}

		if (fds[1].revents & (POLLHUP + POLLERR))
		{
			break;
		}
	}

	if (encrypt_key)
	{
		encryption_decryption_deinit(encrypt_key);
	}

	exit(SUCCESS_CODE);
}

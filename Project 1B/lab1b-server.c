
/*
 * NAME: Anirudh Veeraragavan
 * EMAIL: aveeraragavan@g.ucla.edu
 * ID: 004767663
 */

#include <stdio.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <mcrypt.h>
#include <fcntl.h>

// Global Constants
int ERR_CODE = 1;
int SUCCESS_CODE = 0;
int EOF_CODE = 4;
int INTER_CODE = 3;
int CR_CODE = 13;
int LF_CODE = 10;

// Global Variables
int cid;
int key_size = -1;
MCRYPT crypt_fd;
MCRYPT decrypt_fd;

// INPUT: Name of sys call that threw error
// Prints reason for error and terminates program
void process_failed_sys_call(const char syscall[])
{
	int err = errno;
	fprintf(stderr, "%s\n", "An error has occurred.");
	fprintf(stderr, "The system call '%s' failed with error code %d\n", syscall, err);
	fprintf(stderr, "This error code means: %s\n", strerror(err));
	exit(ERR_CODE);
}

// INPUT: Signal code
// Handles any errors
void signal_handler(int num)
{
	if (num == SIGINT)
	{
		kill(cid, SIGINT);
	}
	if (num == SIGPIPE)
	{
		exit(ERR_CODE);
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
						  char** port, char** encrypt)
{
	struct option long_options[] =
	{
		{"port", required_argument, NULL, 'p'},
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
			case 'e':
				*encrypt = extract_key(optarg);
				break;
			case '?':
				fprintf(stderr, "%s\n", "ERROR: Invalid argument.");
				fprintf(stderr, "%s\n", "Usage: lab1b-client [--port=port#] [--log=filename]");
				exit(ERR_CODE);
		}
	}
}


// INPUT: Port number
// Configure server and block until client connects, return socket fd
int get_client_connection(const char port[])
{
	// Use IP, continuous stream, and TCP
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	{
		process_failed_sys_call("socket");
	}

	// Configure IP address settings
	struct sockaddr_in server;
	memset((char *) &server, 0, sizeof(server));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(port));

	// Bind socket to specified IP address
	if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		process_failed_sys_call("bind");
	}

	// Listen to socket for connections
	if (listen(sockfd, 5) < 0)
	{
		process_failed_sys_call("listen");
	}

	// Block until client connects
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd == -1)
	{
		process_failed_sys_call("accept");
	}

	return newsockfd;
}


// INPUT: Array to store pipes in
// Create pipes while checking for errors
void create_pipe(int fd[])
{
	if (pipe(fd) < 0)
	{
		process_failed_sys_call("pipe");
	}
}

// INPUT: Arrays for pipes
// Fork, and if child redirect pipes and execute shell
void create_shell_process(int tofd[], int fromfd[])
{
	cid = fork();
	if (cid < 0)
	{
		process_failed_sys_call("fork");
	}
	/* The child will have cid of 0 */
	else if (!cid)
	{
		close(tofd[1]);
		close(fromfd[0]);

		dup2(tofd[0], 0);
		close(tofd[0]);

		dup2(fromfd[1], 1);	
		dup2(fromfd[1], 2);
		close(fromfd[1]);
	
		char **cmd = NULL;
		if (execvp("/bin/bash", cmd) == -1)
		{
			process_failed_sys_call("execvp");
		}
	}
}

// INPUT: n/a
// Harvest shell exit status
void print_child_process_status()
{
	int status;
	if (waitpid(-1, &status, 0) == -1)
	{
		process_failed_sys_call("waitpid");
	}
	fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d", 
			WTERMSIG(status), WEXITSTATUS(status));
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
	char* encrypt_key = NULL;

	int err = process_cli_arguments(argc, argv, &port_num, &encrypt_key);
	if (err == -1)
	{
		fprintf(stderr, "%s\n", "ERROR: Invalid usage.");
		fprintf(stderr, "%s\n", "You must specify a port number.");
		fprintf(stderr, "%s\n", "Usage: lab1b-client [--port=port#] [--log=filename]");
		exit(ERR_CODE);
	}

	if (encrypt_key)
	{
		encryption_decryption_init(encrypt_key);
	}

	int newsockfd = get_client_connection(port_num);

	// Write to 1, read from 0
	int toshell[2];
	int fromshell[2];
	create_pipe(toshell);
	create_pipe(fromshell);

	create_shell_process(toshell, fromshell);

	close(toshell[0]);
	close(fromshell[1]);

	// Set up polling
	struct pollfd fds[2];
	fds[0].fd = newsockfd;
	fds[0].events = POLLIN | POLLHUP | POLLERR;
	fds[0].revents = 0;

	fds[1].fd = fromshell[0];
	fds[1].events = POLLIN | POLLHUP | POLLERR;
	fds[1].revents = 0;

	// Read input from socket one char at a time, write one at a time
	// Read input from shell 256 char at a time, but write to socket one at a time
	// Encrypt/decrypt one char at a time
	while(1)
	{
		if (poll(fds, 2, -1) < 0)
		{
			process_failed_sys_call("poll");
		}

		// Input from socket
		if (fds[0].revents & POLLIN)
		{
			char buf[1];
			memset((char *) &buf, 0, sizeof(char));
			int bytes_read = read(newsockfd, buf, sizeof(char));
			if (bytes_read < 0)
			{
				process_failed_sys_call("read");
			}

			if (encrypt_key)
			{
				if (mdecrypt_generic(decrypt_fd, buf, sizeof(char)) != 0)
				{
					process_failed_sys_call("mdecrypt_generic");
				}
			}

			if ((int)buf[0] == EOF_CODE)
			{
				close(toshell[1]);
				continue;
			}

			if ((int)buf[0] == INTER_CODE)
			{
				kill(cid, SIGINT);
				break;
			}

			// Map <cr> or <lf> into <lf> 
			if ((int)buf[0] == CR_CODE || (int)buf[0] == LF_CODE) 
			{
				char line_end = '\n';
				write(toshell[1], &line_end, sizeof(char));
				if (encrypt_key)
				{
					if (mcrypt_generic(crypt_fd, &line_end, sizeof(char)) != 0)
					{
						process_failed_sys_call("mcrypt_generic");
					}
				}
				write(newsockfd, &line_end, sizeof(char));
				continue;
			} 

			write(toshell[1], &buf[0], sizeof(char));

			if (encrypt_key)
			{
				if (mcrypt_generic(crypt_fd, &buf[0], sizeof(char)) != 0)
				{
					process_failed_sys_call("mcrypt_generic");
				}
			}

			write(newsockfd, &buf[0], sizeof(char));
		}

		// Input from shell
		if (fds[1].revents & POLLIN)
		{
			char buffer[256];
			memset((char *) &buffer, 0, sizeof(buffer));
			int bytes_read = read(fromshell[0], buffer, sizeof(buffer));
			if (bytes_read < 0)
			{
				process_failed_sys_call("read");
			}

			int i;
			for (i = 0; i < bytes_read; ++i)
			{
				if (encrypt_key)
				{
					if (mcrypt_generic(crypt_fd, &buffer[i], sizeof(char)) != 0)
					{
						process_failed_sys_call("mcrypt_generic");
					}
				}
				write(newsockfd, &buffer[i], sizeof(char));
			}
		}

		if (fds[0].revents & (POLLHUP + POLLERR))
		{
			close(fromshell[0]);
			continue;
		}

		if (fds[1].revents & (POLLHUP + POLLERR))
		{
			kill(cid, SIGINT);
			break;
		}
	}

	close(fromshell[0]);
	print_child_process_status();

	if (encrypt_key)
	{	
		encryption_decryption_deinit();
	}

	exit(SUCCESS_CODE);
}

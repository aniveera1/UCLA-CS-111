/*
 * NAME: Anirudh Veeraragavan
 * EMAIL: aniveera1@gmail.com
 * ID: 004767663
 */

#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int BUFFER_SIZE = 256;
int EOF_CODE = 4;
int INTER_CODE = 3;
int CR_CODE = 13;
int LF_CODE = 10;
int TIMEOUT = 0;
int ERR_CODE = 1;
int SUCCESS_CODE = 0;

/* CLI Options */
struct option long_options[] =
{
	{"shell", no_argument, NULL, 's'},
	{0, 0, 0, 0}
};
int option_index = 0;

/* Global variables to share data */
struct termios old_term_settings;
int shell = 0;
int cid;

void process_failed_sys_call(const char syscall[])
{
	int err = errno;
	fprintf(stderr, "%s\n", "An error has occurred.");
	fprintf(stderr, "The system call '%s' failed with error code %d\n", syscall, err);
	fprintf(stderr, "This error code means: %s\n", strerror(err));
	exit(ERR_CODE);
}

void restore_term_env()
{
	if (tcsetattr(0, TCSANOW, &old_term_settings) == -1)
	{
		process_failed_sys_call("tcsetattr");
	}
}

void signal_handler(int num)
{
 	if (shell && num == SIGINT)
  	{
    	kill(getpid(), SIGTERM);
  	}
  	if (num == SIGPIPE)
  	{
    	exit(ERR_CODE);
  	}
}

void process_cli_arguments(int argc, char** argv)
{
	while(1)
	{
		int arg = getopt_long(argc, argv, "s", long_options, &option_index);

		/* No more args */
		if (arg == -1)
		{
			break;
		}

		switch(arg)
		{
			case 's':
			    signal(SIGINT, signal_handler);
				signal(SIGPIPE, signal_handler);
				shell = 1;
				break;
			case '?':
				fprintf(stderr, "%s\n", "An error has occurred.");
				fprintf(stderr, "%s\n", "An invalid option was entered.");
				fprintf(stderr, "%s\n", "Usage: lab1a [--shell]");
				exit(ERR_CODE);
		}
	}
}

void apply_new_term_settings(struct termios settings)
{
	/* Settings for character-at-a-time, no-echo mode */
	settings.c_iflag = ISTRIP;
	settings.c_oflag = 0;
	settings.c_lflag = 0;

	if (tcsetattr(0, TCSANOW, &settings) == -1)
	{
		process_failed_sys_call("tcsetattr");
	}
}

void create_pipe(int fd[])
{
	if (pipe(fd) == -1)
	{
		process_failed_sys_call("pipe");
	}
}

void create_shell_process(int tofd[], int fromfd[])
{
	cid = fork();
	if (cid == -1)
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

void print_child_process_status()
{
	if (shell)
	{
		int status;
		if (waitpid(-1, &status, 0) == -1)
		{
			process_failed_sys_call("waitpid");
		}
		fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d", 
				WTERMSIG(status), WEXITSTATUS(status));
	}
}

int process_keyboard_input(int no_child, int fd)
{
	char buf[1];
	int bytes_read = read(0, buf, sizeof(char));
	if (bytes_read == -1)
	{
		process_failed_sys_call("read");
	}

	if ((int)buf[0] == EOF_CODE)
	{
		if (!no_child)
		{
			close(fd);
		}
		return EOF_CODE;
	}

	if ((int)buf[0] == INTER_CODE)
	{
		if (!no_child)
		{
			kill(cid, SIGTERM);
		}
		return INTER_CODE;
	}

	/* Map <cr> or <lf> into <cr><lf> */
	if ((int)buf[0] == CR_CODE || (int)buf[0] == LF_CODE) 
	{
		char line_end[] = {'\r', '\n'};
		write(1, line_end, sizeof(char) * 2);

		if (!no_child)
		{
			write(fd, &line_end[1], sizeof(line_end[1]));
		}
		
		return CR_CODE;
	} 

	write(1, &buf[0], sizeof(char));

	if (!no_child)
	{
		write(fd, &buf[0], sizeof(char));
	}

	return 0;
}

int process_child_input(int fd)
{
	char buf[BUFFER_SIZE];
	int bytes_read = read(fd, buf, sizeof(buf));
	if (bytes_read == -1)
	{
		process_failed_sys_call("read");
	}

	int i;
	for (i = 0; i < bytes_read; ++i)
	{
		if ((int)buf[i] == EOF_CODE)
		{
			return EOF_CODE;
		}

		/* Map <cr> or <lf> into <cr><lf> */
		if ((int)buf[i] == CR_CODE || (int)buf[i] == LF_CODE)
		{
			char line_end[] = {'\r', '\n'};
			write(1, line_end, sizeof(char) * 2);
			continue;
		} 

		write(1, &buf[i], sizeof(char));
	}
	return 0;
}

int main(int argc, char **argv)
{
    process_cli_arguments(argc, argv);
  
	if (tcgetattr(0, &old_term_settings) == -1)
	{
		process_failed_sys_call("tcgetattr");
	}
	apply_new_term_settings(old_term_settings);

	atexit(restore_term_env);

	int toshell[2];
	int fromshell[2];
	create_pipe(toshell);
	create_pipe(fromshell);

	if (shell)
	{
		create_shell_process(toshell, fromshell);
	}

	close(toshell[0]);
	close(fromshell[1]);

	if (shell)
	{
		struct pollfd fds[2];
		fds[0].fd = 0;
		fds[0].events = POLLIN;
		fds[0].revents = 0;

		fds[1].fd = fromshell[0];
		fds[1].events = POLLIN | POLLHUP | POLLERR;
		fds[1].revents = 0;

		while(1)
		{
			if (poll(fds, 2, -1) == -1)
			{
				process_failed_sys_call("poll");
			}
			if (fds[0].revents & POLLIN)
			{
				int err = process_keyboard_input(0, toshell[1]);

				if (err == EOF_CODE)
				{
					continue;
				}
				else if (err == INTER_CODE)
				{
				  	break;
				}
				else if (err == CR_CODE) 
				{
					continue;
				}
			}
			if (fds[1].revents & POLLIN)
			{
				if (process_child_input(fromshell[0]) == EOF_CODE)
				{
				  	close(fromshell[0]);
					break;
				}

			}
			if (fds[1].revents & (POLLHUP + POLLERR))
			{
			  	kill(cid, SIGINT);
				break;
			}
		}
	}
	else
	{
		while(1)
		{
			close(toshell[1]);
			close(fromshell[0]);

			int err = process_keyboard_input(1, -1);

			if (err == EOF_CODE)
			{
				break;
			}
		}
	}
	
	print_child_process_status();	

	exit(SUCCESS_CODE);
}

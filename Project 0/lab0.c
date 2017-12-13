/*
 * NAME: Anirudh Veeraragavan
 * EMAIL: aveeraragavan@g.ucla.edu
 * ID: 004767663
 */

#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

/* CLI Options */
struct option long_options[] =
  {
    {"segfault", no_argument, NULL, 'a'},
    {"catch", no_argument, NULL, 'b'},
    {"input", required_argument, NULL, 'c'},
    {"output", required_argument, NULL, 'd'},
    {0, 0, 0, 0}
  };
int option_index = 0;

void process_cli_arguments(int argc, char** argv,
			   int* segfault, int* catch,
			   char** input_file, char** output_file) {
  while(1) {
    /* Get next arg */
    int arg = getopt_long(argc, argv, "abc:d:",
			  long_options, &option_index);

    if (arg == -1)
      break;

    switch(arg) {
    case 'a':
      *segfault = 1;
      break;
    case 'b':
      *catch = 1;
      break;
    case 'c':
      *input_file = optarg;
      break;
    case 'd':
      *output_file = optarg;
      break;
    case '?':
      fprintf(stderr, "%s\n", "An error has occurred");
      fprintf(stderr, "%s\n", "An invalid option was entered");
      fprintf(stderr, "%s\n", "Usage: lab0 [--input=filename] [--output=filename] [--segfault] [--catch]");
      exit(1);
    }
  }
}

void sighandler() {
  fprintf(stderr, "%s\n", "A segmentation fault has occurred.");
  fprintf(stderr, "%s\n", "This fault occurred because of invalid access to storage.");
  fprintf(stderr, "%s\n", "It was caused by either the option --segfault or the program trying to access unallocated memory.");
  exit(4);
}

void seg_fault() {
  char* ptr = NULL;
  *ptr = 'a';
}

int main(int argc, char **argv) {
  int segfault = 0;
  int catch = 0;
  char* input_file = NULL;
  char* output_file = NULL;

  process_cli_arguments(argc, argv,
			&segfault, &catch,
			&input_file, &output_file);

  /* Redirect input file */
  if (input_file) {
    int ifd = open(input_file, O_RDONLY);
    if (ifd == -1) {
      int erropen = errno;
      fprintf(stderr, "An error has occurred due to --input\n");
      fprintf(stderr, "The file '%s' was not open due to errno code %d\n", input_file, erropen);
      fprintf(stderr, "This errno code means: %s\n", strerror(erropen));
      exit(2);
    }

    close(0);
    dup(ifd);
    close(ifd);
  }

  /* Redirect output file */
  if (output_file) {
    int ofd = creat(output_file, 0666);
    if (ofd == -1) {
      int errcreat = errno;
      fprintf(stderr, "An error has occurred due to --output\n");
      fprintf(stderr, "The file '%s' was not created due to errno code %d\n", output_file, errcreat);
      fprintf(stderr, "This errno code means: %s\n", strerror(errcreat));
      exit(3);
    }

    close(1);
    dup(ofd);
    close(ofd);
  }

  /* Register signal handler */
  if (catch)
    signal(SIGSEGV, sighandler);

  /* Cause segfault */
  if (segfault)
    seg_fault();

  char* buf[1];

  /* Read until EOF */
  while (read(0, buf, sizeof(char)) > 0)
    write(1, buf, sizeof(char));

  exit(0);
}

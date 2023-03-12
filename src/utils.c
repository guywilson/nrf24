#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "utils.h"

void hexDump(void * buffer, uint32_t bufferLen)
{
    int         i;
    int         j = 0;
    uint8_t *   buf;
    static char szASCIIBuf[17];

    buf = (uint8_t *)buffer;

    for (i = 0;i < bufferLen;i++) {
        if ((i % 16) == 0) {
            if (i != 0) {
                szASCIIBuf[j] = 0;
                j = 0;

                printf("  |%s|", szASCIIBuf);
            }
                
            printf("\n%08X\t", i);
        }

        if ((i % 2) == 0 && (i % 16) > 0) {
            printf(" ");
        }

        printf("%02X", buf[i]);
        szASCIIBuf[j++] = isprint(buf[i]) ? buf[i] : '.';
    }

    /*
    ** Print final ASCII block...
    */
    szASCIIBuf[j] = 0;
    printf("  |%s|\n", szASCIIBuf);
}


void daemonise() {
	pid_t			pid;
	pid_t			sid;

	fprintf(stdout, "Starting daemon...\n");
	fflush(stdout);

	do {
		pid = fork();
	}
	while ((pid == -1) && (errno == EAGAIN));

	if (pid < 0) {
		fprintf(stderr, "Forking daemon failed...\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}
	if (pid > 0) {
		fprintf(stdout, "Exiting child process...\n");
		fflush(stdout);
		exit(EXIT_SUCCESS);
	}

	sid = setsid();
	
	if(sid < 0) {
		fprintf(stderr, "Failed calling setsid()...\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);    
	
	umask(0);

	if((chdir("/") == -1)) {
		fprintf(stderr, "Failed changing directory\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}
	
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
//	close(STDERR_FILENO);
}

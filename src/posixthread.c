#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "posixthread.h"


static void * _threadRunner(void * pParms) {
	void *			pThreadRtn = NULL;
	bool			go = true;

	pxt_handle_t * hpxt = (pxt_handle_t *)pParms;

	while (go) {
		pThreadRtn = hpxt->run(hpxt->pThreadParm);

		if (!hpxt->isRestartable) {
			go = false;
		}
		else {
			pxtSleep(seconds, 1);
		}
	}

	return pThreadRtn;
}

void pxtSleep(TimeUnit u, uint64_t t) {
	switch (u) {
		case hours:
			sleep(t * 3600);
			break;

		case minutes:
			sleep(t * 60);
			break;

		case seconds:
			sleep(t);
			break;

		case milliseconds:
			usleep(t * 1000L);
			break;

		case microseconds:
			usleep(t);
			break;
	}
}

void pxtCreate(pxt_handle_t * hpxt, void * (* thread)(void *), bool isRestartable) {
    hpxt->isRestartable = isRestartable;
    hpxt->run = thread;
}

int pxtStart(pxt_handle_t * hpxt, void * pParms) {
    int             error;

    hpxt->pThreadParm = pParms;

    error = pthread_create(&hpxt->tid, NULL, &_threadRunner, hpxt);

    if (error) {
        printf("Error creating thread\n");
    }

    return error;
}

void pxtStop(pxt_handle_t * hpxt) {
	pthread_kill(hpxt->tid, SIGKILL);
}

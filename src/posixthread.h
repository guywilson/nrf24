#include <stdint.h>
#include <stdbool.h>

#ifndef __POSIX_THREAD
#define __POSIX_THREAD

struct _pxt_handle_t;

typedef struct _pxt_handle_t        pxt_handle_t;

typedef enum {
    hours,
    minutes,
    seconds,
    milliseconds,
    microseconds
}
TimeUnit;

void    pxtSleep(TimeUnit u, uint64_t t);
void    pxtCreate(pxt_handle_t * hpxt, void * (* thread)(void *), bool isRestartable);
int     pxtStart(pxt_handle_t * hpxt, void * pParms);
void    pxtStop(pxt_handle_t * hpxt);

#endif

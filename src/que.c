#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "cfgmgr.h"
#include "logger.h"
#include "que.h"

struct _que_handle_t {
    que_item_t *    pQueue;

    uint32_t        queueLength;
    uint32_t        numItems;

    int             headIndex;
    int             tailIndex;
};

int qInit(que_handle_t * hque, uint32_t size) {
    hque->pQueue = (que_item_t *)malloc(sizeof(que_item_t) * size);

    if (hque->pQueue == NULL) {
        lgLogFatal(lgGetHandle(), "qInit() - Failed to allocate queue memory of size %u", size);
        return -1;
    }

    lgLogInfo(lgGetHandle(), "qInit() - Instantiated queue with %u items", size);
    
    hque->queueLength = size;

    hque->headIndex = 0;
    hque->tailIndex = 0;
    hque->numItems = 0;

    return 0;
}

void qDestroy(que_handle_t * hque) {
    free(hque->pQueue);
}

uint32_t qGetQueLength(que_handle_t * hque) {
    return hque->queueLength;
}

que_item_t * qGetItem(que_handle_t * hque, que_item_t * item) {
    if (hque->numItems == 0) {
        lgLogError(lgGetHandle(), "qGetItem() - Queue is empty");
        return NULL;
    }

    memcpy(item, &hque->pQueue[hque->headIndex++], sizeof(que_item_t));
    hque->numItems--;

    if (hque->headIndex == hque->queueLength) {
        lgLogDebug(lgGetHandle(), "qGetItem() - Head wrap around");
        hque->headIndex = 0;
    }

    return item;
}

int qPutItem(que_handle_t * hque, que_item_t item) {
    if (hque->numItems == hque->queueLength) {
        lgLogError(lgGetHandle(), "qPutItem() - Queue is full");
        return -1;
    }

    memcpy(&hque->pQueue[hque->tailIndex++], &item, sizeof(que_item_t));
    hque->numItems++;

    if (hque->tailIndex == hque->queueLength) {
        lgLogDebug(lgGetHandle(), "qPutItem() - Tail wrap around");
        hque->tailIndex = 0;
    }

    return 0;
}

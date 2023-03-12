#include <stdint.h>
#include <stdbool.h>

#ifndef __INCL_QUE
#define __INCL_QUE

typedef struct {
    void *          item;
    uint32_t        itemLength;
}
que_item_t;

struct _que_handle_t;
typedef struct _que_handle_t            que_handle_t;

int             qInit(que_handle_t * hque, uint32_t size);
void            qDestroy(que_handle_t * hque);
uint32_t        qGetQueLength(que_handle_t * hque);
que_item_t *    qGetItem(que_handle_t * hque, que_item_t * item);
int             qPutItem(que_handle_t * hque, que_item_t item);

#endif

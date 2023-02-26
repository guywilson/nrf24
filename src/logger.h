#include <stdbool.h>

#ifndef __INCL_LOGGER
#define __INCL_LOGGER

#define MAX_LOG_LENGTH          512

/*
** Supported log levels...
*/
#define LOG_LEVEL_INFO          0x01
#define LOG_LEVEL_STATUS        0x02
#define LOG_LEVEL_DEBUG         0x04
#define LOG_LEVEL_ERROR         0x08
#define LOG_LEVEL_FATAL         0x10

#define LOG_LEVEL_ALL           (LOG_LEVEL_INFO | LOG_LEVEL_STATUS | LOG_LEVEL_DEBUG | LOG_LEVEL_ERROR | LOG_LEVEL_FATAL)

struct _log_handle_t;
typedef struct _log_handle_t        log_handle_t;

log_handle_t *  lgGetHandle();
int             lgOpen(const char * pszLogFile, const char * pszLogFlags);
int             lgOpenStdout(const char * pszLogFlags);
void            lgClose(log_handle_t * hlog);
void            lgSetLogLevel(log_handle_t * hlog, int logLevel);
int             lgGetLogLevel(log_handle_t * hlog);
bool            lgCheckLogLevel(log_handle_t * hlog, int logLevel);
void            lgNewline(log_handle_t * hlog);
int             lgLogInfo(log_handle_t * hlog, const char * fmt, ...);
int             lgLogStatus(log_handle_t * hlog, const char * fmt, ...);
int             lgLogDebug(log_handle_t * hlog, const char * fmt, ...);
int             lgLogDebugNoCR(log_handle_t * hlog, const char * fmt, ...);
int             lgLogError(log_handle_t * hlog, const char * fmt, ...);
int             lgLogFatal(log_handle_t * hlog, const char * fmt, ...);

#endif

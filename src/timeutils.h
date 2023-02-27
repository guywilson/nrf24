#ifndef __INCL_TIMEUTILS
#define __INCL_TIMEUTILS

#define TIMESTAMP_STR_LEN               32
#define SECONDS_PER_MINUTE				60
#define SECONDS_PER_HOUR				(SECONDS_PER_MINUTE * 60)
#define SECONDS_PER_DAY					(SECONDS_PER_HOUR * 24)

#define SUNDAY							1
#define MONDAY							2
#define TUESDAY							3
#define WEDNESDAY						4
#define THURSDAY						5
#define FRIDAY							6
#define SATURDAY						7

void tmInitialiseUptimeClock();
char * tmGetUptime();
char * tmGetTimeStamp(bool includeMicroseconds);
char * tmGetSimpleTimeStamp();
int tmGetYear();
int tmGetMonth();
int tmGetDay();
int tmGetDayOfWeek();
int tmGetHour();
int tmGetMinute();
int tmGetSecond();
int tmGetMicrosecond();

#endif

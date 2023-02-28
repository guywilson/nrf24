#include "version.h"

#define __BDATE__      "2023-02-28 09:20:47"
#define __BVERSION__   "0.1.002"

const char * getVersion()
{
    return __BVERSION__;
}

const char * getBuildDate()
{
    return __BDATE__;
}

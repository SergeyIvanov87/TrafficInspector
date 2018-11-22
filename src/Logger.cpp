#include <stdlib.h>
#include <stdarg.h>
#include "Utils/Logger.h"


FILE *logFile = nullptr;
void initializeLogger(const char *filePath)
{

    if(logFile == nullptr)
    {
        if(!filePath ||   (logFile = fopen(filePath, "w+")) ==nullptr)
        {
            logFile = stdout;
        }
    }
    logger("Logger Initialized");
}

void logger(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    FILE *tmp = logFile ? logFile : stdout;
    vfprintf(tmp, fmt, ap);
    fprintf(tmp, "\n");
    fflush(tmp);

    va_end(ap);
}

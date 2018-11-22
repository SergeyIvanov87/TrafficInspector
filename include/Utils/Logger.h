#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

extern FILE *logFile;
void initializeLogger(const char *path);

void logger(const char *fmt, ...);
#endif /* LOGGER_H */


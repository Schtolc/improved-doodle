#include "helpers.h"

#include <stdio.h>
#include <unistd.h>
#include <zconf.h>
#include <stdlib.h>

void die(int line_number, const char *format, ...) {
    va_list vargs;
    va_start (vargs, format);
    fprintf(stderr, "%d: ", line_number);
    vfprintf(stderr, format, vargs);
    fprintf(stderr, ".\n");
    va_end (vargs);
    exit(1);
}

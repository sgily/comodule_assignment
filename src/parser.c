#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <zephyr.h>
#include <ctype.h>
#include <errno.h>
#include <sys/cbprintf.h>

// convert string to float
// on failure returns 0 and sets errno
static float strtod(char *str) {
    float res = 0, fact = 1;
    uint8_t point_found = 0;

    if (str == NULL) {
        errno = -EINVAL;
        return 0;
    }
    if (*str == '-') {
        fact = -1;
        ++str;
    }
    for(; *str != '\0'; ++str) {
        if (*str == '.') {
            point_found = 1;
        } else {
            int d = *str - '0';
            if (d >= 0 && d <= 9) {
                if (point_found)
                    fact /= 10;
                res = res*10.0f + d;
            } else {
                // error, return 0
                errno = -EINVAL;
                res = 0;
                break;
            }
        }
    }

    return res*fact;
}

// here token is the floating point number (latitude or longitude)
static int parse_dd_token(char *buf, double *result, char **saveptr) {
    int err = 0;
    char *tok;

    tok = strtok_r(buf, ":, ", saveptr);
    errno = 0;
    *result = strtod(tok);
    if (*result == 0 && errno)
        err = errno;

    return err;
}

static int validate_coordinates(dd_coordinates* dd) {
    int err = 0;

    if (dd->lat > LATITUDE_MAX || dd->lat < LATITUDE_MIN)
        err = -ERANGE;
    if (dd->lon > LONGITUDE_MAX || dd->lon < LONGITUDE_MIN)
        err = -ERANGE;

    return err;
}

// fills decimal degree data struct and validates results
static int parse_dd_location(const char* buf, size_t length, dd_coordinates* result) {
    int err = 0;
    char bufcpy[length];
    char *saveptr;

    //copy input buffer because it will be modified
    strcpy(bufcpy, buf);
    err = parse_dd_token(bufcpy, &result->lat, &saveptr);
    err = parse_dd_token(NULL, &result->lon, &saveptr);
    if (!err)
        err = validate_coordinates(result);
    
    return err;
}

// parse coordinates string and format it into string
// to up to 5 decimal places (beyond, the output getz fuzzy
// because of the rounding)
int format_coordinates(const char* data_buf, size_t data_length, 
                        char* result_buf, size_t result_length) {
    int err = 0;
    dd_coordinates res = {.lat = 0, .lon = 0};

    err = parse_dd_location(data_buf, data_length, &res);
    if (!err)
        snprintfcb(result_buf, result_length, "%3.5f %3.5f", res.lat, res.lon);
    
    return err;
}
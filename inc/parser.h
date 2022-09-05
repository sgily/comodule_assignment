#ifndef PARSER_H_
#define PARSER_H_

#include <stddef.h>

#define LATITUDE_MAX 90.0f
#define LATITUDE_MIN -90.0f
#define LONGITUDE_MAX 180.0f
#define LONGITUDE_MIN -180.0f

// struct for holding decimal degree data
typedef struct {
    double lat;
    double lon;
} dd_coordinates;

// parser interface
// params:
//  data_buf        input buffer
//  data_length     input buffer size
//  result_buf      outputbuffer for storing results
//  result_length   output buffer size
// return:
//  0 on success, error code on failure
int format_coordinates(const char* data_buf, size_t data_length, 
                        char* result_buf, size_t result_length);


#endif

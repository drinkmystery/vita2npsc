/*
    Deflate-Inflate convenience methods for key compression.
    Include dictionary with set of strings for more efficient packing
*/

#include <stdlib.h>
#include <stdint.h>

#define MIN_KEY_SIZE 512
#define MAX_KEY_SIZE 2048

int deflateKey( const uint8_t *license, size_t in_size, uint8_t *out, size_t out_size );
int inflateKey( const uint8_t *in, size_t in_size, uint8_t *license, size_t out_size );
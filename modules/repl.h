#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string>
#include <string.h>

typedef struct
{
    char *buffer;
    size_t buffer_length;
    size_t input_length;
} InputBuffer;

InputBuffer *new_input_buffer();

void print_prompt();

void read_input(InputBuffer *input_buffer);
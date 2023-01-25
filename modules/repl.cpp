#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string>
#include <string.h>

#include "repl.h"

InputBuffer *new_input_buffer()
{
    InputBuffer *input_buffer = (InputBuffer *)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

void print_prompt() { printf("db > "); }

void read_input(InputBuffer *input_buffer)
{
    std::string line;
    std::getline(std::cin, line);
    char *text = &line[0];
    int bytes_read = line.size();

    input_buffer->buffer = text;
    input_buffer->buffer_length = bytes_read;
    input_buffer->input_length = bytes_read;

    if (bytes_read <= 0)
    {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }
}
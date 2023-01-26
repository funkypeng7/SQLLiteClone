#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string>
#include <string.h>

#include "modules.hpp"


void InputBuffer::read_input()
{
    std::getline(std::cin, buffer);
    int bytes_read = buffer.length();

    buffer_length = bytes_read;

    if (bytes_read <= 0)
    {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }

    
}

void  print_prompt() { printf("db > "); };
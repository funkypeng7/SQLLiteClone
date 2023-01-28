#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string>
#include <string.h>

#include "modules.hpp"

InputBuffer::InputBuffer()
{
    buffer = "";
    buffer_length = 0;
    input_length = 0;
}

void InputBuffer::read_input()
{
    std::getline(std::cin, buffer);
    int bytes_read = buffer.length();

    buffer_length = bytes_read;
}

void print_prompt() { printf("db > "); };
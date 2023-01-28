#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string>
#include <string.h>

#include "modules/modules.hpp"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Must supply a database filename.\n");
        exit(EXIT_FAILURE);
    }
    char *filename = argv[1];
    Table table = Table(filename);
    InputBuffer input_buffer = InputBuffer();
    while (true)
    {
        print_prompt();
        input_buffer.read_input();

        if (input_buffer.buffer[0] == '.')
        {
            switch (do_meta_command(input_buffer, table))
            {
            case (META_COMMAND_SUCCESS):
                continue;
            case (META_COMMAND_UNRECOGNIZED_COMMAND):
                printf("Unrecognized command '%s'\n", input_buffer.buffer.c_str());
                continue;
            }
        }

        Statement statement = Statement(input_buffer);
        switch (statement.prepareResult)
        {
        case (PREPARE_SUCCESS):
            break;
        case (PREPARE_SYNTAX_ERROR):
            printf("Syntax error. Could not parse statement.\n");
            continue;
        case (PREPARE_UNRECOGNIZED_STATEMENT):
            printf("Unrecognized keyword at start of '%s'.\n",
                   input_buffer.buffer.c_str());
            continue;
        }

        switch (table.execute_statement(statement))
        {
        case (EXECUTE_SUCCESS):
            printf("Executed.\n");
            break;
        case (EXECUTE_TABLE_FULL):
            printf("Error: Table full.\n");
            break;
        }
    }
}
#include "modules.hpp"

// TODO: make table method
MetaCommandResult do_meta_command(InputBuffer input_buffer, Table table)
{
    if (input_buffer.buffer.compare(".exit") == 0)
    {
        table.db_close();
        exit(EXIT_SUCCESS);
    }
    else if (input_buffer.buffer.compare(".constants") == 0)
    {
        printf("Constants:\n");
        print_constants();
        return META_COMMAND_SUCCESS;
    }
    else if (input_buffer.buffer.compare(".btree") == 0)
    {
        printf("Tree:\n");
        Cursor::print_tree(table.pager, 0, 0);
        return META_COMMAND_SUCCESS;
    }

    else
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

Statement::Statement(InputBuffer input_buffer)
{
    if (input_buffer.buffer.compare(0, 6, "insert") == 0)
    {
        type = STATEMENT_INSERT;
        int args_assigned = sscanf(
            input_buffer.buffer.c_str(), "insert %d %s %s", &(row_to_insert.id),
            row_to_insert.username, row_to_insert.email);
        if (args_assigned < 3)
        {
            prepareResult = PREPARE_SYNTAX_ERROR;
            return;
        }
        prepareResult = PREPARE_SUCCESS;
        return;
    }
    else if (input_buffer.buffer.compare("select") == 0)
    {
        type = STATEMENT_SELECT;
        prepareResult = PREPARE_SUCCESS;
        return;
    }

    prepareResult = PREPARE_UNRECOGNIZED_STATEMENT;
    return;
}

void print_constants()
{
    printf("ROW_SIZE: %d\n", ROW_SIZE);
    printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
    printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
    printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
    printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
    printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}


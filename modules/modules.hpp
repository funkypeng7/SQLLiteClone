#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string>
#include <string.h>

#ifndef MODULES_H
#define MODULES_H

// Enums

enum ExecuteResult
{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
};

enum MetaCommandResult
{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} ;

 enum PrepareResult
{
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT
} ;

 enum StatementType
{
    STATEMENT_INSERT,
    STATEMENT_SELECT
} ;

// Classes
class InputBuffer;
class Row;
class Statement;
class Table;

// Input Buffer / REPL

class InputBuffer
{
    public:
    std::string buffer = "";
    size_t buffer_length = 0;
    size_t input_length = 0;
    
    void read_input();
};

void print_prompt();

// Rows 


#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

class Row
{
public:
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];

    void print_row();
    void serialize_row(void *destination);
    void deserialize_row(void *source);
};

// Commands and Statements

MetaCommandResult do_meta_command(InputBuffer input_buffer);

class Statement
{
public:
    PrepareResult prepareResult;
    StatementType type;
    Row row_to_insert; // Only used by insert statement
    
    Statement(InputBuffer input_buffer);

    ExecuteResult execute_statement(Table table);
};

#endif

#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;


// Table
#define TABLE_MAX_PAGES 100
const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

class Table
{
public:
    uint32_t num_rows;
    void *pages[TABLE_MAX_PAGES];

    Table();

    void *row_slot(uint32_t row_num);
    ExecuteResult execute_statement(Statement statement);
    ExecuteResult execute_insert(Statement statement);
    ExecuteResult execute_select(Statement statement);
};

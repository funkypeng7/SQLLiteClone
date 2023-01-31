#include <iostream>
#include <fstream>
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
    EXECUTE_TABLE_FULL,
    EXECUTE_DUPLICATE_KEY
};

enum MetaCommandResult
{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
};

enum PrepareResult
{
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT
};

enum StatementType
{
    STATEMENT_INSERT,
    STATEMENT_SELECT
};

enum NodeType
{
    NODE_INTERNAL,
    NODE_LEAF
};

// Classes
class InputBuffer;
class Row;
class Statement;
class Pager;
class Table;

// Input Buffer / REPL

class InputBuffer
{
public:
    std::string buffer;
    size_t buffer_length;
    size_t input_length;

    InputBuffer();
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
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];

    void print_row();
    void serialize_row(char *destination);
    void deserialize_row(void *source);
};

// Commands and Statements

MetaCommandResult do_meta_command(InputBuffer input_buffer, Table table);

class Statement
{
public:
    PrepareResult prepareResult;
    StatementType type;
    Row row_to_insert; // Only used by insert statement

    Statement(InputBuffer input_buffer);

    ExecuteResult execute_statement(Table table);
};

#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

// Table and pager
#define TABLE_MAX_PAGES 100
const uint32_t PAGE_SIZE = 4096;

class Pager
{
public:
    const char *filename;
    uint32_t file_length;
    uint32_t num_pages;
    char *pages[TABLE_MAX_PAGES];

    Pager();
    void connect_file(const char *filenameIn);

    char *get_page(uint32_t page_num);
    void flush(uint32_t page_num);
    uint32_t get_unused_page_num();
};

class Table
{
public:
    Pager pager;
    uint32_t root_page_num;

    Table();
    Table(const char *filename);

    void db_close();

    ExecuteResult execute_statement(Statement statement);
    ExecuteResult execute_insert(Statement statement);
    ExecuteResult execute_select(Statement statement);

    void create_new_root(uint32_t right_child_page_num);
};
extern const uint8_t COMMON_NODE_HEADER_SIZE;
extern const uint32_t LEAF_NODE_HEADER_SIZE;
extern const uint32_t LEAF_NODE_CELL_SIZE;
extern const uint32_t LEAF_NODE_SPACE_FOR_CELLS;
extern const uint32_t LEAF_NODE_MAX_CELLS;

class Cursor
{
public:
    Table *table;
    uint32_t page_num;
    uint32_t cell_num;
    bool end_of_table; // Indicates a position one past the last element

    Cursor();
    static Cursor table_start(Table *tableIn);
    static Cursor table_find(Table *table, uint32_t key);
    static Cursor internal_node_find(Table *table, uint32_t page_num, uint32_t key);
    static uint32_t internal_node_find_child(void *node, uint32_t key);
    static Cursor leaf_node_find(Table *table, uint32_t page_num, uint32_t key);
    char *position();
    void advance();
    void leaf_node_insert(uint32_t key, Row value);
    void leaf_node_split_and_insert(uint32_t key, Row value);

    // Todo: create node object
    static char *leaf_node_num_cells(void *node);
    static char *leaf_node_cell(void *node, uint32_t cell_num);
    static char *leaf_node_key(void *node, uint32_t cell_num);
    static char *leaf_node_value(void *node, uint32_t cell_num);
    static uint32_t *leaf_node_next_leaf(void *node);
    static void initialize_leaf_node(void *node);
    static NodeType get_node_type(void *node);
    static void set_node_type(void *node, NodeType type);

    static void initialize_internal_node(void *node);
    static void internal_node_insert(Table *table, uint32_t parent_page_num,
                                     uint32_t child_page_num);
    static uint32_t *internal_node_num_keys(void *node);
    static uint32_t *internal_node_right_child(void *node);
    static uint32_t *internal_node_cell(void *node, uint32_t cell_num);
    static uint32_t *internal_node_child(void *node, uint32_t child_num);
    static uint32_t *internal_node_key(void *node, uint32_t key_num);
    static void update_internal_node_key(void *node, uint32_t old_key, uint32_t new_key);
    static uint32_t *node_parent(void *node);
    static uint32_t get_node_max_key(void *node);
    static bool is_node_root(void *node);
    static void set_node_root(void *node, bool is_root);

    static void print_tree(Pager pager, uint32_t page_num, uint32_t indentation_level);
};
;

void print_constants();
#endif

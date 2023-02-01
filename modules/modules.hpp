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
    NODE_NOT_INITIALIZED,
    NODE_INTERNAL,
    NODE_LEAF,
};

// Classes
class InputBuffer;
class Row;
class Statement;
class Pager;
class Table;
class Node;
class NotInitializedNode;
class LeafNode;
class InternalNode;

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
    Node *nodes[TABLE_MAX_PAGES];

    Pager();
    void connect_file(const char *filenameIn);

    void set_node(uint32_t page_num, Node *node);
    template <class T>
    T *get_node(uint32_t page_num);
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

class Cursor
{
public:
    Table *table;
    uint32_t page_num;
    uint32_t cell_num;
    bool end_of_table; // Indicates a position one past the last element

    Cursor();
    Cursor(Table *table);

    static Cursor table_start(Table *tableIn);
    Row *position();
    void advance();

    void table_find(uint32_t key);
    void leaf_node_find(uint32_t key);
    void internal_node_find(uint32_t key);

    void leaf_node_insert(uint32_t key, Row value);
    void leaf_node_split_and_insert(uint32_t key, Row value);

    void internal_node_insert(uint32_t parent_page_num,
                                     uint32_t child_page_num);

    static void print_tree(Pager pager, uint32_t page_num, uint32_t indentation_level);
};

extern const uint8_t COMMON_NODE_HEADER_SIZE;
extern const uint32_t LEAF_NODE_HEADER_SIZE;
extern const uint32_t LEAF_NODE_CELL_SIZE;
extern const uint32_t LEAF_NODE_SPACE_FOR_CELLS;

class Node
{
public:
    NodeType type;
    uint32_t node_parent_page_num;
    bool is_root;

    Node();
};

class NotInitializedNode : public Node
{
public:
    NotInitializedNode();
};

#define LEAF_NODE_MAX_CELLS 13

class LeafNode : public Node
{
public:
    uint32_t num_cells;
    uint32_t next_leaf_page_num;
    uint32_t keys[LEAF_NODE_MAX_CELLS];
    Row values[LEAF_NODE_MAX_CELLS];

    LeafNode();
    LeafNode(char *page); // deserialize
    static LeafNode *clone(LeafNode *old_node);

    uint32_t get_max_key();
    void serialize();
};

/* Keep this small for testing */
#define INTERNAL_NODE_MAX_CELLS 3

class InternalNode : public Node
{
protected:
public:
    // Todo create class to cover uint32_t and name it PageNumber
    uint32_t num_keys;
    uint32_t right_child_page_num;
    uint32_t keys[INTERNAL_NODE_MAX_CELLS];
    uint32_t children_page_nums[INTERNAL_NODE_MAX_CELLS];

    InternalNode();
    InternalNode(char *page); // deserialize

    uint32_t get_internal_node_child(uint32_t child_num);
    void update_internal_node_key(uint32_t old_key, uint32_t new_key);
    uint32_t find_child(uint32_t key);

    uint32_t get_max_key();
    void serialize();

};

void print_constants();
#endif

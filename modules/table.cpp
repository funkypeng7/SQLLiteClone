#include "modules.hpp"

Table::Table() {}

Table::Table(const char *filename)
{
    pager.connect_file(filename);
    root_page_num = 0;

    if (pager.num_pages == 0)
    {
        // New database file. Initialize page 0 as leaf node.
        void *root_node = pager.get_page(0);
        Cursor::initialize_leaf_node(root_node);
        Cursor::set_node_root(root_node, true);
    }
}

void Table::db_close()
{
    for (uint32_t i = 0; i < pager.num_pages; i++)
    {
        if (pager.pages[i] == NULL)
        {
            continue;
        }
        pager.flush(i);
        pager.pages[i] = NULL;
    }
}

ExecuteResult Table::execute_statement(Statement statement)
{
    switch (statement.type)
    {
    case (STATEMENT_INSERT):
        return execute_insert(statement);
    case (STATEMENT_SELECT):
        return execute_select(statement);
    }
}

ExecuteResult Table::execute_insert(Statement statement)
{
    char *node = pager.get_page(root_page_num);
    uint32_t num_cells = *Cursor::leaf_node_num_cells(node);
    uint32_t key_to_insert = statement.row_to_insert.id;
    Cursor cursor = Cursor::table_find(this, key_to_insert);

    if (cursor.cell_num < num_cells)
    {
        uint32_t key_at_index = *Cursor::leaf_node_key(node, cursor.cell_num);
        if (key_at_index == key_to_insert)
        {
            return EXECUTE_DUPLICATE_KEY;
        }
    }

    char *slot = cursor.position();
    cursor.leaf_node_insert(statement.row_to_insert.id, statement.row_to_insert);

    return EXECUTE_SUCCESS;
}

ExecuteResult Table::execute_select(Statement statement)
{
    Row row;
    Cursor cursor = Cursor(this);

    while (!(cursor.end_of_table))
    {
        row.deserialize_row(cursor.position());
        row.print_row();
        cursor.advance();
    }
    return EXECUTE_SUCCESS;
}

void Table::create_new_root(uint32_t right_child_page_num)
{
    /*
    Handle splitting the root.
    Old root copied to new page, becomes left child.
    Address of right child passed in.
    Re-initialize root page to contain the new root node.
    New root node points to two children.
    */

    void *root = pager.get_page(root_page_num);
    void *right_child = pager.get_page(right_child_page_num);
    uint32_t left_child_page_num = pager.get_unused_page_num();
    void *left_child = pager.get_page(left_child_page_num);

    /* Left child has data copied from old root */
    memcpy(left_child, root, PAGE_SIZE);
    Cursor::set_node_root(left_child, false);

    /* Root node is a new internal node with one key and two children */
    Cursor::initialize_internal_node(root);
    Cursor::set_node_root(root, true);
    *Cursor::internal_node_num_keys(root) = 1;
    *Cursor::internal_node_child(root, 0) = left_child_page_num;
    uint32_t left_child_max_key = Cursor::get_node_max_key(left_child);
    *Cursor::internal_node_key(root, 0) = left_child_max_key;
    *Cursor::internal_node_right_child(root) = right_child_page_num;
}
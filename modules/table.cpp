#include "modules.hpp"

Table::Table() {}

Table::Table(const char *filename)
{
    pager = Pager();
    pager.connect_file(filename);
    root_page_num = 0;

    if (pager.num_pages == 0)
    {
        // New database file. Initialize page 0 as leaf node.
        LeafNode *root_node = new LeafNode();
        root_node->is_root = true;
        pager.set_node(0, root_node);
    }
}

void Table::db_close()
{
    for (uint32_t i = 0; i < pager.num_pages; i++)
    {
        if (pager.nodes[i] == NULL)
        {
            continue;
        }
        pager.flush(i);
        pager.nodes[i] = NULL;
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
    LeafNode *node = pager.get_node<LeafNode>(root_page_num);
    uint32_t num_cells = node->num_cells;
    uint32_t key_to_insert = statement.row_to_insert.id;
    Cursor cursor = Cursor(this);
    cursor.table_find(key_to_insert);

    if (cursor.cell_num < num_cells)
    {
        uint32_t key_at_index = node->keys[cursor.cell_num];
        if (key_at_index == key_to_insert)
        {
            return EXECUTE_DUPLICATE_KEY;
        }
    }
    cursor.leaf_node_insert(statement.row_to_insert.id, statement.row_to_insert);

    return EXECUTE_SUCCESS;
}

ExecuteResult Table::execute_select(Statement statement)
{
    Row *row;
    Cursor cursor = Cursor::table_start(this);

    while (!(cursor.end_of_table))
    {
        row = cursor.position();
        row->print_row();
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

    LeafNode *root = pager.get_node<LeafNode>(root_page_num);
    LeafNode *right_child = pager.get_node<LeafNode>(right_child_page_num);

    uint32_t left_child_page_num = pager.get_unused_page_num();

    /* Left child has data copied from old root */
    LeafNode *left_child = LeafNode::clone(root);
    left_child->is_root = false;
    pager.set_node(left_child_page_num, left_child);

    /* Root node is a new internal node with one key and two children */
    InternalNode *new_root = new InternalNode();
    new_root->is_root = true;
    new_root->num_keys = 1;
    uint32_t left_child_max_key = left_child->get_max_key();
    new_root->keys[0] = left_child_max_key;
    new_root->set_child_page_num(0, left_child_page_num);

    new_root->right_child_page_num = right_child_page_num;
    pager.set_node(root_page_num, new_root);

    left_child->node_parent_page_num = root_page_num;
    right_child->node_parent_page_num = root_page_num;
}

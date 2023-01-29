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

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        void *page = pager.pages[i];
        if (page)
        {
            pager.pages[i] = NULL;
        }
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
    if ((*Cursor::leaf_node_num_cells(node) >= LEAF_NODE_MAX_CELLS))
    {
        return EXECUTE_TABLE_FULL;
    }
    Cursor cursor = Cursor(this, true);
    char *slot = cursor.position();
    cursor.leaf_node_insert(statement.row_to_insert.id, statement.row_to_insert);

    return EXECUTE_SUCCESS;
}

ExecuteResult Table::execute_select(Statement statement)
{
    Row row;
    Cursor cursor = Cursor(this, false);

    while (!(cursor.end_of_table))
    {
        row.deserialize_row(cursor.position());
        row.print_row();
        cursor.advance();
    }
    return EXECUTE_SUCCESS;
}

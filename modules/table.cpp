#include "modules.hpp"

Table::Table(const char *filename)
{
    pager = Pager(filename);
    num_rows = pager.file_length / ROW_SIZE;

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        pages[i] = NULL;
    }
}

void Table::db_close()
{
    uint32_t num_full_pages = num_rows / ROWS_PER_PAGE;
    for (uint32_t i = 0; i < num_full_pages; i++)
    {
        if (pager.pages[i] == NULL)
        {
            continue;
        }
        pager.flush(i, PAGE_SIZE);
        pager.pages[i] = NULL;
    }
    // There may be a partial page to write to the end of the file
    // This should not be needed after we switch to a B-tree
    uint32_t num_additional_rows = num_rows % ROWS_PER_PAGE;
    if (num_additional_rows > 0)
    {
        uint32_t page_num = num_full_pages;
        if (pager.pages[page_num] != NULL)
        {
            pager.flush(page_num, num_additional_rows * ROW_SIZE);
            pager.pages[page_num] = NULL;
        }
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

void *Table::row_slot(uint32_t row_num)
{
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void *page = pager.get_page(page_num);
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return static_cast<char *>(page) + byte_offset;
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
    if (num_rows >= TABLE_MAX_ROWS)
    {
        return EXECUTE_TABLE_FULL;
    }

    statement.row_to_insert.serialize_row(row_slot(num_rows));
    num_rows += 1;

    return EXECUTE_SUCCESS;
}

ExecuteResult Table::execute_select(Statement statement)
{
    Row row;
    for (uint32_t i = 0; i < num_rows; i++)
    {
        row.deserialize_row(row_slot(i));
        row.print_row();
    }
    return EXECUTE_SUCCESS;
}

#include "modules.hpp"

Table::Table()
{
    num_rows = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        pages[i] = NULL;
    }
}

void* Table::row_slot(uint32_t row_num) {
  uint32_t page_num = row_num / ROWS_PER_PAGE;
  void* page = pages[page_num];
  if (page == NULL) {
    // Allocate memory only when we try to access page
    page = pages[page_num] = malloc(PAGE_SIZE);
  }
  uint32_t row_offset = row_num % ROWS_PER_PAGE;
  uint32_t byte_offset = row_offset * ROW_SIZE;
  return static_cast<char*>(page) + byte_offset;
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

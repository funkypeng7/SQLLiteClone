#include "modules.hpp"

Cursor::Cursor(Table *tableIn, bool startAtEnd)
{
    table = tableIn;
    if (startAtEnd)
    {
        row_num = table->num_rows;
        end_of_table = true;
    }
    else
    {
        row_num = 0;
        end_of_table = (table->num_rows == 0);
    }
}

char *Cursor::position()
{
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void *page = table->pager.get_page(page_num);
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return static_cast<char *>(page) + byte_offset;
}

void Cursor::advance()
{
    row_num += 1;
    if (row_num >= table->num_rows)
    {
        end_of_table = true;
    }
}
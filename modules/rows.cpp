#include "modules.hpp"

void serialize_row(Row* source, void* destination) {
  memcpy(static_cast<char*>(destination) + ID_OFFSET, &(source->id), ID_SIZE);
  memcpy(static_cast<char*>(destination) + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
  memcpy(static_cast<char*>(destination) + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination) {
  memcpy(&(destination->id), static_cast<char*>(source) + ID_OFFSET, ID_SIZE);
  memcpy(&(destination->username), static_cast<char*>(source) + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&(destination->email), static_cast<char*>(source) + EMAIL_OFFSET, EMAIL_SIZE);
}

void* row_slot(Table* table, uint32_t row_num) {
  uint32_t page_num = row_num / ROWS_PER_PAGE;
  void* page = table->pages[page_num];
  if (page == NULL) {
    // Allocate memory only when we try to access page
    page = table->pages[page_num] = malloc(PAGE_SIZE);
  }
  uint32_t row_offset = row_num % ROWS_PER_PAGE;
  uint32_t byte_offset = row_offset * ROW_SIZE;
  return static_cast<char*>(page) + byte_offset;
}
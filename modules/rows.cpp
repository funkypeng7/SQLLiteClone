#include "modules.hpp"

void Row::serialize_row(char *destination)
{
  memcpy(static_cast<char *>(destination) + ID_OFFSET, &id, ID_SIZE);
  memcpy(static_cast<char *>(destination) + USERNAME_OFFSET, &username, USERNAME_SIZE);
  memcpy(static_cast<char *>(destination) + EMAIL_OFFSET, &email, EMAIL_SIZE);
}

void Row::deserialize_row(void *source)
{
  memcpy(&id, static_cast<char *>(source) + ID_OFFSET, ID_SIZE);
  memcpy(&username, static_cast<char *>(source) + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&email, static_cast<char *>(source) + EMAIL_OFFSET, EMAIL_SIZE);
}

void Row::print_row()
{
  printf("(%d, %s, %s)\n", id, username, email);
}
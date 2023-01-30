#include "modules.hpp"

Pager::Pager()
{
    filename = "";
    file_length = 0;
    num_pages = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        pages[i] = NULL;
    }
}

void Pager::connect_file(const char *filenameIn)
{
    filename = filenameIn;
    std::ifstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open())
    {
        std::ofstream fileOut(filename);
        fileOut.close();

        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open())
        {
            printf("Unable to open file.\n");
            exit(EXIT_FAILURE);
        }
    }

    std::streampos fsize = file.tellg();
    file.seekg(0, std::ios::end);
    file_length = file.tellg() - fsize;

    num_pages = file_length / PAGE_SIZE;
    if (file_length % PAGE_SIZE != 0)
    {
        printf("Db file is not a whole number of pages. Corrupt file.\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        pages[i] = NULL;
    }
    file.close();
}

char *Pager::get_page(uint32_t page_num)
{
    if (page_num > TABLE_MAX_PAGES)
    {
        printf("Tried to fetch page number out of bounds. %d > %d\n", page_num,
               TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }

    if (pages[page_num] == NULL)
    {
        char *page = new char[PAGE_SIZE];
        if (page_num <= num_pages)
        {
            std::ifstream file(filename, std::ios::in | std::ios::binary);
            if (!file.is_open())
            {
                printf("Unable to open file\n");
                exit(EXIT_FAILURE);
            }

            file.seekg(page_num * PAGE_SIZE, std::ios::beg);
            if (!file.is_open())
            {
                printf("Unable to open file\n");
                exit(EXIT_FAILURE);
            }

            file.read(page, PAGE_SIZE);
            std::streamsize bytes_read = file.gcount();
            file.close();
        }
        pages[page_num] = page;

        if (page_num >= num_pages)
        {
            num_pages = page_num + 1;
        }
    }
    return pages[page_num];
}

void Pager::flush(uint32_t page_num)
{
    if (pages[page_num] == NULL)
    {
        printf("Tried to flush null page\n");
        exit(EXIT_FAILURE);
    }
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    file.seekp(page_num * PAGE_SIZE, std::ios::beg);
    if (!file.good())
    {
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    file.write(pages[page_num], PAGE_SIZE);

    if ((file.rdstate() & std::ofstream::failbit) != 0)
    {
        printf("Error writing: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

/*
Until we start recycling free pages, new pages will always
go onto the end of the database file
*/
uint32_t Pager::get_unused_page_num() { return num_pages; }
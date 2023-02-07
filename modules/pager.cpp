#include "modules.hpp"

Pager::Pager()
{
    filename = "";
    file_length = 0;
    num_pages = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        nodes[i] = NULL;
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
        nodes[i] = NULL;
    }
    file.close();
}

void Pager::set_node(uint32_t page_num, Node *node)
{
    get_node<Node>(page_num);
    delete nodes[page_num];
    nodes[page_num] = node;
}

template <class T>
T *Pager::get_node(uint32_t page_num)
{
    // Make sure the class T is an instance of Node
    static_assert(std::is_base_of<Node, T>::value);

    if (page_num > TABLE_MAX_PAGES)
    {
        printf("Tried to fetch page number out of bounds. %d > %d\n", page_num,
               TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }
    if (nodes[page_num] == NULL)
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

        // Work out type of page and create node
        Node *node_to_return;

        uint32_t raw_type = *(uint8_t*)page;
        NodeType node_type = (NodeType)raw_type;
        if(raw_type < 0 || raw_type > 2) node_type = NODE_NOT_INITIALIZED;

        if (node_type == NODE_NOT_INITIALIZED && (typeid(T) == typeid(NotInitializedNode) || typeid(T) == typeid(Node)))
        {
            node_to_return = new NotInitializedNode();
        }
        else if (node_type == NODE_LEAF && (typeid(T) == typeid(LeafNode) || typeid(T) == typeid(Node)))
        {
            node_to_return = new LeafNode(page);
        }
        else if (node_type == NODE_INTERNAL && (typeid(T) == typeid(InternalNode) || typeid(T) == typeid(Node)))
        {
            node_to_return = new InternalNode(page);
        }
        else
        {
            throw std::runtime_error("Pager::get_page is requesting incorrect type of node");
        }

        nodes[page_num] = static_cast<Node *>(node_to_return);

        if (page_num >= num_pages)
        {
            num_pages = page_num + 1;
        }
    }

    return static_cast<T *>(nodes[page_num]);
};

template Node *Pager::get_node<Node>(uint32_t);
template NotInitializedNode *Pager::get_node<NotInitializedNode>(uint32_t);
template LeafNode *Pager::get_node<LeafNode>(uint32_t);
template InternalNode *Pager::get_node<InternalNode>(uint32_t);

void Pager::flush(uint32_t page_num)
{
    if (nodes[page_num] == NULL)
    {
        printf("Tried to flush null page\n");
        exit(EXIT_FAILURE);
    }

    Node *node = get_node<Node>(page_num);
    char *page;
    switch (node->type) {
        case NODE_LEAF:
            page = static_cast<LeafNode*>(node)->serialize();
            break;
        case NODE_INTERNAL:
            page = static_cast<InternalNode*>(node)->serialize();
            break;
        default:
            printf("Cannot serialize blank node or not intilized node");
            exit(EXIT_FAILURE);
    }

    std::ofstream file(filename, std::ios::out | std::ios::binary);
    file.seekp(page_num * PAGE_SIZE, std::ios::beg);
    if (!file.good())
    {
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    file.write(page, PAGE_SIZE);

    if ((file.rdstate() & std::ofstream::failbit) != 0)
    {
        printf("Error writing: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    delete page;
    delete nodes[page_num];
    exit(EXIT_SUCCESS);
}

/*
Until we start recycling free pages, new pages will always
go onto the end of the database file
*/
uint32_t Pager::get_unused_page_num() { return num_pages; }

#include "modules.hpp"

Cursor::Cursor()
{
    table = NULL;
    page_num = 0;
    cell_num = 0;
}

Cursor::Cursor(Table *tableIn)
{
    table = tableIn;
    page_num = table->root_page_num;
    void *root_node = table->pager.get_page(table->root_page_num);
    uint32_t num_cells = *leaf_node_num_cells(root_node);
    cell_num = 0;
    end_of_table = (num_cells == 0);
}

Cursor Cursor::table_find(Table *table, uint32_t key)
{
    uint32_t root_page_num = table->root_page_num;
    void *root_node = table->pager.get_page(root_page_num);

    if (get_node_type(root_node) == NODE_LEAF)
    {
        return Cursor::leaf_node_find(table, root_page_num, key);
    }
    else
    {
        printf("Need to implement searching an internal node\n");
        exit(EXIT_FAILURE);
    }
}

Cursor Cursor::leaf_node_find(Table *table, uint32_t page_num, uint32_t key)
{
    void *node = table->pager.get_page(page_num);
    uint32_t num_cells = *Cursor::leaf_node_num_cells(node);

    Cursor cursor;
    cursor.table = table;
    cursor.page_num = page_num;

    // Binary search
    uint32_t min_index = 0;
    uint32_t one_past_max_index = num_cells;
    while (one_past_max_index != min_index)
    {
        uint32_t index = (min_index + one_past_max_index) / 2;
        uint32_t key_at_index = *Cursor::leaf_node_key(node, index);
        if (key == key_at_index)
        {
            cursor.cell_num = index;
            return cursor;
        }
        if (key < key_at_index)
        {
            one_past_max_index = index;
        }
        else
        {
            min_index = index + 1;
        }
    }

    cursor.cell_num = min_index;
    return cursor;
}

char *Cursor::position()
{
    void *page = table->pager.get_page(page_num);
    return leaf_node_value(page, cell_num);
}

void Cursor::advance()
{
    void *node = table->pager.get_page(page_num);
    cell_num += 1;

    if (cell_num >= *leaf_node_num_cells(node))
    {
        end_of_table = true;
    }
}

/*
 * Common Node Header Layout
 */
const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
const uint8_t COMMON_NODE_HEADER_SIZE =
    NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

/*
 * Leaf Node Header Layout
 */
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE =
    COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;

/*
 * Leaf Node Body Layout
 */
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET =
    LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS =
    LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

void Cursor::leaf_node_insert(uint32_t key, Row value)
{
    void *node = table->pager.get_page(page_num);

    uint32_t num_cells = *leaf_node_num_cells(node);
    if (num_cells >= LEAF_NODE_MAX_CELLS)
    {
        // Node full
        printf("Need to implement splitting a leaf node.\n");
        exit(EXIT_FAILURE);
    }

    if (cell_num < num_cells)
    {
        // Make room for new cell
        for (uint32_t i = num_cells; i > cell_num; i--)
        {
            memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1),
                   LEAF_NODE_CELL_SIZE);
        }
    }

    *(leaf_node_num_cells(node)) += 1;
    *(leaf_node_key(node, cell_num)) = key;

    value.serialize_row(leaf_node_value(node, cell_num));
}

char *Cursor::leaf_node_num_cells(void *node)
{
    return static_cast<char *>(node) + LEAF_NODE_NUM_CELLS_OFFSET;
}

char *Cursor::leaf_node_cell(void *node, uint32_t cell_num)
{
    return static_cast<char *>(node) + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

char *Cursor::leaf_node_key(void *node, uint32_t cell_num)
{
    return leaf_node_cell(node, cell_num);
}

char *Cursor::leaf_node_value(void *node, uint32_t cell_num)
{
    return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void Cursor::initialize_leaf_node(void *node)
{
    set_node_type(node, NODE_LEAF);
    *leaf_node_num_cells(node) = 0;
}

NodeType Cursor::get_node_type(void *node)
{
    uint8_t value = *((uint8_t *)(static_cast<char *>(node) + NODE_TYPE_OFFSET));
    return (NodeType)value;
}

void Cursor::set_node_type(void *node, NodeType type)
{
    *((uint8_t *)(static_cast<char *>(node) + NODE_TYPE_OFFSET)) = (uint8_t)type;
}

void Cursor::print_leaf_node(void *node)
{
    uint32_t num_cells = *leaf_node_num_cells(node);
    printf("leaf (size %d)\n", num_cells);
    for (uint32_t i = 0; i < num_cells; i++)
    {
        uint32_t key = *leaf_node_key(node, i);
        printf("  - %d : %d\n", i, key);
    }
}

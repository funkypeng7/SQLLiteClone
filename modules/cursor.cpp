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
        return leaf_node_find(table, root_page_num, key);
    }
    else
    {
        return internal_node_find(table, root_page_num, key);
    }
}

Cursor Cursor::internal_node_find(Table *table, uint32_t page_num, uint32_t key)
{
    void *node = table->pager.get_page(page_num);
    uint32_t num_keys = *internal_node_num_keys(node);

    /* Binary search to find index of child to search */
    uint32_t min_index = 0;
    uint32_t max_index = num_keys; /* there is one more child than key */

    while (min_index != max_index)
    {
        uint32_t index = (min_index + max_index) / 2;
        uint32_t key_to_right = *internal_node_key(node, index);
        if (key_to_right >= key)
        {
            max_index = index;
        }
        else
        {
            min_index = index + 1;
        }
    }

    uint32_t child_num = *internal_node_child(node, min_index);
    void *child = table->pager.get_page(child_num);
    switch (get_node_type(child))
    {
    case NODE_LEAF:
        return leaf_node_find(table, child_num, key);
    case NODE_INTERNAL:
        return internal_node_find(table, child_num, key);
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

const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) / 2;
const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT =
    (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;

void Cursor::leaf_node_insert(uint32_t key, Row value)
{
    void *node = table->pager.get_page(page_num);

    uint32_t num_cells = *leaf_node_num_cells(node);
    if (num_cells >= LEAF_NODE_MAX_CELLS)
    {
        leaf_node_split_and_insert(key, value);
        return;
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
    set_node_root(node, false);
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

void Cursor::leaf_node_split_and_insert(uint32_t key, Row value)
{
    /*
    Create a new node and move half the cells over.
    Insert the new value in one of the two nodes.
    Update parent or create a new parent.
    */

    void *old_node = table->pager.get_page(page_num);
    uint32_t new_page_num = table->pager.get_unused_page_num();
    void *new_node = table->pager.get_page(new_page_num);
    initialize_leaf_node(new_node);

    /*
    All existing keys plus new key should be divided
    evenly between old (left) and new (right) nodes.
    Starting from the right, move each key to correct position.
    */
    for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--)
    {
        void *destination_node;
        if (i >= LEAF_NODE_LEFT_SPLIT_COUNT)
        {
            destination_node = new_node;
        }
        else
        {
            destination_node = old_node;
        }
        uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
        char *destination = leaf_node_cell(destination_node, index_within_node);

        if (i == cell_num)
        {
            value.serialize_row(destination);
        }
        else if (i > cell_num)
        {
            memcpy(destination, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
        }
        else
        {
            memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
        }
    }

    /* Update cell count on both leaf nodes */
    *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

    if (is_node_root(old_node))
    {
        return table->create_new_root(new_page_num);
    }
    else
    {
        printf("Need to implement updating parent after split\n");
        exit(EXIT_FAILURE);
    }
}

/*
 * Internal Node Header Layout
 */
const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET =
    INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE;
const uint32_t INTERNAL_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE +
                                           INTERNAL_NODE_NUM_KEYS_SIZE +
                                           INTERNAL_NODE_RIGHT_CHILD_SIZE;

/*
 * Internal Node Body Layout
 */
const uint32_t INTERNAL_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CELL_SIZE =
    INTERNAL_NODE_CHILD_SIZE + INTERNAL_NODE_KEY_SIZE;

void Cursor::initialize_internal_node(void *node)
{
    set_node_type(node, NODE_INTERNAL);
    set_node_root(node, false);
    *internal_node_num_keys(node) = 0;
}

uint32_t *Cursor::internal_node_num_keys(void *node)
{
    return (uint32_t *)(static_cast<char *>(node) + INTERNAL_NODE_NUM_KEYS_OFFSET);
}

uint32_t *Cursor::internal_node_right_child(void *node)
{
    return (uint32_t *)(static_cast<char *>(node) + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

uint32_t *Cursor::internal_node_cell(void *node, uint32_t cell_num)
{
    return (uint32_t *)(static_cast<char *>(node) + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE);
}

uint32_t *Cursor::internal_node_child(void *node, uint32_t child_num)
{
    uint32_t num_keys = *internal_node_num_keys(node);
    if (child_num > num_keys)
    {
        printf("Tried to access child_num %d > num_keys %d\n", child_num, num_keys);
        exit(EXIT_FAILURE);
    }
    else if (child_num == num_keys)
    {
        return internal_node_right_child(node);
    }
    else
    {
        return internal_node_cell(node, child_num);
    }
}

uint32_t *Cursor::internal_node_key(void *node, uint32_t key_num)
{
    return internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE;
}

uint32_t Cursor::get_node_max_key(void *node)
{
    switch (get_node_type(node))
    {
    case NODE_INTERNAL:
        return *internal_node_key(node, *internal_node_num_keys(node) - 1);
    case NODE_LEAF:
        return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
    }
}

bool Cursor::is_node_root(void *node)
{
    uint8_t value = *((uint8_t *)(static_cast<char *>(node) + IS_ROOT_OFFSET));
    return (bool)value;
}

void Cursor::set_node_root(void *node, bool is_root)
{
    uint8_t value = is_root;
    *((uint8_t *)(static_cast<char *>(node) + IS_ROOT_OFFSET)) = value;
}

void indent(uint32_t level)
{
    for (uint32_t i = 0; i < level; i++)
    {
        printf("  ");
    }
}

void Cursor::print_tree(Pager pager, uint32_t page_num, uint32_t indentation_level)
{
    void *node = pager.get_page(page_num);
    uint32_t num_keys, child;

    switch (get_node_type(node))
    {
    case (NODE_LEAF):
        num_keys = *leaf_node_num_cells(node);
        indent(indentation_level);
        printf("- leaf (size %d)\n", num_keys);
        for (uint32_t i = 0; i < num_keys; i++)
        {
            indent(indentation_level + 1);
            printf("- %d\n", *leaf_node_key(node, i));
        }
        break;
    case (NODE_INTERNAL):
        num_keys = *internal_node_num_keys(node);
        indent(indentation_level);
        printf("- internal (size %d)\n", num_keys);
        for (uint32_t i = 0; i < num_keys; i++)
        {
            child = *internal_node_child(node, i);
            print_tree(pager, child, indentation_level + 1);

            indent(indentation_level + 1);
            printf("- key %d\n", *internal_node_key(node, i));
        }
        child = *internal_node_right_child(node);
        print_tree(pager, child, indentation_level + 1);
        break;
    }
}
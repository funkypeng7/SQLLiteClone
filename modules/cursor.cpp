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
    page_num = 0;
    cell_num = 0;
}

Cursor Cursor::table_start(Table *tableIn)
{
    Cursor cursor = Cursor(tableIn);
    cursor.table_find(0);

    LeafNode *node = tableIn->pager.get_node<LeafNode>(cursor.page_num);
    cursor.end_of_table = node->num_cells == 0;

    return cursor;
}

void Cursor::table_find(uint32_t key)
{
    page_num = table->root_page_num;
    Node *root_node = table->pager.get_node<Node>(page_num);

    if (root_node->type == NODE_LEAF)
    {
        return leaf_node_find(key);
    }
    else
    {
        return internal_node_find(key);
    }
}

void Cursor::leaf_node_find(uint32_t key)
{
    LeafNode *node = table->pager.get_node<LeafNode>(page_num);

    // Binary search
    uint32_t min_index = 0;
    uint32_t one_past_max_index = node->num_cells;
    while (one_past_max_index != min_index)
    {
        uint32_t index = (min_index + one_past_max_index) / 2;
        uint32_t key_at_index = node->keys[index];
        if (key == key_at_index)
        {
            cell_num = index;
            return;
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

    cell_num = min_index;
}

void Cursor::internal_node_find(uint32_t key)
{
    InternalNode *node = table->pager.get_node<InternalNode>(page_num);

    uint32_t child_index = node->find_child(key);
    page_num = node->children_page_nums[child_index];

    Node *child = table->pager.get_node<Node>(page_num);
    switch (child->type)
    {
    case NODE_LEAF:
        return leaf_node_find(key);
    case NODE_INTERNAL:
        return internal_node_find(key);
    case NODE_NOT_INITIALIZED:
        throw std::runtime_error("Internal node find, found child key of un_initialized node");
    }
}

Row *Cursor::position()
{
    LeafNode *node = table->pager.get_node<LeafNode>(page_num);
    return &node->values[cell_num];
}

void Cursor::advance()
{
    LeafNode *node = table->pager.get_node<LeafNode>(page_num);
    cell_num += 1;

    if (cell_num >= node->num_cells)
    {
        /* Advance to next leaf node */
        uint32_t next_page_num = node->next_leaf_page_num;
        if (next_page_num == 0)
        {
            /* This was rightmost leaf */
            end_of_table = true;
        }
        else
        {
            page_num = next_page_num;
            cell_num = 0;
        }
    }
}

// TODO: Translate all info to #define

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
const uint32_t LEAF_NODE_NEXT_LEAF_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NEXT_LEAF_OFFSET =
    LEAF_NODE_NUM_CELLS_OFFSET + LEAF_NODE_NUM_CELLS_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE +
                                       LEAF_NODE_NUM_CELLS_SIZE +
                                       LEAF_NODE_NEXT_LEAF_SIZE;

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
// const uint32_t LEAF_NODE_MAX_CELLS =
//     LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) / 2;
const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT =
    (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;

void Cursor::leaf_node_insert(uint32_t key, Row row)
{
    LeafNode *node = table->pager.get_node<LeafNode>(page_num);
    if (node->num_cells >= LEAF_NODE_MAX_CELLS)
    {
        leaf_node_split_and_insert(key, row);
        return;
    }

    if (cell_num < node->num_cells)
    {
        // Make room for new cell
        for (uint32_t i = node->num_cells; i > cell_num; i--)
        {
            node->keys[i] = node->keys[i - 1];
            node->values[i] = node->values[i - 1];
        }
    }

    node->num_cells++;

    node->keys[cell_num] = key;
    node->values[cell_num] = row;
}

void Cursor::leaf_node_split_and_insert(uint32_t key, Row row)
{
    /*
    Create a new node and move half the cells over.
    Insert the new value in one of the two nodes.
    Update parent or create a new parent.
    */

    LeafNode *old_node = table->pager.get_node<LeafNode>(page_num);
    uint32_t old_max = old_node->get_max_key();

    uint32_t new_page_num = table->pager.get_unused_page_num();
    LeafNode *new_node = new LeafNode();
    new_node->node_parent_page_num = old_node->node_parent_page_num;
    new_node->next_leaf_page_num = old_node->next_leaf_page_num;

    table->pager.set_node(new_page_num, new_node);
    old_node->next_leaf_page_num = new_page_num;

    /*
    All existing keys plus new key should be divided
    evenly between old (left) and new (right) nodes.
    Starting from the right, move each key to correct position.
    */
    for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--)
    {
        LeafNode *destination_node;
        if (i >= LEAF_NODE_LEFT_SPLIT_COUNT)
        {
            destination_node = new_node;
        }
        else
        {
            destination_node = old_node;
        }
        uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;

        if (i == cell_num)
        {
            destination_node->keys[index_within_node] = key;
            destination_node->values[index_within_node] = row;
        }
        else if (i > cell_num)
        {
            destination_node->keys[index_within_node] = old_node->keys[index_within_node - 1];
            destination_node->values[index_within_node] = old_node->values[index_within_node - 1];
        }
        else
        {
            destination_node->keys[index_within_node] = old_node->keys[index_within_node];
            destination_node->values[index_within_node] = old_node->values[index_within_node];
        }
    }

    /* Update cell count on both leaf nodes */
    old_node->num_cells = LEAF_NODE_LEFT_SPLIT_COUNT;
    new_node->num_cells = LEAF_NODE_RIGHT_SPLIT_COUNT;

    if (old_node->is_root)
    {
        table->create_new_root(new_page_num);
        return;
    }
    else
    {
        uint32_t parent_page_num = old_node->node_parent_page_num;
        uint32_t new_max = old_node->get_max_key();
        InternalNode *parent = table->pager.get_node<InternalNode>(parent_page_num);

        parent->update_internal_node_key(old_max, new_max);
        internal_node_insert(parent_page_num, new_page_num);
        return;
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
/* Keep this small for testing */
// const uint32_t INTERNAL_NODE_MAX_CELLS = 3;

void Cursor::internal_node_insert(uint32_t parent_page_num, uint32_t child_page_num)
{
    /*
    Add a new child/key pair to parent that corresponds to child
    */

    InternalNode *parent = table->pager.get_node<InternalNode>(parent_page_num);
    LeafNode *child = table->pager.get_node<LeafNode>(child_page_num);

    uint32_t child_max_key = child->get_max_key();
    uint32_t index = parent->find_child(child_max_key);

    uint32_t original_num_keys = parent->num_keys;
    parent->num_keys = original_num_keys + 1;

    if (original_num_keys >= INTERNAL_NODE_MAX_CELLS)
    {
        printf("Need to implement splitting internal node\n");
        exit(EXIT_FAILURE);
    }

    uint32_t right_child_page_num = parent->right_child_page_num;
    LeafNode *right_child = table->pager.get_node<LeafNode>(right_child_page_num);

    if (child_max_key > right_child->get_max_key())
    {
        /* Replace right child */
        parent->keys[original_num_keys] = right_child->get_max_key();
        parent->children_page_nums[original_num_keys] = right_child_page_num;
        parent->right_child_page_num = child_page_num;
    }
    else
    {
        /* Make room for the new cell */
        for (uint32_t i = original_num_keys; i > index; i--)
        {
            parent->keys[i] = parent->keys[i - 1];
            parent->children_page_nums[i] = parent->children_page_nums[i - 1];
        }
        parent->keys[index] = child_max_key;
        parent->children_page_nums[index] = child_page_num;
    }
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
    Node *node = pager.get_node<Node>(page_num);
    uint32_t num_keys;
    switch (node->type)
    {
    case NODE_LEAF:
        num_keys = static_cast<LeafNode *>(node)->num_cells;
    case NODE_INTERNAL:
        num_keys = static_cast<InternalNode *>(node)->num_keys;
    }
    uint32_t child;

    switch (node->type)
    {
    case (NODE_LEAF):
    {
        LeafNode *leafNode = static_cast<LeafNode *>(node);
        indent(indentation_level);
        printf("- leaf (size %d)\n", num_keys);
        for (uint32_t i = 0; i < num_keys; i++)
        {
            indent(indentation_level + 1);
            printf("- %d\n", leafNode->keys[i]);
        }
        break;
    }
    case (NODE_INTERNAL):
    {
        InternalNode *internalNode = static_cast<InternalNode *>(node);

        indent(indentation_level);
        printf("- internal (size %d)\n", num_keys);
        for (uint32_t i = 0; i < num_keys; i++)
        {
            child = internalNode->children_page_nums[i];
            print_tree(pager, child, indentation_level + 1);

            indent(indentation_level + 1);
            printf("- key %d\n", internalNode->keys[i]);
        }
        child = internalNode->right_child_page_num;
        print_tree(pager, child, indentation_level + 1);
        break;
    }
    }
}

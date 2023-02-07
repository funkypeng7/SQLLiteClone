#include "modules.hpp"

LeafNode::LeafNode() : Node()
{
    type = NODE_LEAF;
    num_cells = 0;
    node_parent_page_num = 0;
    next_leaf_page_num = 0;

    for (int i = 0; i < LEAF_NODE_MAX_CELLS; i++)
    {
        keys[i] = 0;
    }
};


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

LeafNode::LeafNode(char *page) : Node(page) {
    memcpy(&num_cells, static_cast<char *>(page) + LEAF_NODE_NUM_CELLS_OFFSET, LEAF_NODE_NUM_CELLS_SIZE);
    memcpy(&next_leaf_page_num, static_cast<char *>(page) + LEAF_NODE_NEXT_LEAF_OFFSET, LEAF_NODE_NEXT_LEAF_SIZE);
    for (int i = 0; i < num_cells; i++)
    {
        uint32_t cell_offset = LEAF_NODE_HEADER_SIZE + LEAF_NODE_CELL_SIZE * i;
        memcpy(&keys[i], static_cast<char *>(page) + cell_offset, LEAF_NODE_KEY_SIZE);
        Row row;
        row.deserialize_row(page + cell_offset + LEAF_NODE_KEY_SIZE);
        values[i] = row;
    }
}

char *LeafNode::serialize()
{
    char *page = serialize_header();
    memcpy(static_cast<char *>(page) + LEAF_NODE_NUM_CELLS_OFFSET, &num_cells, LEAF_NODE_NUM_CELLS_SIZE);
    memcpy(static_cast<char *>(page) + LEAF_NODE_NEXT_LEAF_OFFSET, &next_leaf_page_num, LEAF_NODE_NEXT_LEAF_SIZE);
    for (int i = 0; i < num_cells; i++)
    {
        uint32_t cell_offset = LEAF_NODE_HEADER_SIZE + LEAF_NODE_CELL_SIZE * i;
        memcpy(static_cast<char *>(page) + cell_offset, &keys[i], LEAF_NODE_KEY_SIZE);
        values[i].serialize_row(page + cell_offset + LEAF_NODE_KEY_SIZE);
    }
    return page;
}

LeafNode *LeafNode::clone(LeafNode *old_node)
{
    LeafNode *new_node = new LeafNode();
    new_node->node_parent_page_num = old_node->node_parent_page_num;
    new_node->is_root = old_node->is_root;
    new_node->num_cells = old_node->num_cells;
    new_node->next_leaf_page_num = old_node->next_leaf_page_num;

    for (int i = 0; i < LEAF_NODE_MAX_CELLS; i++)
    {
        new_node->keys[i] = old_node->keys[i];
        new_node->values[i] = old_node->values[i];
    }

    return new_node;
}

uint32_t LeafNode::get_max_key()
{
    return keys[num_cells - 1];
};



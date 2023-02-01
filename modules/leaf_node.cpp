#include "modules.hpp"

LeafNode::LeafNode() : Node()
{
    type = NODE_LEAF;
    num_cells = 0;
    next_leaf_page_num = 0;

    for (int i = 0; i < LEAF_NODE_MAX_CELLS; i++)
    {
        keys[i] = 0;
    }
};

LeafNode::LeafNode(char *page) : Node() {
    // TODO
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

void LeafNode::serialize(){};

// void Cursor::initialize_leaf_node(void *node)
// {
//     set_node_type(node, NODE_LEAF);
//     set_node_root(node, false);
//     *leaf_node_num_cells(node) = 0;
//     *leaf_node_next_leaf(node) = 0;
// }

// char *Cursor::leaf_node_num_cells(void *node)
// {
//     return static_cast<char *>(node) + LEAF_NODE_NUM_CELLS_OFFSET;
// }

// char *Cursor::leaf_node_cell(void *node, uint32_t cell_num)
// {
//     return static_cast<char *>(node) + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
// }

// char *Cursor::leaf_node_key(void *node, uint32_t cell_num)
// {
//     return leaf_node_cell(node, cell_num);
// }

// char *Cursor::leaf_node_value(void *node, uint32_t cell_num)
// {
//     return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
// }

// uint32_t *Cursor::leaf_node_next_leaf(void *node)
// {
//     return (uint32_t *)(static_cast<char *>(node) + LEAF_NODE_NEXT_LEAF_OFFSET);
// }

// uint32_t Cursor::get_node_max_key(void *node)
// {
//     switch (get_node_type(node))
//     {
//     case NODE_INTERNAL:
//         return *internal_node_key(node, *internal_node_num_keys(node) - 1);
//     case NODE_LEAF:
//         return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
//     }
// }

#include "modules.hpp"

InternalNode::InternalNode() : Node()
{
    type = NODE_INTERNAL;
    num_keys = 0;
    right_child_page_num = 0;

    for (int i = 0; i < LEAF_NODE_MAX_CELLS; i++)
    {
        keys[i] = 0;
        children_page_nums[i] = 0;
    }
};

InternalNode::InternalNode(char *page) : Node() {
    // TODO
}

uint32_t InternalNode::get_internal_node_child(uint32_t child_num)
{
    if (child_num > num_keys)
    {
        printf("Tried to access child_num %d > num_keys %d\n", child_num, num_keys);
        exit(EXIT_FAILURE);
    }
    else if (child_num == num_keys)
    {
        return right_child_page_num;
    }
    else
    {
        return children_page_nums[child_num];
    }
}
void InternalNode::update_internal_node_key(uint32_t old_key, uint32_t new_key)
{
    uint32_t old_child_index = find_child(old_key);
    keys[old_child_index] = new_key;
}

uint32_t InternalNode::find_child(uint32_t key)
{
    /*
    Return the index of the child which should contain
    the given key.
    */


    /* Binary search */
    uint32_t min_index = 0;
    uint32_t max_index = num_keys; /* there is one more child than key */

    while (min_index != max_index)
    {
        uint32_t index = (min_index + max_index) / 2;
        uint32_t key_to_right = keys[index];
        if (key_to_right >= key)
        {
            max_index = index;
        }
        else
        {
            min_index = index + 1;
        }
    }

    return min_index;
}

uint32_t InternalNode::get_max_key()
{
    return keys[num_keys - 1];
};

void InternalNode::serialize(){};


// void Cursor::initialize_internal_node(void *node)
// {
//     set_node_type(node, NODE_INTERNAL);
//     set_node_root(node, false);
//     *internal_node_num_keys(node) = 0;
// }

// uint32_t *Cursor::internal_node_num_keys(void *node)
// {
//     return (uint32_t *)(static_cast<char *>(node) + INTERNAL_NODE_NUM_KEYS_OFFSET);
// }

// uint32_t *Cursor::internal_node_right_child(void *node)
// {
//     return (uint32_t *)(static_cast<char *>(node) + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
// }

// uint32_t *Cursor::internal_node_cell(void *node, uint32_t cell_num)
// {
//     return (uint32_t *)(static_cast<char *>(node) + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE);
// }

// uint32_t *Cursor::internal_node_child(void *node, uint32_t child_num)
// {
//     uint32_t num_keys = *internal_node_num_keys(node);
//     if (child_num > num_keys)
//     {
//         printf("Tried to access child_num %d > num_keys %d\n", child_num, num_keys);
//         exit(EXIT_FAILURE);
//     }
//     else if (child_num == num_keys)
//     {
//         return internal_node_right_child(node);
//     }
//     else
//     {
//         return internal_node_cell(node, child_num);
//     }
// }

// uint32_t *Cursor::internal_node_key(void *node, uint32_t key_num)
// {
//     return (uint32_t *)(static_cast<char*>((void *)internal_node_cell(node, key_num)) + INTERNAL_NODE_CHILD_SIZE);
// }

// void Cursor::update_internal_node_key(void *node, uint32_t old_key, uint32_t new_key)
// {
//     uint32_t old_child_index = internal_node_find_child(node, old_key);
//     *internal_node_key(node, old_child_index) = new_key;
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

// uint32_t Cursor::internal_node_find_child(void *node, uint32_t key)
// {
//     /*
//     Return the index of the child which should contain
//     the given key.
//     */
//     uint32_t num_keys = *internal_node_num_keys(node);

//     /* Binary search */
//     uint32_t min_index = 0;
//     uint32_t max_index = num_keys; /* there is one more child than key */

//     while (min_index != max_index)
//     {
//         uint32_t index = (min_index + max_index) / 2;
//         uint32_t key_to_right = *internal_node_key(node, index);
//         if (key_to_right >= key)
//         {
//             max_index = index;
//         }
//         else
//         {
//             min_index = index + 1;
//         }
//     }

//     return min_index;
// }

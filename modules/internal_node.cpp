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

InternalNode::InternalNode(char *page) : Node(page)
{
    memcpy(&num_keys, static_cast<char *>(page) + INTERNAL_NODE_NUM_KEYS_OFFSET, INTERNAL_NODE_NUM_KEYS_SIZE);
    memcpy(&right_child_page_num, static_cast<char *>(page) + INTERNAL_NODE_RIGHT_CHILD_OFFSET, INTERNAL_NODE_RIGHT_CHILD_SIZE);
    for (int i = 0; i < num_keys; i++)
    {
        uint32_t cell_offset = LEAF_NODE_HEADER_SIZE + LEAF_NODE_CELL_SIZE * i;
        memcpy(&keys[i], static_cast<char *>(page) + cell_offset, INTERNAL_NODE_KEY_SIZE);
        memcpy(&children_page_nums[i], static_cast<char *>(page) + cell_offset + INTERNAL_NODE_KEY_SIZE, INTERNAL_NODE_CHILD_SIZE);
    }
}

char *InternalNode::serialize()
{
    char *page = serialize_header();
    strncpy(page + INTERNAL_NODE_NUM_KEYS_OFFSET, (char *)num_keys, INTERNAL_NODE_NUM_KEYS_SIZE);
    strncpy(page + INTERNAL_NODE_RIGHT_CHILD_OFFSET, (char *)right_child_page_num, INTERNAL_NODE_RIGHT_CHILD_SIZE);
    for (int i = 0; i < num_keys; i++)
    {
        uint32_t cell_offset = INTERNAL_NODE_HEADER_SIZE + INTERNAL_NODE_CELL_SIZE * i;
        strncpy(page + cell_offset, (char *)keys[i], INTERNAL_NODE_KEY_SIZE);
        strncpy(page + cell_offset + INTERNAL_NODE_KEY_SIZE, (char *)children_page_nums[i], INTERNAL_NODE_CHILD_SIZE);
    }
    return page;
}

uint32_t InternalNode::get_child_page_num(uint32_t child_num)
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

void InternalNode::set_child_page_num(uint32_t index, uint32_t child_page_num)
{
    children_page_nums[index] = child_page_num;
}

void InternalNode::update_internal_node_key(uint32_t old_key, uint32_t new_key)
{
    uint32_t old_child_index = find_child_page_num(old_key);
    keys[old_child_index] = new_key;
}

uint32_t InternalNode::find_child_page_num(uint32_t key)
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


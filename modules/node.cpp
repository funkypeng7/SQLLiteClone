#include "modules.hpp"

Node::Node()
{
    node_parent_page_num = 0;
    is_root = false;
};

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

Node::Node(char *page)
{
    uint8_t typeOut;
    memcpy(&typeOut, static_cast<char *>(page) + NODE_TYPE_OFFSET, NODE_TYPE_SIZE);
    type = NodeType(typeOut);
    memcpy(&is_root, static_cast<char *>(page) + IS_ROOT_OFFSET, IS_ROOT_SIZE);
    memcpy(&node_parent_page_num, static_cast<char *>(page) + PARENT_POINTER_SIZE, PARENT_POINTER_SIZE);
}

char *Node::serialize_header()
{
    char *page = new char[PAGE_SIZE];
    memcpy(static_cast<char *>(page) + NODE_TYPE_OFFSET, &type , NODE_TYPE_SIZE);
    memcpy(static_cast<char *>(page) + IS_ROOT_OFFSET, &is_root, IS_ROOT_SIZE);
    memcpy(static_cast<char *>(page) + PARENT_POINTER_SIZE, &node_parent_page_num, PARENT_POINTER_SIZE);
    return page;
}

NotInitializedNode::NotInitializedNode() : Node()
{
    type = NODE_NOT_INITIALIZED;
};

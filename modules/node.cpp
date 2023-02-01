#include "modules.hpp"

Node::Node() {
    node_parent_page_num = 0;
    is_root = false;
};


NotInitializedNode::NotInitializedNode() : Node() {
    type = NODE_NOT_INITIALIZED;
};


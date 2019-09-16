#pragma once

#include "G_Tree.h"
#include "global.h"

namespace IGTree {
    extern G_Tree tree;
    void init();
    void read();
    void save();
    bool load();
};
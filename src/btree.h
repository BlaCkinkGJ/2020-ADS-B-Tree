#ifndef _B_TREE_H
#define _B_TREE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef B_TREE_DEALLOC_ITEM
#pragma message                                                                \
        "[WARN] I am not recommended to use dynamic allocation in `item->data`..."
#endif

#define B_TREE_MIN_DEGREE 2
#define B_TREE_NOT_FIND 0

#ifdef _2_3_TREE
// 2-3 tree
#define B_TREE_NR_CHILD 3
#define B_TREE_NR_KEYS 2
#else
#define B_TREE_NR_CHILD(DEG) (2 * (DEG)) // 4(2-3-4), 3(2-3)
#define B_TREE_NR_KEYS(DEG) (B_TREE_NR_CHILD(DEG) - 1) // 3(2-3-4), 2(2-3)
#endif

// 2-3-4 tree
// #define B_TREE_NR_CHILD 4
// #define B_TREE_NR_KEYS 3

#ifndef key_t
typedef unsigned int key_t;
#endif

#define pr_info(msg, ...)                                                      \
        fprintf(stderr, "[{%lfs} %s(%s):%d] " msg,                             \
                ((double)clock() / CLOCKS_PER_SEC), __FILE__, __func__,        \
                __LINE__, ##__VA_ARGS__)

struct btree_search_result {
        int index;
        struct btree_node *node;
};

struct btree_item {
        key_t key;
        void *data;
};

struct btree_node {
        int n; /**< Number of keys */
        bool is_leaf;

        struct btree_item *items;
        struct btree_node **child;
};

struct btree {
        int min_degree;
        struct btree_node *root;
};

struct btree *btree_alloc(int min_degree);
struct btree_search_result btree_search(struct btree *tree, key_t key);
void btree_insert(struct btree *tree, key_t key, void *data);
void btree_traverse(struct btree *tree);
int btree_delete(struct btree *tree, key_t key);
void btree_free(struct btree *tree);

#endif
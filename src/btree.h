#ifndef _B_TREE_H
#define _B_TREE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define B_TREE_MIN_DEGREE 2
#define B_TREE_NOT_FIND -1

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

        struct btree_item items[2 * B_TREE_MIN_DEGREE - 1];
        struct btree_node *child[2 * B_TREE_MIN_DEGREE];
};

struct btree {
        struct btree_node *root;
};

struct btree *btree_create(void);
struct btree_search_result btree_search(struct btree *T, key_t k);
void btree_insert(struct btree *tree, key_t key, void *data);

static inline struct btree_node *btree_alloc_node(void)
{
        return ((struct btree_node *)calloc(1, sizeof(struct btree_node)));
}

#endif
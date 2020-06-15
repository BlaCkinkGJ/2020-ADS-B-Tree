#include <stdlib.h>
#include <errno.h>
#include "btree.h"

struct btree *btree_create(void)
{
        struct btree *tree = (struct btree *)malloc(sizeof(struct btree));
        if (!tree) {
                pr_info("Allocation tree failed\n");
                goto exception;
        }
        struct btree_node *node = btree_alloc_node();
        if (!node) {
                pr_info("Allocation node failed\n");
                goto exception;
        }

        node->is_leaf = true;
        node->n = 0;

        tree->root = node;

        return tree;

exception:
        if (node) {
                free(node);
                tree->root = NULL;
        }

        if (tree) {
                free(tree);
        }

        return NULL;
}

static struct btree_search_result __btree_search(struct btree_node *x, key_t k)
{
        int i = 1;
        struct btree_search_result result;
        while (i <= x->n && k > x->items[i].key) {
                i = i + 1;
        }

        if (i <= x->n && k == x->items[i].key) {
                result.index = i;
                result.node = x;
                return result;
        } else if (x->is_leaf) {
                result.index = B_TREE_NOT_FIND;
                result.node = NULL;
                return result;
        } else {
                return __btree_search(x->child[i], k);
        }
}

struct btree_search_result btree_search(struct btree *T, key_t k)
{
        return __btree_search(T->root, k);
}

static void btree_split_child(struct btree_node *x, int i)
{
        struct btree_node *z = btree_alloc_node();
        struct btree_node *y = x->child[i];

        static const int t = B_TREE_MIN_DEGREE;

        z->is_leaf = y->is_leaf;
        z->n = t - 1;

        for (int j = 1; j <= t - 1; j++) {
                z->items[j] = y->items[j + t];
        }

        if (!y->is_leaf) {
                for (int j = 1; j <= t; j++) {
                        z->child[j] = y->child[j + t];
                }
        }

        y->n = t - 1;

        for (int j = x->n + 1; j >= i + 1; j--) {
                x->child[j + 1] = x->child[j];
        }
        x->child[i + 1] = z;

        for (int j = x->n; j >= i; j--) {
                x->items[j + 1] = x->items[j];
        }
        x->items[i] = y->items[t];
        x->n = x->n + 1;
}

static void btree_insert_non_full(struct btree_node *x, struct btree_item *k)
{
        int i = x->n;
        static const int t = B_TREE_MIN_DEGREE;
        if (x->is_leaf) {
                while (i >= 1 && k->key < x->items[i].key) {
                        printf("%d ", x->items[i].key);
                        x->items[i + 1] = x->items[i];
                        i = i - 1;
                }
                x->items[i + 1] = *k;
                x->n = x->n + 1;
        } else {
                while (i >= 1 && k->key < x->items[i].key) {
                        i = i - 1;
                }
                i = i + 1;
                if (x->child[i]->n == (2 * t - 1)) {
                        btree_split_child(x, i);
                        if (k->key > x->items[i].key) {
                                i = i + 1;
                        }
                }
                btree_insert_non_full(x->child[i], k);
        }
}

static void __btree_insert(struct btree *T, struct btree_item *k)
{
        struct btree_node *r = T->root;
        static const int t = B_TREE_MIN_DEGREE;
        if (r->n == (2 * t - 1)) {
                struct btree_node *s = btree_alloc_node();
                T->root = s;
                s->is_leaf = false;
                s->n = 0;
                s->child[1] = r;
                btree_split_child(s, 1);
                btree_insert_non_full(s, k);
        } else {
                btree_insert_non_full(r, k);
        }
}

void btree_insert(struct btree *tree, key_t key, void *data)
{
        struct btree_item item = { .key = key, .data = data };
        __btree_insert(tree, &item);
}

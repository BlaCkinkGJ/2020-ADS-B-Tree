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

        node->parent = NULL;
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
        int i = 0;
        struct btree_search_result result;
        while (i < x->n && k > x->items[i].key) {
                i = i + 1;
        }

        if (i < x->n && k == x->items[i].key) {
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
        static const int t = B_TREE_MIN_DEGREE;

        struct btree_node *z = btree_alloc_node();
        struct btree_node *y = x->child[i - 1];

        z->is_leaf = y->is_leaf;
#ifdef _2_3_TREE
        z->n = 0;
#else
        z->n = t - 1;
#endif

        for (int j = 0; j < t - 1; j++) {
                z->items[j] = y->items[j + t];
        }

        if (!y->is_leaf) {
                for (int j = 0; j < t; j++) {
                        z->child[j] = y->child[j + t];
                }
        }

        y->n = t - 1;

        for (int j = x->n; j >= i; j--) {
                x->child[j + 1] = x->child[j];
        }
        x->child[i] = z;

        for (int j = x->n; j >= i; j--) {
                x->items[j] = x->items[j - 1];
        }
        x->items[i - 1] = y->items[t - 1];
        x->n = x->n + 1;

        y->parent = x;
        z->parent = x;
}

static void btree_insert_non_full(struct btree_node *x, struct btree_item *k)
{
        int i = x->n;

        if (x->is_leaf) {
                while (i >= 1 && k->key < x->items[i - 1].key) {
                        x->items[i] = x->items[i - 1];
                        i = i - 1;
                }
                x->items[i] = *k;
                x->n = x->n + 1;
        } else {
                while (i >= 1 && k->key < x->items[i - 1].key) {
                        i = i - 1;
                }
                if (x->child[i]->n == B_TREE_NR_KEYS) {
                        btree_split_child(x, i + 1);
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
        if (r->n == B_TREE_NR_KEYS) {
                struct btree_node *s = btree_alloc_node();
                T->root = s;
                s->is_leaf = false;
                s->n = 0;
                s->child[0] = r;

                s->parent = NULL;
                r->parent = s;

                btree_split_child(s, 1);
                btree_insert_non_full(s, k);
        } else {
                btree_insert_non_full(r, k);
        }
        btree_traverse(T);
        printf("------------------------\n");
}

void btree_insert(struct btree *tree, key_t key, void *data)
{
        struct btree_item item = { .key = key, .data = data };
        __btree_insert(tree, &item);
}

void __btree_traverse(struct btree_node *node, int indent)
{
        if (node == NULL) {
                return;
        }
        printf("parent(%d)\t",
               (node->parent == NULL ? -1 : (int)node->parent->items[0].key));

        for (int i = 0; i < indent; i++) {
                printf("\t");
        }

        for (int i = 0; i < node->n; i++) {
                printf("%d ", node->items[i].key);
        }
        printf("(%d)\n", node->n);

        for (int i = 0; i <= node->n; i++) {
                __btree_traverse(node->child[i], indent + 1);
        }
}

void btree_traverse(struct btree *tree)
{
        __btree_traverse(tree->root, 0);
}

#ifdef _2_3_TREE
static int __btree_delete(struct btree_node *node, key_t key)
{
}
#else
static int __btree_delete(struct btree_node *node, key_t key)
{
}
#endif

int btree_delete(struct btree *tree, key_t key)
{
}
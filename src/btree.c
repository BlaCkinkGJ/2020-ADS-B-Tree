#include <stdlib.h>
#include <errno.h>
#include "btree.h"

static struct btree_node *btree_alloc_node(struct btree *T)
{
        struct btree_node *node = NULL;
        const int nr_keys = B_TREE_NR_KEYS(T->min_degree);
        const int nr_child = B_TREE_NR_CHILD(T->min_degree);

        node = (struct btree_node *)malloc(sizeof(struct btree_node));
        if (!node) {
                pr_info("Node allocation failed...\n");
                goto exception;
        }
        node->n = 0;
        node->is_leaf = false;
        node->items =
                (struct btree_item *)calloc(nr_keys, sizeof(struct btree_item));
        if (!node->items) {
                pr_info("Node item allocation failed...\n");
                goto exception;
        }
        node->child = (struct btree_node **)calloc(nr_child,
                                                   sizeof(struct btree_node *));
        if (!node->child) {
                pr_info("Node child allocation failed...\n");
                goto exception;
        }
        return node;
exception:
        if (node->child) {
                free(node->child);
        }
        if (node->items) {
                free(node->items);
        }
        if (node) {
                free(node);
        }
        return NULL;
}

static void btree_dealloc_node(struct btree_node *node)
{
        if (node != NULL) {
#ifdef B_TREE_DEALLOC_ITEM
                for (int i = 0; i < node->n; i++) {
                        if (node->items[i].data) {
                                free(node->items[i].data);
                        }
                }
#endif
                if (node->child) {
                        free(node->child);
                }
                if (node->items) {
                        free(node->items);
                }
                free(node);
        }
}

struct btree *btree_alloc(int min_degree)
{
        struct btree *tree = NULL;
        struct btree_node *node = NULL;

        if (min_degree < B_TREE_MIN_DEGREE) {
                pr_info("Degree must over 2\n");
                return NULL;
        }

        tree = (struct btree *)malloc(sizeof(struct btree));
        if (!tree) {
                pr_info("Allocation tree failed\n");
                goto exception;
        }
        tree->min_degree = min_degree; /**< DO NOT CHANGE */

        node = btree_alloc_node(tree);
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
                btree_dealloc_node(node);
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
                result.index = B_TREE_NOT_FOUND;
                result.node = NULL;
                return result;
        } else {
                return __btree_search(x->child[i], k);
        }
}

struct btree_search_result btree_search(struct btree *tree, key_t key)
{
        return __btree_search(tree->root, key);
}

static void btree_split_child(struct btree *T, struct btree_node *x, int i)
{
        const int t = T->min_degree;

        struct btree_node *z = btree_alloc_node(T);
        struct btree_node *y = x->child[i - 1];

        z->is_leaf = y->is_leaf;
        z->n = t - 1;

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
}

static void btree_insert_non_full(struct btree *T, struct btree_node *x,
                                  struct btree_item *k)
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
                if (x->child[i]->n == B_TREE_NR_KEYS(T->min_degree)) {
                        btree_split_child(T, x, i + 1);
                        if (k->key > x->items[i].key) {
                                i = i + 1;
                        }
                }
                btree_insert_non_full(T, x->child[i], k);
        }
}

static void __btree_insert(struct btree *T, struct btree_item *k)
{
        struct btree_node *r = T->root;
        if (r->n == B_TREE_NR_KEYS(T->min_degree)) {
                struct btree_node *s = btree_alloc_node(T);
                T->root = s;
                s->is_leaf = false;
                s->n = 0;
                s->child[0] = r;

                btree_split_child(T, s, 1);
                btree_insert_non_full(T, s, k);
        } else {
                btree_insert_non_full(T, r, k);
        }
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

static void __btree_clear(struct btree_node *node)
{
        if (node) {
                if (!node->is_leaf) {
                        for (int i = 0; i < (node->n + 1); i++) {
                                __btree_clear(node->child[i]);
                        }
                }
                btree_dealloc_node(node);
        }
}

static void btree_clear(struct btree *tree)
{
        __btree_clear(tree->root);
        tree->root = NULL;
}

static struct btree_item btree_get_predecessor(struct btree_node *node)
{
        while (!node->is_leaf) {
                node = node->child[node->n];
        }
        return node->items[node->n - 1];
}

static struct btree_item btree_get_successor(struct btree_node *node)
{
        while (!node->is_leaf) {
                node = node->child[0];
        }
        return node->items[0];
}

static void btree_merge_child(struct btree *T, struct btree_node *p, int i)
{
        const int t = T->min_degree;
        struct btree_node *child[] = {
                p->child[i],
                p->child[i + 1],
        };

        child[0]->n = B_TREE_NR_KEYS(T->min_degree);
        child[0]->items[t - 1] = p->items[i];

        for (int j = 0; j < t - 1; j++) {
                child[0]->items[j + t] = child[1]->items[j];
        }

        if (!child[0]->is_leaf) {
                for (int j = 0; j < t; j++) {
                        child[0]->child[j + t] = child[1]->child[j];
                }
        }

        p->n -= 1;

        for (int j = i; j < p->n; j++) {
                p->items[j] = p->items[j + 1];
                p->child[j + 1] = p->child[j + 2];
        }

        btree_dealloc_node(child[1]);
        if (p->n == 0) {
                btree_dealloc_node(p);
                if (p == T->root) {
                        T->root = child[0];
                }
        }
}

static int __btree_delete(struct btree *T, struct btree_node *x, key_t key)
{
        const int t = T->min_degree;
        int i = 0;

        while (i < x->n && key > x->items[i].key) {
                i = i + 1;
        }

        if (i < x->n && key == x->items[i].key) {
                if (x->is_leaf) { /**< case 1 */
                        x->n -= 1;
                        for (; i < x->n; i++) {
                                x->items[i] = x->items[i + 1];
                        }
                        goto end;
                } else { /**< case 2 */
                        struct btree_node *prev = x->child[i];
                        struct btree_node *next = x->child[i + 1];
                        if (prev->n >= t) { /**< case 2a */
                                struct btree_item prev_item = { 0 };

                                prev_item = btree_get_predecessor(prev);
                                __btree_delete(T, prev, prev_item.key);
                                x->items[i] = prev_item;

                                goto end;
                        } else if (next->n >= t) { /**< case 2b */
                                struct btree_item next_item = { 0 };

                                next_item = btree_get_successor(next);
                                __btree_delete(T, next, next_item.key);
                                x->items[i] = next_item;
                                goto end;
                        } else { /**< case 2c */
                                btree_merge_child(T, x, i);
                                __btree_delete(T, prev, key);
                        }
                }
        } else { /**< case 3 */
                struct btree_node *child = x->child[i];
                if (child->n == t - 1) {
                        struct btree_node *left = NULL;
                        struct btree_node *right = NULL;
                        int j = 0;

                        if (i > 0) { /**< get left child */
                                left = x->child[i - 1];
                        }

                        if (i < x->n) { /**< get right child */
                                right = x->child[i + 1];
                        }

                        if (left && left->n >= t) {
                                for (j = child->n; j > 0; --j) {
                                        child->items[j] = child->items[j - 1];
                                }
                                child->items[0] = x->items[i - 1];

                                if (!left->is_leaf) {
                                        for (j = child->n + 1; j > 0; j--) {
                                                child->child[j] =
                                                        child->child[j - 1];
                                        }
                                        child->child[0] = left->child[left->n];
                                }

                                child->n += 1;
                                x->items[i - 1] = left->items[left->n - 1];
                                left->n -= 1;

                        } else if (right && right->n >= t) {
                                child->items[child->n] = x->items[i];
                                child->n += 1;

                                x->items[i] = right->items[0];
                                right->n -= 1;

                                for (j = 0; j < right->n; j++) {
                                        right->items[j] = right->items[j + 1];
                                }

                                if (!right->is_leaf) {
                                        child->child[child->n] =
                                                right->child[0];
                                        for (j = 0; j <= right->n; j++) {
                                                right->child[j] =
                                                        right->child[j + 1];
                                        }
                                }
                        } else if (left) {
                                btree_merge_child(T, x, i - 1);
                                child = left;
                        } else if (right) {
                                btree_merge_child(T, x, i);
                        } // end of left, right adjust
                } // child[i] has "t-1" keys
                __btree_delete(T, child, key);
        } // end of find key location

end:
        return 0;
}

int btree_delete(struct btree *tree, key_t key)
{
        struct btree_node *node = NULL;
        struct btree_node *root = tree->root;

        node = (btree_search(tree, key)).node;
        if (!node) {
                pr_info("Cannot find specific node\n");
                return -EINVAL;
        }

        if (root->n == 0 && root->is_leaf) {
                btree_clear(tree);
                return 0;
        }

        return __btree_delete(tree, root, key);
}

void btree_free(struct btree *tree)
{
        if (tree) {
                struct btree_node *root = tree->root;
                while (root->n > 0) {
                        key_t key = root->items[0].key;
                        btree_delete(tree, key);
                        root = tree->root;
                }
                btree_dealloc_node(tree->root);
                free(tree);
        }
}

#include "btree.h"
#include "unity.h"

struct btree *tree;

#define ARR_SIZE(T) ((int)(sizeof(T) / sizeof(key_t)))

key_t keys[] = { 50, 80, 10, 20 };

void setUp(void)
{
        tree = btree_create();
        TEST_ASSERT_NOT_NULL(tree);
}

void tearDown(void)
{
}

void testBtree(void)
{
        struct btree_search_result result;
        for (int i = 0; i < ARR_SIZE(keys); i++) {
                btree_insert(tree, keys[i], NULL);
        }
        for (int i = 0; i < ARR_SIZE(keys); i++) {
                result = btree_search(tree, keys[i]);
                TEST_ASSERT_NOT_NULL(result.node);
        }

        result = btree_search(tree, 0);
        TEST_ASSERT_NULL(result.node);

        btree_traverse(tree);
}

int main(void)
{
        UNITY_BEGIN();
        RUN_TEST(testBtree);
        return UNITY_END();
}
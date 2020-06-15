#include "btree.h"
#include "unity.h"

struct btree *tree;

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
        for (key_t key = 1; key <= 100; key++) {
                btree_insert(tree, key, NULL);
        }
        result = btree_search(tree, 100);
        TEST_ASSERT_NOT_NULL(result.node);
}

int main(void)
{
        UNITY_BEGIN();
        RUN_TEST(testBtree);
        return UNITY_END();
}
#include "btree.h"
#include "unity.h"
#include <time.h>

struct btree *tree;

#define SHUFFLE
#define MAX_SIZE 100000
#define REMAIN 5
#define TEST_LOOP 2
#define ARR_SIZE(T) ((int)(sizeof(T) / sizeof(key_t)))

key_t keys[MAX_SIZE] = { 0 };

#ifdef SHUFFLE
static void shuffle(key_t *keys, int n)
{
        int choice = 0;
        int temp = 0;
        srand(time(NULL));
        for (int i = 0; i < n; i++) {
                choice = rand() % n;
                temp = keys[choice];
                keys[choice] = keys[i];
                keys[i] = temp;
        }
}
#endif

void setUp(void)
{
        tree = btree_alloc();
        TEST_ASSERT_NOT_NULL(tree);

#ifdef SHUFFLE
        for (int i = 0; i < ARR_SIZE(keys); i++) {
                keys[i] = i;
        }
        shuffle(keys, ARR_SIZE(keys));
#endif
}

void tearDown(void)
{
        btree_free(tree);
}

void testBtree(void)
{
        struct btree_search_result result;
        int ret;
        for (int i = 0; i < TEST_LOOP; i++) {
                for (int i = 0; i < ARR_SIZE(keys); i++) {
                        btree_insert(tree, keys[i], NULL);
                }
                for (int i = 0; i < ARR_SIZE(keys); i++) {
                        result = btree_search(tree, keys[i]);
                        TEST_ASSERT_NOT_NULL(result.node);
                }

                result = btree_search(tree, MAX_SIZE + 1);
                TEST_ASSERT_NULL(result.node);
                for (int i = 0; i < ARR_SIZE(keys) - REMAIN; i++) {
                        //int del = i;
                        //int del = ARR_SIZE(keys) - i - 1;
                        key_t del = keys[i];
                        result = btree_search(tree, del);
                        TEST_ASSERT_NOT_NULL(result.node);
                        ret = btree_delete(tree, del);
                        TEST_ASSERT_EQUAL(0, ret);
                        result = btree_search(tree, del);
                        TEST_ASSERT_NULL(result.node);
                }
        }

        btree_traverse(tree);
}

int main(void)
{
        UNITY_BEGIN();
        RUN_TEST(testBtree);
        return UNITY_END();
}
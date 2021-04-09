#include <stdio.h>
#include "CUnit/CUnit.h"

#include "func.h"

static void test_maxi(void)
{
    CU_ASSERT(maxi(0, 2) == 2);
    CU_TEST(maxi(2, 2) == 2);
    CU_ASSERT(maxi(2, 3) == 1);
}
//------------------------------
static int test_func_suite_init()
{
    printf("\ntest_func_suite_init...\n");
    return 0;
}

static int test_func_suite_clean()
{
    printf("\ntest_func_suite_clean...\n");
    return 0;
}

int test_func_suite()
{
    CU_pSuite pSuite = NULL;

    /* add a suite to the registry */
    pSuite = CU_add_suite("suite_sum", test_func_suite_init, test_func_suite_clean);
    if (NULL == pSuite)
    {
        return CU_get_error();
    }

    if ((NULL == CU_add_test(pSuite, "test_maxi", test_maxi)))
    {
        return CU_get_error();
    }

    return CUE_SUCCESS;
}

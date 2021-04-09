#include <stdio.h>
#include <assert.h>
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include "CUnit/Automated.h"
#include "CUnit/Console.h"

#include "test_func.h"

int main()
{
    if (CUE_SUCCESS != CU_initialize_registry())
    {
        return CU_get_error();
    }

    test_func_suite();
    //-----------------------------------------------
    // CU_list_tests_to_file();
    // CU_automated_run_tests();
    //---------------
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    //---------------
    // CU_console_run_tests();
    //---------------

    CU_cleanup_registry();
    return 0;
}
/*
 * Copyright (C) 2021 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       iondb-pkg tests
 *
 * @author      Tobias Westphal <tobias.westphal@haw-hamburg.de>
 *
 * @}
 */

#ifdef MODULE_FATFS
#include "fs/fatfs.h"
#endif
#ifdef MODULE_VFS
#include "vfs.h"
#endif
#ifdef MODULE_MTD
#include "mtd.h"
#endif
#ifdef Module_BOARD
#include "board.h"
#endif

#include "kernel_defines.h"
#include "ion_master_table.h"

#include <sys/time.h>
#include "math.h"
#include <malloc.h>

#ifdef MODULE_MTD_SDCARD
#include "mtd_sdcard.h"
#include "sdcard_spi.h"
#include "sdcard_spi_params.h"
#endif

#if FATFS_FFCONF_OPT_FS_NORTC == 0
#include "periph/rtc.h"
#endif

#ifdef MODULE_FATFS
static fatfs_desc_t fatfs = {
    .vol_idx = 0};

static vfs_mount_t _test_vfs_mount = {
    .mount_point = MNT_PATH_VFS,
    .fs = &fatfs_file_system,
    .private_data = (void *)&fatfs,
};

/* provide mtd devices for use within diskio layer of fatfs */
mtd_dev_t *fatfs_mtd_devs[FF_VOLUMES];
#endif

#ifdef MODULE_MTD_NATIVE
/* mtd device for native is provided in boards/native/board_init.c */
extern mtd_dev_t *mtd0;
#elif MODULE_MTD_SDCARD
#define SDCARD_SPI_NUM ARRAY_SIZE(sdcard_spi_params)
extern sdcard_spi_t sdcard_spi_devs[SDCARD_SPI_NUM];
mtd_sdcard_t mtd_sdcard_devs[SDCARD_SPI_NUM];
/* always default to first sdcard*/
static mtd_dev_t *mtd1 = (mtd_dev_t *)&mtd_sdcard_devs[0];
#endif

#define ENABLE_DEBUG (1)
#include "debug.h"

#define KEY_TYPE int32_t
#define VALUE_TYPE int32_t
#define INDEX_TYPE uint32_t

#define TEST_02_KEYS           \
    {                          \
        23, 27, -1, -10,       \
            100, 300, 137, 50, \
            122, 700, 21, 24   \
    }
#define TEST_02_VALUES         \
    {                          \
        25, 24, 77, 11,        \
            -212, 21, 21, 21,  \
            444, 845, -123, 37 \
    }

#define TEST_03_KEYS            \
    {                           \
        1, 12, 4, -10,          \
            8, 7, 3, 50,        \
            122, 111, -112, 144 \
    }
#define TEST_03_VALUES                     \
    {                                      \
        -100, 10024, 10077, 10011,         \
            100212, -10021, 10021, 10021,  \
            100444, 100845, 100123, -10037 \
    }

#define TEST_04_KEYS      \
    {                     \
        -32, -16, -8, -4, \
            -2, -1, 1, 2, \
            4, 8, 16, 32  \
    }

#define TEST_04_VALUES                     \
    {                                      \
        -100, 10024, 10077, 10011,         \
            -100212, 10021, 10021, 10021,  \
            -100444, 100845, 100123, 10037 \
    }

#define TEST_04_UPDATED_VALUES             \
    {                                      \
        10024, -100, 10011, 10077,         \
            10021, -10021, -10021, 100212, \
            100444, 100123, 10037, -100845 \
    }

#define TEST_08_KEYS       \
    {                      \
        -32, -1, 16, -32,  \
            -1, -1, -5, 2, \
            16, 16, 16, 32 \
    }

#define TEST_08_KEY_DUPLICATES \
    {                          \
        2, 3, 4                \
    }

#define TEST_08_VALUES          \
    {                           \
        -321, -11, 161, -322,   \
            -12, -13, 999, 999, \
            162, 163, 164, 999  \
    }

#define AMOUNT_TYPES_DICTIONARY 6

/* Test Variables */
KEY_TYPE test02Keys[] = TEST_02_KEYS;
VALUE_TYPE test02Values[] = TEST_02_VALUES;
#define TEST_02_KEY_LENGTH ARRAY_SIZE(test02Keys)

KEY_TYPE test03Keys[] = TEST_03_KEYS;
VALUE_TYPE test03Values[] = TEST_03_VALUES;
#define TEST_03_KEY_LENGTH ARRAY_SIZE(test03Keys)

KEY_TYPE test04Keys[] = TEST_04_KEYS;
VALUE_TYPE test04Values[] = TEST_04_VALUES;
VALUE_TYPE test04UpdatedValues[] = TEST_04_UPDATED_VALUES;
#define TEST_04_KEY_LENGTH ARRAY_SIZE(test04Keys)

KEY_TYPE test04KeyChecksum = 0; // TODO make as parameter
VALUE_TYPE test04ValueChecksum = 0;

KEY_TYPE test08Keys[] = TEST_08_KEYS;
VALUE_TYPE test08Values[] = TEST_08_VALUES;
KEY_TYPE test08KeyDuplicates[] = TEST_08_KEY_DUPLICATES;
#define TEST_08_KEY_LENGTH ARRAY_SIZE(test08Keys)
#define TEST_08_KEY_DUPLICATES_LENGTH ARRAY_SIZE(test08KeyDuplicates)

VALUE_TYPE test08DuplicatesChecksums[TEST_08_KEY_DUPLICATES_LENGTH] = {0}; // TODO make as parameter

/* Global Variables */
ion_byte_t RETRIEVE_SPACE_KEY[sizeof(KEY_TYPE)] = {0};
ion_byte_t RETRIEVE_SPACE_VALUE[sizeof(VALUE_TYPE)] = {0};

INDEX_TYPE DICT_SIZE_GLOB = 0; /* Here only initialized, in main set to special value */

/* --- Helper Functions Begin --- */

static void printf_dictionary_type(ion_dictionary_type_t type);
static ion_err_t clear_dict_n_master_table(ion_dictionary_t *dict, ion_dictionary_id_t dict_id);

/* Write the corresponding dicitionary type to stdout without newline */
static void printf_dictionary_type(ion_dictionary_type_t type)
{
    switch (type)
    {
    case dictionary_type_bpp_tree_t:
        printf("BPP_TREE");
        break;
    case dictionary_type_flat_file_t:
        printf("FLAT_FILE");
        break;
    case dictionary_type_open_address_file_hash_t:
        printf("OPEN_ADDRESS_FILE_HASH");
        break;
    case dictionary_type_open_address_hash_t:
        printf("OPEN_ADDRESS_HASH");
        break;
    case dictionary_type_skip_list_t:
        printf("SKIP_LIST");
        break;
    case dictionary_type_linear_hash_t:
        printf("LINEAR_HASH");
        break;
    /* case dictionary_type_flat_file_timeseries_t:
        printf("FLAT_FILE_TIMESERIES");
        break; */
    case dictionary_type_error_t:
        printf("ERROR_TYPE");
        break;
    }
}

/* Close/Delete dictionary if parameters aren't NULL/0 and Close/Delete master table in all cases  */
static ion_err_t clear_dict_n_master_table(ion_dictionary_t *dict, ion_dictionary_id_t dict_id)
{
    ion_err_t err = err_ok;

    if (dict != NULL && dict_id != 0)
    {
        if (ion_close_dictionary(dict) != err_ok)
        {
            err += 1;
            DEBUG("Dictionary Close Failed \n");
        }

        if (ion_delete_dictionary(dict, dict_id) != err_ok)
        {
            err += 1;
            DEBUG("Dictionary Delete Failed \n");
        }
    }
    if (ion_close_master_table() != err_ok)
    {
        err += 1;
        DEBUG("Master Table Close Failed \n");
    }

    if(ion_delete_master_table() != err_ok)
    {
        err += 1;
        DEBUG("Master Table Delete Failed \n");
    }

    return err;
}

/* --- Helper Functions End --- */

/* --- TEST Begin --- */

/* TEST01: Create, Close, Open, Delete of a Dictionary  */
static int test01(ion_dictionary_type_t current_type)
{
    ion_dictionary_handler_t handler = {0};
    ion_dictionary_t dict = {0};
    ion_status_t status = {0};

    puts("[Test1]: Create, Close, Open, Delete of a Dictionary");

    printf("Initializing the master table ");
    status.error = ion_init_master_table();
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to initialize master table: %d\n", status.error);
        return 1;
    }
    puts("- [WORKED]");
    /* Used to bind dictionary specific function to the handler */
    ion_switch_handler(current_type, &handler);

    ion_key_type_t k_type = key_type_numeric_signed;
    ion_key_size_t k_size = sizeof(KEY_TYPE);
    ion_value_size_t v_size = sizeof(VALUE_TYPE);

    printf("CREATE dictionary ");
    status.error = ion_master_table_create_dictionary(&handler, &dict, k_type, k_size, v_size, DICT_SIZE_GLOB);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to create the dictionary with error %d\n", status.error);
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }
    puts("- [WORKED]");

    /* Read the dictionary id for later opening usage  */
    ion_dictionary_id_t dict_id = dict.instance->id;

    printf("CLOSE dictionary and master table ");
    status.error = ion_close_dictionary(&dict);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to close the dictionary \n");
        clear_dict_n_master_table(&dict, dict_id);
        return 1;
    }

    status.error = ion_close_master_table();
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to close the master table \n");
        clear_dict_n_master_table(&dict, dict_id);
        return 1;
    }
    puts("- [WORKED]");

    printf("REOPEN master table ");
    status.error = ion_init_master_table();
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to initialize master table: %d\n", status.error);
        clear_dict_n_master_table(&dict, dict_id);
        return 1;
    }
    puts("- [WORKED]");

    printf("OPEN closed dictionary ");
    status.error = ion_open_dictionary(&handler, &dict, dict_id);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to open dictionary \n");
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }
    puts("- [WORKED]");

    printf("DELETE opened dictionary ");
    status.error = ion_delete_dictionary(&dict, dict_id);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to delete dictionary: %d\n", status.error);
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }
    puts("- [WORKED]");

    printf("OPEN deleted dictionary and EXPECT ERROR ");
    status.error = ion_open_dictionary(&handler, &dict, dict_id);
    if (status.error == err_ok)
    {
        printf("- [FAILED]: opening the dictionary worked \n");
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }
    puts("- [WORKED]");

    /* Clearing Boiler Plate */

    if (clear_dict_n_master_table(NULL, 0) != err_ok)
    {
        printf("[FAILED]: to close/delete dictionary and master table \n");
        return 1;
    }

    /* Clearing Boiler Plate */

    puts("[TEST01 SUCCESS]");
    return 0;
}

/* TEST02: Read, Insert, Read, Delete, Read of Keys of test02Keys and test02Values */
static int test02(ion_dictionary_type_t current_type)
{
    puts("[TEST02]: Read, Insert, Read, Delete, Read of Keys of Test_02_Key and Test_02_Values");

    /* Boiler Plate Begin */
    ion_dictionary_handler_t handler = {0};
    ion_dictionary_t dict = {0};
    ion_status_t status = {0};

    status.error = ion_init_master_table();
    if (status.error != err_ok)
    {
        printf("[FAILED]: to initialize master table: %d\n", status.error);
        return 1;
    }

    /* Used to bind dictionary specific function to the handler */
    ion_switch_handler(current_type, &handler);

    ion_key_type_t k_type = key_type_numeric_signed;
    ion_key_size_t k_size = sizeof(KEY_TYPE);
    ion_value_size_t v_size = sizeof(VALUE_TYPE);

    status.error = ion_master_table_create_dictionary(&handler, &dict, k_type, k_size, v_size, DICT_SIZE_GLOB);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to create the dictionary with error %d\n", status.error);
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }

    /* Read the dictionary id for later opening usage  */
    ion_dictionary_id_t dict_id = dict.instance->id;

    /* Boiler Plate End */

    INDEX_TYPE i = 0;
    VALUE_TYPE in_value = 0;
    VALUE_TYPE out_value = 0;

    printf("READ non-inserted test02Keys keys and EXPECT ERRORS ");
    for (i = 0; i < TEST_02_KEY_LENGTH; i++)
    {
        status = dictionary_get(&dict, IONIZE(test02Keys[i], KEY_TYPE), RETRIEVE_SPACE_VALUE);
        if (status.error == err_ok)
        {
            printf("- [FAILED]: Read Status at i: %d didn't throw error\n", i);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    printf("INSERT test02Keys keys with test02Values values ");
    for (i = 0; i < TEST_02_KEY_LENGTH; i++)
    {
        in_value = test02Values[i];
        status = dictionary_insert(&dict, IONIZE(test02Keys[i], KEY_TYPE), &in_value);
        if (status.error != err_ok)
        {
            printf("- [FAILED]: Insert Status at i: %d with error %d\n", i, status.error);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    printf("READ test02Keys keys with test02Values values ");
    for (i = 0; i < TEST_02_KEY_LENGTH; i++)
    {
        status = dictionary_get(&dict, IONIZE(test02Keys[i], KEY_TYPE), RETRIEVE_SPACE_VALUE);
        out_value = NEUTRALIZE(RETRIEVE_SPACE_VALUE, VALUE_TYPE);
        if (status.error != err_ok || out_value != test02Values[i])
        {
            printf("- [FAILED]: Read Status at i: %d with error %d | Read: %d, Expected %d\n", i, status.error, out_value, test02Values[i]);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    printf("DELETE test02Keys keys ");
    for (i = 0; i < TEST_02_KEY_LENGTH; i++)
    {
        status = dictionary_delete(&dict, IONIZE(test02Keys[i], KEY_TYPE));
        if (status.error != err_ok)
        {
            printf("- [FAILED]: Delete Status at i: %d with error %d\n", i, status.error);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    printf("READ deleted test02Keys keys and EXPECT ERRORS ");
    for (i = 0; i < TEST_02_KEY_LENGTH; i++)
    {
        status = dictionary_get(&dict, IONIZE(test02Keys[i], KEY_TYPE), RETRIEVE_SPACE_VALUE);
        if (status.error == err_ok)
        {
            printf("- [FAILED]: Read Status at i: %d didn't throw error\n", i);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    /* Clearing Boiler Plate */

    if (clear_dict_n_master_table(&dict, dict_id) != err_ok)
    {
        printf("[FAILED]: to close/delete dictionary and master table \n");
        return 1;
    }

    /* Clearing Boiler Plate */

    puts("[TEST02 SUCCESS]");
    return 0;
}

/* TEST03: Insert Keys, Close Dictionary, Open Dictionary, Read Keys of test03Keys and test03Values */
static int test03(ion_dictionary_type_t current_type)
{
    puts("[TEST03]: Insert Keys, Close Dictionary, Open Dictionary, Read Keys of Test_03_Keys and Test_03_Values");

    /* Boiler Plate Begin */
    ion_dictionary_handler_t handler = {0};
    ion_dictionary_t dict = {0};
    ion_status_t status = {0};

    status.error = ion_init_master_table();
    if (status.error != err_ok)
    {
        printf("[FAILED]: to initialize master table: %d\n", status.error);
        return 1;
    }

    /* Used to bind dictionary specific function to the handler */
    ion_switch_handler(current_type, &handler);

    ion_key_type_t k_type = key_type_numeric_signed;
    ion_key_size_t k_size = sizeof(KEY_TYPE);
    ion_value_size_t v_size = sizeof(VALUE_TYPE);

    status.error = ion_master_table_create_dictionary(&handler, &dict, k_type, k_size, v_size, DICT_SIZE_GLOB);
    if (status.error != err_ok)
    {
        printf("[FAILED]: to create the dictionary with error %d\n", status.error);
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }

    /* Read the dictionary id for later opening usage  */
    ion_dictionary_id_t dict_id = dict.instance->id;

    /* Boiler Plate End */

    INDEX_TYPE i = 0;
    VALUE_TYPE in_value = 0;
    VALUE_TYPE out_value = 0;

    printf("INSERT test03Keys keys with test03Values values ");
    for (i = 0; i < TEST_03_KEY_LENGTH; i++)
    {
        in_value = test03Values[i];
        status = dictionary_insert(&dict, IONIZE(test03Keys[i], KEY_TYPE), &in_value);
        if (status.error != err_ok)
        {
            printf("- [FAILED]: Insert Status at i: %d with error %d\n", i, status.error);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    printf("CLOSE dictionary ");
    status.error = ion_close_dictionary(&dict);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to close dictionary \n");
        clear_dict_n_master_table(&dict, dict_id);
        return 1;
    }
    puts("- [WORKED]");

    printf("OPEN dictionary ");
    status.error = ion_open_dictionary(&handler, &dict, dict_id);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to open dictionary \n");
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }
    puts("- [WORKED]");

    printf("READ test03Keys keys with test03Values Values ");
    for (i = 0; i < TEST_03_KEY_LENGTH; i++)
    {
        status = dictionary_get(&dict, IONIZE(test03Keys[i], KEY_TYPE), RETRIEVE_SPACE_VALUE);
        out_value = NEUTRALIZE(RETRIEVE_SPACE_VALUE, VALUE_TYPE);

        if (status.error != err_ok || out_value != test03Values[i])
        {
            printf("- [FAILED]: Read Status at i: %d with error %d| Read: %d, Expected: %d\n", i, status.error, out_value, test03Values[i]);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    /* Clearing Boiler Plate */

    if (clear_dict_n_master_table(&dict, dict_id) != err_ok)
    {
        printf("[FAILED]: to close/delete dictionary and master table \n");
        return 1;
    }

    /* Clearing Boiler Plate */

    puts("[TEST03 SUCCESS]");
    return 0;
}

/* TEST04: Update, Read, Delete, Insert, Update, Read of Keys of test04Keys, test04Values and test04UpdatedValues */
static int test04(ion_dictionary_type_t current_type)
{
    puts("[TEST04]: Update, Read, Delete, Insert, Update, Read of Keys of Test_04_Key and Test_04_Values");

    /* Boiler Plate Begin */
    ion_dictionary_handler_t handler = {0};
    ion_dictionary_t dict = {0};
    ion_status_t status = {0};

    status.error = ion_init_master_table();
    if (status.error != err_ok)
    {
        printf("[FAILED]: to initialize master table: %d\n", status.error);
        return 1;
    }

    /* Used to bind dictionary specific function to the handler */
    ion_switch_handler(current_type, &handler);

    ion_key_type_t k_type = key_type_numeric_signed;
    ion_key_size_t k_size = sizeof(KEY_TYPE);
    ion_value_size_t v_size = sizeof(VALUE_TYPE);

    status.error = ion_master_table_create_dictionary(&handler, &dict, k_type, k_size, v_size, DICT_SIZE_GLOB);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to create the dictionary with error %d\n", status.error);
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }

    /* Read the dictionary id for later opening usage  */
    ion_dictionary_id_t dict_id = dict.instance->id;

    /* Boiler Plate End */

    INDEX_TYPE i = 0;
    VALUE_TYPE in_value = 0;
    VALUE_TYPE out_value = 0;

    printf("UPDATE test04Keys keys with test04UpdatedValues values ");
    for (i = 0; i < TEST_04_KEY_LENGTH; i++)
    {
        in_value = test04UpdatedValues[i];
        status = dictionary_update(&dict, IONIZE(test04Keys[i], KEY_TYPE), &in_value);
        if (status.error != err_ok)
        {
            printf("- [FAILED]: Update Status at i: %d with error %d\n", i, status.error);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    printf("READ test04Keys keys and expect test04UpdatedValues ");
    for (i = 0; i < TEST_04_KEY_LENGTH; i++)
    {
        status = dictionary_get(&dict, IONIZE(test04Keys[i], KEY_TYPE), RETRIEVE_SPACE_VALUE);
        out_value = NEUTRALIZE(RETRIEVE_SPACE_VALUE, VALUE_TYPE);
        if (status.error != err_ok || out_value != test04UpdatedValues[i])
        {
            printf("- [FAILED]: Read Status at i: %d with error %d | Read: %d, Expected: %d\n", i, status.error, out_value, test04UpdatedValues[i]);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    printf("DELETE test04Keys keys ");
    for (i = 0; i < TEST_04_KEY_LENGTH; i++)
    {
        status = dictionary_delete(&dict, IONIZE(test04Keys[i], KEY_TYPE));
        if (status.error != err_ok)
        {
            printf("- [FAILED]: Delete Status at i: %d with error %d\n", i, status.error);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    printf("INSERT test04Keys keys with test04Values values ");
    for (i = 0; i < TEST_04_KEY_LENGTH; i++)
    {
        in_value = test04Values[i];
        status = dictionary_insert(&dict, IONIZE(test04Keys[i], KEY_TYPE), &in_value);
        if (status.error != err_ok)
        {
            printf("- [FAILED]: Insert Status at i: %d with error %d\n", i, status.error);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    printf("UPDATE test04Keys keys with test04UpdatedValues values ");
    for (i = 0; i < TEST_04_KEY_LENGTH; i++)
    {
        in_value = test04UpdatedValues[i];
        status = dictionary_update(&dict, IONIZE(test04Keys[i], KEY_TYPE), &in_value);
        if (status.error != err_ok)
        {
            printf("- [FAILED]: Update Status at i: %d with error %d\n", i, status.error);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    printf("READ test04Keys keys and EXPECT test04UpdatedValues ");
    for (i = 0; i < TEST_04_KEY_LENGTH; i++)
    {
        status = dictionary_get(&dict, IONIZE(test04Keys[i], KEY_TYPE), RETRIEVE_SPACE_VALUE);
        out_value = NEUTRALIZE(RETRIEVE_SPACE_VALUE, VALUE_TYPE);
        if (status.error != err_ok || out_value != test04UpdatedValues[i])
        {
            printf("- [FAILED]: Read Status at i: %d with error %d | Read: %d, Expected: %d\n", i, status.error, out_value, test04UpdatedValues[i]);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    /* Clearing Boiler Plate */

    if (clear_dict_n_master_table(&dict, dict_id) != err_ok)
    {
        printf("[FAILED]: to close/delete dictionary and master table \n");
        return 1;
    }

    /* Clearing Boiler Plate */

    puts("[TEST04 SUCCESS]");
    return 0;
}

/* TEST05: Cursor Equality */
static int test05(ion_dictionary_type_t current_type)
{
    puts("[TEST05]: Cursor Equality");

    /* Boiler Plate Begin */
    ion_dictionary_handler_t handler = {0};
    ion_dictionary_t dict = {0};
    ion_status_t status = {0};

    status.error = ion_init_master_table();
    if (status.error != err_ok)
    {
        printf("[FAILED]: to initialize master table: %d\n", status.error);
        return 1;
    }

    /* Used to bind dictionary specific function to the handler */
    ion_switch_handler(current_type, &handler);

    ion_key_type_t k_type = key_type_numeric_signed;
    ion_key_size_t k_size = sizeof(KEY_TYPE);
    ion_value_size_t v_size = sizeof(VALUE_TYPE);

    status.error = ion_master_table_create_dictionary(&handler, &dict, k_type, k_size, v_size, DICT_SIZE_GLOB);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to create the dictionary with error %d\n", status.error);
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }

    /* Read the dictionary id for later opening usage  */
    ion_dictionary_id_t dict_id = dict.instance->id;

    /* Boiler Plate End */

    INDEX_TYPE i = 0;
    VALUE_TYPE in_value = 0;

    printf("INSERT test04Keys keys with test04Values values ");
    for (i = 0; i < TEST_04_KEY_LENGTH; i++)
    {
        in_value = test04Values[i];
        status = dictionary_insert(&dict, IONIZE(test04Keys[i], KEY_TYPE), &in_value);
        if (status.error != err_ok)
        {
            printf("- [FAILED]: Insert Status at i: %d with error %d\n", i, status.error);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    ion_predicate_t predicate = {0};
    ion_dict_cursor_t *cursor = NULL;
    ion_cursor_status_t cursor_status = {0};

    ion_record_t ion_record = {0};
    ion_record.key = RETRIEVE_SPACE_KEY;
    ion_record.value = RETRIEVE_SPACE_VALUE;

    printf("Find test04Keys keys with test04Values values ");
    INDEX_TYPE decrement_counter = TEST_04_KEY_LENGTH;
    for (i = 0; i < TEST_04_KEY_LENGTH; i++)
    {
        dictionary_build_predicate(&predicate, predicate_equality, IONIZE(test04Keys[i], KEY_TYPE));
        if (dictionary_find(&dict, &predicate, &cursor) == err_not_implemented)
        {
            puts("[OVER: FIND NOT IMPLEMENTED]");
            clear_dict_n_master_table(&dict, dict_id);
            return 0;
        }
        while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized)
        {
            if (NEUTRALIZE(ion_record.key, KEY_TYPE) != test04Keys[i] || (NEUTRALIZE(ion_record.value, VALUE_TYPE) != test04Values[i]))
            {
                printf("- [FAILED]: to find searched key: %d\n", test04Keys[i]);
                clear_dict_n_master_table(&dict, dict_id);
                return 1;
            }
            decrement_counter -= 1;
        }
    }
    cursor->destroy(&cursor);
    if (decrement_counter != 0)
    {
        printf("- [FAILED]: to find all searched keys: %d are missing\n", decrement_counter);
        clear_dict_n_master_table(&dict, dict_id);
        return 1;
    }

    puts("- [WORKED]");

    /* Clearing Boiler Plate */

    if (clear_dict_n_master_table(&dict, dict_id) != err_ok)
    {
        printf("[FAILED]: to close/delete dictionary and master table \n");
        return 1;
    }

    /* Clearing Boiler Plate */

    puts("[TEST05 SUCCESS]");
    return 0;
}

/* TEST06: Cursor Range */
static int test06(ion_dictionary_type_t current_type)
{
    puts("[TEST06]: Cursor Range");

    /* Boiler Plate Begin */
    ion_dictionary_handler_t handler = {0};
    ion_dictionary_t dict = {0};
    ion_status_t status = {0};

    status.error = ion_init_master_table();
    if (status.error != err_ok)
    {
        printf("[FAILED]: to initialize master table: %d\n", status.error);
        return 1;
    }

    /* Used to bind dictionary specific function to the handler */
    ion_switch_handler(current_type, &handler);

    ion_key_type_t k_type = key_type_numeric_signed;
    ion_key_size_t k_size = sizeof(KEY_TYPE);
    ion_value_size_t v_size = sizeof(VALUE_TYPE);

    status.error = ion_master_table_create_dictionary(&handler, &dict, k_type, k_size, v_size, DICT_SIZE_GLOB);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to create the dictionary with error %d\n", status.error);
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }

    /* Read the dictionary id for later opening usage  */
    ion_dictionary_id_t dict_id = dict.instance->id;

    /* Boiler Plate End */

    INDEX_TYPE i = 0;
    VALUE_TYPE in_value = 0;

    printf("INSERT test04Keys keys with test04Values values ");
    for (i = 0; i < TEST_04_KEY_LENGTH; i++)
    {
        in_value = test04Values[i];
        status = dictionary_insert(&dict, IONIZE(test04Keys[i], KEY_TYPE), &in_value);
        if (status.error != err_ok)
        {
            printf("- [FAILED]: Insert Status at i: %d with error %d\n", i, status.error);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    ion_predicate_t predicate = {0};
    ion_dict_cursor_t *cursor = NULL;
    ion_cursor_status_t cursor_status = {0};

    ion_record_t ion_record = {0};
    ion_record.key = RETRIEVE_SPACE_KEY;
    ion_record.value = RETRIEVE_SPACE_VALUE;

    printf("TRAVERSING RANGE CURSOR ");
    dictionary_build_predicate(&predicate, predicate_range, IONIZE(test04Keys[0], KEY_TYPE), IONIZE(test04Keys[TEST_04_KEY_LENGTH - 1], KEY_TYPE));
    if (dictionary_find(&dict, &predicate, &cursor) == err_not_implemented)
    {
        puts("[OVER: FIND NOT IMPLEMENTED]");
        clear_dict_n_master_table(&dict, dict_id);
        return 0;
    }

    KEY_TYPE localKeyChecksum = 0;
    VALUE_TYPE localValueChecksum = 0;

    while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized)
    {
        localKeyChecksum += NEUTRALIZE(ion_record.key, KEY_TYPE);
        localValueChecksum += NEUTRALIZE(ion_record.value, VALUE_TYPE);
    }
    cursor->destroy(&cursor);
    if (localKeyChecksum != test04KeyChecksum || localValueChecksum != test04ValueChecksum)
    {
        printf("- [FAILED]: Checksums don't match test04KeyChecksum: %d, but localKeyChecksum: %d | test04ValueChecksum: %d, but localValueChecksum: %d\n", test04KeyChecksum, localKeyChecksum, test04ValueChecksum, localValueChecksum);
        clear_dict_n_master_table(&dict, dict_id);
        return 1;
    }
    puts("- [WORKED]");

    /* Clearing Boiler Plate */

    if (clear_dict_n_master_table(&dict, dict_id) != err_ok)
    {
        printf("[FAILED]: to close/delete dictionary and master table \n");
        return 1;
    }

    /* Clearing Boiler Plate */

    puts("[TEST06 SUCCESS]");
    return 0;
}

/* TEST07: Cursor All Records */
static int test07(ion_dictionary_type_t current_type)
{
    puts("[TEST07]: Cursor All Records");

    /* Boiler Plate Begin */
    ion_dictionary_handler_t handler = {0};
    ion_dictionary_t dict = {0};
    ion_status_t status = {0};

    status.error = ion_init_master_table();
    if (status.error != err_ok)
    {
        printf("[FAILED]: to initialize master table: %d\n", status.error);
        return 1;
    }

    /* Used to bind dictionary specific function to the handler */
    ion_switch_handler(current_type, &handler);

    ion_key_type_t k_type = key_type_numeric_signed;
    ion_key_size_t k_size = sizeof(KEY_TYPE);
    ion_value_size_t v_size = sizeof(VALUE_TYPE);

    status.error = ion_master_table_create_dictionary(&handler, &dict, k_type, k_size, v_size, DICT_SIZE_GLOB);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to create the dictionary with error %d\n", status.error);
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }

    /* Read the dictionary id for later opening usage  */
    ion_dictionary_id_t dict_id = dict.instance->id;

    /* Boiler Plate End */

    INDEX_TYPE i = 0;
    VALUE_TYPE in_value = 0;

    printf("INSERT test04Keys keys with test04Values values ");
    for (i = 0; i < TEST_04_KEY_LENGTH; i++)
    {
        in_value = test04Values[i];
        status = dictionary_insert(&dict, IONIZE(test04Keys[i], KEY_TYPE), &in_value);
        if (status.error != err_ok)
        {
            printf("- [FAILED]: Insert Status at i: %d with error %d\n", i, status.error);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    ion_predicate_t predicate = {0};
    ion_dict_cursor_t *cursor = NULL;
    ion_cursor_status_t cursor_status = {0};

    ion_record_t ion_record = {0};
    ion_record.key = RETRIEVE_SPACE_KEY;
    ion_record.value = RETRIEVE_SPACE_VALUE;

    printf("TRAVERSING ALL RECORDS CURSOR ");
    dictionary_build_predicate(&predicate, predicate_all_records);
    if (dictionary_find(&dict, &predicate, &cursor) == err_not_implemented)
    {
        puts("[OVER: FIND NOT IMPLEMENTED]");
        clear_dict_n_master_table(&dict, dict_id);
        return 0;
    }

    KEY_TYPE localKeyChecksum = 0;
    VALUE_TYPE localValueChecksum = 0;

    while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized)
    {
        localKeyChecksum += NEUTRALIZE(ion_record.key, KEY_TYPE);
        localValueChecksum += NEUTRALIZE(ion_record.value, VALUE_TYPE);
    }
    cursor->destroy(&cursor);
    if (localKeyChecksum != test04KeyChecksum || localValueChecksum != test04ValueChecksum)
    {
        printf("- [FAILED]: Checksums don't match test04KeyChecksum: %d, but localKeyChecksum: %d | test04ValueChecksum: %d, but localValueChecksum: %d\n", test04KeyChecksum, localKeyChecksum, test04ValueChecksum, localValueChecksum);
        clear_dict_n_master_table(&dict, dict_id);
        return 1;
    }
    puts("- [WORKED]");

    /* Clearing Boiler Plate */

    if (clear_dict_n_master_table(&dict, dict_id) != err_ok)
    {
        printf("[FAILED]: to close/delete dictionary and master table \n");
        return 1;
    }

    /* Clearing Boiler Plate */

    puts("[TEST07 SUCCESS]");
    return 0;
}

/* TEST08: Testing Duplicates with Read and Cursor using test08Keys keys and test08Values values */
static int test08(ion_dictionary_type_t current_type)
{
    puts("[TEST08]: Duplicate Test");

    /* Boiler Plate Begin */
    ion_dictionary_handler_t handler = {0};
    ion_dictionary_t dict = {0};
    ion_status_t status = {0};

    status.error = ion_init_master_table();
    if (status.error != err_ok)
    {
        printf("[FAILED]: to initialize master table: %d\n", status.error);
        return 1;
    }

    /* Used to bind dictionary specific function to the handler */
    ion_switch_handler(current_type, &handler);

    ion_key_type_t k_type = key_type_numeric_signed;
    ion_key_size_t k_size = sizeof(KEY_TYPE);
    ion_value_size_t v_size = sizeof(VALUE_TYPE);

    status.error = ion_master_table_create_dictionary(&handler, &dict, k_type, k_size, v_size, DICT_SIZE_GLOB);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to create the dictionary with error %d\n", status.error);
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }

    /* Read the dictionary id for later opening usage  */
    ion_dictionary_id_t dict_id = dict.instance->id;

    /* Boiler Plate End */

    INDEX_TYPE i = 0;
    VALUE_TYPE in_value = 0;
    VALUE_TYPE out_value = 0;

    printf("INSERT test08Keys keys with test08Values values ");
    for (i = 0; i < TEST_08_KEY_LENGTH; i++)
    {
        in_value = test08Values[i];
        status = dictionary_insert(&dict, IONIZE(test08Keys[i], KEY_TYPE), &in_value);
        if (status.error != err_ok)
        {
            if (status.error == err_duplicate_key)
            {
                puts("- [OVER: DUPLICATES ARE NOT ALLOWED]");
                clear_dict_n_master_table(&dict, dict_id);
                return 0;
            }
            printf("- [FAILED]: Insert Status at i: %d with error %d\n", i, status.error);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    INDEX_TYPE walked_in = 0;

    printf("READ FIRST 3 test08Keys keys and EXPECT ONE of test08Values ");
    for (i = 0; i < TEST_08_KEY_DUPLICATES_LENGTH; i++)
    {
        status = dictionary_get(&dict, IONIZE(test08Keys[i], KEY_TYPE), RETRIEVE_SPACE_VALUE);

        if (status.error != err_ok)
        {
            printf("- [FAILED]: Read Status at i: %d with error %d \n", i, status.error);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
        else
        {
            out_value = NEUTRALIZE(RETRIEVE_SPACE_VALUE, VALUE_TYPE);
            for (int u = 0; u < test08KeyDuplicates[i]; u++)
            {
                if ((out_value < 0 && out_value == (test08Values[i] - u)) || (out_value > 0 && out_value == (test08Values[i] + u)))
                {
                    walked_in += 1;
                }
            }
        }
    }
    if (walked_in != TEST_08_KEY_DUPLICATES_LENGTH)
    {
        printf("- [FAILED]: Didn't read one of expected values: Values Read %d \n", walked_in);
        clear_dict_n_master_table(&dict, dict_id);
        return 1;
    }
    puts("- [WORKED]");

    ion_predicate_t predicate = {0};
    ion_dict_cursor_t *cursor = NULL;
    ion_cursor_status_t cursor_status = {0};

    ion_record_t ion_record = {0};
    ion_record.key = RETRIEVE_SPACE_KEY;
    ion_record.value = RETRIEVE_SPACE_VALUE;

    int duplicates = 0;

    printf("TRAVERSING EQUALITY CURSOR for Duplicates & count duplicates + their values ");

    for (i = 0; i < TEST_08_KEY_DUPLICATES_LENGTH; i++)
    {
        dictionary_build_predicate(&predicate, predicate_equality, IONIZE(test08Keys[i], KEY_TYPE));
        if (dictionary_find(&dict, &predicate, &cursor) == err_not_implemented)
        {
            puts("[OVER: FIND NOT IMPLEMENTED]");
            clear_dict_n_master_table(&dict, dict_id);
            return 0;
        }

        VALUE_TYPE localDuplicateValueChecksum = 0;
        duplicates = 0;

        while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized)
        {
            duplicates += 1;
            localDuplicateValueChecksum += NEUTRALIZE(ion_record.value, VALUE_TYPE);
        }
        if (duplicates != test08KeyDuplicates[i] ||
            localDuplicateValueChecksum != test08DuplicatesChecksums[i])
        {
            printf("- [FAILED]: Duplicate count: %d, expected: %d\n | \
            DuplicateValueChecksum: %d, expected: %d",
                   duplicates, test08KeyDuplicates[i],
                   localDuplicateValueChecksum, test08DuplicatesChecksums[i]);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
        cursor->destroy(&cursor);
    }
    puts("- [WORKED]");

    /* Clearing Boiler Plate */

    if (clear_dict_n_master_table(&dict, dict_id) != err_ok)
    {
        printf("[FAILED]: to close/delete dictionary and master table \n");
        return 1;
    }

    /* Clearing Boiler Plate */

    puts("[TEST08 SUCCESS]");
    return 0;
}

/* TEST09: Testing config_t usage and the manipulation */
static int test09(ion_dictionary_type_t current_type)
{
    puts("[TEST09]: Config Test");

    /* Boiler Plate Begin */
    ion_dictionary_handler_t handler = {0};
    ion_dictionary_t dict = {0};
    ion_status_t status = {0};

    status.error = ion_init_master_table();
    if (status.error != err_ok)
    {
        printf("[FAILED]: to initialize master table: %d\n", status.error);
        return 1;
    }

    /* Used to bind dictionary specific function to the handler */
    ion_switch_handler(current_type, &handler);

    ion_key_type_t k_type = key_type_numeric_signed;
    ion_key_size_t k_size = sizeof(KEY_TYPE);
    ion_value_size_t v_size = sizeof(VALUE_TYPE);

    status.error = ion_master_table_create_dictionary(&handler, &dict, k_type, k_size, v_size, DICT_SIZE_GLOB);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to create the dictionary with error %d\n", status.error);
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }

    /* Read the dictionary id for later opening usage  */
    ion_dictionary_id_t dict_id = dict.instance->id;

    /* Boiler Plate End */

    ion_dictionary_config_info_t config = {0};

    printf("Read config from master table ");
    status.error = ion_lookup_in_master_table(dict_id, &config);
    if(status.error != err_ok) {
        printf("- [FAILED]: to lookup the config with error %d\n", status.error);
        clear_dict_n_master_table(&dict, dict_id);
        return 1;
    }
    puts("- [WORKED]");

    printf("Check config parameters from master table ");
    if(config.id != dict_id || 
    config.type != k_type ||
    config.key_size != k_size ||
    config.value_size != v_size ||
    config.dictionary_size != DICT_SIZE_GLOB ||
    config.dictionary_type != current_type ||
    config.dictionary_status != ion_dictionary_status_ok ||
    config.use_type != 0) {
        printf("- [FAILED]: Read config isn't as expected\n");
        clear_dict_n_master_table(&dict, dict_id);
        return 1;
    }
    puts("- [WORKED]");

    printf("Write config use_type parameter ");
    config.use_type = 0xff;
    status.error = ion_master_table_write(&config, ION_MASTER_TABLE_CALCULATE_POS);
    if(status.error != err_ok) {
        printf("- [FAILED]: to write the config with error %d\n", status.error);
        clear_dict_n_master_table(&dict, dict_id);
        return 1;
    }
    puts("- [WORKED]");

    printf("Read config from master table again");
    status.error = ion_lookup_in_master_table(dict_id, &config);
    if(status.error != err_ok) {
        printf("- [FAILED]: to lookup the config a second time with error %d\n", status.error);
        clear_dict_n_master_table(&dict, dict_id);
        return 1;
    }
    puts("- [WORKED]");

    printf("Check changed config use_type parameter ");
    if(config.use_type != 0xff) {
        printf("- [FAILED]: to read the config.use_type\n");
        clear_dict_n_master_table(&dict, dict_id);
        return 1;
    }
    puts("- [WORKED]");

    /* Clearing Boiler Plate */

    if (clear_dict_n_master_table(&dict, dict_id) != err_ok)
    {
        printf("[FAILED]: to close/delete dictionary and master table \n");
        return 1;
    }

    /* Clearing Boiler Plate */

    puts("[TEST09 SUCCESS]");
    return 0;
}

/* TEST10: Testing byte arrays as values and use test02Keys */
static int test10(ion_dictionary_type_t current_type)
{
    puts("[TEST10]: Config Test");

    /* Boiler Plate Begin */
    ion_dictionary_handler_t handler = {0};
    ion_dictionary_t dict = {0};
    ion_status_t status = {0};

    status.error = ion_init_master_table();
    if (status.error != err_ok)
    {
        printf("[FAILED]: to initialize master table: %d\n", status.error);
        return 1;
    }

    /* Used to bind dictionary specific function to the handler */
    ion_switch_handler(current_type, &handler);

    ion_key_type_t k_type = key_type_numeric_signed;
    ion_key_size_t k_size = sizeof(KEY_TYPE);
    ion_value_size_t v_size = 30;

    status.error = ion_master_table_create_dictionary(&handler, &dict, k_type, k_size, v_size, DICT_SIZE_GLOB);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to create the dictionary with error %d\n", status.error);
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }

    /* Read the dictionary id for later opening usage  */
    ion_dictionary_id_t dict_id = dict.instance->id;

    /* Boiler Plate End */

    int32_t i = 0;  /* signed integer because the second for loop count to zero */
    int32_t u = 0;

    /* Be carefull with malloc and always allocate as much memory space as set at creation of the dictionary */
    /* With less memory given, a segmentation error might occur */
    ion_byte_t* local_retrieve_space_value_ptr = malloc(v_size); /* Definition */
    memset(local_retrieve_space_value_ptr, 0, v_size); /* Initialize to 0 */

    printf("INSERT test02Keys keys with generated values of value size 30 ");
    for (i = 0; i < (int32_t)TEST_02_KEY_LENGTH; i++)
    {
        for (u = 0; u < v_size; u++)
        {
            local_retrieve_space_value_ptr[u] = (uint8_t) i;
        }

        status = dictionary_insert(&dict, IONIZE(test02Keys[i], KEY_TYPE), local_retrieve_space_value_ptr);
        if (status.error != err_ok)
        {
            printf("- [FAILED]: Insert Status at i: %d with error %d\n", i, status.error);
            /* Free allocated space by malloc */
            free(local_retrieve_space_value_ptr);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
    }
    puts("- [WORKED]");

    printf("READ test02Keys keys and generated values");
    for (i = (int32_t)TEST_02_KEY_LENGTH - 1; i >= 0 ; i--)
    {
        memset(local_retrieve_space_value_ptr, 0, v_size); /* Set to 0 */
        status = dictionary_get(&dict, IONIZE(test02Keys[i], KEY_TYPE), local_retrieve_space_value_ptr);

        if (status.error != err_ok)
        {
            printf("- [FAILED]: Read Status at i: %d with error %d \n", i, status.error);
            /* Free allocated space by malloc */
            free(local_retrieve_space_value_ptr);
            clear_dict_n_master_table(&dict, dict_id);
            return 1;
        }
        else
        {
            for (u = 0; u < v_size; u++)
            {
                if (local_retrieve_space_value_ptr[u] != (uint8_t) i) 
                {
                    printf("- [FAILED]: Read false retrieved value at i: %d with u %d \n", i, u);
                    /* Free allocated space by malloc */
                    free(local_retrieve_space_value_ptr);
                    clear_dict_n_master_table(&dict, dict_id);
                    return 1;
                }
            }
        }
    }
    puts("- [WORKED]");

    /* Free allocated space by malloc */
    free(local_retrieve_space_value_ptr);

    /* Clearing Boiler Plate */

    if (clear_dict_n_master_table(&dict, dict_id) != err_ok)
    {
        printf("[FAILED]: to close/delete dictionary and master table \n");
        return 1;
    }

    /* Clearing Boiler Plate */

    puts("[TEST10 SUCCESS]");
    return 0;
}


/* --- TEST End --- */

int main(void)
{
    puts("[IONDB TEST]");

    /* Mount Begin */
#if MODULE_VFS && MODULE_FATFS

#if MODULE_MTD_SDCARD
    INDEX_TYPE error = 0;
    for (unsigned int i = 0; i < SDCARD_SPI_NUM; i++)
    {
        mtd_sdcard_devs[i].base.driver = &mtd_sdcard_driver;
        mtd_sdcard_devs[i].sd_card = &sdcard_spi_devs[i];
        mtd_sdcard_devs[i].params = &sdcard_spi_params[i];
        fatfs_mtd_devs[i] = &mtd_sdcard_devs[i].base;

        error = mtd_init(&mtd_sdcard_devs[i].base);
        if (error != 0)
            printf("%d mtd_init error code %d \n", __LINE__, error);
    }
#endif

#if defined(MODULE_MTD_NATIVE) || defined(MODULE_MTD_MCI)
    fatfs_mtd_devs[fatfs.vol_idx] = mtd0;
#elif defined(MODULE_FATFS)
    fatfs_mtd_devs[fatfs.vol_idx] = mtd1;
#endif

    INDEX_TYPE ret = vfs_mount(&_test_vfs_mount);
    if (ret != 0)
    {
        printf("[MOUNT]: NOT SUCCESSFULL\n");
        return -1;
    }
#endif

    /* Mount End */

    INDEX_TYPE i = 0;
    INDEX_TYPE u = 0;

    /* Calculate Checksum of test04-Arrays */
    for (i = 0; i < TEST_04_KEY_LENGTH; i++)
    {
        test04KeyChecksum += test04Keys[i];
        test04ValueChecksum += test04Values[i];
    }

    /* Calculate duplicate checksum for test08*/
    for (i = 0; i < TEST_08_KEY_LENGTH; i++)
    {
        for (u = 0; u < TEST_08_KEY_DUPLICATES_LENGTH; u++)
        {
            if (test08Keys[i] == test08Keys[u])
            {
                test08DuplicatesChecksums[u] += test08Values[i];
            }
        }
    }

    /* Apply special values */

    DICT_SIZE_GLOB = 100;
    
    ion_dictionary_handler_t test_handler = {0};
    INDEX_TYPE testsFailed = 0;

    
    for (i = 0; i < AMOUNT_TYPES_DICTIONARY; i++)
    {
        if(ion_switch_handler(i, &test_handler) == err_uninitialized) {
            continue;
        }
        printf("################ ");
        printf_dictionary_type(i);
        puts(" ################");
        testsFailed += test01(i);
        puts("");
        testsFailed += test02(i);
        puts("");
        testsFailed += test03(i);
        puts("");
        testsFailed += test04(i);
        puts("");
        testsFailed += test05(i);
        puts("");
        testsFailed += test06(i);
        puts("");
        testsFailed += test07(i);
        puts("");
        testsFailed += test08(i);
        puts("");
        testsFailed += test09(i);
        puts("");
        testsFailed += test10(i);
        puts("");
        puts("");
    }
    printf("[%d TEST(S) FAILED]\n", testsFailed);

    /* Unmount Begin */

#if MODULE_VFS && MODULE_FATFS
    ret = vfs_umount(&_test_vfs_mount);
    if (ret != 0)
    {
        printf("[UNMOUNT]: NOT SUCESSFULL\n");
    }
    else
    {
        printf("[UNMOUNT]: SUCESSFULL\n");
    }

#endif

    /* Unmount End */

    return 0;
}


#include <stdio.h>
#include <string.h>
#include "ion_master_table.h"
#include "shell.h"
#include "debug.h"

static int _hello(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    printf("%s\n", "Hello world");
    return 1;
}
#define KEY_TYPE int32_t
#define VALUE_TYPE int32_t
#define INDEX_TYPE uint32_t
ion_byte_t RETRIEVE_SPACE_KEY[sizeof(KEY_TYPE)] = {0};
ion_byte_t RETRIEVE_SPACE_VALUE[sizeof(VALUE_TYPE)] = {0};
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

KEY_TYPE test02Keys[] = TEST_02_KEYS;
VALUE_TYPE test02Values[] = TEST_02_VALUES;
#define TEST_02_KEY_LENGTH ARRAY_SIZE(test02Keys)

INDEX_TYPE DICT_SIZE_GLOB = 100;
static ion_err_t clear_dict_n_master_table(ion_dictionary_t *dict, ion_dictionary_id_t dict_id);
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

/* TEST02: Read, Insert, Read, Delete, Read of Keys of test02Keys and test02Values */
static int _test(int argc, char **argv)
{
    (void)argc;
    (void)argv;
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
    ion_switch_handler(0, &handler);

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

static const shell_command_t shell_commands[] = {
    {"hello", "Prints hello world", _hello},
    {"test", "test method", _test},
    {NULL, NULL, NULL}};

int main(void)
{
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}

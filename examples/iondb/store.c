#include "ion_master_table.h"
#include "debug.h"

#ifdef MODULE_MTD_SDCARD
#include "mtd_sdcard.h"
#include "sdcard_spi.h"
#include "sdcard_spi_params.h"
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
#define KEY_TYPE int32_t
#define VALUE_TYPE int32_t
#define INDEX_TYPE uint32_t
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

    if (ion_delete_master_table() != err_ok)
    {
        err += 1;
        DEBUG("Master Table Delete Failed \n");
    }

    return err;
}

int _test(void)
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
int db_init(void)
{

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
    return 0;
}
int save(int key, int value)
{
    printf("Saving Key:%i, Value:%i\n", key, value);
    return 0;
};
int load(int key, int value)
{
    printf("Loading Key:%i, Value:%i\n", key, value);
    return 0;
};
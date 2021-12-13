#include "ion_master_table.h"
#include "debug.h"
#ifndef person_t
struct person
{
    int16_t id;
    int status;
    double lat;
    double lon;
    uint64_t timestamp;
};
typedef struct person person_t;
#endif
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
#define KEY_TYPE int32_t
#define VALUE_TYPE int32_t
#define INDEX_TYPE uint32_t
ion_byte_t RETRIEVE_SPACE_KEY[sizeof(KEY_TYPE)] = {0};
ion_byte_t RETRIEVE_SPACE_VALUE[sizeof(VALUE_TYPE)] = {0};

KEY_TYPE test04Key = -32;
VALUE_TYPE test04Value = -100;
VALUE_TYPE test04UpdatedValue = 34;

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

ion_dictionary_t dict = {0};
ion_dictionary_handler_t handler = {0};
ion_status_t status = {0};

int init_db(void)
{
    puts("[DB:Bgin mouning sd-card]");
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
    ion_dictionary_type_t current_type = 0;
    puts("[TEST04]: Update, Read, Delete, Insert, Update, Read of Keys of Test_04_Key and Test_04_Values");

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

    return 0;
}
int close_db(void)
{

    if (clear_dict_n_master_table(&dict, dict.instance->id) != err_ok)
    {
        printf("[FAILED]: to close/delete dictionary and master table \n");
        return 1;
    }

    /* Clearing Boiler Plate */

#if MODULE_VFS && MODULE_FATFS
    int ret = vfs_umount(&_test_vfs_mount);
    if (ret != 0)
    {
        printf("[UNMOUNT]: NOT SUCESSFULL\n");
        return 1;
    }
    else
    {
        printf("[UNMOUNT]: SUCESSFULL\n");
        return 0;
    }

#endif
    return 0;
}
int save_person2(person_t person)
{
    init_db();

    INDEX_TYPE i = 0;
    VALUE_TYPE in_value = 0;
    VALUE_TYPE out_value = 0;

    printf("UPDATE test04Keys keys with test04UpdatedValues values ");
    in_value = test04UpdatedValue;
    status = dictionary_update(&dict, IONIZE(test04Key, KEY_TYPE), &in_value);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: Update Status at i: %d with error %d\n", i, status.error);
        close_db();
        return 1;
    }
    puts("- [WORKED]");

    printf("READ test04Keys keys and expect test04UpdatedValues ");
    status = dictionary_get(&dict, IONIZE(test04Key, KEY_TYPE), RETRIEVE_SPACE_VALUE);
    out_value = NEUTRALIZE(RETRIEVE_SPACE_VALUE, VALUE_TYPE);
    if (status.error != err_ok || out_value != test04UpdatedValue)
    {
        printf("- [FAILED]: Read Status at i: %d with error %d | Read: %d, Expected: %d\n", i, status.error, out_value, test04UpdatedValue);
        close_db();
        return 1;
    }
    puts("- [WORKED]");

    printf("DELETE test04Keys keys ");
    status = dictionary_delete(&dict, IONIZE(test04Key, KEY_TYPE));
    if (status.error != err_ok)
    {
        printf("- [FAILED]: Delete Status at i: %d with error %d\n", i, status.error);
        close_db();
        return 1;
    }
    puts("- [WORKED]");

    printf("INSERT test04Keys keys with test04Values values ");
    in_value = test04Value;
    status = dictionary_insert(&dict, IONIZE(test04Key, KEY_TYPE), &in_value);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: Insert Status at i: %d with error %d\n", i, status.error);
        close_db();
        return 1;
    }
    puts("- [WORKED]");

    printf("UPDATE test04Keys keys with test04UpdatedValues values ");
    in_value = test04UpdatedValue;
    status = dictionary_update(&dict, IONIZE(test04Key, KEY_TYPE), &in_value);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: Update Status at i: %d with error %d\n", i, status.error);
        close_db();
        return 1;
    }
    puts("- [WORKED]");

    printf("READ test04Keys keys and EXPECT test04UpdatedValues ");
    status = dictionary_get(&dict, IONIZE(test04Key, KEY_TYPE), RETRIEVE_SPACE_VALUE);
    out_value = NEUTRALIZE(RETRIEVE_SPACE_VALUE, VALUE_TYPE);
    if (status.error != err_ok || out_value != test04UpdatedValue)
    {
        printf("- [FAILED]: Read Status at i: %d with error %d | Read: %d, Expected: %d\n", i, status.error, out_value, test04UpdatedValue);
        close_db();
        return 1;
    }
    puts("- [WORKED]");
    close_db();
    return 0;
}

// person_t[] get_all_persons(void)
// {

//     // VALUE_TYPE out_value = 0;
//     // for (i = 0; i < TEST_02_KEY_LENGTH; i++)
//     // {
//     //     status = dictionary_get(&dict, IONIZE(test02Keys[i], KEY_TYPE), RETRIEVE_SPACE_VALUE);
//     //     out_value = NEUTRALIZE(RETRIEVE_SPACE_VALUE, VALUE_TYPE);
//     //     if (status.error != err_ok || out_value != test02Values[i])
//     //     {
//     //         printf("- [FAILED]: Read Status at i: %d with error %d | Read: %d, Expected %d\n", i, status.error, out_value, test02Values[i]);
//     //         clear_dict_n_master_table(&dict, dict_id);
//     //         return 1;
//     //     }
//     // }
//     // puts("- [WORKED]");
//     return {};
// }

person_t find_person_by_id(char *id)
{
    person_t p = {.id = 22};
    return p;
}
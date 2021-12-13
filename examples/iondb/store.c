#include "ion_master_table.h"
#include "debug.h"
#ifndef person_t
struct person
{
    char id[14];
    int status;
    double lat;
    double lon;
    uint64_t timestamp;
};
typedef struct person person_t;
typedef struct person *personPtr;
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
#define KEY_TYPE char[]
#define VALUE_TYPE person_t
#define INDEX_TYPE uint32_t
ion_key_type_t k_type = key_type_char_array;
ion_key_size_t k_size = 14 * sizeof(char);
ion_value_size_t v_size = sizeof(VALUE_TYPE);
// ion_byte_t RETRIEVE_SPACE_KEY[k_size] = {0};
// ion_byte_t RETRIEVE_SPACE_VALUE[sizeof(VALUE_TYPE)] = {0};

// KEY_TYPE test04Key = -32;
// VALUE_TYPE test04Value = -100;
// VALUE_TYPE test04UpdatedValue = 34;

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

int db_init(void)
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

    status.error = ion_init_master_table();
    if (status.error != err_ok)
    {
        printf("[FAILED]: to initialize master table: %d\n", status.error);
        return 1;
    }

    /* Used to bind dictionary specific function to the handler */
    ion_switch_handler(current_type, &handler);

    status.error = ion_master_table_create_dictionary(&handler, &dict, k_type, k_size, v_size, DICT_SIZE_GLOB);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: to create the dictionary with error %d\n", status.error);
        clear_dict_n_master_table(NULL, 0);
        return 1;
    }

    return 0;
}
int db_close(void)
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

int save_person(person_t person)
{
    printf("Updating person\n");
    status = dictionary_update(&dict, IONIZE(*(person.id), KEY_TYPE), &person);
    if (status.error != err_ok)
    {
        printf("- [FAILED]: Update Status, error %d\n", status.error);
        return 1;
    }
    puts("- [WORKED]");
    return 0;
}

    ion_byte_t RETRIEVE_SPACE_VALUE[sizeof(VALUE_TYPE)] = {0};

void printPerson2(person_t p)
{
    printf("Person{id=%.14s,lat=%f,lon=%f,status=%d,timestamp=%" PRIu64 "}\n", p.id, p.lat, p.lat, p.status, p.timestamp);
}
int find_person_by_id(char *id, personPtr p)
{
    // person_t* p = {0};
    printf("Finding person by id\n");
    status = dictionary_get(&dict, IONIZE(*id, KEY_TYPE), RETRIEVE_SPACE_VALUE);
    VALUE_TYPE out = NEUTRALIZE(RETRIEVE_SPACE_VALUE, VALUE_TYPE);
    if (status.error != err_ok)
    {
        printf("- [FAILED]:  Error %d\n", status.error);
        return 1;
    }
    puts("- [WORKED]");
    // p = &out;
    printPerson2(out);
    return 0;
}

// printf("READ test04Keys keys and expect test04UpdatedValues ");
// status = dictionary_get(&dict, IONIZE(test04Key, KEY_TYPE), RETRIEVE_SPACE_VALUE);
// out_value = NEUTRALIZE(RETRIEVE_SPACE_VALUE, VALUE_TYPE);
// if (status.error != err_ok || out_value != test04UpdatedValue)
// {
//     printf("- [FAILED]: Read Status at i: %d with error %d | Read: %d, Expected: %d\n", i, status.error, out_value, test04UpdatedValue);
//     close_db();
//     return 1;
// }
// puts("- [WORKED]");

// printf("DELETE test04Keys keys ");
// status = dictionary_delete(&dict, IONIZE(test04Key, KEY_TYPE));
// if (status.error != err_ok)
// {
//     printf("- [FAILED]: Delete Status at i: %d with error %d\n", i, status.error);
//     close_db();
//     return 1;
// }
// puts("- [WORKED]");

// printf("INSERT test04Keys keys with test04Values values ");
// in_value = test04Value;
// status = dictionary_insert(&dict, IONIZE(test04Key, KEY_TYPE), &in_value);
// if (status.error != err_ok)
// {
//     printf("- [FAILED]: Insert Status at i: %d with error %d\n", i, status.error);
//     close_db();
//     return 1;
// }
// puts("- [WORKED]");

// printf("UPDATE test04Keys keys with test04UpdatedValues values ");
// in_value = test04UpdatedValue;
// status = dictionary_update(&dict, IONIZE(test04Key, KEY_TYPE), &in_value);
// if (status.error != err_ok)
// {
//     printf("- [FAILED]: Update Status at i: %d with error %d\n", i, status.error);
//     close_db();
//     return 1;
// }
// puts("- [WORKED]");

// printf("READ test04Keys keys and EXPECT test04UpdatedValues ");
// status = dictionary_get(&dict, IONIZE(test04Key, KEY_TYPE), RETRIEVE_SPACE_VALUE);
// out_value = NEUTRALIZE(RETRIEVE_SPACE_VALUE, VALUE_TYPE);
// if (status.error != err_ok || out_value != test04UpdatedValue)
// {
//     printf("- [FAILED]: Read Status at i: %d with error %d | Read: %d, Expected: %d\n", i, status.error, out_value, test04UpdatedValue);
//     close_db();
//     return 1;
// }
// puts("- [WORKED]");
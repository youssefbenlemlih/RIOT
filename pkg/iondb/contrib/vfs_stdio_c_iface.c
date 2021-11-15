/*
 * Copyright (C) HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkg_iondb
 * @{
 *
 * @file
 * @brief       This code contains implementations for stdio.h file functions
                for vfs interface. Since the iondb lib does not have a vfs interface this is the wrapper.
 *
 * @author      Tobias Westphal <tobias.westphal@haw-hamburg.de>
 *
 * @}
 */

#include "vfs_stdio_c_iface.h"

#if defined(VFS)

#define FILEPATH_STATIC_LENGTH (VFS_NAME_MAX + MNT_PATH_LENGTH)
char filepath_static[FILEPATH_STATIC_LENGTH] = {0};

/**
@brief        A structure that translates a file object to a C-compatible
            struct.
*/

struct _VFS_File
{
    int fd_number; /**< The VFS File number > */
    char *filepath;
};

int ion_vfs_fclose(
    VFS_FILE *stream)
{
    int ret = -1;

    if (stream != NULL)
    {
        ret = vfs_close(stream->fd_number);
    }
    else
    {
        return ret;
    }

#if IONDB_VFS_DEBUG
    printf("%d ion_vfs_fclose ret = %d \n", __LINE__, ret);
#endif

    /* Free memory that has been allocated in ion_vfs_fopen */
    free(stream->filepath);
    free(stream);
    stream = NULL;
    return ret;
}

int ion_vfs_feof(
    VFS_FILE *stream)
{
    off_t cur_pos = vfs_lseek(stream->fd_number, 0, SEEK_CUR);
    if (cur_pos == vfs_lseek(stream->fd_number, 0, SEEK_END))
    {
        return -1;
    }
    vfs_lseek(stream->fd_number, cur_pos, SEEK_SET);
    return 0;
}

int ion_vfs_fflush(
    VFS_FILE *stream)
{
    int ret = -1;
    ion_fpos_t file_pos;

    if (stream == NULL)
    {
        /* Might produce unfreed data */
        return ret;
    }

    /* To remember the file pos */
    file_pos = ion_vfs_ftell(stream);

    /* To provocate writing to a file */
    ret = vfs_close(stream->fd_number);

    stream->fd_number = vfs_open(stream->filepath, O_RDWR, 0);

    if (stream->fd_number < 0)
    {
        /* Is freed only when an error occurs */
        free(stream->filepath);
        free(stream);
        return stream->fd_number;
    }
    /* -1 on failure, 0 on success */
    return ion_vfs_fsetpos(stream, &file_pos);
}

int ion_vfs_fsetpos(
    VFS_FILE *stream,
    ion_fpos_t *pos)
{
    off_t new_pos;
    if ((stream != NULL))
    {
        new_pos = vfs_lseek(stream->fd_number, *(off_t *)pos, SEEK_SET);
        if (new_pos >= 0)
        {
            /* On success */
            return 0;
        }
    }
    return -1;
}

int ion_vfs_fgetpos(
    VFS_FILE *stream,
    ion_fpos_t *pos)
{
    *pos = vfs_lseek(stream->fd_number, 0, SEEK_CUR);
    return 0;
}

VFS_FILE *
ion_vfs_fopen(
    char *filename,
    char *mode)
{

    int operation;
    ion_boolean_e seek_start = boolean_false;

    if (strcmp("error", MNT_PATH_VFS) == 0)
    {
        printf("%d Error  MNT_PATH_VFS Not Redefined\n", __LINE__);
        return NULL;
    }
    if (filename == NULL)
    {
        printf("%d Error filename is Null\n", __LINE__);
        return NULL;
    }

    if ((strcmp(mode, "r") == 0) || (strcmp(mode, "rb") == 0))
    {
        /*    Open a file for reading. The file must exist. */
        /* check to see if file exists */

        operation = O_RDONLY;
    }
    else if ((strcmp(mode, "w") == 0) || (strcmp(mode, "wb") == 0))
    {
        /* Create an empty file for writing. */
        /* If a file with the same name already exists */
        /* its content is erased and the file is */
        /* considered as a new empty file. */

        operation = O_WRONLY | O_CREAT | O_TRUNC;
        /* Open a file for update both reading and writing. The file must exist. */
    }
    else if (strstr(mode, "r+") != NULL)
    {
        operation = O_RDWR;
        seek_start = boolean_true;
    }
    /* Create an empty file for both reading and writing. */
    else if (strstr(mode, "w+") != NULL)
    {
        /* if the file exists it its truncated */

        operation = O_RDWR | O_CREAT | O_TRUNC;
        seek_start = boolean_true;
    }
    else if (strcmp(mode, "a+") == 0)
    {
        operation = O_RDWR | O_CREAT | O_APPEND;
    }
    else
    {
#if IONDB_VFS_DEBUG
        printf("Incorrect Args\n");
#endif
        return NULL; /*incorrect args */
    }

    /* Should be freed in the future */
    struct _VFS_File *file = malloc(sizeof(struct _VFS_File));

    int filepath_length = strlen(filename) + sizeof(MNT_PATH_VFS);
    file->filepath = malloc(filepath_length);

    /** Create char array to concat Mount Path to the beginning of the filename */
    strcpy(file->filepath, MNT_PATH_VFS);
    strcat(file->filepath, filename);

    for (int i = 0; i < filepath_length; i++)
    {
        file->filepath[i] = toupper(file->filepath[i]);
    }

    file->fd_number = vfs_open(file->filepath, operation, 0);

    if (file->fd_number < 0)
    {

#if IONDB_VFS_DEBUG
        printf("%d FD Number Error %d\n", __LINE__, file->fd_number);
#endif
        /* Is freed only when an error occurs */
        free(file->filepath);
        free(file);
        return NULL;
    }

    if (seek_start == 1) /* TODO is this actually Necesarry? */
    {
        /* TODO Error not handled */
        vfs_lseek(file->fd_number, (off_t)0, SEEK_SET);
    }

#if IONDB_VFS_DEBUG
    printf("%d ion_vfs_fopen ret = file \n", __LINE__);
#endif

    return file;
}

size_t
ion_vfs_fread(
    void *ptr,
    size_t size,
    size_t nmemb,
    VFS_FILE *stream)
{
    /* read (size) amount of bytes per nmemb * (nmemb) total number of items */
    int num_bytes = vfs_read(stream->fd_number, (char *)ptr, size * nmemb);

    return num_bytes / size;
}

int ion_vfs_fseek(
    VFS_FILE *stream,
    long int offset,
    int whence)
{
    if (NULL == stream)
    {
        return -1;
    }

    off_t new_pos = vfs_lseek(stream->fd_number, (off_t)offset, whence);

    if (new_pos < 0)
    {
        return -1;
    }
    return 0;
}

long int
ion_vfs_ftell(
    VFS_FILE *stream)
{
    off_t pos = vfs_lseek(stream->fd_number, 0, SEEK_CUR);

    if (((long int)pos) < 0)
    {
        pos = -1;
    }

    return (long int)pos;
}

size_t
ion_vfs_fwrite(
    void *ptr,
    size_t size,
    size_t nmemb,
    VFS_FILE *stream)
{
    ssize_t bytes_written = vfs_write(stream->fd_number, (uint8_t *)ptr, size * nmemb);

    if (bytes_written < 0)
    {
        return 0;
    }

    return ((size_t)(((size_t)bytes_written) / ((size_t)size)));
}

int ion_vfs_remove(
    char *filename)
{
    memset(filepath_static, 0, FILEPATH_STATIC_LENGTH);
    strcpy(filepath_static, MNT_PATH_VFS);
    strcat(filepath_static, filename);

    for (uint i = 0; i < FILEPATH_STATIC_LENGTH; i++)
    {
        filepath_static[i] = toupper(filepath_static[i]);
    }

#if IONDB_VFS_DEBUG
    DUMP(filepath_static, "%s");
#endif

    return (vfs_unlink(filepath_static) == 0) ? 0 : 1;
}

void ion_vfs_rewind(
    VFS_FILE *stream)
{
    vfs_lseek(stream->fd_number, (off_t)0, SEEK_SET);
}

void ion_vfs_printint(
    int ion_vfs_printint_parameter)
{
    DUMP(ion_vfs_printint_parameter, "%i");
}

int ION_VFS_File_Exists(
    char *filename)
{
    if (strcmp("error", MNT_PATH_VFS) == 0)
    {
        return -1;
    }

    /** Create char array to concat Mount Path to the beginning of the filename */
    memset(filepath_static, 0, FILEPATH_STATIC_LENGTH);
    strcpy(filepath_static, MNT_PATH_VFS);
    strcat(filepath_static, filename);

    /* return fd number on success (>= 0); <0 on error */
    int fd = vfs_open(filepath_static, O_RDONLY, 0);
    if (fd >= 0)
    {
        vfs_close(fd);
        return 1;
    }
    else
    {
        return 0;
    }
}

int VFS_File_Delete_All(
    void)
{
    // File root = SD.open("/");
    //
    // while (true) {
    //     File entry = root.openNextFile();
    //
    //     if (!entry) {
    //         break;
    //     }
    //
    //     entry.close();
    //
    //     bool is_ok = SD.remove(entry.name());
    //
    //     if (!is_ok) {
    //         return false;
    //     }
    // }

    return err_not_implemented;
}

#endif

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
 * @brief       This code contains definitions for stdio.h file functions
                for vfs interface. Since the iondb lib does not have a vfs interface this is the wrapper.
 *
 * @author      Tobias Westphal <tobias.westphal@haw-hamburg.de>
 *
 * @}
 */


#if !defined(VFS_STDIO_C_IFACE_H_)
#define VFS_STDIO_C_IFACE_H_

#include "kv_system.h"

#if defined(VFS)

/** A Mount path for the VFS file Interface is missing if this directive is still error. Define one to use this interface.*/
#ifndef MNT_PATH_VFS
#define MNT_PATH_VFS "error"
#endif

#if ION_DEBUG
#define IONDB_VFS_DEBUG (0)
#endif

#include <ctype.h>
#include <vfs.h>
#include <fcntl.h>

#if defined(__cplusplus)
extern "C" {
#endif

#include "kv_stdio_intercept.h"

/**
@brief        Wrapper around vfs File.
*/
typedef struct _VFS_File VFS_FILE;


/**
@brief        Wrapper around vfs file close method.
@param        stream
                A pointer to the C file struct type associated with an SD
                file object.
@returns    @c 0 on success, <0 on error.
*/
int
ion_vfs_fclose(
    VFS_FILE *stream
);

/**
@brief        This function tests for the end-of-file indicator in the given
            stream.
@param        stream
                A pointer to the C file struct type associated with an SD
                file object representing the file to test.
@returns    A non-zero (@c -1) value if the end of file has been reached,
            @c 0 otherwise.
*/
int
ion_vfs_feof(
    VFS_FILE *stream
);

/**
@brief        Flush the output buffer of a stream to the file.
@param        stream
                A pointer to the C file struct type associated with an SD
                file object representing the file to flush.
@returns    @c 0, always.
*/
int
ion_vfs_fflush(
    VFS_FILE *stream
);

/**
@brief        Get the position of the file's cursor.
@details    Wrapper around vfs file position method.
@param        stream
                A pointer to the C file struct type associated with an
                SD file object.
@param        pos
                A pointer to a file position variable to set to the current
                position of the file's cursor.
@returns    @c 0, always.
*/
int
ion_vfs_fgetpos(
    VFS_FILE        *stream,
    ion_fpos_t    *pos
);

/**
@brief        Open a reference to an vfs file given it's name.
@details    Wrapper around vfs file open method. This function
            will allocate memory, that must be freed by using @ref ion_vfs_fclose().
@param        filepath
                String containing the path to file (basic filename).
@param        mode
                Which mode to open the file under.
@returns    A pointer to a file struct representing a file for reading,
            or @c NULL if an error occurred.
*/
VFS_FILE *
ion_vfs_fopen(
    char    *filename,
    char    *mode
);

/**
@brief        Read data from an vfs file.
@details    A wrapper around vfs file read method.

            A total of @p size * @p nmemb bytes will be read into @p ptr
            on a success.
@param        ptr
                A pointer to the memory segment to be read into.
@param        size
                The number of bytes to be read (per @p nmemb items).
@param        nmemb
                The total number of items to read (each of size @p size).
@param        stream
                A pointer to C file struct type associated with an SD
                file object.
@returns    The number of items that have been read.
*/
size_t
ion_vfs_fread(
    void    *ptr,
    size_t    size,
    size_t    nmemb,
    VFS_FILE *stream
);

/**
@brief        Wrapper around vfs file lseek method.
@param        stream
                A pointer to a C file struct type associated with an SD
                file object.
@param        offset
                The number of bytes to move from @p whence.
@param        whence
                Where, in the file, to move from. Valid options are
                    - SEEK_SET:    From the beginning of the file.
                    - SEEK_CUR: From the current position in the file.
                    - SEEK_END: From the end of the file, moving backwards.
@returns    @c 0 for success, a non-zero integer otherwise.
*/
int
ion_vfs_fseek(
    VFS_FILE        *stream,
    long int    offset,
    int            whence
);

/**
@brief        Set the current position of an vfs file.
@details    The parameter @pos should be retrieved using fgetpos.
@param        stream
                A pointer to a C file struct type associated with an SD
                file object.
@param        pos
                A pointer to a file position describing where to set the file
                cursor to.
@returns    @c 0 on success, a non-zero integer otherwise.
*/
int
ion_vfs_fsetpos(
    VFS_FILE        *stream,
    ion_fpos_t    *pos
);

/**
@brief        Print an integer.
@param        i
                The integer to print.
*/
void
ion_vfs_printint(
    int i
);

/**
@brief        Reveals the file position of the given stream.
@param        stream
                A pointer to the C file struct associated with the vfs
                file object.
@returns    On success, the current position indicator of the file is returned.
            Otherwise, @c -1L is returned.
*/
long int
ion_vfs_ftell(
    VFS_FILE *stream
);

/**
@brief        Write data to an vfs file.
@details    Wrapper around vfs file write method.
@param        ptr
                A pointer to the data that is to be written.
@param        size
                The number of bytes to be written (for each of the @p nmemb
                items).
@param        nmemb
                The number of items to be written (each @p size bytes long).
@param        stream
                A pointer to a C file struct type associated with an VFS_FILE
                file object.
@returns    The number of items successfully written.
*/
size_t
ion_vfs_fwrite(
    void    *ptr,
    size_t    size,
    size_t    nmemb,
    VFS_FILE *stream
);

/**
@brief        Remove a file from the file system used by vfs.
@details    Wrapper around vfs file remove method.
@param        filename
                A pointer to the string data containing the path to the file
                that is to be deleted.
@returns    @c 0 if the file was removed successfully, @c 1 otherwise.
*/
int
ion_vfs_remove(
    char *filename
);

/**
@brief        Set the file position to the beginning of the vfs file
            for a given File stream.
@param        stream
                A pointer to a C file struct type associated with an SD
                file object.
*/
void
ion_vfs_rewind(
    VFS_FILE *stream
);

/**
@brief        Check to see if an File exists.
@param        filepath
                A pointer to the string data containing the path to the
                file (basic filename).
@returns    @c 1 if the file exists, @c 0 otherwise.
*/
int
ION_VFS_File_Exists(
    char *filepath
);

/**
@brief        Deletes all files on the vfs layer of a device.
@returns    @p 1 if all deletes were successful, @c 0 otherwise.
*/
int
ION_VFS_File_Delete_All( 
    void
);

#if defined(__cplusplus)
}
#endif

#endif /* Clause VFS */

#endif

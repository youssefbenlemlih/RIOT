
/******************************************************************************/ // TODO Comments
/**
@file        sd_stdio_c_iface.h
@author        Tobias Westphal
@brief        This code contains definitions for stdio.h file functions
            for vfs interface.
@details    Since the iondb lib does not have a vfs interface this is the wrapper 
@copyright    Copyright 2017
            The University of British Columbia,
            IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
    may be used to endorse or promote products derived from this software without
    specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/

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
@brief        Wrapper around Arduino File type (a C++ object).
*/
typedef struct _VFS_File VFS_FILE;


/**
@brief        Wrapper around Arduino File type (a C++ object).
*/
typedef struct _VFS_File VFS_FILE;

/**
@brief        Wrapper around Arduino SD file close method.
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
@details    Wrapper around Arduino SD file position method.
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
@brief        Open a reference to an Arduino SD file given it's name.
@details    Wrapper around Arduino SD file open method. This function
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
@brief        Read data from an Arduino SD file.
@details    A wrapper around Arduino SD file read method.

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
@brief        Wrapper around Arduino SD file read method.
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
@brief        Set the current position of an Arduino SD file.
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
                A pointer to the C file struct associated with the Arduino
                file object.
@returns    On success, the current position indicator of the file is returned.
            Otherwise, @c -1L is returned.
*/
long int
ion_vfs_ftell(
    VFS_FILE *stream
);

/**
@brief        Write data to an Arduino SD file.
@details    Wrapper around Arduino SD file write method.
@param        ptr
                A pointer to the data that is to be written.
@param        size
                The number of bytes to be written (for each of the @p nmemb
                items).
@param        nmemb
                The number of items to be written (each @p size bytes long).
@param        stream
                A pointer to a C file struct type associated with an SD
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
@brief        Remove a file from the Arduino SD file system.
@details    Wrapper around Arduino SD file remove method.
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
@brief        Set the file position to the beginning of the file
            for a given Arduino SD File stream.
@param        stream
                A pointer to a C file struct type associated with an SD
                file object.
*/
void
ion_vfs_rewind(
    VFS_FILE *stream
);

/**
@brief        Check to see if an Arduino SD File exists.
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
@brief        Deletes all files on the Arduino device.
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

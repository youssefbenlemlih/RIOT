/******************************************************************************/
/**
@file		flat_file_types.h
@author		Eric Huang
@brief		Implementation specific type definitions for the flat file store.
@copyright	Copyright 2017
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

#if !defined(FLAT_FILE_TIMESERIES_TYPES_H)
#define FLAT_FILE_TIMESERIES_TYPES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "dictionary.h"
#include "vfs_stdio_c_iface.h"

/**
@brief		This type describes the status flag within a flat file row.
*/
typedef ion_byte_t ion_flat_file_timeseries_row_status_t;

/**
@brief		Signifies that this row in the flat file is currently occupied and should not be overwritten.
*/
#define ION_FLAT_FILE_TIMESERIES_STATUS_OCCUPIED	1
/**
@brief		Signifies that this row in the flat file is currently empty and is okay to be overwritten.
*/
#define ION_FLAT_FILE_TIMESERIES_STATUS_EMPTY		0

/**
@brief		Signals to @ref flat_file_scan to scan in a forward direction.
*/
#define ION_FLAT_FILE_TIMESERIES_SCAN_FORWARDS		1
/**
@brief		Signals to @ref flat_file_scan to scan in a backward direction.
*/
#define ION_FLAT_FILE_TIMESERIES_SCAN_BACKWARDS	0

#define ION_FLAT_FILE_TIMESERIES_OPTIMIZED 0

/**
@brief		Metadata container that holds flat file specific information.
*/
typedef struct {
	/**> Parent structure that holds dictionary level information. */
	ion_dictionary_parent_t super;
	/**> Flag to toggle whether or not to activate "sorted mode" for storage. */
	ion_boolean_t			sorted_mode;
	/**> This signifies where the actual record data starts, in case we want to
		 write some metadata at the beginning of the flat file's file. */
	ion_fpos_t				start_of_data;
	/**> This marks the eof position within the file, so that we can efficiently find it. */
	ion_fpos_t				eof_position;
	/**> This comes from the given dictionary size, and signifies how many
		 records we want to buffer at a time. This is a trade-off between
		 better performance and increased memory usage. */
	ion_dictionary_size_t	num_buffered;
	/**> Memory buffer capable of holding @p row_size number of rows. This is used
		 for many purposes throughout the flat file. */
	ion_byte_t				*buffer;
	/**> The file descriptor of the file this flat file instance operates on. */
	FILE					*data_file;
	/**> This value expresses the size of one row inside the @p data_file. A row is defined
		 as a record + metadata. Change this if @ref ion_flat_file_row_t changes!*/
	size_t					row_size;
	/**> When a scan is performed, a region (defined as @p num_in_buffer number of records) is loaded into
		 memory. We can utilize this fact to do efficient cached reads as long as the buffer is intact.
		 This is expressed as an index that points to the first record in the region. @p num_in_buffer-1 would
		 be the last index in the region. */
	ion_fpos_t	current_loaded_region;
	/**> Expresses how many valid records are currently in the buffer. */
	size_t		num_in_buffer;
#if ION_FLAT_FILE_TIMESERIES_OPTIMIZED
	ion_key_t		last_inserted;
	ion_byte_t		empty;
#endif
} ion_flat_file_timeseries_t;

/**
@brief		Container for the rows written in the flat file data file.
*/
typedef struct {
	/**> A flag indicating the status of the row. */
	ion_flat_file_timeseries_row_status_t	row_status;
	/**> The key stored in this row. */
	ion_key_t					key;
	/**> The value stored in this row. */
	ion_value_t					value;
} ion_flat_file_timeseries_row_t;

/**
@brief		The function signature of a flat file predicate, used in searches.
*/
typedef ion_boolean_t (*ion_flat_file_timeseries_predicate_t)(
	ion_flat_file_timeseries_t *,
	ion_flat_file_timeseries_row_t *,
	va_list *args
);

/**
@brief		Implementation cursor type for the flat file store cursor.
*/
typedef struct {
	/**> Supertype of the dictionary cursor. */
	ion_dict_cursor_t	super;
	/**> Holds the index of the current location in our search. */
	ion_fpos_t			current_location;
} ion_flat_file_timeseries_cursor_t;

#if defined(__cplusplus)
}
#endif

#endif

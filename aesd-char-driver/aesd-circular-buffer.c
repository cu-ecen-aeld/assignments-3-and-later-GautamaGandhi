/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

#include "aesd-circular-buffer.h"

// Function to increment pointer
static uint8_t nextPtr(uint8_t ptr) 
{
    if (ptr + 1 == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) // Wrap around if AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED number of locations reached
        return 0;
    else
        return ptr + 1;
} 

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{
    // Temporary offset variable to hold char_offset
    int temp_offset = char_offset;

    // Variable to hold total size in bytes of all buffers present
    int total_byte_size = 0;
    for (int i = 0; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++) {
        total_byte_size += buffer->entry[i].size;
    }

    // Check if char_offset is more than total byte size or total bytes is less than or equal to zero
    if ((char_offset > total_byte_size) || (total_byte_size <= 0)) {
        return NULL;
    }

    // Read_ptr variable to store buffer's current read pointer
    uint8_t read_ptr = buffer->out_offs;

    for (int i = 0; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++) {
        // Decrement temp_offset by the size of current buffer entry at read_ptr
        temp_offset -= buffer->entry[read_ptr].size;
        // Check if temp_offset is less than 0 to determine if corresponding byte and buffer entry are found
        if (temp_offset < 0) {
            // Return temp_offset to original value before subtraction and store
            temp_offset += buffer->entry[read_ptr].size;
            *entry_offset_byte_rtn = temp_offset;
            // Return corresponding buffer entry
            return &buffer->entry[read_ptr];
        }
        // Increment read_ptr
        read_ptr = nextPtr(read_ptr);
    }
    /**
    * TODO: implement per description
    */
    return NULL;
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    /**
    * TODO: implement per description
    */

    // Increment buffer's read pointer if the buffer->full flag is set
    if (buffer->full) {
        buffer->out_offs = nextPtr(buffer->out_offs);
    }

    // Adding buffer pointer and buffer size
    buffer->entry[buffer->in_offs].buffptr = add_entry->buffptr;
    buffer->entry[buffer->in_offs].size = add_entry->size;

    buffer->in_offs = nextPtr(buffer->in_offs);

    // buffer->full flag is set if read pointer is equal to write pointer
    if (buffer->in_offs == buffer->out_offs) {
        buffer->full = true;
    }
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}

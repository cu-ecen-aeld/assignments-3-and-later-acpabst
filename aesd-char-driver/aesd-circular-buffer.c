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
    size_t index;
    struct aesd_buffer_entry *entry;
    size_t total_chars = 0;
    size_t offset_remaining = char_offset;
    AESD_CIRCULAR_BUFFER_FOREACH(entry,buffer,index) {
         total_chars+= entry->size;
    }
    index = buffer->out_offs;
    entry = &(buffer->entry[index]);
    
    if(total_chars <= char_offset) {
        // there is not enough data in the buffer
        return NULL;
    }
    //  prewrap if necessary
    while(char_offset > total_chars) {
	char_offset -= total_chars;
    }
    // find the entry where the requested char is
    while(offset_remaining >= entry->size) {
        offset_remaining -= entry->size;
	if(index + 1 == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) {
	    index = 0;
	} else {
            index++;
	}
	entry = &(buffer->entry[index]);
    }
    // set the byte number of the requested char
    if(offset_remaining == entry->size) {
	*entry_offset_byte_rtn = 0;
    } else {
	*entry_offset_byte_rtn = offset_remaining;
    }
    // return the entry where the char is located
    return entry;
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
    if(buffer->full) {
	// buffer is full - add to buffer at in_offs, advance both out_offs and in_offs
	buffer->entry[buffer->in_offs] = *add_entry;
	if(buffer->in_offs + 1 == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) {
	    // reached the end of the buffer again, start at the beginning
	    buffer->in_offs = 0;
	} else {
            buffer->in_offs++;
        }
        if(buffer->out_offs + 1 == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) {
            // reached the end of the buffer again, start at the beginning
            buffer->out_offs = 0;
        } else {
	    buffer->out_offs++;
	}
    }
    else {
	// buffer is not full - add to buffer, increment in_offs
	buffer->entry[buffer->in_offs] = *add_entry;
        if(buffer->in_offs + 1 == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) {
            // the buffer is now full
	    buffer->in_offs = 0;
	    buffer->full = true;
	} else {
	    buffer->in_offs++;
	}
    }
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}

/*
 * aesdchar.h
 *
 *  Created on: Oct 23, 2019
 *      Author: Dan Walkes
 */

#ifndef AESD_CHAR_DRIVER_AESDCHAR_H_
#define AESD_CHAR_DRIVER_AESDCHAR_H_

#define AESD_DEBUG 1  //Remove comment on this line to enable debug

#undef PDEBUG             /* undef it, just in case */
#ifdef AESD_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "aesdchar: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#define AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED 10

struct aesd_buffer_entry
{
    /**
     * A location where the buffer contents in buffptr are stored
     */
    const char *buffptr;
    /**
     * Number of bytes stored in buffptr
     */
    size_t size;
};

struct aesd_dev
{
    /**
     * TODO: Add structure(s) and locks needed to complete assignment requirements
     */
    //mutex* aesd_mutex

    /* The buffer */
    struct aesd_buffer_entry  entry[AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED];
    
    /* Pointer to the next write location */
    uint8_t in_offs;

    /* Pointer to the beginning of the buffer. Will move upon overwrite */
    uint8_t out_offs; 	
    
    /* Indicates whether the buffer is full */
    bool full;
 
    /* TODO Partial buffer */    
    
    /* Char device structure */
    struct cdev cdev;     
};




#endif /* AESD_
	  CHAR_DRIVER_AESDCHAR_H_ */

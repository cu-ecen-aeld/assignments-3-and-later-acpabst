/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include <linux/slab.h>

#include <linux/mutex.h>

#include "aesdchar.h"
#include "aesd-circular-buffer.h"

int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Andy Pabst"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;
struct mutex aesd_mutex;
bool TMP_FLAG = false;

int aesd_open(struct inode *inode, struct file *filp)
{
    /**
     * TODO: handle open
     */
    struct aesd_dev *dev;
	
    PDEBUG("Open");
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = dev;
    
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("Release");
    /**
     * TODO: handle release
     */
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    if(TMP_FLAG) {
        *f_pos = 60;
	TMP_FLAG = false;
    }
    ssize_t retval = 0;
    size_t read_count = 0;
    uint i;
    uint k;
    size_t start_entry_index;
    struct aesd_dev *aesd_device = filp->private_data; 
    char *tmp_buf = kmalloc(count, GFP_KERNEL);
    
    PDEBUG("----READING----");
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
   
    // lock data
    mutex_lock(aesd_device->aesd_mutex);

    if (*f_pos >= aesd_device->buffer->total_size) {
        // don't have enough data for this position
	// this will happen if the function returns null, so maybe just put this in there?
	// what to do? return an error?
	return 0;
    }

    size_t *byte_offset = kmalloc(sizeof(size_t),GFP_KERNEL);
    struct aesd_buffer_entry *point_entry;
    point_entry = aesd_circular_buffer_find_entry_offset_for_fpos(aesd_device->buffer, *f_pos, byte_offset); 
    // TODO if the entry is NULL
    if (point_entry == NULL) {
        PDEBUG("houston we have a problem");
	start_entry_index = 0;
	*byte_offset = 0;
    } else {
        // find the index of the returned entry
        for (i = 0; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++) { 
	    if (point_entry == &(aesd_device->buffer->entry[i])) {
                start_entry_index = i;
	        break;
	    }
        }
    }


    if (!aesd_device->buffer->full) {   
        read_count = aesd_circular_buffer_read_helper(aesd_device->buffer, 
			start_entry_index, AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED, 
			tmp_buf, count, read_count, *byte_offset);
    } else {
        if (aesd_device->buffer->out_offs <= start_entry_index) {
	    // seeked position is on the back of the buffer
	    read_count = aesd_circular_buffer_read_helper(aesd_device->buffer, 
                        start_entry_index, AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED, 
                        tmp_buf, count, read_count, *byte_offset);
	    read_count = aesd_circular_buffer_read_helper(aesd_device->buffer,
                        0, aesd_device->buffer->out_offs,
                        tmp_buf, count, read_count, *byte_offset);
	} else {
	    // seeked position is on the front of the buffer
	    read_count = aesd_circular_buffer_read_helper(aesd_device->buffer, 
                        start_entry_index, aesd_device->buffer->out_offs, 
                        tmp_buf, count, read_count, *byte_offset);
	}
    }

    // unlock data
    mutex_unlock(aesd_device->aesd_mutex);

    retval = copy_to_user(buf, tmp_buf, count);
    if (retval) {
	    // copy data to user failed
	    PDEBUG("copy_to_user failed. Number of failed bytes: %li", retval);
	    // TODO
    }
    
    // TODO release mutex
    retval = read_count;
    
    // free memory used locally
    kfree(byte_offset);
    kfree(tmp_buf);
    
    // update file position and return
    PDEBUG("f_pos: %lld", *f_pos);
    if ((*f_pos) < aesd_device->buffer->total_size) {
	(*f_pos) += read_count;
	PDEBUG("f_pos now: %lld", *f_pos);
        return retval;
    }
    PDEBUG("Reading complete.");
    return 0;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    uint i;
    ssize_t retval = -ENOMEM;
    ssize_t write_size = count;
    struct aesd_dev *aesd_device = filp->private_data;
    // create entry from buffer and count provided
    struct aesd_buffer_entry *add_entry = kmalloc(sizeof(*add_entry),GFP_KERNEL); 
    // kmalloc a buffer to store the data we are given
    char *data = kmalloc(count, GFP_KERNEL);
    char *tmp = kmalloc(count + aesd_device->partial_entry->size, GFP_KERNEL);
    
    PDEBUG("----WRITING----");
    retval = copy_from_user(data, buf, count);
    if (retval) {
	    // copy data to user failed
	    PDEBUG("copy_to_user failed. Number of failed bytes: %li", retval);
	    // TODO
    }
   
    // lock data
    mutex_lock(aesd_device->aesd_mutex);

    if (aesd_device->partial) {
        for (i = 0; i < aesd_device->partial_entry->size; i++) {
	    tmp[i] = aesd_device->partial_entry->buffptr[i];
	}
	for (i = 0; i < count; i++) {
       	    tmp[aesd_device->partial_entry->size + i] = data[i];
	}
	kfree(data);
	*data = kmalloc(count+aesd_device->partial_entry->size, GFP_KERNEL);
	write_size += aesd_device->partial_entry->size;
	for (i = 0; i < write_size; i++) {
	    data[i] = tmp[i];
	}
    } else {
    }

    // check for new line character at the end of the buffer
    if (data[write_size-1] != '\n') {
        PDEBUG("Partial Write!");
        aesd_device->partial = true;
        aesd_device->partial_entry->size = write_size;
        aesd_device->partial_entry->buffptr = data;
	// entry created is not needed
	kfree(add_entry);
    } else {
        PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
  
        aesd_device->partial = false;	
        add_entry->size = write_size;
        add_entry->buffptr = data;
        PDEBUG("data written: %s",add_entry->buffptr);

        PDEBUG("in_offs: %i", aesd_device->buffer->in_offs);

        // add created entry to the buffer
        aesd_circular_buffer_add_entry(aesd_device->buffer, add_entry);
        PDEBUG("in_offs now: %i", aesd_device->buffer->in_offs);
    }

    // unlock data
    mutex_lock(aesd_device->aesd_mutex);

    kfree(tmp);

    retval = count;
    return retval;
}
struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    uint i;
    struct aesd_circular_buffer *aesd_buffer = kmalloc(sizeof(struct aesd_circular_buffer),GFP_KERNEL);
    struct aesd_buffer_entry *aesd_partial_entry = kmalloc(sizeof(struct aesd_buffer_entry),GFP_KERNEL);

    PDEBUG("Initialize device");

    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    /**
     * TODO: initialize the AESD specific portion of the device
     */
    // init locking primitive
    
    mutex_init(&aesd_mutex);
    aesd_device.aesd_mutex = &aesd_mutex;
    
    aesd_circular_buffer_init(aesd_buffer);
    aesd_device.buffer = aesd_buffer;
    aesd_device.partial_entry = aesd_partial_entry;
    aesd_device.partial_entry->size = 0;
    aesd_device.partial = false;
    aesd_device.buffer->full = false;
    aesd_device.buffer->in_offs = 0;
    aesd_device.buffer->out_offs = 0;
    aesd_device.buffer->total_size = 0;


    for(i = 0; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++) {
	struct aesd_buffer_entry *add_entry = kmalloc(sizeof(*add_entry),GFP_KERNEL);
        aesd_device.buffer->entry[i] = *add_entry;
    }
    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno;
    size_t index;
    //struct aesd_circular_buffer buffer = aesd_device.buffer;
    struct aesd_buffer_entry *entry;
    
    PDEBUG("De-initialize device");
    
    devno = MKDEV(aesd_major, aesd_minor);
    cdev_del(&aesd_device.cdev);

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */
    AESD_CIRCULAR_BUFFER_FOREACH(entry, aesd_device.buffer, index) {
	PDEBUG("data: %s",entry->buffptr);
	kfree(entry->buffptr);
    }
    kfree(aesd_device.buffer);
    kfree(aesd_device.partial_entry);

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);

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
struct mutex mutex;
//struct aesd_circular_buffer *buffer;

int aesd_open(struct inode *inode, struct file *filp)
{
    /**
     * TODO: handle open
     */
    struct aesd_dev *dev;
	
    PDEBUG("Open\n");
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = dev;
    
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    size_t read_count = 0;
    uint i;
    struct aesd_dev *aesd_device = filp->private_data; 
    char *tmp_buf = kmalloc(count, GFP_KERNEL);

    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
   
    // TODO obtain mutex
    /*
    if(aesd_device->buffer->full) {
	for(i =  aesd_device->buffer->out_offs; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++) {
	    if(read_count + aesd_device->buffer->entry[i].size < count) {
	        // if we have room for the entire entry
		// TODO change to internal buffer (copy to user)
		tmp_buf[read_count] = aesd_device->buffer->entry[i].buffptr;
		read_count += aesd_device->buffer->entry[i].size;
	    } else {
		// we will run over the requested number of bytes, partial read
		// TODO
		// don't forget to break the loop
	    }
	}
    }
    */
    for(i = 0; i <= aesd_device->buffer->out_offs; i++) {
	PDEBUG("Going to read %zu from entry %i", aesd_device->buffer->entry[i].size, i);
        if(read_count + aesd_device->buffer->entry[i].size < count) {
            // if we have room for the entire entry
	    uint k;
	    for(k = read_count; k < aesd_device->buffer->entry[i].size; k++) {
		    tmp_buf[k] = aesd_device->buffer->entry[i].buffptr[k];
            }
	    //tmp_buf[aesd_device->buffer->entry[i].size] = 0;
            read_count += aesd_device->buffer->entry[i].size;
	    PDEBUG("These should be the same: %i, %i", read_count, k);
         } else {
            // we will run over the requested number of bytes, partial read
            // TODO
            // don't forget to break the loop
         } 	
    }
    retval = copy_to_user(buf, tmp_buf, count);
    // TODO release mutex
    // retval = read_count;
    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
    struct aesd_dev *aesd_device = filp->private_data;
    // create entry from buffer and count provided
    struct aesd_buffer_entry *add_entry = kmalloc(sizeof(*add_entry),GFP_KERNEL); 
    // kmalloc a buffer to store the data we are given
    char *data = kmalloc(count, GFP_KERNEL);
    copy_from_user(data, buf, count);

    uint i;
   
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
   
    add_entry->size = count;
    add_entry->buffptr = data;
    PDEBUG("data written: %s",add_entry->buffptr);
    // TODO obtain mutex here
    // add created entry to the buffer
    aesd_circular_buffer_add_entry(aesd_device->buffer, add_entry);
    // TODO release mutex

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
    struct aesd_circular_buffer *aesd_buffer = kmalloc(sizeof(struct aesd_circular_buffer),GFP_KERNEL);
    uint i;

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
    
    //mutex_init(&aesd_mutex);
    //aesd_device.aesd_mutex = *mutex;
    
    aesd_circular_buffer_init(aesd_buffer);
    aesd_device.buffer = aesd_buffer;
    aesd_device.buffer->full = false;
    aesd_device.buffer->in_offs = 0;
    aesd_device.buffer->out_offs = 0;

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

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);

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
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
   
    // TODO obtain mutex
    
    if(!aesd_device->buffer->full) {
	for(i =  aesd_device->buffer->out_offs; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++) {
	    if(read_count + aesd_device->buffer->entry[i].size < count) {
	        // if we have room for the entire entry
		// TODO change to internal buffer (copy to user)
		buf[read_count] = aesd_device->buffer->entry[i].buffptr;
		read_count += aesd_device->buffer->entry[i].size;
	    } else {
		// we will run over the requested number of bytes, partial read
		// TODO
		// don't forget to break the loop
	    }
	}
    }
    for(i = 0; i < aesd_device->buffer->out_offs; i++) {
        if(read_count + aesd_device->buffer->entry[i].size < count) {
            // if we have room for the entire entry
	    // TODO change to internal buffer (copy to user)
            buf[read_count] = aesd_device->buffer->entry[i].buffptr;
            read_count += aesd_device->buffer->entry[i].size;
         } else {
            // we will run over the requested number of bytes, partial read
            // TODO
            // don't forget to break the loop
         } 	
    }
    // TODO copy_to_user
    // TODO release mutex
    retval = read_count;
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
    uint i;
    add_entry->size = count;
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
    for(i = 0; i < count; i++) {
       // TODO if null character, set flag
       // 	if the flag is set, write to buffer, else write to partial buffer
       data[i] = buf[i];
    } 
    add_entry->buffptr = data;
    // TODO obtain mutex here
    // add created entry to the buffer
    aesd_circular_buffer_add_entry(aesd_device->buffer, add_entry);
    // TODO release mutex
  
  
    // free memory? I think this is the wrong place to do this
    kfree(add_entry);
    kfree(data);

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

    struct aesd_circular_buffer *buffer;

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
    // allocate memory if needed - do I need to allocate filp->private_data. See scull?
    //mutex_init(&aesd_mutex);
    //aesd_device.aesd_mutex = *mutex;
    PDEBUG("Initialize device");
    //aesd_circular_buffer_init(buffer);
    //aesd_device.buffer = buffer;
    //aesd_device.buffer->full = false;
    //aesd_device.buffer->in_offs = 0;
    //aesd_device.buffer->out_offs = 0;
     
    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    size_t index;
    struct aesd_buffer_entry *entry;
    
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */
    //AESD_CIRCULAR_BUFFER_FOREACH(entry, aesd_device.buffer, index) {
    //    kfree(entry->buffptr);
    //}
    //kfree(entry);
    //kfree(aesd_device);

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
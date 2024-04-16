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

#include <linux/mutex.h>

#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Andy Pabst"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;
struct mutex mutex;

int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");
    /**
     * TODO: handle open
     */
    struct aesd_dev *dev
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = dev;

    //
    
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
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle read
     */
    // TODO obtain mutex
    for(uint8 i = filp->private_data->out_offs; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED, i++) {
       buf[retval] = filp->private_data->entry[i]->buffptr;
       retval+= filp->private_data->entry[i]->size;
    }
    if (filp->private_data->full) {
        if (filp->private_data->out_offs > 0) {
	    // circle back
	    for(unsigned int i = 0, i < filp->private_data->out_offs, i++) {
	        buf[retval] = filp->private_data->entry[i]->buffptr;
		retval += filp->private_data->entry[i]->size;
	    }
        }
    }
    // TODO release mutex
    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle write
     */
    // assume each write will end with a new line character
    // create entry from buffer and count provided
    struct aesd_buffer_entry *add_entry;
    aesd_buffer_entry->size = count;
    // kmalloc a buffer to store the data we are given
    char *data = kmalloc(count, GFP_KERNEL);
    for(unsigned int i = 0; i < count; i++) {
       // TODO if null character, set flag
       // 	if the flag is set, write to buffer, else write to partial buffer
       data[i] = buf[i];
    } 
    add_entry->buffptr = data;
    // TODO obtain mutex here
    // add created entry to the buffer
    filp->private_data->entry[in_offs] = add_entry;
    if(flip->private_date->full) {
        // buffer is full, we are overwriting an entry
	// advance both in_off and out_offs
        if(filp->private_data->in_offs + 1 == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) {
            // reached the end of the buffer again, start at the beginning
            filp->private_data->in_offs = 0;
        } else {
            filp->private_data->in_offs++;
        }
        if(filp->private_data->out_offs + 1 == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) {
	    filp->private_data->out_offs = 0;
	} else {
	    filp->private_data->out_offs++;
	}
    } else {
	// buffer is not full, increment in_offs only
	if (filp->private_data->in_offs + 1 == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) {
            // buffer is now full
	    filp->private_data->in_offs = 0;
	    filp->private_data->full = true;
	} else {
            filp_private_data->in_offs++;
	}
    }
    // release mutex

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
    
    aesd_device->full = false;
    aesd_device->in_offs = 0;
    aesd_device->read_ptr = 0;
     
    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */
    // free memory if needed
    // TODO for each entry, free the data
    free(aesd_device);

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);

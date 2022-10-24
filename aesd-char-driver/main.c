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
#include "aesdchar.h"
#include "linux/slab.h"
#include "linux/string.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Gautama Gandhi"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    struct aesd_dev *dev; /* device information */
    
    PDEBUG("open");
    
	dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
	filp->private_data = dev; /* for other methods */
    /**
     * TODO: handle open
     */
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

/**
 * filp -> File pointer to get aesd_dev
 * buf -> Buffer to fill
 * count -> User space buffer size
 * f_pos -> File position
 * 
 * Return number of bytes transferred to the buf
*/
ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    size_t offset_byte_pos;
    int temp_buffer_count = 0;
    struct aesd_dev *dev = filp->private_data;
    struct aesd_buffer_entry *temp_buffer;

    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);

    mutex_lock(&aesd_device.lock);

    temp_buffer = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->aesd_circular_buffer, *f_pos, &offset_byte_pos);

    if (temp_buffer == NULL) {
        *f_pos = 0;
        goto out;
    }
    
    if ((temp_buffer->size - offset_byte_pos) < count) {
        *f_pos += (temp_buffer->size - offset_byte_pos);
        temp_buffer_count = temp_buffer->size - offset_byte_pos;
    } else {
        *f_pos += count;
        temp_buffer_count = count;
    }

    if (copy_to_user(buf, temp_buffer->buffptr+offset_byte_pos, temp_buffer_count)) {
		retval = -EFAULT;
		goto out;
	}

    retval = temp_buffer_count;
    /**
     * TODO: handle read
     */
    out: mutex_unlock(&aesd_device.lock);

    PDEBUG("Return Value %ld", retval);

    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0; // Return Value
    
    char *temp_buffer; // Local buffer to store data recieved from write command and later copy it to global copy buffer
    int i; // Loop iterator
    int packet_complete_flag = 0; // Flag indicating complete packet
    int temp_iterator = 0; // Temp_iterator to hold value after packet is complete
    struct aesd_buffer_entry aesd_buffer_write_entry; // Aesd_buffer_entry struct value
    struct aesd_dev *dev;

    int temp_size_increment = 0; // Variable to hold temporary size increment value based on complete/incomplete packets

    char *ret_ptr;

    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);

    dev = filp->private_data;

    mutex_lock(&aesd_device.lock);

    // Mallocing into local buffer
    temp_buffer = (char *)kmalloc(count, GFP_KERNEL);
    if (temp_buffer == NULL) {
        retval = -ENOMEM;
        goto out;
    }
        
    // Copying from userspace to kernel space
    if (copy_from_user(temp_buffer, buf, count)) {
        retval = -EFAULT;
		goto out;
	}

    // Iterating over bytes received to check for "\n" character
    for (i = 0; i < count; i++) {
        if (temp_buffer[i] == '\n') {
            packet_complete_flag = 1; // Setting packet complete flag
            temp_iterator = i+1; // Setting temp_iterator value to store
            break;
        }
    }

    // Check if copy buffer size is 0; mallocing and copying local buffer into global buffer
    if (dev->buffer_size == 0) {
        dev->copy_buffer_ptr = (char *)kmalloc(count, GFP_KERNEL);
        if (dev->copy_buffer_ptr == NULL) {
            retval = -ENOMEM;
            goto free;
        }
        memcpy(dev->copy_buffer_ptr, temp_buffer, count);
        dev->buffer_size += count;
    } 
    else {
        // Case when write command is issued without '\n'
        if (packet_complete_flag)
            temp_size_increment = temp_iterator;
        else
            temp_size_increment = count;

        // Realloc copy buffer size based on temporary size increment variable
        dev->copy_buffer_ptr = (char *)krealloc(dev->copy_buffer_ptr, dev->buffer_size + temp_size_increment, GFP_KERNEL);
        if (dev->copy_buffer_ptr == NULL) {
            retval = -ENOMEM;
            goto free;
        }

        // Copying temp_buffer contents into copy_buffer
        memcpy(dev->copy_buffer_ptr + dev->buffer_size, temp_buffer, temp_size_increment);
        dev->buffer_size += temp_size_increment;        
    }
    
    // Adding entry onto circular buffer if packet is complete
    if (packet_complete_flag) {

        // Adding entry onto circular buffer
        aesd_buffer_write_entry.buffptr = dev->copy_buffer_ptr;
        aesd_buffer_write_entry.size = dev->buffer_size;
        ret_ptr = aesd_circular_buffer_add_entry(&dev->aesd_circular_buffer, &aesd_buffer_write_entry);
    
        // Freeing return_pointer if buffer is full 
        if (ret_ptr != NULL)
            kfree(ret_ptr);
        
        dev->buffer_size = 0;
    } 

    retval = count;

    /**
     * TODO: handle write
     */
    free: kfree(temp_buffer);
    out: mutex_unlock(&aesd_device.lock);
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
    mutex_init(&aesd_device.lock);

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    uint8_t index;
    struct aesd_buffer_entry *entry;
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */

    AESD_CIRCULAR_BUFFER_FOREACH(entry, &aesd_device.aesd_circular_buffer, index) {
      kfree(entry->buffptr);
    }
    mutex_destroy(&aesd_device.lock);

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);

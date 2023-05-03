/* EC535 Lab 5 - Kernel Module to Read/Write GPIO - almailam & samgul */

#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> // printk
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system_misc.h> /* cli(), *_flags */
#include <linux/uaccess.h>
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/sched.h> // timer
#include <linux/jiffies.h> // HZ
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/time.h>


MODULE_LICENSE("Dual BSD/GPL");


#define LED 67
#define PUMP 68

#define MY_DEV_NAME "gpio_control"

/* Declaration of memory.c functions */
static ssize_t mygpio_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static ssize_t mygpio_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
static void mygpio_exit(void);
static int mygpio_init(void);


/* Structure that declares the usual file */
/* access functions */
struct file_operations mygpio_fops = {
   write: mygpio_write,
   read: mygpio_read
};

/* Declaration of the init and exit functions */
module_init(mygpio_init);
module_exit(mygpio_exit);

static unsigned capacity = 128;
static unsigned bite = 128;
module_param(capacity, uint, S_IRUGO);
module_param(bite, uint, S_IRUGO);

/* Global variables of the driver */
/* Major number */
static int mygpio_major = 61;
/* Buffer to store data */
static char *mygpio_buffer;
/* length of the current message */
static int mygpio_len;

const unsigned user_input_capacity = 256;
// Buffer to store data from user
static char *user_input;

// Kernel module initialization function
static int mygpio_init(void) {
    int result;
    int error;
    
    /* Registering device */
    result = register_chrdev(mygpio_major, "mygpio", &mygpio_fops);
    if (result < 0) {
        printk(KERN_ALERT "mygpio: cannot obtain major number %d\n", mygpio_major);
        return result;
    }
 
    /* Allocating mygpio for the buffer */
    mygpio_buffer = kmalloc(capacity, GFP_KERNEL);
    if (!mygpio_buffer) {
        printk(KERN_ALERT "Insufficient kernel memory\n");
        mygpio_exit();
        return -ENOMEM;
    }
    memset(mygpio_buffer, 0, capacity);
    mygpio_len = 0;

    user_input = kmalloc(user_input_capacity, GFP_KERNEL); 
	if (!user_input)
	{ 
		printk(KERN_ALERT "Insufficient kernel memory\n"); 
		result = -ENOMEM;
        return result;
	} 
	memset(user_input, 0, user_input_capacity);
    
    /* Setup LEDs */
    // Initialize LED
    error = gpio_is_valid(LED);
    if (error < 0) { printk(KERN_ALERT "Error: LED GPIO not valid\n"); return -1; }
    error = gpio_request(LED, "LED");
    if (error < 0) { printk(KERN_ALERT "Error: LED GPIO request fail\n"); return -1; }
    gpio_direction_output(LED, 0);
    
    // Initialize Pump
    error = gpio_is_valid(PUMP);
    if (error < 0) { printk(KERN_ALERT "Error: PUMP GPIO not valid\n"); return -1; }
    error = gpio_request(PUMP, "PUMP");
    if (error < 0) { printk(KERN_ALERT "Error: PUMP GPIO request fail\n"); return -1; }
    gpio_direction_output(PUMP, 0);

    gpio_set_value(LED, 0);
    gpio_set_value(PUMP, 0);

    printk(KERN_ALERT "All Outputs initialized \n");
    
    printk(KERN_ALERT "Done inserting mygpio\n");

    return 0;
}

// Kernel module exit function
static void mygpio_exit(void) {
    /* Freeing the major number */
    unregister_chrdev(mygpio_major, "mygpio");

    /* Freeing buffer memory */
    if (mygpio_buffer) kfree(mygpio_buffer);

    printk(KERN_ALERT "Removing mygpio module\n");
  
    gpio_set_value(LED, 0);
    gpio_set_value(PUMP, 0);

    // Turn off LEDs and free the GPIO pins
    gpio_free(LED);
    gpio_free(PUMP);
}

static ssize_t mygpio_read(struct file *filp, char *buf,
                           size_t count, loff_t *f_pos)
{
   printk(KERN_INFO "Read Function Called");
   return 0;
}

// Kernel module buffer write function
static ssize_t mygpio_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{ 
	int temp;
    char action[3];
    char device[4];
	char tbuf[256], *tbptr = tbuf;

	/* end of buffer reached */
	if (*f_pos >= user_input_capacity)
	{
		return -ENOSPC;
	}

	/* do not eat more than a bite */
	if (count > bite) count = bite;

	/* do not go over the end */
	if (count > user_input_capacity - *f_pos)
		count = user_input_capacity - *f_pos;

	// Assume that buffer contains a valid message from the user
	// Copy data from user
	memset(user_input, 0, user_input_capacity);
	if (copy_from_user(user_input + *f_pos, buf, count))
	{
		return -EFAULT;
	}

	tbptr += sprintf(tbptr,								   
		"write called: process id %d, command %s, count %d,  offset %lld, chars ",
		current->pid, current->comm, count, *f_pos);

	for (temp = *f_pos; temp < count + *f_pos; temp++)					  
		tbptr += sprintf(tbptr, "%c", user_input[temp]);

    sscanf(user_input, "%s %s", action, device);

    printk(KERN_INFO "action -> %s\ndevice -> %s\n", action, device);

	// Register a timer or updating a timer
	if(strncmp(action, "-t", 2) == 0) {
        if(strncmp(device, "LED", 3) == 0) {
            gpio_set_value(LED, !gpio_get_value(LED));
        } else if(strncmp(device, "PUM", 3) == 0) {
            gpio_set_value(PUMP, !gpio_get_value(PUMP));
        }
    } else if(strncmp(action, "-y", 2) == 0) {
        if(strncmp(device, "LED", 3) == 0) {
            gpio_set_value(LED, 1);
        } else if(strncmp(device, "PUM", 3) == 0) {
            gpio_set_value(PUMP, 1);
        }
	} else if(strncmp(action, "-n", 2) == 0) {
        if(strncmp(device, "LED", 3) == 0) {
            gpio_set_value(LED, 0);
        } else if(strncmp(device, "PUM", 3) == 0) {
            gpio_set_value(PUMP, 0);
        }
    }

	return count;
}
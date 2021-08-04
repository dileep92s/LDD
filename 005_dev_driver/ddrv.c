#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>

#define LOCAL_BUFFER_SIZE 512
char local_buffer[LOCAL_BUFFER_SIZE];

dev_t device_number;
unsigned baseminor = 0;
unsigned count = 1;
const char *name = "dummy_driver";

struct cdev cdev_struct;
struct class *class_struct;
struct device *device_struct;

int ddrv_open(struct inode *, struct file *);
int ddrv_release(struct inode *, struct file *);
ssize_t ddrv_read(struct file *, char __user *, size_t, loff_t *);
ssize_t ddrv_write(struct file *, const char __user *, size_t, loff_t *);

struct file_operations fops_struct = {.owner = THIS_MODULE,
                                      .open = ddrv_open,
                                      .release = ddrv_release,
                                      .read = ddrv_read,
                                      .write = ddrv_write};

static int __init ddrv_init(void)
{
    int retval;

    /*  Allocates a range of char device numbers. The major number will be chosen dynamically, and returned (along with the first minor number) in dev. Returns zero or a negative error code.*/
    retval = alloc_chrdev_region(&device_number, baseminor, count, name);
    if (retval < 0)
    {
        pr_err("%s - alloc_chrdev_region\n", __func__);
        return retval;
    }

    pr_info("%s - MAJOR - %d MINOR - %d\n", __func__, MAJOR(device_number), MINOR(device_number));

    /*  https://code.woboq.org/linux/linux/fs/char_dev.c.html#cdev_init */
    /*  Initializes cdev, remembering fops, making it ready to add to the system with cdev_add. */
    cdev_init(&cdev_struct, &fops_struct);
    cdev_struct.owner = THIS_MODULE;

    /*  cdev_add adds the device represented by p to the system, making it live immediately. A negative error code is returned on failure. */
    retval = cdev_add(&cdev_struct, device_number, count);
    if (retval < 0)
    {
        pr_err("%s - error: cdev_add\n", __func__);
        goto cdev_add_fail;
    }

    /*  This is used to create a struct class pointer that can then be used in calls to device_create.
    Returns struct class pointer on success, or ERR_PTR on error.
    Note, the pointer created here is to be destroyed when finished by making a call to class_destroy. */
    class_struct = class_create(THIS_MODULE, "ddrv_class");
    if (IS_ERR(class_struct))
    {
        pr_err("%s - error: class_create\n", __func__);
        retval = PTR_ERR(class_struct);
        goto class_create_fail;
    }

    /**
     * device_create - creates a device and registers it with sysfs
     * @class: pointer to the struct class that this device should be registered to
     * @parent: pointer to the parent struct device of this new device, if any
     * @devt: the dev_t for the char device to be added
     * @drvdata: the data to be added to the device for callbacks
     * @fmt: string for the device's name
     *
     * This function can be used by char device classes.  A struct device
     * will be created in sysfs, registered to the specified class.
     *
     * A "dev" file will be created, showing the dev_t for the device, if
     * the dev_t is not 0,0.
     * If a pointer to a parent struct device is passed in, the newly created
     * struct device will be a child of that device in sysfs.
     * The pointer to the struct device will be returned from the call.
     * Any further sysfs files that might be required can be created using this
     * pointer.
     *
     * Returns &struct device pointer on success, or ERR_PTR() on error.
     *
     * Note: the struct class passed to this function must have previously
     * been created with a call to class_create().
     */
    device_struct = device_create(class_struct, NULL, device_number, NULL, "ddrv");
    if (IS_ERR(device_struct))
    {
        pr_err("%s - error: class_create\n", __func__);
        retval = PTR_ERR(device_struct);
        goto device_create_fail;
    }

    pr_info("%s - init success\n", __func__);
    return 0;

device_create_fail:
    class_destroy(class_struct);
class_create_fail:
    cdev_del(&cdev_struct);
cdev_add_fail:
    unregister_chrdev_region(device_number, count);
    pr_err("%s - error: init failed\n", __func__);
    return retval;
}

static void __exit ddrv_exit(void)
{
    pr_info("%s\n", __func__);

    /**
     * device_destroy - removes a device that was created with device_create()
     * @class: pointer to the struct class that this device was registered with
     * @devt: the dev_t of the device that was previously registered
     *
     * This call unregisters and cleans up a device that was created with a
     * call to device_create().
     */
    device_destroy(class_struct, device_number);

    class_destroy(class_struct);

    cdev_del(&cdev_struct);

    unregister_chrdev_region(device_number, count);
}

int ddrv_open(struct inode *inode, struct file *file)
{
    pr_info("%s\n", __func__);
    return 0;
}

int ddrv_release(struct inode *inode, struct file *file)
{
    pr_info("%s\n", __func__);
    return 0;
}

ssize_t ddrv_read(struct file *file, char __user *buffer, size_t size, loff_t *offset)
{
    pr_info("%s - extry - size %ld offset %lld\n", __func__, size, (*offset));

    if (size > LOCAL_BUFFER_SIZE)
        size = LOCAL_BUFFER_SIZE;

    if ((*offset) >= LOCAL_BUFFER_SIZE)
        size = 0;

    if (copy_to_user(buffer, &local_buffer, size))
    {
        return -EFAULT;
    }

    (*offset) += size;

    pr_info("%s - exit - size %ld offset %lld\n", __func__, size, (*offset));

    return size;
}

ssize_t ddrv_write(struct file *file, const char __user *buffer, size_t size, loff_t *offset)
{
    pr_info("%s - extry - size %ld offset %lld\n", __func__, size, (*offset));

    if (size > LOCAL_BUFFER_SIZE)
        return -ENOMEM;

    if ((*offset) >= LOCAL_BUFFER_SIZE)
        return -ENOMEM;

    memset(&local_buffer, 0, LOCAL_BUFFER_SIZE);

    if (copy_from_user(&local_buffer, buffer, size))
    {
        return -EFAULT;
    }

    (*offset) += size;

    pr_info("%s - exit - size %ld offset %lld\n", __func__, size, (*offset));

    return size;
}

module_init(ddrv_init);
module_exit(ddrv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DILEEP");
MODULE_DESCRIPTION("DUMMY DEV DRIVER");
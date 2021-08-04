#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include "platform_data.h"

/* https://www.kernel.org/doc/Documentation/driver-model/platform.txt */

#define LOCAL_BUFFER_SIZE 512
char local_buffer[LOCAL_BUFFER_SIZE];

dev_t device_number;
unsigned baseminor = 0;
unsigned count = 10;
const char *name = "pf_driver";

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

int pfdrv_probe(struct platform_device *);
int pfdrv_remove(struct platform_device *);

struct platform_driver platform_driver_struct = {.probe = pfdrv_probe,
                                                 .remove = pfdrv_remove,
                                                 .driver.name = "pfdev0"};
int __init pfdrv_init(void)
{
    int retval;

    pr_info("%s\n", __func__);

    retval = alloc_chrdev_region(&device_number, baseminor, count, name);
    if (retval < 0)
    {
        pr_err("%s - alloc_chrdev_region\n", __func__);
        return retval;
    }

    pr_info("%s - MAJOR - %d; MINOR - %d\n", __func__, MAJOR(device_number), MINOR(device_number));

    class_struct = class_create(THIS_MODULE, "pfdrv_class");
    if (IS_ERR(class_struct))
    {
        pr_err("%s - error: class_create\n", __func__);
        retval = PTR_ERR(class_struct);
        goto class_create_fail;
    }

    retval = platform_driver_register(&platform_driver_struct);
    if (retval < 0)
    {
        pr_err("%s - error: platform_driver_register\n", __func__);
        goto platform_driver_register_fail;
    }
    return 0;

platform_driver_register_fail:
    class_destroy(class_struct);
class_create_fail:
    unregister_chrdev_region(device_number, count);
    pr_err("%s - error: init failed\n", __func__);
    return retval;
}

void __exit pfdrv_exit(void)
{
    pr_info("%s\n", __func__);
    platform_driver_unregister(&platform_driver_struct);
    class_destroy(class_struct);
    unregister_chrdev_region(device_number, count);
}

int pfdrv_probe(struct platform_device *platform_device_struct)
{
    // struct platform_data *dev_platform_data = (struct platform_data *)platform_device_struct->dev.platform_data;
    struct platform_data *dev_platform_data = (struct platform_data *)dev_get_platdata(&platform_device_struct->dev);
    int retval;
    dev_t dev_num = (device_number + platform_device_struct->id);

    pr_info("%s : name - %s; id - %d; serial_number - %s\n",
            __func__,
            platform_device_struct->name,
            platform_device_struct->id,
            dev_platform_data->serial_number);

    // dev_platform_data->device_buffer = kzalloc(LOCAL_BUFFER_SIZE, GFP_KERNEL);
    // if (dev_platform_data->device_buffer == NULL)
    // {
    //     pr_err("%s - error: kzalloc\n", __func__);
    //     return -ENOMEM;
    // }

    cdev_init(&dev_platform_data->cdev_struct, &fops_struct);
    dev_platform_data->cdev_struct.owner = THIS_MODULE;
    retval = cdev_add(&dev_platform_data->cdev_struct, dev_num, count);
    if (retval < 0)
    {
        pr_err("%s - error: cdev_add\n", __func__);
        return retval;
    }

    device_struct = device_create(class_struct, NULL, dev_num, NULL, "ddrv");
    if (IS_ERR(device_struct))
    {
        pr_err("%s - error: class_create\n", __func__);
        retval = PTR_ERR(device_struct);
        goto device_create_fail;
    }

    pr_info("%s - init success\n", __func__);
    return 0;

device_create_fail:
    cdev_del(&dev_platform_data->cdev_struct);
    return retval;
}

int pfdrv_remove(struct platform_device *platform_device_struct)
{
    struct platform_data *dev_platform_data = (struct platform_data *)platform_device_struct->dev.platform_data;
    dev_t dev_num = (device_number + platform_device_struct->id);

    pr_info("%s : name - %s; id - %d; serial_number - %s\n",
            __func__,
            platform_device_struct->name,
            platform_device_struct->id,
            dev_platform_data->serial_number);

    device_destroy(class_struct, dev_num);
    cdev_del(&dev_platform_data->cdev_struct);

    return 0;
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

module_init(pfdrv_init);
module_exit(pfdrv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DILEEP");
MODULE_DESCRIPTION("dummy platform driver");
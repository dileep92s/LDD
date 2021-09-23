#include "linux/module.h"
#include "linux/device.h"
#include "linux/fs.h"
#include "linux/platform_device.h"
#include "pcd_platform.h"

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(msg) "pcd_platform_driver:%s: " msg, __func__

#define MINOR_BASE 0
#define MINOR_COUNT 1

#define MEMSIZE 512
char dev_buffer[MEMSIZE];

dev_t device_number;
struct class *class_pcd;
struct device *device_pcd;

struct pcdev_private_data
{
    struct pcdev_platform_data pcdev_data;
    char *buffer;
    dev_t dev_num;
    // struct cdev cdev;
};

struct pcdrv_private_data
{
    int total_devices;
    dev_t dev_num_base;
    struct class *class_pcd;
    struct device *device_pcd;
} pcdrv_data;

int pcd_open(struct inode *inode, struct file *fp)
{
    pr_info("\n");
    return 0;
}
int pcd_release(struct inode *inode, struct file *fp)
{
    pr_info("\n");
    return 0;
}
loff_t pcd_llseek(struct file *fp, loff_t offset, int whence)
{
    pr_info("\n");
    return 0;
}
ssize_t pcd_read(struct file *fp, char __user *buffer, size_t size, loff_t *fpos)
{
    pr_info("\n");
    return 0;
}
ssize_t pcd_write(struct file *fp, const char __user *buffer, size_t size, loff_t *fpos)
{
    pr_info("\n");
    return -ENOMEM;
}

struct file_operations pcd_fops = {.open = pcd_open,
                                   .release = pcd_release,
                                   .write = pcd_write,
                                   .read = pcd_read,
                                   .llseek = pcd_llseek,
                                   .owner = THIS_MODULE};

int pcd_platform_driver_probe(struct platform_device *pdev)
{
    struct pcdev_platform_data *pcdev_pf_data;

    pcdev_pf_data = (struct pcdev_platform_data *)dev_get_platdata(&pdev->dev);

    pr_info("\n");
    return 0;
}

int pcd_platform_driver_remove(struct platform_device *pdev)
{
    pr_info("\n");
    return 0;
}

struct platform_driver pcd_platform_driver = {
    .probe = pcd_platform_driver_probe,
    .remove = pcd_platform_driver_remove,
    .driver = {.name = "pseudo-char-device"}};

static int __init pcdev_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&pcdrv_data.dev_num_base, MINOR_BASE, MINOR_COUNT, "pfdev");
    if (ret < 0)
    {
        pr_err("alloc_chrdev_region failed");
        return ret;
    }

    pcdrv_data.class_pcd = class_create(THIS_MODULE, "pfdev_class");
    if (IS_ERR(pcdrv_data.class_pcd))
    {
        pr_err("class_create failed");
        unregister_chrdev_region(pcdrv_data.dev_num_base, MINOR_COUNT);
        return PTR_ERR(pcdrv_data.class_pcd);
    }

    platform_driver_register(&pcd_platform_driver);
    pr_info("init done\n");
    return 0;
}

static void __exit pcdev_exit(void)
{
    platform_driver_unregister(&pcd_platform_driver);
    class_destroy(pcdrv_data.class_pcd);
    unregister_chrdev_region(pcdrv_data.dev_num_base, MINOR_COUNT);
    pr_info("exit done\n");
}

module_init(pcdev_init);
module_exit(pcdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("register psuedo platform driver");

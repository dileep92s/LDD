#include "linux/module.h"
#include "linux/fs.h"
#include "linux/cdev.h"
#include "linux/device.h"
#include "linux/kdev_t.h"
#include "linux/uaccess.h"

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(msg) "%s: " msg, __func__

#define MEMSIZE 512
char dev_buffer[MEMSIZE];

dev_t device_number;
#define INITIAL_MINOR 0
#define MINOR_COUNT 1

int pcd_open(struct inode *inode, struct file *fp)
{
    pr_info("open");
    return 0;
}
int pcd_release(struct inode *inode, struct file *fp)
{
    pr_info("");
    return 0;
}
loff_t pcd_llseek(struct file *fp, loff_t offset, int whence)
{
    pr_info("whence %d, offset %lld", whence, offset);
    pr_info("currfp %lld", fp->f_pos);

    switch (whence)
    {
    case SEEK_SET:
    {
        if (offset > MEMSIZE || offset < 0)
            return -EINVAL;
        fp->f_pos = offset;
        break;
    }
    case SEEK_CUR:
    {
        loff_t new_pos = fp->f_pos + offset;
        if (new_pos > MEMSIZE || new_pos < 0)
            return -EINVAL;
        fp->f_pos = new_pos;
        break;
    }
    case SEEK_END:
    {
        loff_t new_pos = MEMSIZE + offset;
        if (new_pos > MEMSIZE || new_pos < 0)
            return -EINVAL;
        fp->f_pos = new_pos;
        break;
    }
    default:
        return -EINVAL;
    }

    pr_info("newfp %lld", fp->f_pos);

    return fp->f_pos;
}
ssize_t pcd_read(struct file *fp, char __user *buffer, size_t size, loff_t *fpos)
{
    pr_info("read req: %zu bytes", size);
    pr_info("curr fpos: %lld", *fpos);

    if ((*fpos + size) > MEMSIZE)
    {
        size = MEMSIZE - (*fpos);
    }

    if (copy_to_user(buffer, &dev_buffer[*fpos], size))
    {
        return -EFAULT;
    }

    *fpos += size;

    pr_info("bytes read: %zu bytes", size);
    pr_info("new fpos: %lld", *fpos);

    return size;
}
ssize_t pcd_write(struct file *fp, const char __user *buffer, size_t size, loff_t *fpos)
{
    pr_info("write req: %zu bytes", size);
    pr_info("curr fpos: %lld", *fpos);

    if ((*fpos + size) > MEMSIZE)
    {
        size = MEMSIZE - (*fpos);
    }

    if (size == 0)
    {
        pr_err("no space left on device");
        return -ENOMEM;
    }

    if (copy_from_user(&dev_buffer[*fpos], buffer, size))
    {
        return -EFAULT;
    }

    *fpos += size;

    pr_info("bytes written: %zu bytes", size);
    pr_info("new fpos: %lld", *fpos);

    return size;
}

struct cdev pcd_cdev;
struct file_operations pcd_fops = {.open = pcd_open,
                                   .release = pcd_release,
                                   .write = pcd_write,
                                   .read = pcd_read,
                                   .llseek = pcd_llseek,
                                   .owner = THIS_MODULE};

struct class *class_pcd;
struct device *device_pcd;

static int __init pcd_driver_init(void)
{
    int ret;

    /* dynamically allocate a device number (major, minor_initial, num_devices, name) */
    ret = alloc_chrdev_region(&device_number, INITIAL_MINOR, MINOR_COUNT, "pcd_devices");
    if (ret < 0)
        goto out;

    pr_info("device number = %d:%d", MAJOR(device_number), MINOR(device_number));

    /* initialize cdev with fops */
    cdev_init(&pcd_cdev, &pcd_fops);
    pcd_cdev.owner = THIS_MODULE;

    /* register a device */
    ret = cdev_add(&pcd_cdev, device_number, MINOR_COUNT);
    if (ret < 0)
        goto chrdev_unreg;

    /* create device class under /sys/class */
    class_pcd = class_create(THIS_MODULE, "pcd_class");
    if (IS_ERR(class_pcd))
    {
        pr_err("class creation failed");
        ret = PTR_ERR(class_pcd);
        goto cdev_del;
    }

    /* populate the sysfs with device incan you please mation */
    device_pcd = device_create(class_pcd, NULL, device_number, NULL, "pcd");
    if (IS_ERR(device_pcd))
    {
        pr_err("device creation failed");
        ret = PTR_ERR(device_pcd);
        goto class_del;
    }

    pr_info("init ok\n");

    memset(dev_buffer, 0, MEMSIZE);

    return 0;

class_del:
    class_destroy(class_pcd);
cdev_del:
    cdev_del(&pcd_cdev);
chrdev_unreg:
    unregister_chrdev_region(device_number, 1);
out:
    pr_err("module insertion failed");
    return ret;
}

static void __exit pcd_driver_exit(void)
{
    /* destroy device */
    device_destroy(class_pcd, device_number);
    class_destroy(class_pcd);
    cdev_del(&pcd_cdev);
    unregister_chrdev_region(device_number, 1);

    pr_info("exit ok\n");
}

module_init(pcd_driver_init);
module_exit(pcd_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("pseudo char device");
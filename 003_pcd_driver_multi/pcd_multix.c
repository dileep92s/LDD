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

#define NUM_DEVICES 4

#define MEM_SIZE_DEV1 512
#define MEM_SIZE_DEV2 512
#define MEM_SIZE_DEV3 512
#define MEM_SIZE_DEV4 512

char buff_dev1[MEM_SIZE_DEV1];
char buff_dev2[MEM_SIZE_DEV2];
char buff_dev3[MEM_SIZE_DEV3];
char buff_dev4[MEM_SIZE_DEV4];

struct pcd_device_data
{
    char *buffer;
    unsigned size;
    const char *serial_num;
    int perm;
    struct cdev cdev;
};

struct pcd_driver_data
{
    int total_devices;
    dev_t device_number;
    struct class *class;
    struct device *device;
    struct pcd_device_data device_data[NUM_DEVICES];
};

struct pcd_driver_data driver_data = {
    .total_devices = NUM_DEVICES,
    .device_data = {
        [0] = {.buffer = buff_dev1, .size = MEM_SIZE_DEV1, .serial_num = "PCDEV001", .perm = O_RDONLY},
        [1] = {.buffer = buff_dev2, .size = MEM_SIZE_DEV2, .serial_num = "PCDEV002", .perm = O_WRONLY},
        [2] = {.buffer = buff_dev3, .size = MEM_SIZE_DEV3, .serial_num = "PCDEV003", .perm = O_RDWR},
        [3] = {.buffer = buff_dev4, .size = MEM_SIZE_DEV4, .serial_num = "PCDEV004", .perm = O_RDONLY}}};

#define INITIAL_MINOR 0

int pcd_open(struct inode *inode, struct file *fp);
int pcd_release(struct inode *inode, struct file *fp);
loff_t pcd_llseek(struct file *fp, loff_t offset, int whence);
ssize_t pcd_read(struct file *fp, char __user *buffer, size_t size, loff_t *fpos);
ssize_t pcd_write(struct file *fp, const char __user *buffer, size_t size, loff_t *fpos);
int check_permission(int dev_perm, int acc_mode);

struct file_operations pcd_fops = {.open = pcd_open,
                                   .release = pcd_release,
                                   .write = pcd_write,
                                   .read = pcd_read,
                                   .llseek = pcd_llseek,
                                   .owner = THIS_MODULE};

static int __init pcd_driver_init(void)
{
    int ret;
    int i;

    /* dynamically allocate a device number (major, minor_initial, num_devices, name) */
    ret = alloc_chrdev_region(&driver_data.device_number, INITIAL_MINOR, NUM_DEVICES, "pcd_devices");
    if (ret < 0)
    {
        pr_err("alloc_chrdev_region failed");
        goto out;
    }

    /* create device class under /sys/class */
    driver_data.class = class_create(THIS_MODULE, "pcd_class");
    if (IS_ERR(driver_data.class))
    {
        pr_err("class creation failed");
        ret = PTR_ERR(driver_data.class);
        goto chrdev_unreg;
    }

    for (i = 0; i < NUM_DEVICES; i++)
    {
        pr_info("device number = %d:%d", MAJOR(driver_data.device_number + i), MINOR(driver_data.device_number + i));

        /* initialize cdev with fops */
        cdev_init(&driver_data.device_data[i].cdev, &pcd_fops);

        /* register a device */
        driver_data.device_data[i].cdev.owner = THIS_MODULE;
        ret = cdev_add(&driver_data.device_data[i].cdev, driver_data.device_number + i, 1);
        if (ret < 0)
            goto class_del;

        /* populate the sysfs with device incan you please mation */
        driver_data.device = device_create(driver_data.class, NULL, driver_data.device_number + i, NULL, "pcdevm%d", i + 1);
        if (IS_ERR(driver_data.device))
        {
            pr_err("device creation failed");
            ret = PTR_ERR(driver_data.device);
            goto cdev_del;
        }
    }

    pr_info("init ok\n");
    return 0;

cdev_del:
class_del:
    for (; i >= 0; i--)
    {
        device_destroy(driver_data.class, driver_data.device_number + 1);
        cdev_del(&driver_data.device_data[i].cdev);
    }
    class_destroy(driver_data.class);
chrdev_unreg:
    unregister_chrdev_region(driver_data.device_number, NUM_DEVICES);
out:
    pr_err("module insertion failed");
    return ret;
}

static void __exit pcd_driver_exit(void)
{
    int i;
    /* destroy device */
    for (i = 0; i < NUM_DEVICES; i++)
    {
        device_destroy(driver_data.class, driver_data.device_number + i);
        cdev_del(&driver_data.device_data[i].cdev);
    }
    class_destroy(driver_data.class);
    unregister_chrdev_region(driver_data.device_number, NUM_DEVICES);

    pr_info("exit ok\n");
}

int check_permission(int dev_perm, int acc_mode)
{
    if ((dev_perm == O_RDONLY) && (FMODE_WRITE & acc_mode))
        return -EPERM;
    if ((dev_perm == O_WRONLY) && (FMODE_READ & acc_mode))
        return -EPERM;
    return 0;
}

int pcd_open(struct inode *inode, struct file *fp)
{
    int minor;
    int ret;
    struct pcd_device_data *dev_data;

    minor = MINOR(inode->i_rdev);
    pr_info("open minor - %d", minor);

    dev_data = container_of(inode->i_cdev, struct pcd_device_data, cdev);
    fp->private_data = dev_data;

    ret = check_permission(dev_data->perm, fp->f_mode);

    if (ret == 0)
    {
        pr_info("open ok");
    }
    else
    {
        pr_err("open perm error");
    }

    return ret;
}

int pcd_release(struct inode *inode, struct file *fp)
{
    pr_info("release ok");
    return 0;
}

loff_t pcd_llseek(struct file *fp, loff_t offset, int whence)
{
    struct pcd_device_data *dev_data = (struct pcd_device_data *)fp->private_data;
    int max_size = dev_data->size;

    pr_info("whence %d, offset %lld", whence, offset);
    pr_info("currfp %lld", fp->f_pos);

    switch (whence)
    {
    case SEEK_SET:
    {
        if (offset > max_size || offset < 0)
            return -EINVAL;
        fp->f_pos = offset;
        break;
    }
    case SEEK_CUR:
    {
        loff_t new_pos = fp->f_pos + offset;
        if (new_pos > max_size || new_pos < 0)
            return -EINVAL;
        fp->f_pos = new_pos;
        break;
    }
    case SEEK_END:
    {
        loff_t new_pos = max_size + offset;
        if (new_pos > max_size || new_pos < 0)
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
    struct pcd_device_data *dev_data = (struct pcd_device_data *)fp->private_data;
    int max_size = dev_data->size;

    pr_info("read req: %zu bytes", size);
    pr_info("curr fpos: %lld", *fpos);

    if ((*fpos + size) > max_size)
    {
        size = max_size - (*fpos);
    }

    if (copy_to_user(buffer, &dev_data->buffer[*fpos], size))
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
    struct pcd_device_data *dev_data = (struct pcd_device_data *)fp->private_data;
    int max_size = dev_data->size;

    pr_info("write req: %zu bytes", size);
    pr_info("curr fpos: %lld", *fpos);

    if ((*fpos + size) > max_size)
    {
        size = max_size - (*fpos);
    }

    if (size == 0)
    {
        pr_err("no space left on device");
        return -ENOMEM;
    }

    if (copy_from_user(&dev_data->buffer[*fpos], buffer, size))
    {
        return -EFAULT;
    }

    *fpos += size;

    pr_info("bytes written: %zu bytes", size);
    pr_info("new fpos: %lld", *fpos);

    return size;
}

module_init(pcd_driver_init);
module_exit(pcd_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("pseudo char device");
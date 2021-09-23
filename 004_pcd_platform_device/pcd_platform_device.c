#include "linux/module.h"
#include "linux/device.h"
#include "linux/fs.h"
#include "linux/platform_device.h"
#include "pcd_platform.h"
#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(msg) "pcd_platform_device:%s: " msg, __func__

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(msg) "pcd_platform_device:%s: " msg, __func__

void pcdev_release(struct device *dev);

struct pcdev_platform_data pcdev_data[2] = {[0] = {.size = 512, .perm = O_RDWR, .serial_number = "PCDEV001"},
                                            [1] = {.size = 512, .perm = O_RDWR, .serial_number = "PCDEV002"}};

struct platform_device pcdev1 = {.name = "pseudo-char-device",
                                 .id = 0,
                                 .dev = {.platform_data = &pcdev_data[0],
                                         .release = pcdev_release}};
struct platform_device pcdev2 = {.name = "pseudo-char-device",
                                 .id = 1,
                                 .dev = {.platform_data = &pcdev_data[1],
                                         .release = pcdev_release}};

static int __init pcdev_init(void)
{
    platform_device_register(&pcdev1);
    platform_device_register(&pcdev2);
    pr_info("init done\n");
    return 0;
}

static void __exit pcdev_exit(void)
{
    platform_device_unregister(&pcdev1);
    platform_device_unregister(&pcdev2);
    pr_info("exit done\n");
}

void pcdev_release(struct device *dev)
{
    pr_info("release done\n");
}

module_init(pcdev_init);
module_exit(pcdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("register psuedo platform device");

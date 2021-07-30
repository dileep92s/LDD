#include "linux/module.h"
#include "linux/device.h"
#include "linux/fs.h"
#include "linux/platform_device.h"
#include "pcd_platform.h"

void pcdev_release(struct device *dev);

struct pcdev_platform_data pcdev_data[2] = {
    [0] = {.size = 512, .perm = O_RDWR, .serial_number = "PCDEV001"},
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
    pr_info("init done");
    return 0;
}

static void __exit pcdev_exit(void)
{
    platform_device_unregister(&pcdev1);
    platform_device_unregister(&pcdev2);
    pr_info("exit done");
}

void pcdev_release(struct device *dev)
{
    pr_info("release done");
}

module_init(pcdev_init);
module_exit(pcdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("register psuedo platform device");

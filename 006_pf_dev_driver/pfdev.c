#include <linux/module.h>
#include <linux/platform_device.h>
#include "platform_data.h"

/* https://www.kernel.org/doc/Documentation/driver-model/platform.txt */

void pfdev_release(struct device *dev);

/* data required for driver - like permissions, serial num etc*/
struct platform_data platform_data_dev0 = {.serial_number = "PF_DEV_000"};

/* creates an entry in /sys/devices/platform/ */
struct platform_device platform_device_struct = {.name = "pfdev0",
                                                 .id = 0,
                                                 .dev.release = pfdev_release,
                                                 .dev.platform_data = (void *)&platform_data_dev0};

int __init pfdev_init(void)
{
    pr_info("%s\n", __func__);
    platform_device_register(&platform_device_struct);
    return 0;
}

void __exit pfdev_exit(void)
{
    pr_info("%s\n", __func__);
    platform_device_unregister(&platform_device_struct);
}

void pfdev_release(struct device *dev)
{
    pr_info("%s - %s\n", __func__, ((struct platform_data *)(dev->platform_data))->serial_number);
}

module_init(pfdev_init);
module_exit(pfdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DILEEP");
MODULE_DESCRIPTION("dummy platform device");
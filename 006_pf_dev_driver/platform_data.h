#pragma once

#include <linux/module.h>
#include <linux/cdev.h>

struct platform_data
{
    const char *serial_number;
    struct cdev cdev_struct;
    char *device_buffer;
    size_t device_buffer_size;
};
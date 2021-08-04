#include<linux/module.h>

static int __init test_ko_entry(void)
{
    pr_info("test_ko_entry\n");
    return 0;
}

static void __exit test_ko_exit(void)
{
    pr_info("test_ko_exit\n");
}

module_init(test_ko_entry);
module_exit(test_ko_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DS");
MODULE_DESCRIPTION("test_ko");
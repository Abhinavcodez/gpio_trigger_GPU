#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xc1514a3b, "free_irq" },
	{ 0xfe990052, "gpio_free" },
	{ 0xfe2a48cd, "class_remove_file_ns" },
	{ 0xb1b9cfc9, "cdev_del" },
	{ 0x19edaabb, "device_destroy" },
	{ 0x75646747, "class_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x3c12dfe, "cancel_work_sync" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x7682ba4e, "__copy_overflow" },
	{ 0x8c8569cb, "kstrtoint" },
	{ 0x2d3385d3, "system_wq" },
	{ 0xc5b6f236, "queue_work_on" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x6ca9b86a, "class_create" },
	{ 0x2e3443fd, "device_create" },
	{ 0x22d6de43, "cdev_init" },
	{ 0xec957a9, "cdev_add" },
	{ 0x8dc23e5d, "class_create_file_ns" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xe1c1b72c, "gpio_to_desc" },
	{ 0x357d93b1, "gpiod_direction_input" },
	{ 0xc6a53552, "gpiod_to_irq" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0x479b90bb, "param_ops_int" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x122c3a7e, "_printk" },
	{ 0xf9a482f9, "msleep" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0x15ba50a6, "jiffies" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xa916b694, "strnlen" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0xe2fd41e5, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "334AC25878BB56263D9138D");

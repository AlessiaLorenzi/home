#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x14522340, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x4f1939c7, "per_cpu__current_task" },
	{ 0x5a34a45c, "__kmalloc" },
	{ 0x6363cd37, "sockfd_lookup" },
	{ 0x7ca65271, "single_open" },
	{ 0x6307fc98, "del_timer" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xc1d878fa, "single_release" },
	{ 0xd691cba2, "malloc_sizes" },
	{ 0x96461b6f, "seq_puts" },
	{ 0x71356fba, "remove_wait_queue" },
	{ 0x3d1457eb, "alloc_tty_driver" },
	{ 0x78253cfe, "tty_hung_up_p" },
	{ 0xd87b0b1f, "skb_clone" },
	{ 0x6edb8870, "dev_get_by_name" },
	{ 0xfc4f55f3, "down_interruptible" },
	{ 0x77e93f6f, "seq_printf" },
	{ 0x1a6d6e4f, "remove_proc_entry" },
	{ 0xfa0d49c7, "__register_chrdev" },
	{ 0x8b493492, "tty_register_driver" },
	{ 0x6a9f26c9, "init_timer_key" },
	{ 0xfcbb547, "put_tty_driver" },
	{ 0x712aa29b, "_spin_lock_irqsave" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0xb72ec8a3, "seq_read" },
	{ 0x7d11c268, "jiffies" },
	{ 0x6f8edcc2, "tty_set_operations" },
	{ 0xffc7c184, "__init_waitqueue_head" },
	{ 0xffd5a395, "default_wake_function" },
	{ 0xde0bdcff, "memset" },
	{ 0xed113e22, "proc_mkdir" },
	{ 0x150853cf, "down_trylock" },
	{ 0x273a9d2b, "tty_get_baud_rate" },
	{ 0xea147363, "printk" },
	{ 0x77c14a86, "tty_ldisc_flush" },
	{ 0x7ec9bfbc, "strncpy" },
	{ 0x85f8a266, "copy_to_user" },
	{ 0xb4390f9a, "mcount" },
	{ 0x6dcaeb88, "per_cpu__kernel_stack" },
	{ 0x9900a644, "dev_remove_pack" },
	{ 0x4b07e779, "_spin_unlock_irqrestore" },
	{ 0x45450063, "mod_timer" },
	{ 0xd3c80841, "skb_pull" },
	{ 0x1c740bd6, "init_net" },
	{ 0x8b6c553c, "fput" },
	{ 0x92515071, "tty_register_device" },
	{ 0xce01606b, "skb_queue_tail" },
	{ 0x7dceceac, "capable" },
	{ 0xc61a60a9, "tty_unregister_device" },
	{ 0x67b27ec1, "tty_std_termios" },
	{ 0xb601be4c, "__x86_indirect_thunk_rdx" },
	{ 0xe59fe7f4, "tty_wait_until_sent" },
	{ 0x25421969, "__alloc_skb" },
	{ 0xaabc9808, "tty_driver_flush_buffer" },
	{ 0x93fca811, "__get_free_pages" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x3bd1b1f6, "msecs_to_jiffies" },
	{ 0xd62c833f, "schedule_timeout" },
	{ 0x1000e51, "schedule" },
	{ 0x3d75cbcf, "kfree_skb" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0x6d6b15ff, "create_proc_entry" },
	{ 0x325b87b8, "tty_insert_flip_string" },
	{ 0x266c7c38, "wake_up_process" },
	{ 0xcc5005fe, "msleep_interruptible" },
	{ 0x2044fa9e, "kmem_cache_alloc_trace" },
	{ 0xf604490f, "tty_unregister_driver" },
	{ 0xde5f5c0b, "tty_hangup" },
	{ 0x4302d0eb, "free_pages" },
	{ 0x642e54ac, "__wake_up" },
	{ 0x650fb346, "add_wait_queue" },
	{ 0x5ca8e4f6, "seq_lseek" },
	{ 0x37a0cba, "kfree" },
	{ 0x236c8c64, "memcpy" },
	{ 0x33d92f9a, "prepare_to_wait" },
	{ 0xd3f74cf1, "interruptible_sleep_on_timeout" },
	{ 0x3f1899f1, "up" },
	{ 0x9ccb2622, "finish_wait" },
	{ 0x4fa751de, "tty_flip_buffer_push" },
	{ 0xbf1cac9f, "dev_add_pack" },
	{ 0xd110dab, "dev_queue_xmit" },
	{ 0x207b7e2c, "skb_put" },
	{ 0x3302b500, "copy_from_user" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "8C62004267587E7383E8BF1");

static const struct rheldata _rheldata __used
__attribute__((section(".rheldata"))) = {
	.rhel_major = 6,
	.rhel_minor = 10,
	.rhel_release = 754,
};
#ifdef RETPOLINE
	MODULE_INFO(retpoline, "Y");
#endif

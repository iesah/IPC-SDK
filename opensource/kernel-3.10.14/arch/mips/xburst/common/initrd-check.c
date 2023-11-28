
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/kdev_t.h>
#include <linux/syscalls.h>

static void check_file(char *fname)
{
	int err;
	pr_info("check file %s ... ",fname);
	err = sys_access((const char __user *) fname, O_RDONLY);
	if (err < 0)
		pr_info("failed %d.\n", err);
	else
		pr_info("successed.\n");
}

static int __init check_rootfs(void)
{
	int err;

	if (sys_access((const char __user *) "/dev", O_RDWR) < 0) {
		pr_info("Dir /dev is not exist.\n");
		return 0;
	}

	if (sys_access((const char __user *) "/dev/console", O_RDWR) < 0) {
		printk("create /dev/console\n");
		err = sys_mknod((const char __user __force *) "/dev/console",
				S_IFCHR | S_IRUSR | S_IWUSR,
				new_encode_dev(MKDEV(5, 1)));
		if (err < 0)
			goto out;
	}

	check_file("/linuxrc");
	check_file("/init");
	check_file("/init.rc");
	check_file("/sbin/adbd");
	check_file("/bin/busybox");

	return 0;

out:
	printk(KERN_WARNING "Failed to create dev\n");
	return err;
}

late_initcall(check_rootfs);

#include <linux/device.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/string.h>

#define TARGET_DEVICE_NAME "CSC3551"
#define NEW_DEVICE_NAME "CLSA0101"

/* Calculate the length of target and new device names at compile time */
const size_t TARGET_DEVICE_NAME_LEN =
	sizeof(TARGET_DEVICE_NAME) - 1;	 // Subtract 1 to ignore null terminator
const size_t NEW_DEVICE_NAME_LEN =
	sizeof(NEW_DEVICE_NAME) - 1;  // Subtract 1 to ignore null terminator

/* Define kprobe structure */
static struct kprobe kp;

/* Pre-handler: called just before the probed instruction is executed */
static int pre_handler(struct kprobe* p, struct pt_regs* regs) {
	struct device* dev;
	const char* device_name;

	/* Extract arguments from the probed function, assuming x86-64 calling
	 * convention */
	dev = (struct device*)regs->di;		  // First argument (dev)
	device_name = (const char*)regs->si;  // Second argument (device_name)

	/* Check if the device_name matches the target using strncmp with
	 * compile-time length */
	if(device_name
	   && strncmp(device_name, TARGET_DEVICE_NAME, TARGET_DEVICE_NAME_LEN)
			  == 0) {
		pr_info("Device name matched: %s. Overwriting to %s.\n", device_name,
				NEW_DEVICE_NAME);
		/* Overwrite the device_name to NEW_DEVICE_NAME */
		strncpy((char*)device_name, NEW_DEVICE_NAME, NEW_DEVICE_NAME_LEN);
	}

	return 0;
}

/* Register kprobe at the entry of the cs35l41_hda_probe function */
static int __init my_kprobe_init(void) {
	int ret;

	/* Specify the symbol to be probed */
	kp.symbol_name = "cs35l41_hda_probe";

	/* Assign the pre_handler */
	kp.pre_handler = pre_handler;

	/* Register the kprobe */
	ret = register_kprobe(&kp);
	if(ret < 0) {
		pr_err("Failed to register kprobe: %d\n", ret);
		return ret;
	}

	pr_info("Kprobe registered for cs35l41_hda_probe function.\n");
	return 0;
}

/* Cleanup when module is unloaded */
static void __exit my_kprobe_exit(void) {
	unregister_kprobe(&kp);
	pr_info("Kprobe unregistered.\n");
}

module_init(my_kprobe_init);
module_exit(my_kprobe_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple kprobe module to patch cs35l41_hda_probe device "
				   "name.");

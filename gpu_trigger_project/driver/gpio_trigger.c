#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/gpio.h>
#include<linux/interrupt.h>
#include<linux/workqueue.h>
#include<linux/uaccess.h>
#include<linux/delay.h>
#include<linux/printk.h>
#include<linux/kern_levels.h>

static dev_t dev_num;
static struct class* gpio_trigger_class;
static struct cdev my_cdev;

#define DEVICE_NAME "gpio_trigger"
#define CLASS_NAME  "gpio_trigger_class"
#define GPU_TRIGGER_IOCTL _IO('K', 1)

static int gpio_num = 980;   // default GPIO
module_param(gpio_num, int, S_IRUGO);
MODULE_PARM_DESC(gpio_num, "GPIO number to use as trigger");

static int irq_number;
static struct work_struct gpu_work;
static int trigger_mode = 0;   // default mode

// ---- Result buffer for user-space reads ----
static char result_buffer[128];
static int result_size = 0;
static DEFINE_MUTEX(result_lock);

// ---------- Work handler ----------
static void trigger_gpu_computation(void)
{
    pr_info("gpio_trigger: Simulating GPU computation...\n");
    msleep(100);

    mutex_lock(&result_lock);
    snprintf(result_buffer, sizeof(result_buffer),
             "GPU computation done! jiffies=%lu\n", jiffies);
    result_size = strlen(result_buffer);
    mutex_unlock(&result_lock);

    pr_info("gpio_trigger: GPU computation finished.\n");
}

static void gpu_work_handler(struct work_struct* work)
{
    pr_info("gpio_trigger: Work handler called.\n");
    trigger_gpu_computation();
}

// ---------- ISR ----------
static irqreturn_t my_gpio_isr(int irq, void* dev_id)
{
    pr_info("gpio_trigger: GPIO interrupt received! Scheduling work.\n");
    schedule_work(&gpu_work);
    return IRQ_HANDLED;
}

// ---------- char dev ops ----------
static int dev_open(struct inode* inode, struct file* file) { return 0; }
static int dev_release(struct inode* inode, struct file* file) { return 0; }

static long dev_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
    case GPU_TRIGGER_IOCTL:
        pr_info("gpio_trigger: IOCTL command received.\n");
        schedule_work(&gpu_work);
        return 0;
    default:
        return -EINVAL;
    }
}

// --- read() implementation ---
static ssize_t dev_read(struct file* file, char __user *buf,
                        size_t count, loff_t *ppos)
{
    ssize_t ret;

    if (*ppos > 0) // EOF already sent
        return 0;

    mutex_lock(&result_lock);
    if (result_size == 0) {
        mutex_unlock(&result_lock);
        return 0;
    }

    if (count < result_size) {
        mutex_unlock(&result_lock);
        return -EINVAL;
    }

    if (copy_to_user(buf, result_buffer, result_size)) {
        mutex_unlock(&result_lock);
        return -EFAULT;
    }

    *ppos = result_size;
    ret = result_size;
    mutex_unlock(&result_lock);
    return ret;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .unlocked_ioctl = dev_ioctl,
    .read = dev_read,
};

// ---------- Sysfs attribute ----------
static ssize_t mode_show(const struct class *class,
                         const struct class_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", trigger_mode);
}

static ssize_t mode_store(const struct class *class,
                          const struct class_attribute *attr,
                          const char *buf, size_t count)
{
    int val;

    if (kstrtoint(buf, 10, &val))
        return -EINVAL;

    if (val < 0 || val > 2)
        return -EINVAL;

    trigger_mode = val;
    pr_info("gpio_trigger: Trigger mode set to %d\n", trigger_mode);

    switch (trigger_mode) {
        case 0:
            pr_info("gpio_trigger: Mode 0 (Interrupt)\n");
            break;
        case 1:
            pr_info("gpio_trigger: Mode 1 (IOCTL)\n");
            break;
        case 2:
            pr_info("gpio_trigger: Mode 2 (Sysfs write, direct trigger)\n");
            schedule_work(&gpu_work);
            break;
    }

    return count;
}

CLASS_ATTR_RW(mode);

// ---------- module init/exit ----------
static int __init gpio_trigger_init(void)
{
    int ret;
    struct device *dev;

    pr_info("gpio_trigger: Initializing.\n");

    // char dev allocation
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret) return ret;

    gpio_trigger_class = class_create(CLASS_NAME);
    if (IS_ERR(gpio_trigger_class)) {
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(gpio_trigger_class);
    }

    dev = device_create(gpio_trigger_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(dev)) {
        class_destroy(gpio_trigger_class);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(dev);
    }

    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev_num, 1);

    // sysfs entry
    ret = class_create_file(gpio_trigger_class, &class_attr_mode);
    if (ret) {
        pr_err("gpio_trigger: Failed to create sysfs entry\n");
    }

    // GPIO setup
    if (!gpio_is_valid(gpio_num)) {
        pr_alert("gpio_trigger: Invalid GPIO %d\n", gpio_num);
        return -ENODEV;
    }
    gpio_request(gpio_num, "sysfs");
    gpio_direction_input(gpio_num);

    // map GPIO to IRQ
    irq_number = gpio_to_irq(gpio_num);
    pr_info("gpio_trigger: Using GPIO %d mapped to IRQ %d\n", gpio_num, irq_number);

    ret = request_irq(irq_number, my_gpio_isr,
                      IRQF_TRIGGER_RISING, DEVICE_NAME, NULL);
    if (ret) {
        pr_alert("gpio_trigger: Cannot request IRQ %d\n", irq_number);
        return ret;
    }

    INIT_WORK(&gpu_work, gpu_work_handler);
    mutex_init(&result_lock);

    pr_info("gpio_trigger: Module loaded.\n");
    return 0;
}

static void __exit gpio_trigger_exit(void)
{
    free_irq(irq_number, NULL);
    gpio_free(gpio_num);

    class_remove_file(gpio_trigger_class, &class_attr_mode);

    cdev_del(&my_cdev);
    device_destroy(gpio_trigger_class, dev_num);
    class_destroy(gpio_trigger_class);
    unregister_chrdev_region(dev_num, 1);

    cancel_work_sync(&gpu_work);
    pr_info("gpio_trigger: Module unloaded.\n");
}

module_init(gpio_trigger_init);
module_exit(gpio_trigger_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("GPIO-triggered Character Device Driver with multiple trigger modes + read support");
MODULE_VERSION("1.1");
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define DEVICE_NAME "mychardev"
#define CLASS_NAME  "myclass"

static dev_t my_dev;  // 设备号
static struct cdev my_cdev;  // 字符设备结构体
static struct class *my_class;  // 设备类
static struct device *my_device;  // 设备

static int major = 240;  // 主设备号
static int minor = 0;  // 次设备号
static int count = 1;  // 设备数量

// 打开设备
static int device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device opened\n");
    return 0;
}

// 关闭设备
static int device_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device closed\n");
    return 0;
}

// 读取设备
static ssize_t device_read(struct file *filp, char __user *buf, size_t count, loff_t *offset) {
    char data[] = "Hello from the kernel!\n";
    if (*offset >= sizeof(data)) {
        return 0;
    }
    if (copy_to_user(buf, data + *offset, count > sizeof(data) - *offset ? sizeof(data) - *offset : count)) {
        return -EFAULT;
    }
    *offset += count;
    return count;
}

// 写入设备
static ssize_t device_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset) {
    char user_data[80];
    if (count > sizeof(user_data)) {
        return -EINVAL;
    }
    if (copy_from_user(user_data, buf, count)) {
        return -EFAULT;
    }
    user_data[count] = '\0';
    printk(KERN_INFO "Received: %s", user_data);
    return count;
}

// 文件操作结构体
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = device_open,
    .release = device_release,
    .read    = device_read,
    .write   = device_write,
};

// 初始化模块
static int __init mychardev_init(void) {
    int ret;

    // 分配设备号
    ret = alloc_chrdev_region(&my_dev, minor, count, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ALERT "Failed to allocate device number\n");
        return ret;
    }
    major = MAJOR(my_dev);

    // 初始化cdev结构体
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    // 添加cdev到内核
    ret = cdev_add(&my_cdev, my_dev, count);
    if (ret < 0) {
        unregister_chrdev_region(my_dev, count);
        printk(KERN_ALERT "Failed to add cdev\n");
        return ret;
    }

    // 创建设备类
    my_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(my_class)) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(my_dev, count);
        return PTR_ERR(my_class);
    }

    // 创建设备节点
    my_device = device_create(my_class, NULL, my_dev, NULL, DEVICE_NAME);
    if (IS_ERR(my_device)) {
        class_destroy(my_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(my_dev, count);
        return PTR_ERR(my_device);
    }

    printk(KERN_INFO "Device registered successfully\n");
    return 0;
}

// 卸载模块
static void __exit mychardev_exit(void) {
    device_destroy(my_class, my_dev);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(my_dev, count);
    printk(KERN_INFO "Device unregistered\n");
}

module_init(mychardev_init);
module_exit(mychardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple character device driver");
/* Pseudo Character Device Driver */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>

#define DEV_MEM_SIZE    512

char pseudo_buffer[DEV_MEM_SIZE];

dev_t pseudo_dev;
struct cdev pseudo_cdev;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ahmet Can GULMEZ");
MODULE_DESCRIPTION("Pseudo Character Device Driver");

int pseudo_open(struct inode *inode, struct file *file)
{
   printk(KERN_INFO "open requested\n");
   return 0;
}

int pseudo_release(struct inode *inode, struct file *file)
{
   printk(KERN_INFO "release requested\n");
   return 0;
}

ssize_t pseudo_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
   printk(KERN_INFO "read requested for %zu bytes\n", count);
   printk(KERN_INFO "current file position = %lld\n", *offset);

   if (*offset + count > DEV_MEM_SIZE)
      count = DEV_MEM_SIZE - *offset;

   if (copy_to_user(buf, &pseudo_buffer[*offset], count) != 0)
      return -EFAULT;

   *offset += count;

   printk(KERN_INFO "number of bytes successfully read = %zu\n", count);
   printk(KERN_INFO "updated file position = %lld\n", *offset);

   return count;
}

ssize_t pseudo_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{ 
   printk(KERN_INFO "write requested for %zu bytes\n", count);
   printk(KERN_INFO "current file position = %lld\n", *offset);

   if (*offset + count > DEV_MEM_SIZE)
      count = DEV_MEM_SIZE - *offset;
 
   if (!count)
      return -ENOMEM;

   if (copy_from_user(&pseudo_buffer[*offset], buf, count) != 0)
      return -EFAULT;

   *offset += count;

   printk(KERN_INFO "number of bytes successfully written = %zu\n", count);
   printk(KERN_INFO "updated file position = %lld\n", *offset);

   return count;
}

loff_t pseudo_lseek(struct file *file, loff_t offset, int whence)
{
   printk(KERN_INFO "lseek requested with offset = %lld\n", offset);

   switch (whence) {
      case SEEK_SET:
         if (offset < 0 || offset > DEV_MEM_SIZE)
            return -EINVAL;
         file->f_pos = offset;
         break;
      case SEEK_CUR:
         if (offset + file->f_pos < 0 || offset + file->f_pos > DEV_MEM_SIZE)
            return -EINVAL;
         file->f_pos += offset;
         break;
      case SEEK_END:
         if (offset < 0 || offset > DEV_MEM_SIZE)
            return -EINVAL;
         file->f_pos = DEV_MEM_SIZE - offset;
         break;
      default:
         return -EINVAL;
   }

   return offset;
}

struct file_operations pseudo_fops = {
   .owner = THIS_MODULE,
   .open = pseudo_open,
   .release = pseudo_release,
   .read = pseudo_read,
   .write = pseudo_write,
   .llseek = pseudo_lseek
};
struct class *pseudo_class;
struct device *pseudo_device;

static int __init pseudo_init(void)
{
   int ret;

   printk(KERN_INFO "Pseudo char driver loaded!\n");

   /* Dynamically allocate a device number */
   ret = alloc_chrdev_region(&pseudo_dev, 0, 1, "pseudo_char_device");
   if (ret < 0)
      goto err_alloc;

   printk(KERN_INFO "Device number (%d,%d)\n", MAJOR(pseudo_dev), MINOR(pseudo_dev));
   
   /* Initialize the cdev structure with file ops */
   cdev_init(&pseudo_cdev, &pseudo_fops);
   
   /* Register cdev structure with VFS */
   ret = cdev_add(&pseudo_cdev, pseudo_dev, 1);
   if (ret < 0)
      goto err_cdev;

   /* Create device class under /sys/class */
   pseudo_class = class_create("pseudo");
   if (IS_ERR(pseudo_class)) {
      ret = PTR_ERR(pseudo_class);
      goto err_class;
   }
   
   /* Populate the sysfs with device information */
   pseudo_device = device_create(pseudo_class, NULL, pseudo_dev, NULL, "pseudo0");
   if (IS_ERR(pseudo_device)) {
      ret = PTR_ERR(pseudo_device);
      goto err_device;
   }

   return 0;

err_device:
   class_destroy(pseudo_class);

err_class:
   cdev_del(&pseudo_cdev);

err_cdev:
   unregister_chrdev_region(pseudo_dev, 1);

err_alloc:
   return ret;
}
 
static void __exit pseudo_exit(void)
{
   device_destroy(pseudo_class, pseudo_dev);
   class_destroy(pseudo_class);
   cdev_del(&pseudo_cdev);
   unregister_chrdev_region(pseudo_dev, 1);

   printk(KERN_INFO "Pseudo char driver unloaded\n");
}
 
module_init(pseudo_init);
module_exit(pseudo_exit);

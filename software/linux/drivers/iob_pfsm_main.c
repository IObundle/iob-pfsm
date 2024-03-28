/* iob_pfsm_main.c: driver for iob_pfsm
 * using device platform. No hardcoded hardware address:
 * 1. load driver: insmod iob_pfsm.ko
 * 2. run user app: ./user/user
 */

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>

#include "iob_class/iob_class_utils.h"
#include "iob_pfsm.h"

#define NUM_DEVICES 2

static int iob_pfsm_probe(struct platform_device *);
static int iob_pfsm_remove(struct platform_device *);

static ssize_t iob_pfsm_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t iob_pfsm_write(struct file *, const char __user *, size_t,
                              loff_t *);
static loff_t iob_pfsm_llseek(struct file *, loff_t, int);
static int iob_pfsm_open(struct inode *, struct file *);
static int iob_pfsm_release(struct inode *, struct file *);

DEFINE_MUTEX(iob_pfsm_mutex);
struct pfsm_driver {
  dev_t devnum;
  struct class *class;
  struct list_head list;
};

static struct pfsm_driver pfsm_driver = {
    .devnum = 0,
    .class = NULL,
    .list = LIST_HEAD_INIT(pfsm_driver.list),
};

#include "iob_pfsm_sysfs_multi.h"

static const struct file_operations iob_pfsm_fops = {
    .owner = THIS_MODULE,
    .write = iob_pfsm_write,
    .read = iob_pfsm_read,
    .llseek = iob_pfsm_llseek,
    .open = iob_pfsm_open,
    .release = iob_pfsm_release,
};

static const struct of_device_id of_iob_pfsm_match[] = {
    {.compatible = "iobundle,pfsm0"},
    {},
};

static struct platform_driver iob_pfsm_driver = {
    .driver =
        {
            .name = "iob_pfsm",
            .owner = THIS_MODULE,
            .of_match_table = of_iob_pfsm_match,
        },
    .probe = iob_pfsm_probe,
    .remove = iob_pfsm_remove,
};

//
// Module init and exit functions
//
static int iob_pfsm_probe(struct platform_device *pdev) {
  struct resource *res;
  int result = 0;
  struct iob_data *iob_pfsm_data = NULL;

  mutex_lock(&iob_pfsm_mutex);
  if (MINOR(pfsm_driver.devnum) >= NUM_DEVICES) {
    pr_err("[Driver] %s: No more devices allowed!\n", IOB_PFSM_DRIVER_NAME);
    return -ENODEV;
  }
  mutex_unlock(&iob_pfsm_mutex);

  pr_info("[Driver] %s: probing.\n", IOB_PFSM_DRIVER_NAME);

  iob_pfsm_data = (struct iob_data *)devm_kzalloc(
      &pdev->dev, sizeof(struct iob_data), GFP_KERNEL);
  if (iob_pfsm_data == NULL) {
    pr_err("[Driver]: Failed to allocate memory for iob_pfsm_data\n");
    return -ENOMEM;
  }

  // add device to list
  mutex_lock(&iob_pfsm_mutex);
  list_add_tail(&iob_pfsm_data->list, &pfsm_driver.list);
  mutex_unlock(&iob_pfsm_mutex);

  // Get the I/O region base address
  res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  if (!res) {
    pr_err("[Driver]: Failed to get I/O resource!\n");
    result = -ENODEV;
    goto r_get_resource;
  }

  // Request and map the I/O region
  iob_pfsm_data->regbase = devm_ioremap_resource(&pdev->dev, res);
  if (IS_ERR(iob_pfsm_data->regbase)) {
    result = PTR_ERR(iob_pfsm_data->regbase);
    goto r_ioremmap;
  }
  iob_pfsm_data->regsize = resource_size(res);

  cdev_init(&iob_pfsm_data->cdev, &iob_pfsm_fops);
  iob_pfsm_data->cdev.owner = THIS_MODULE;
  iob_pfsm_data->class = NULL;

  mutex_lock(&iob_pfsm_mutex);
  iob_pfsm_data->devnum = pfsm_driver.devnum;
  pfsm_driver.devnum =
      MKDEV(MAJOR(pfsm_driver.devnum), MINOR(pfsm_driver.devnum) + 1);
  mutex_unlock(&iob_pfsm_mutex);

  result = cdev_add(&iob_pfsm_data->cdev, iob_pfsm_data->devnum, 1);
  if (result) {
    pr_err("[Driver] %s: Char device registration failed!\n",
           IOB_PFSM_DRIVER_NAME);
    goto r_cdev_add;
  }

  iob_pfsm_data->device = device_create(
      pfsm_driver.class, NULL, iob_pfsm_data->devnum, iob_pfsm_data, "%s%d",
      IOB_PFSM_DRIVER_NAME, MINOR(iob_pfsm_data->devnum));
  if (iob_pfsm_data->device == NULL) {
    pr_err("[Driver] %s: Can not create device file!\n", IOB_PFSM_DRIVER_NAME);
    goto r_device;
  }

  // Associate iob_data to device
  pdev->dev.platform_data = iob_pfsm_data;              // pdev functions
  iob_pfsm_data->device->platform_data = iob_pfsm_data; // sysfs functions

  result = iob_pfsm_create_device_attr_files(iob_pfsm_data->device);
  if (result) {
    pr_err("Cannot create device attribute file......\n");
    goto r_dev_file;
  }

  dev_info(&pdev->dev, "initialized with %d\n", MINOR(iob_pfsm_data->devnum));
  goto r_ok;

r_dev_file:
  device_destroy(iob_pfsm_data->class, iob_pfsm_data->devnum);
  cdev_del(&(iob_pfsm_data->cdev));
r_device:
r_cdev_add:
  // iounmap is managed by devm
r_ioremmap:
r_get_resource:
r_ok:

  return result;
}

static int iob_pfsm_remove(struct platform_device *pdev) {
  struct iob_data *iob_pfsm_data = (struct iob_data *)pdev->dev.platform_data;
  iob_pfsm_remove_device_attr_files(iob_pfsm_data);
  cdev_del(&(iob_pfsm_data->cdev));

  // remove from list
  mutex_lock(&iob_pfsm_mutex);
  list_del(&iob_pfsm_data->list);
  mutex_unlock(&iob_pfsm_mutex);

  // Note: no need for iounmap, since we are using devm_ioremap_resource()
  dev_info(&pdev->dev, "remove.\n");

  return 0;
}

static int __init iob_pfsm_init(void) {
  int ret = 0;
  pr_info("[Driver] %s: initializing.\n", IOB_PFSM_DRIVER_NAME);

  // Allocate char device
  ret = alloc_chrdev_region(&pfsm_driver.devnum, 0, NUM_DEVICES,
                            IOB_PFSM_DRIVER_NAME);
  if (ret) {
    pr_err("[Driver] %s: Failed to allocate char device region\n",
           IOB_PFSM_DRIVER_NAME);
    goto r_exit;
  }

  // Create device class // todo: make a dummy driver just to create and own the
  // class: https://stackoverflow.com/a/16365027/8228163
  if ((pfsm_driver.class =
           class_create(THIS_MODULE, IOB_PFSM_DRIVER_CLASS)) == NULL) {
    printk("Device class can not be created!\n");
    goto r_alloc_region;
  }

  ret = platform_driver_register(&iob_pfsm_driver);
  if (ret < 0) {
    pr_err("[Driver] %s: Failed to register platform driver\n",
           IOB_PFSM_DRIVER_NAME);
    goto r_class;
  }

r_class:
  class_destroy(pfsm_driver.class);
r_alloc_region:
  unregister_chrdev_region(pfsm_driver.devnum, NUM_DEVICES);
r_exit:
  return ret;
}

static void __exit iob_pfsm_exit(void) {
  pr_info("[Driver] %s: exiting.\n", IOB_PFSM_DRIVER_NAME);

  mutex_lock(&iob_pfsm_mutex);
  pfsm_driver.devnum = MKDEV(MAJOR(pfsm_driver.devnum), 0);
  mutex_unlock(&iob_pfsm_mutex);

  platform_driver_unregister(&iob_pfsm_driver);

  unregister_chrdev_region(pfsm_driver.devnum, NUM_DEVICES);
}

//
// File operations
//

static int iob_pfsm_open(struct inode *inode, struct file *file) {
  struct iob_data *iob_pfsm_data = NULL;

  pr_info("[Driver] iob_pfsm device opened\n");

  if (!mutex_trylock(&iob_pfsm_mutex)) {
    pr_info("Another process is accessing the device\n");

    return -EBUSY;
  }

  // assign iob_pfsm_data to file private_data
  list_for_each_entry(iob_pfsm_data, &pfsm_driver.list, list) {
    if (iob_pfsm_data->devnum == inode->i_rdev) {
      file->private_data = iob_pfsm_data;
      return 0;
    }
  }

  return 0;
}

static int iob_pfsm_release(struct inode *inode, struct file *file) {
  pr_info("[Driver] iob_pfsm device closed\n");

  mutex_unlock(&iob_pfsm_mutex);

  return 0;
}

static ssize_t iob_pfsm_read(struct file *file, char __user *buf, size_t count,
                             loff_t *ppos) {
  int size = 0;
  u32 value = 0;
  struct iob_data *iob_pfsm_data = (struct iob_data *)file->private_data;

  /* read value from register */
  switch (*ppos) {
  case IOB_PFSM_CURRENT_STATE_ADDR:
    value =
        iob_data_read_reg(iob_pfsm_data->regbase, IOB_PFSM_CURRENT_STATE_ADDR,
                          IOB_PFSM_CURRENT_STATE_W);
    size = (IOB_PFSM_CURRENT_STATE_W >> 3); // bit to bytes
    pr_info("[Driver] Read CURRENT_STATE!\n");
    break;
  case IOB_PFSM_VERSION_ADDR:
    value = iob_data_read_reg(iob_pfsm_data->regbase, IOB_PFSM_VERSION_ADDR,
                              IOB_PFSM_VERSION_W);
    size = (IOB_PFSM_VERSION_W >> 3); // bit to bytes
    pr_info("[Driver] Read version!\n");
    break;
  default:
    // invalid address - no bytes read
    return 0;
  }

  // Read min between count and REG_SIZE
  if (size > count)
    size = count;

  if (copy_to_user(buf, &value, size))
    return -EFAULT;

  return count;
}

static ssize_t iob_pfsm_write(struct file *file, const char __user *buf,
                              size_t count, loff_t *ppos) {
  int size = 0;
  u32 value = 0;
  struct iob_data *iob_pfsm_data = (struct iob_data *)file->private_data;

  switch (*ppos) {
  case IOB_PFSM_MEM_WORD_SELECT_ADDR:
    size = (IOB_PFSM_MEM_WORD_SELECT_W >> 3); // bit to bytes
    if (read_user_data(buf, size, &value))
      return -EFAULT;
    iob_data_write_reg(iob_pfsm_data->regbase, value,
                       IOB_PFSM_MEM_WORD_SELECT_ADDR,
                       IOB_PFSM_MEM_WORD_SELECT_W);
    pr_info("[Driver] MEM_WORD_SELECT iob_pfsm: 0x%x\n", value);
    break;
  case IOB_PFSM_SOFTRESET_ADDR:
    size = (IOB_PFSM_SOFTRESET_W >> 3); // bit to bytes
    if (read_user_data(buf, size, &value))
      return -EFAULT;
    iob_data_write_reg(iob_pfsm_data->regbase, value, IOB_PFSM_SOFTRESET_ADDR,
                       IOB_PFSM_SOFTRESET_W);
    pr_info("[Driver] SOFTRESET iob_pfsm: 0x%x\n", value);
    break;
  default:
    // MEMORY address range
    if (((*ppos) >= IOB_PFSM_MEMORY_ADDR) &&
        ((*ppos) < IOB_PFSM_MEM_WORD_SELECT_ADDR)) {
      size = (IOB_PFSM_MEMORY_W >> 3); // bit to bytes
      if (read_user_data(buf, size, &value))
        return -EFAULT;
      iob_data_write_reg(iob_pfsm_data->regbase, value, (*ppos),
                         IOB_PFSM_MEMORY_W);
      pr_info("[Driver] MEMORY[%x] iob_pfsm: 0x%x\n", value,
              (unsigned int)((*ppos) - IOB_PFSM_MEMORY_ADDR));
    } else {
      pr_info("[Driver] Invalid write address 0x%x\n", (unsigned int)*ppos);
      return 0;
    }
    // invalid address - no bytes written
  }

  return count;
}

/* Custom lseek function
 * check: lseek(2) man page for whence modes
 */
static loff_t iob_pfsm_llseek(struct file *filp, loff_t offset, int whence) {
  loff_t new_pos = -1;
  struct iob_data *iob_pfsm_data = (struct iob_data *)filp->private_data;

  switch (whence) {
  case SEEK_SET:
    new_pos = offset;
    break;
  case SEEK_CUR:
    new_pos = filp->f_pos + offset;
    break;
  case SEEK_END:
    new_pos = (1 << IOB_PFSM_SWREG_ADDR_W) + offset;
    break;
  default:
    return -EINVAL;
  }

  // Check for valid bounds
  if (new_pos < 0 || new_pos > iob_pfsm_data->regsize) {
    return -EINVAL;
  }

  // Update file position
  filp->f_pos = new_pos;

  return new_pos;
}

module_init(iob_pfsm_init);
module_exit(iob_pfsm_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("IObundle");
MODULE_DESCRIPTION("IOb-PFSM Drivers");
MODULE_VERSION("0.10");

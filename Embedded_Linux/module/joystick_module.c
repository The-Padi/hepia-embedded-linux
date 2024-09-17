#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/fs.h>

#define DEVICE_NAME "joystick"
#define GPIO_LEFT 16
#define GPIO_RIGHT 17
#define GPIO_UP 21
#define GPIO_DOWN 23
#define GPIO_BUTTON 47

// Joystick states
enum
{
    JOYSTICK_NONE,
    JOYSTICK_LEFT,
    JOYSTICK_RIGHT,
    JOYSTICK_UP,
    JOYSTICK_DOWN,
    JOYSTICK_BUTTON
};

static int major;
static struct class *joystick_class;
struct device *joystick_device;

static int joystick_state;

// Operation prototypes
static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);
static irqreturn_t joystick_irq_handler(int, void *);

// The driver's file operations
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
};

/**
 * Driver initialization code.
 */
static int __init joystick_dev_init(void)
{
    int result;

    // 1) Register the device by dynamically obtaining a major number
    result = register_chrdev(0, DEVICE_NAME, &fops);
    if (result < 0)
    {
        pr_err("Failed to allocate major number\n");
        return result;
    }
    major = result;

    // 2) Create the class
    joystick_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(joystick_class))
    {
        pr_err("Failed to create class\n");
        unregister_chrdev(major, DEVICE_NAME);
        return PTR_ERR(joystick_class);
    }

    // 3) Create the device in /dev
    joystick_device = device_create(joystick_class, NULL, MKDEV(major, 0), NULL, "JoyStick");
    if (IS_ERR(joystick_device))
    {
        pr_err("Failed to add cdev\n");
        class_destroy(joystick_class);
        unregister_chrdev(major, DEVICE_NAME);
        return result;
    }

    // 6) Request the necessary GPIOs
    result = gpio_request(GPIO_LEFT, "joystick_left");
    if (result < 0)
    {
        pr_err("Failed to request GPIO_LEFT\n");
        goto err_gpio;
    }

    result = gpio_request(GPIO_RIGHT, "joystick_right");
    if (result < 0)
    {
        pr_err("Failed to request GPIO_RIGHT\n");
        goto err_gpio;
    }

    result = gpio_request(GPIO_UP, "joystick_up");
    if (result < 0)
    {
        pr_err("Failed to request GPIO_UP\n");
        goto err_gpio;
    }

    result = gpio_request(GPIO_DOWN, "joystick_down");
    if (result < 0)
    {
        pr_err("Failed to request GPIO_DOWN\n");
        goto err_gpio;
    }

    result = gpio_request(GPIO_BUTTON, "joystick_button");
    if (result < 0)
    {
        pr_err("Failed to request GPIO_BUTTON\n");
        goto err_gpio;
    }

    // 7) Register an IRQ handler per GPIO
    result = request_irq(gpio_to_irq(GPIO_LEFT), joystick_irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "joystick_left", NULL);
    if (result < 0)
    {
        pr_err("Failed to request IRQ for GPIO_LEFT\n");
        goto err_irq_left;
    }

    result = request_irq(gpio_to_irq(GPIO_RIGHT), joystick_irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "joystick_right", NULL);
    if (result < 0)
    {
        pr_err("Failed to request IRQ for GPIO_RIGHT\n");
        goto err_irq_right;
    }

    result = request_irq(gpio_to_irq(GPIO_UP), joystick_irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "joystick_up", NULL);
    if (result < 0)
    {
        pr_err("Failed to request IRQ for GPIO_UP\n");
        goto err_irq_up;
    }

    result = request_irq(gpio_to_irq(GPIO_DOWN), joystick_irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "joystick_down", NULL);
    if (result < 0)
    {
        pr_err("Failed to request IRQ for GPIO_DOWN\n");
        goto err_irq_down;
    }

    result = request_irq(gpio_to_irq(GPIO_BUTTON), joystick_irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "joystick_button", NULL);
    if (result < 0)
    {
        pr_err("Failed to request IRQ for GPIO_button\n");
        goto err_irq_button;
    }

    pr_info("Joystick driver initialized\n");
    return 0;

err_irq_left:
    // Free GPIO
    gpio_free(GPIO_LEFT);
err_irq_right:
    // Free GPIO
    gpio_free(GPIO_RIGHT);
err_irq_up:
    // Free GPIO
    gpio_free(GPIO_UP);
err_irq_down:
    // Free GPIO
    gpio_free(GPIO_DOWN);
err_irq_button:
    // Free GPIO
    gpio_free(GPIO_BUTTON);
err_gpio:
    // Destroy Device
    device_destroy(joystick_class, MKDEV(major, 0));
    // Unregister class
    class_destroy(joystick_class);
    // Unregister major number
    unregister_chrdev(major, DEVICE_NAME);
    return result;
}

/**
 * This function is called when the module is unloaded.
 */
static void __exit joystick_dev_exit(void)
{
    // 1) Destroy the device
    device_destroy(joystick_class, MKDEV(major, 0));

    // 2) Destroy the class
    class_destroy(joystick_class);

    // 4) Unregister the device
    unregister_chrdev(major, DEVICE_NAME);

    // 5) Free the IRQs
    free_irq(gpio_to_irq(GPIO_LEFT), NULL);
    free_irq(gpio_to_irq(GPIO_RIGHT), NULL);
    free_irq(gpio_to_irq(GPIO_UP), NULL);
    free_irq(gpio_to_irq(GPIO_DOWN), NULL);
    free_irq(gpio_to_irq(GPIO_BUTTON), NULL);

    // 6) Free the GPIOs
    gpio_free(GPIO_LEFT);
    gpio_free(GPIO_RIGHT);
    gpio_free(GPIO_UP);
    gpio_free(GPIO_DOWN);
    gpio_free(GPIO_BUTTON);

    pr_info("Joystick driver destroyed\n");
}

/**
 * Read operation
 */
static ssize_t dev_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    size_t bytes_to_copy = min(len, sizeof(joystick_state));

    if (copy_to_user(buf, &joystick_state, bytes_to_copy))
    {
        return -EFAULT;
    }

    // Number of bytes successfully copied
    return bytes_to_copy;
}

/**
 * Joystick IRQ handler
 */
static irqreturn_t joystick_irq_handler(int irq, void *dev_id)
{
    // Determine which GPIO triggered the interrupt
    if (irq == gpio_to_irq(GPIO_LEFT))
    {
        int state = gpio_get_value(GPIO_LEFT);
        if (state == 0)
        {
            joystick_state = JOYSTICK_LEFT;
        }
        else if (state == 1)
        {
            joystick_state = JOYSTICK_NONE;
        }
    }
    else if (irq == gpio_to_irq(GPIO_RIGHT))
    {
        int state = gpio_get_value(GPIO_RIGHT);
        if (state == 0)
        {
            joystick_state = JOYSTICK_RIGHT;
        }
        else if (state == 1)
        {
            joystick_state = JOYSTICK_NONE;
        }
    }
    else if (irq == gpio_to_irq(GPIO_UP))
    {
        int state = gpio_get_value(GPIO_UP);
        if (state == 0)
        {
            joystick_state = JOYSTICK_UP;
        }
        else if (state == 1)
        {
            joystick_state = JOYSTICK_NONE;
        }
    }
    else if (irq == gpio_to_irq(GPIO_DOWN))
    {
        int state = gpio_get_value(GPIO_DOWN);
        if (state == 0)
        {
            joystick_state = JOYSTICK_DOWN;
        }
        else if (state == 1)
        {
            joystick_state = JOYSTICK_NONE;
        }
    }
    else if (irq == gpio_to_irq(GPIO_BUTTON))
    {
        int state = gpio_get_value(GPIO_BUTTON);
        if (state == 0)
        {
            joystick_state = JOYSTICK_BUTTON;
        }
        else if (state == 1)
        {
            joystick_state = JOYSTICK_NONE;
        }
    }

    // Notify user-space about the state change
    // pr_info("Joystick state changed: %d\n", joystick_state);

    // Announce that the IRQ has been handled correctly
    return (irqreturn_t)IRQ_HANDLED;
}

module_init(joystick_dev_init);
module_exit(joystick_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael Divià <michael.divia@gmail.com>");
MODULE_DESCRIPTION("Joystick driver");
MODULE_VERSION("0.1");
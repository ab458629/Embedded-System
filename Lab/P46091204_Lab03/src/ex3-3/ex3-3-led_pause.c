#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/timekeeping.h>

#define INTERVAL_MS 100
#define IRQ_NAME "button_1"	// define IRQ_NAME, type cat /proc/interrupts to check the name of IRQ

MODULE_LICENSE("GPL");
MODULE_AUTHOR("James");
MODULE_DESCRIPTION("ex3-3 : Press switch 1 to trigger interrupt to make LED1 remain lit");
MODULE_VERSION("1.0");

static int irq = 0;
static int timeout_ms = INTERVAL_MS;
struct timer_list timer;
int exiting = 0;
static ktime_t last_time;
static char is_press=0;

module_param(irq, int, 0644);
int dev_id = 127;  //device id

static irqreturn_t myinterrupt(int irq, void *dev_id)

{ 
  ktime_t this_time=ktime_get();  //get current time
  if(this_time - last_time > 100000000)
  {
  	is_press ^=0x01;  //1 when button press, 0 when button release	
        if(is_press)
	{
	  del_timer(&timer); // stop the execution of timer
	  gpio_direction_output(24, 1);
	}
	else // when button release, we can re-activate timer not activated
	{
	  mod_timer(&timer, jiffies + msecs_to_jiffies(timeout_ms));
	}
 
  }
 
  last_time= this_time;  //update the last trigger time of ISR
  return (irqreturn_t) IRQ_HANDLED;
}

static void timer_callback(struct timer_list *arg) {
  gpio_direction_output(24, !__gpio_get_value(24));  //led gpio number 
  mod_timer(&timer, jiffies + msecs_to_jiffies(timeout_ms));  //control led on/off frequency  timeout_ms
}

int __init init_module(void) {
  printk("int: init\n");
  printk("This code is from TA");
  gpio_free(24);  //in case LED_1 pin is occupy fill gpio number
  gpio_free(25);  //in case BUTTON_1 pin is occupy  fill gpio number
  gpio_request_one(24, GPIOF_INIT_LOW, "gpio_24");
  gpio_request_one(25, GPIOF_IN, "gpio_25");
  irq = gpio_to_irq(25);
  last_time=ktime_get();  //get current time
  timer_setup(&timer, timer_callback, 0);  //prepare a timer for first use
  mod_timer(&timer, jiffies + msecs_to_jiffies(timeout_ms));
  request_irq(irq, &myinterrupt, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "RTES", &dev_id);
  return 0;
}

void __exit cleanup_module(void) {
  printk("int: exiting\n");
  printk("int: free_irq\n");
  free_irq(irq, &dev_id);
  printk("int: del_timer\n");
  del_timer(&timer);
  printk("int: exited\n");
  gpio_free(24);  //in case LED_1 pin is occupy fill gpio number
  gpio_free(25);  //in case BUTTON_1 pin is occupy  fill gpio number
}

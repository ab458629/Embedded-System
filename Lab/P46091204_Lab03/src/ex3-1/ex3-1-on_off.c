#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <asm/io.h>

#include <linux/timekeeping.h>
#include <linux/gpio.h>

#define BUTTON_1 25 // switch
#define LED_1 24    // LED1

#define IRQ_NAME "button_1" // define IRQ_NAME, we can type cat /proc/interrupts to see name of IRQ

MODULE_LICENSE("GPL");
MODULE_AUTHOR("RTES LAB");
MODULE_DESCRIPTION("irq demo for RTES lab");
MODULE_VERSION("0.3");

static short int button_irq_id = 0;
static ktime_t last_time;
static char is_on = 0;
static char is_press = 0;

irq_handler_t isr (int irq, void *data){
    ktime_t this_time = ktime_get();          //get current time
    if(this_time - last_time > 100000000 ){  //debounce, minimum interval time = 0.1s
        is_press ^= 0x01;     //1 when button press, 0 when button relese
        if(is_press){        //if button press, change the state of LED_1
            gpio_direction_output(LED_1, is_on);
            is_on ^= 0x01;  //1 for LED_1 on, 0 for LED_1 off
        }
    }
    last_time = this_time;  //update the last trigger time of ISR
    return (irq_handler_t) IRQ_HANDLED; //tell system that this ISR is handled
}

int init_module (){
    gpio_free(BUTTON_1);  //in case BUTTON_1 pin is occupy
    gpio_free(LED_1);  //in case LED_1 pin is occupy
    if(gpio_request(BUTTON_1,"BUTTON_1") != 0){ //sign up for using BUTTON_1
        return -1;  //sign up fail, exit with -1
    }
    if(gpio_request(LED_1,"LED") != 0){  //sign up for using LED_1
        return -1;                     //sign up fail, exit with -1
    }
    
    gpio_direction_output(LED_1, is_on);  //tell system we will use LED_1 as output port
    is_on ^= 0x01;  //initialize
    
    last_time = ktime_get();  //get current time
    if( (button_irq_id = gpio_to_irq(BUTTON_1)) < 0 ){  //translate gpio pin id to irq id
        return -1;
    }
    
    printk (KERN_ALERT "\nbutton_isr loaded !\n");  //debug information, check with dmesg
    //go google dmesg if you don't know what it is
    
    request_irq(button_irq_id, (irq_handler_t) isr, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, IRQ_NAME, NULL);
    //sign up IRQ
    //let's break this up
    //button_irq_id : irq id of BUTTON_1, got this with gpio_to_irq()
    //isr           : the name of your ISR, in this case, the function name is "isr"
    //IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING  : bit mask(google it if you don't know what is bit mask),
    //                                            we are using "|" to tell system to trigger isr 
    //                                            when rising edge AND falling edge event happen
    //IRQ_NAME      : the name of this IRQ, you could check this in "/proc/interrupts"
    //NULL          : the address of varible passed to handler function
    return 0;
}

void cleanup_module(void){    //triggered when rmmod use
    free_irq(button_irq_id, NULL);  //free IRQ
    gpio_free(BUTTON_1);    //free BUTTON_1 gpio pin
    gpio_free(LED_1);      //free LED_1 gpio pin
    printk (KERN_ALERT "\n1 button_isr unloaded !\n");  //debug information, check with dmesg
}

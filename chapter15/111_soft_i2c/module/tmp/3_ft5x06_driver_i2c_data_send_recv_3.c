#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/jiffies.h>


#define I2C_SCL 11
#define I2C_SDA 12

struct gpio_desc *i2c_scl_desc;
struct gpio_desc *i2c_sda_desc;

void i2c_start(void){
	gpiod_direction_output(i2c_scl_desc,1);
	gpiod_direction_output(i2c_sda_desc,1);
	mdelay(1);

	gpiod_direction_output(i2c_sda_desc,0);
	mdelay(1);

	gpiod_direction_output(i2c_scl_desc,0);
	mdelay(1);
}

void i2c_stop(void){
	gpiod_direction_output(i2c_scl_desc,0);
	gpiod_direction_output(i2c_sda_desc,0);
	mdelay(1);

	gpiod_direction_output(i2c_sda_desc,1);
	mdelay(1);

	gpiod_direction_output(i2c_scl_desc,1);
	mdelay(1);
}

void i2c_send_ack(int ack){
	gpiod_direction_output(i2c_sda_desc,0);

	if(ack){
		gpiod_direction_output(i2c_sda_desc,0);
	}else{
		gpiod_direction_output(i2c_sda_desc,1);
	}

	gpiod_direction_output(i2c_scl_desc,1);
	mdelay(1);
	gpiod_direction_output(i2c_scl_desc,0);

}

int i2c_recv_ack(void){
	int value=0;

	gpiod_direction_input(i2c_sda_desc);

	gpiod_direction_output(i2c_scl_desc,1);

	mdelay(1);
	
	if(gpiod_get_value(i2c_sda_desc)){
		value=1;
	}else{
		value=0;
	}

	gpiod_direction_output(i2c_scl_desc,0);

	gpiod_direction_output(i2c_sda_desc,1);

	return value;

}

void i2c_send_data(int data){
	int i;
	int value;

	gpiod_direction_output(i2c_scl_desc,0);

	for(i=0;i<8;i++){
		value=(data<<i)&0x80;
		
		if(value){
			gpiod_direction_output(i2c_sda_desc,1);
		}else{
			gpiod_direction_output(i2c_sda_desc,0);
		
		}
	
		gpiod_direction_output(i2c_scl_desc,1);
		mdelay(1);
		gpiod_direction_output(i2c_scl_desc,0);
		mdelay(1);
	}

}


int i2c_recv_data(void){
	int i;
	int temp=0;
	int data=0;

	gpiod_direction_input(i2c_sda_desc);
	mdelay(1);

	for(i=0;i<8;i++){
		gpiod_direction_output(i2c_scl_desc,0);
		mdelay(1);

		gpiod_direction_output(i2c_scl_desc,0);
		mdelay(1);


		data=gpiod_get_value(i2c_sda_desc);

		if(data){
			temp=(temp<<1)|data;
		}else{
			temp=(temp<<1)&~data;
		}
	}


	gpiod_direction_output(i2c_scl_desc,0);

	mdelay(1);

	gpiod_direction_output(i2c_sda_desc,1);

	return temp;

}

static int ft5x06_driver_init(void)
{
	i2c_scl_desc=gpio_to_desc(I2C_SCL);
	if(i2c_scl_desc==NULL){
		printk("gpio_to_desc_error for SCL pin\n");
		return -1;
	
	}

	i2c_sda_desc=gpio_to_desc(I2C_SDA);
	if(i2c_sda_desc==NULL){
		printk("gpio_to_desc_error for SDA pin\n");
		return -1;
	
	}


	gpiod_direction_output(i2c_scl_desc,1);
	gpiod_direction_output(i2c_sda_desc,1);


	return 0;
}


static void ft5x06_driver_exit(void)
{
	gpiod_put(i2c_scl_desc);
	gpiod_put(i2c_sda_desc);

}

module_init(ft5x06_driver_init);
module_exit(ft5x06_driver_exit);

MODULE_LICENSE("GPL");


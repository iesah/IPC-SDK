#include <asm/delay.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/byd_8991.h>

//#define DEBUG

struct platform_byd_8991_data *byd_8991_pdata = NULL;

#ifdef DEBUG
/*PE1  LCD_BACKLIGNT   */
#define PWM_OUT             (32*4 + 1)
#define back_light(n)\
     gpio_direction_output(PWM_OUT, n)

/*PC20 BACK_LIGHT_SELECT */
#define BACK_LIGHT_SEL(n)\
	 gpio_direction_output(byd_8991_pdata->gpio_lcd_back_sel, n)
#else
#define BACK_LIGHT_SEL(n) do{}while(0)
#endif

/*PB30  LCD_RESET   */
#define RESET(n)\
     gpio_direction_output(byd_8991_pdata->gpio_lcd_disp, n)

/*PC0  SPI_CS   */
#define CS(n)\
     gpio_direction_output(byd_8991_pdata->gpio_lcd_cs, n)

/*PC1  SPI_CLK   */
#define SCK(n)\
     gpio_direction_output(byd_8991_pdata->gpio_lcd_clk, n)

/*PC10 SPI_MOSI */
#define SDO(n)\
	 gpio_direction_output(byd_8991_pdata->gpio_lcd_sdo, n)

/*PC11 SPI_MISO */
#define SDI()\
	gpio_get_value(byd_8991_pdata->gpio_lcd_sdi)



void SPI_3W_SET_CMD(unsigned char c)
{
	unsigned char i;

	udelay(10);
	SCK(0);
	SDO(0);
	udelay(10);
	SCK(1);
	udelay(20);
	for(i=0;i<8;i++)
	{
		SCK(0);
		SDO(((c&0x80)>>7));
		udelay(10);
		SCK(1);
		udelay(20);
		c=c<<1;
	}
}

void SPI_3W_SET_PAs(unsigned char d)
{
	unsigned char i;

	udelay(10);
	SCK(0);
	SDO(1);
	udelay(10);
	SCK(1);
	udelay(20);
	for(i=0;i<8;i++)
	{
		SCK(0);
		SDO(((d&0x80)>>7));
		udelay(10);
		SCK(1);
		udelay(20);
		d=d<<1;
	}
}
unsigned char SPI_GET_REG_VAL(void)
{
	unsigned char i;
	unsigned char data = 0;

	udelay(10);
	for(i=0;i<8;i++)
	{
		SCK(0);
		data <<= 1;
		data |=SDI();
		udelay(10);
		SCK(1);
		udelay(20);
		printk("sdi= 0x%x\n",data);
	}
	return data;
}

void SPI_READ_REG(unsigned char reg)
{
	int data = 0;

	CS(0);
	udelay(10);
	SPI_3W_SET_CMD(0xB9); //Set_EXTC
	SPI_3W_SET_PAs(0xFF);
	SPI_3W_SET_PAs(0x83);
	SPI_3W_SET_PAs(0x69);

	SPI_3W_SET_CMD(reg);
	data = SPI_GET_REG_VAL();
	CS(1);
	udelay(10);
	printk("reg(0x%x)=0x%x\n",reg,data);
}

void SET_BRIGHT_CTRL(void)
{
	printk("This is the screen brightness ctrl!\n");
	CS(0);
	udelay(10);

	SPI_3W_SET_CMD(0xB9); //Set_EXTC
	SPI_3W_SET_PAs(0xFF);
	SPI_3W_SET_PAs(0x83);
	SPI_3W_SET_PAs(0x69);

	SPI_3W_SET_CMD(0x53); //SET DISP CTRL
	SPI_3W_SET_PAs(0x2c);

	SPI_3W_SET_CMD(0x51); //SET BRIGHTNESS
	SPI_3W_SET_PAs(0xf0);

	//SPI_3W_SET_CMD(0x55); //SET CABC CMD
	//SPI_3W_SET_PAs(0x01);

	//SPI_3W_SET_CMD(0x5e); //SET MIN BRIGHTNESS
	//SPI_3W_SET_PAs(0xf);

	CS(1);
	udelay(10);
}
void Initial_IC(struct platform_byd_8991_data *pdata)
{
	byd_8991_pdata = pdata;
#ifdef DEBUG
	back_light(1);
#endif

	RESET(1);
	udelay(1000); // Delay 1ms
	RESET(0);
	udelay(10000); // Delay 10ms // This delay time is necessary
	RESET(1);
	udelay(100000); // Delay 100 ms

#ifdef DEBUG
	while(1){
/*The default reg(0x08)'s value is 0x08, if the	IC is working*/
		SPI_READ_REG(0X0A);
	}
#endif

	CS(0);
	udelay(10);

	SPI_3W_SET_CMD(0xB9); //Set_EXTC
	SPI_3W_SET_PAs(0xFF);
	SPI_3W_SET_PAs(0x83);
	SPI_3W_SET_PAs(0x69);

	SPI_3W_SET_CMD(0xB1); //Set Power
	SPI_3W_SET_PAs(0x85);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x34);
	SPI_3W_SET_PAs(0x0A);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x0F);
	SPI_3W_SET_PAs(0x0F);
	SPI_3W_SET_PAs(0x2A);
	SPI_3W_SET_PAs(0x32);
	SPI_3W_SET_PAs(0x3F);
	SPI_3W_SET_PAs(0x3F);
	SPI_3W_SET_PAs(0x01);
	SPI_3W_SET_PAs(0x23);
	SPI_3W_SET_PAs(0x01);
	SPI_3W_SET_PAs(0xE6);
	SPI_3W_SET_PAs(0xE6);
	SPI_3W_SET_PAs(0xE6);
	SPI_3W_SET_PAs(0xE6);
	SPI_3W_SET_PAs(0xE6);

	SPI_3W_SET_CMD(0xB2); // SET Display 480x800
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x23);
	SPI_3W_SET_PAs(0x05);
	SPI_3W_SET_PAs(0x05);
	SPI_3W_SET_PAs(0x70);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0xFF);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x03);
	SPI_3W_SET_PAs(0x03);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x01);

	SPI_3W_SET_CMD(0xB4); // SET Display column inversion
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x18);
	SPI_3W_SET_PAs(0x80);
	SPI_3W_SET_PAs(0x06);
	SPI_3W_SET_PAs(0x02);

	SPI_3W_SET_CMD(0xB6); // SET VCOM
	SPI_3W_SET_PAs(0x3A);
	SPI_3W_SET_PAs(0x3A);

	SPI_3W_SET_CMD(0xD5); // SETGIP
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x03);
	SPI_3W_SET_PAs(0x03);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x01);
	SPI_3W_SET_PAs(0x04);
	SPI_3W_SET_PAs(0x28);
	SPI_3W_SET_PAs(0x70);
	SPI_3W_SET_PAs(0x11);
	SPI_3W_SET_PAs(0x13);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x40);
	SPI_3W_SET_PAs(0x06);
	SPI_3W_SET_PAs(0x51);
	SPI_3W_SET_PAs(0x07);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x41);
	SPI_3W_SET_PAs(0x06);
	SPI_3W_SET_PAs(0x50);
	SPI_3W_SET_PAs(0x07);
	SPI_3W_SET_PAs(0x07);
	SPI_3W_SET_PAs(0x0F);
	SPI_3W_SET_PAs(0x04);
	SPI_3W_SET_PAs(0x00);

	SPI_3W_SET_CMD(0xE0); // Set Gamma
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x13);
	SPI_3W_SET_PAs(0x19);
	SPI_3W_SET_PAs(0x38);
	SPI_3W_SET_PAs(0x3D);
	SPI_3W_SET_PAs(0x3F);
	SPI_3W_SET_PAs(0x28);
	SPI_3W_SET_PAs(0x46);
	SPI_3W_SET_PAs(0x07);
	SPI_3W_SET_PAs(0x0D);
	SPI_3W_SET_PAs(0x0E);
	SPI_3W_SET_PAs(0x12);
	SPI_3W_SET_PAs(0x15);
	SPI_3W_SET_PAs(0x12);
	SPI_3W_SET_PAs(0x14);
	SPI_3W_SET_PAs(0x0F);
	SPI_3W_SET_PAs(0x17);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x13);
	SPI_3W_SET_PAs(0x19);
	SPI_3W_SET_PAs(0x38);
	SPI_3W_SET_PAs(0x3D);
	SPI_3W_SET_PAs(0x3F);
	SPI_3W_SET_PAs(0x28);
	SPI_3W_SET_PAs(0x46);
	SPI_3W_SET_PAs(0x07);
	SPI_3W_SET_PAs(0x0D);
	SPI_3W_SET_PAs(0x0E);
	SPI_3W_SET_PAs(0x12);
	SPI_3W_SET_PAs(0x15);
	SPI_3W_SET_PAs(0x12);
	SPI_3W_SET_PAs(0x14);
	SPI_3W_SET_PAs(0x0F);
	SPI_3W_SET_PAs(0x17);

	SPI_3W_SET_CMD(0xC1); //Set DGC function
	SPI_3W_SET_PAs(0x01);
	//R
	SPI_3W_SET_PAs(0x04);
	SPI_3W_SET_PAs(0x13);
	SPI_3W_SET_PAs(0x1A);
	SPI_3W_SET_PAs(0x20);
	SPI_3W_SET_PAs(0x27);
	SPI_3W_SET_PAs(0x2C);
	SPI_3W_SET_PAs(0x32);
	SPI_3W_SET_PAs(0x36);
	SPI_3W_SET_PAs(0x3F);
	SPI_3W_SET_PAs(0x47);
	SPI_3W_SET_PAs(0x50);
	SPI_3W_SET_PAs(0x59);
	SPI_3W_SET_PAs(0x60);
	SPI_3W_SET_PAs(0x68);
	SPI_3W_SET_PAs(0x71);
	SPI_3W_SET_PAs(0x7B);
	SPI_3W_SET_PAs(0x82);
	SPI_3W_SET_PAs(0x89);
	SPI_3W_SET_PAs(0x91);
	SPI_3W_SET_PAs(0x98);
	SPI_3W_SET_PAs(0xA0);
	SPI_3W_SET_PAs(0xA8);
	SPI_3W_SET_PAs(0xB0);
	SPI_3W_SET_PAs(0xB8);
	SPI_3W_SET_PAs(0xC1);
	SPI_3W_SET_PAs(0xC9);
	SPI_3W_SET_PAs(0xD0);
	SPI_3W_SET_PAs(0xD7);
	SPI_3W_SET_PAs(0xE0);
	SPI_3W_SET_PAs(0xE7);
	SPI_3W_SET_PAs(0xEF);
	SPI_3W_SET_PAs(0xF7);
	SPI_3W_SET_PAs(0xFE);
	SPI_3W_SET_PAs(0xCF);
	SPI_3W_SET_PAs(0x52);
	SPI_3W_SET_PAs(0x34);
	SPI_3W_SET_PAs(0xF8);
	SPI_3W_SET_PAs(0x51);
	SPI_3W_SET_PAs(0xF5);
	SPI_3W_SET_PAs(0x9D);
	SPI_3W_SET_PAs(0x75);
	SPI_3W_SET_PAs(0x00);
	//G
	SPI_3W_SET_PAs(0x04);
	SPI_3W_SET_PAs(0x13);
	SPI_3W_SET_PAs(0x1A);
	SPI_3W_SET_PAs(0x20);
	SPI_3W_SET_PAs(0x27);
	SPI_3W_SET_PAs(0x2C);
	SPI_3W_SET_PAs(0x32);
	SPI_3W_SET_PAs(0x36);
	SPI_3W_SET_PAs(0x3F);
	SPI_3W_SET_PAs(0x47);
	SPI_3W_SET_PAs(0x50);
	SPI_3W_SET_PAs(0x59);
	SPI_3W_SET_PAs(0x60);
	SPI_3W_SET_PAs(0x68);
	SPI_3W_SET_PAs(0x71);
	SPI_3W_SET_PAs(0x7B);
	SPI_3W_SET_PAs(0x82);
	SPI_3W_SET_PAs(0x89);
	SPI_3W_SET_PAs(0x91);
	SPI_3W_SET_PAs(0x98);
	SPI_3W_SET_PAs(0xA0);
	SPI_3W_SET_PAs(0xA8);
	SPI_3W_SET_PAs(0xB0);
	SPI_3W_SET_PAs(0xB8);
	SPI_3W_SET_PAs(0xC1);
	SPI_3W_SET_PAs(0xC9);
	SPI_3W_SET_PAs(0xD0);
	SPI_3W_SET_PAs(0xD7);
	SPI_3W_SET_PAs(0xE0);
	SPI_3W_SET_PAs(0xE7);
	SPI_3W_SET_PAs(0xEF);
	SPI_3W_SET_PAs(0xF7);
	SPI_3W_SET_PAs(0xFE);
	SPI_3W_SET_PAs(0xCF);
	SPI_3W_SET_PAs(0x52);
	SPI_3W_SET_PAs(0x34);
	SPI_3W_SET_PAs(0xF8);
	SPI_3W_SET_PAs(0x51);
	SPI_3W_SET_PAs(0xF5);
	SPI_3W_SET_PAs(0x9D);
	SPI_3W_SET_PAs(0x75);
	SPI_3W_SET_PAs(0x00);
	//B
	SPI_3W_SET_PAs(0x04);
	SPI_3W_SET_PAs(0x13);
	SPI_3W_SET_PAs(0x1A);
	SPI_3W_SET_PAs(0x20);
	SPI_3W_SET_PAs(0x27);
	SPI_3W_SET_PAs(0x2C);
	SPI_3W_SET_PAs(0x32);
	SPI_3W_SET_PAs(0x36);
	SPI_3W_SET_PAs(0x3F);
	SPI_3W_SET_PAs(0x47);
	SPI_3W_SET_PAs(0x50);
	SPI_3W_SET_PAs(0x59);
	SPI_3W_SET_PAs(0x60);
	SPI_3W_SET_PAs(0x68);
	SPI_3W_SET_PAs(0x71);
	SPI_3W_SET_PAs(0x7B);
	SPI_3W_SET_PAs(0x82);
	SPI_3W_SET_PAs(0x89);
	SPI_3W_SET_PAs(0x91);
	SPI_3W_SET_PAs(0x98);
	SPI_3W_SET_PAs(0xA0);
	SPI_3W_SET_PAs(0xA8);
	SPI_3W_SET_PAs(0xB0);
	SPI_3W_SET_PAs(0xB8);
	SPI_3W_SET_PAs(0xC1);
	SPI_3W_SET_PAs(0xC9);
	SPI_3W_SET_PAs(0xD0);
	SPI_3W_SET_PAs(0xD7);
	SPI_3W_SET_PAs(0xE0);
	SPI_3W_SET_PAs(0xE7);
	SPI_3W_SET_PAs(0xEF);
	SPI_3W_SET_PAs(0xF7);
	SPI_3W_SET_PAs(0xFE);
	SPI_3W_SET_PAs(0xCF);
	SPI_3W_SET_PAs(0x52);
	SPI_3W_SET_PAs(0x34);
	SPI_3W_SET_PAs(0xF8);
	SPI_3W_SET_PAs(0x51);
	SPI_3W_SET_PAs(0xF5);
	SPI_3W_SET_PAs(0x9D);
	SPI_3W_SET_PAs(0x75);
	SPI_3W_SET_PAs(0x00);


	SPI_3W_SET_CMD(0x3A); //Set COLMOD
	SPI_3W_SET_PAs(0x77);

	SPI_3W_SET_CMD(0x11); //Sleep Out
	udelay(150000); //at least 120ms

	SPI_3W_SET_CMD(0x29); //Display On
	CS(1);

#ifdef CONFIG_BACKLIGHT_PWM
	BACK_LIGHT_SEL(1);
#else
	BACK_LIGHT_SEL(0);
	SET_BRIGHT_CTRL();
#endif

	byd_8991_pdata = NULL;
	return;
}

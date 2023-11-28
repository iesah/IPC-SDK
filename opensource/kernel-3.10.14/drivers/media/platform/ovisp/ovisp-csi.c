#include "ovisp-isp.h"
#include "ovisp-csi.h"
#include <linux/delay.h>
#include "isp-debug.h"

void check_csi_error(void) {

	unsigned int temp1, temp2;
	while(1){
		dump_csi_reg();
		temp1 = csi_core_read(ERR1);
		temp2 = csi_core_read(ERR2);
		if(temp1 != 0)
			ISP_PRINT(ISP_INFO,"error-------- 1:0x%08x\n", temp1);
		if(temp2 != 0)
			ISP_PRINT(ISP_INFO,"error-------- 2:0x%08x\n", temp2);
	}
}

static unsigned char csi_core_write_part(unsigned int address, unsigned int data, unsigned char shift, unsigned char width)
{
        unsigned int mask = (1 << width) - 1;
        unsigned int temp = csi_core_read(address);
        temp &= ~(mask << shift);
        temp |= (data & mask) << shift;
        csi_core_write(address, temp);

	return 0;
}

static unsigned char  csi_event_disable(unsigned int  mask, unsigned char err_reg_no)
{
	switch (err_reg_no)
	{
		case 1:
			csi_core_write(MASK1, mask | csi_core_read(MASK1));
			break;
		case 2:
			csi_core_write(MASK2, mask | csi_core_read(MASK2));
			break;
		default:
			return ERR_OUT_OF_BOUND;
	}

	return 0;
}

unsigned char csi_set_on_lanes(unsigned char lanes)
{
	//ISP_PRINT(ISP_INFO,"%s:----------> lane num: %d\n", __func__, lanes);
	return csi_core_write_part(N_LANES, (lanes - 1), 0, 2);
}

static void mipi_csih_dphy_test_data_in(unsigned char test_data)
{
        csi_core_write(PHY_TST_CTRL1, test_data);
}

static void mipi_csih_dphy_test_en(unsigned char on_falling_edge)
{
        csi_core_write_part(PHY_TST_CTRL1, on_falling_edge, 16, 1);
}

static void mipi_csih_dphy_test_clock(int value)
{
        csi_core_write_part(PHY_TST_CTRL0, value, 1, 1);
}

static void mipi_csih_dphy_test_data_out(void)
{
	//ISP_PRINT(ISP_INFO,"%s --------:%08x\n", __func__, csi_core_read(PHY_TST_CTRL1));
}

static void mipi_csih_dphy_write(unsigned char address, unsigned char * data, unsigned char data_length)
{
        unsigned i = 0;
        if (data != 0)
        {
                /* set the TESTCLK input high in preparation to latch in the desired test mode */
                mipi_csih_dphy_test_clock(1);
                /* set the desired test code in the input 8-bit bus TESTDIN[7:0] */
                mipi_csih_dphy_test_data_in(address);
                /* set TESTEN input high  */
                mipi_csih_dphy_test_en(1);
		//mdelay(1);
                /* drive the TESTCLK input low; the falling edge captures the chosen test code into the transceiver */
                mipi_csih_dphy_test_clock(0);
		//mdelay(1);
                /* set TESTEN input low to disable further test mode code latching  */
                mipi_csih_dphy_test_en(0);
                /* start writing MSB first */
                for (i = data_length; i > 0; i--)
                {       /* set TESTDIN[7:0] to the desired test data appropriate to the chosen test mode */
                        mipi_csih_dphy_test_data_in(data[i - 1]);
		//	mdelay(1);
                        /* pulse TESTCLK high to capture this test data into the macrocell; repeat these two steps as necessary */
                        mipi_csih_dphy_test_clock(1);
		//	mdelay(1);
                        mipi_csih_dphy_test_clock(0);
                }
        }
	mipi_csih_dphy_test_data_out();
}

static void csi_phy_configure(void)
{
	unsigned char data[4];

	csi_core_write_part(PHY_TST_CTRL0, 1, 0, 1);
//	udelay(5);
	csi_core_write_part(PHY_TST_CTRL0, 0, 0, 1);

	mipi_csih_dphy_test_data_in(0);
	mipi_csih_dphy_test_en(0);
	mipi_csih_dphy_test_clock(0);
	data[0]=0x00;
	//data[0]=0x06; /*448Mbps*/
//data[0]=0x13;
	mipi_csih_dphy_write(0x44,data, 1);

	data[0]=0x1e;
	mipi_csih_dphy_write(0xb0,data, 1);

	data[0]=0x1;
	mipi_csih_dphy_write(0xb1,data, 1);

}
static int csi_phy_ready(unsigned int id)
{
	int ready;

	// TODO: phy0: lane0 is ready. need to be update for other lane
	ready = csi_core_read(PHY_STATE);

#if 0
	ISP_PRINT(ISP_INFO,"%s:phy state ready:0x%08x\n", __func__, ready);
#endif
	if ((ready & (1 << 10 )) && (ready & (1<<4)))
		return 1;

	return 0;
}


int csi_phy_init(void)
{
	//ISP_PRINT(ISP_INFO,"csi_phy_init being called ....\n");
	return 0;
}

int csi_phy_release(void)
{
	csi_core_write_part(CSI2_RESETN, 0, 0, 1);
	csi_core_write_part(CSI2_RESETN, 1, 0, 1);
	return 0;
}

int csi_phy_start(unsigned int id, unsigned int freq, unsigned int lans)
{
	int retries = 30;
	int i;
	//ISP_PRINT(ISP_INFO,"csi_phy_start being called\n");
	csi_set_on_lanes(lans);

	/*reset phy*/
	csi_core_write_part(PHY_SHUTDOWNZ, 0, 0, 1);
	csi_core_write_part(DPHY_RSTZ, 0, 0, 1);
	csi_core_write_part(CSI2_RESETN, 0, 0, 1);

	csi_phy_configure();

	/*active phy*/
	//udelay(10);
	csi_core_write_part(PHY_SHUTDOWNZ, 1, 0, 1);
	//udelay(10);
	csi_core_write_part(DPHY_RSTZ, 1, 0, 1);
	//udelay(10);
	csi_core_write_part(CSI2_RESETN, 1, 0, 1);

	/* MASK all interrupts */
	csi_event_disable(0xffffffff, 1);
	csi_event_disable(0xffffffff, 2);
	/* wait for phy ready */
	for (i = 0; i < retries; i++) {
		if (csi_phy_ready(id))
			break;
		udelay(2);
	}

	//ISP_PRINT(ISP_INFO,"%s:%d\n",__func__, i);
	if (i >= retries) {
		//ISP_PRINT(ISP_ERROR,"CSI PHY is not ready\n");
		return -1;
	}

	//ISP_PRINT(ISP_INFO,"csi_phy_start ok!\n");
//	mdelay(200);
	return 0;
}
int csi_phy_stop(unsigned int id)
{

	ISP_PRINT(ISP_INFO,"csi_phy_stop being called \n");
	/*reset phy*/
	csi_core_write_part(PHY_SHUTDOWNZ, 0, 0, 1);
	csi_core_write_part(DPHY_RSTZ, 0, 0, 1);
	csi_core_write_part(CSI2_RESETN, 0, 0, 1);

	return 0;
}


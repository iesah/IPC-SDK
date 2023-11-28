#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <command.h>
#include <asm/io.h>
#include <asm/cache.h>
#include <asm/arch/clk.h>
#include <asm/dma-default.h>

#include "SynopGMAC_Dev.h"
#include "tiphy_dp83867.h"

/* The amount of time between FLP bursts is 16ms +/- 8ms */
#define MAX_WAIT	40000

static synopGMACdevice	*gmacdev;
static synopGMACdevice	_gmacdev;

#define NUM_RX_DESCS	PKTBUFSRX
#define NUM_TX_DESCS	4

static DmaDesc	_tx_desc[NUM_TX_DESCS];
static DmaDesc	_rx_desc[NUM_RX_DESCS];
static DmaDesc	*tx_desc;
static DmaDesc	*rx_desc;
static int	next_tx;
static int	next_rx;

unsigned char tx_buff[NUM_TX_DESCS * 2048];

__attribute__((__unused__)) static void jzmac_dump_dma_desc2(DmaDesc *desc)
{
	printf("desc: %p, status: 0x%08x buf1: 0x%08x len: %u\n",
	       desc, desc->status, desc->buffer1, desc->length);
}
__attribute__((__unused__)) static void jzmac_dump_rx_desc(void)
{
	int i = 0;
	printf("\n===================rx====================\n");
	for (i = 0; i < NUM_RX_DESCS; i++) {
		jzmac_dump_dma_desc2(rx_desc + i);
	}
	printf("\n=========================================\n");
}
__attribute__((__unused__)) static void jzmac_dump_tx_desc(void)
{
	int i = 0;
	printf("\n===================tx====================\n");
	for (i = 0; i < NUM_TX_DESCS; i++) {
		jzmac_dump_dma_desc2(tx_desc + i);
	}
	printf("\n=========================================\n");
}
__attribute__((__unused__)) static void jzmac_dump_all_desc(void)
{
	jzmac_dump_rx_desc();
	jzmac_dump_tx_desc();
}
__attribute__((__unused__)) static void jzmac_dump_pkt_data(unsigned char *data, int len)
{
	int i = 0;
	printf("\t0x0000: ");
	for (i = 0; i < len; i++) {
		printf("%02x ", data[i]);

		if ((i % 8) == 7)
			printf(" ");

		if ( (i != 0) && ((i % 16) == 15) )
			printf("\n\t0x%04x: ", i+1);
	}
	printf("\n");
}
__attribute__((__unused__)) static void jzmac_dump_arp_reply(unsigned char *data, int len)
{
	int i = 0;

	for (i = 0; i < 6; i++) {
		if (data[i] != 0xff)
			break;
	}

	if (i == 6)  // broadcast pkt
		return;

	if ( (data[12] == 0x08) && (data[13] == 0x06) && (data[20] == 0x00)) {
		jzmac_dump_pkt_data(data, len);
	}
}
__attribute__((__unused__)) static void jzmac_dump_icmp_reply(unsigned char *data, int len)
{
	if ( (data[12] == 0x08) && (data[13] == 0x00) &&
	     (data[23] == 0x01) && (data[34] == 0x00) ) {
		jzmac_dump_pkt_data(data, len);
	}
}

//static u32 full_duplex, phy_mode;

struct jzmac_reg
{
	u32    addr;
	char   * name;
};

static struct jzmac_reg mac[] =
{
	{ 0x0000, "                  Config" },
	{ 0x0004, "            Frame Filter" },
	{ 0x0008, "             MAC HT High" },
	{ 0x000C, "              MAC HT Low" },
	{ 0x0010, "               GMII Addr" },
	{ 0x0014, "               GMII Data" },
	{ 0x0018, "            Flow Control" },
	{ 0x001C, "                VLAN Tag" },
	{ 0x0020, "            GMAC Version" },
	{ 0x0024, "            GMAC Debug  " },
	{ 0x0028, "Remote Wake-Up Frame Filter" },
	{ 0x002C, "  PMT Control and Status" },
	{ 0x0030, "  LPI Control and status" },
	{ 0x0034, "      LPI Timers Control" },
	{ 0x0038, "        Interrupt Status" },
	{ 0x003c, "        Interrupt Mask" },
	{ 0x0040, "          MAC Addr0 High" },
	{ 0x0044, "           MAC Addr0 Low" },
	{ 0x0048, "          MAC Addr1 High" },
	{ 0x004c, "           MAC Addr1 Low" },
	{ 0x0100, "           MMC Ctrl Reg " },
	{ 0x010c, "        MMC Intr Msk(rx)" },
	{ 0x0110, "        MMC Intr Msk(tx)" },
	{ 0x0200, "    MMC Intr Msk(rx ipc)" },
	{ 0x0738, "          AVMAC Ctrl Reg" },
	{ 0, 0 }
};
static struct jzmac_reg dma0[] =
{
	{ 0x0000, "[CH0] CSR0   Bus Mode" },
	{ 0x0004, "[CH0] CSR1   TxPlDmnd" },
	{ 0x0008, "[CH0] CSR2   RxPlDmnd" },
	{ 0x000C, "[CH0] CSR3    Rx Base" },
	{ 0x0010, "[CH0] CSR4    Tx Base" },
	{ 0x0014, "[CH0] CSR5     Status" },
	{ 0x0018, "[CH0] CSR6    Control" },
	{ 0x001C, "[CH0] CSR7 Int Enable" },
	{ 0x0020, "[CH0] CSR8 Missed Fr." },
	{ 0x0024, "[CH0] Recv Intr Wd.Tm." },
	{ 0x0028, "[CH0] AXI Bus Mode   " },
	{ 0x002c, "[CH0] AHB or AXI Status" },
	{ 0x0048, "[CH0] CSR18 Tx Desc  " },
	{ 0x004C, "[CH0] CSR19 Rx Desc  " },
	{ 0x0050, "[CH0] CSR20 Tx Buffer" },
	{ 0x0054, "[CH0] CSR21 Rx Buffer" },
	{ 0x0058, "CSR22 HWCFG          " },
	{ 0, 0 }
};

__attribute__((__unused__)) static void jzmac_dump_dma_regs(const char *func, int line)
{
	struct jzmac_reg *reg = dma0;

	printf("======================DMA Regs start===================\n");
	while(reg->name) {
		printf("%s:\t0x%08x\n", reg->name,
		       synopGMACReadReg((u32 *)gmacdev->DmaBase,reg->addr));
		reg++;
	}
	printf("======================DMA Regs end===================\n");
}

__attribute__((__unused__)) static void jzmac_dump_mac_regs(const char *func, int line)
{
	struct jzmac_reg *reg = mac;

	printf("======================MAC Regs start===================\n");
	while(reg->name) {
		printf("%s:\t0x%08x\n", reg->name,
		       synopGMACReadReg((u32 *)gmacdev->MacBase,reg->addr));
		reg++;
	}
	printf("======================MAC Regs end===================\n");
}

__attribute__((__unused__)) static void jzmac_dump_all_regs(const char *func, int line)
{
	jzmac_dump_dma_regs(func, line);
	jzmac_dump_mac_regs(func, line);
}


/* read cpm's mac phy control register */
static u32 read_cpm_mphyc(void)
{
	u32 data = 0;
	data = *(volatile unsigned int *)(0xB00000E8);
	return data;
}

/* write cpm's mac phy control register */
static void write_cpm_mphyc(u32 data)
{
	*(volatile unsigned int *)(0xB00000E8) = data;
}

static void jzmac_init(void) {
	synopGMAC_wd_disable(gmacdev);
	synopGMAC_jab_disable(gmacdev);
	//synopGMAC_frame_burst_enable(gmacdev);
	synopGMAC_jumbo_frame_enable(gmacdev);
	synopGMAC_rx_own_disable(gmacdev);
	synopGMAC_loopback_off(gmacdev);
	if(gmacdev->DuplexMode == FULLDUPLEX) {
		synopGMAC_set_full_duplex(gmacdev);
	} else {
		synopGMAC_set_half_duplex(gmacdev);
	}
	synopGMAC_retry_enable(gmacdev);
	synopGMAC_pad_crc_strip_disable(gmacdev);
	synopGMAC_back_off_limit(gmacdev,GmacBackoffLimit0);
	synopGMAC_deferral_check_disable(gmacdev);
#if 0
	synopGMAC_tx_enable(gmacdev);
	synopGMAC_rx_enable(gmacdev);
#endif

	if(gmacdev->Speed == SPEED10) {
		synopGMAC_select_mii(gmacdev);
		synopGMACSetBits((u32 *)gmacdev->MacBase, GmacConfig, GmacFESpeed10);
	} else if(gmacdev->Speed == SPEED100) {
		synopGMAC_select_mii(gmacdev);
		synopGMACSetBits((u32 *)gmacdev->MacBase, GmacConfig, GmacFESpeed100);
	} else if(gmacdev->Speed == SPEED1000) {
		synopGMAC_select_gmii(gmacdev);
	}

	/*Frame Filter Configuration*/
	synopGMAC_frame_filter_enable(gmacdev);
	synopGMAC_set_pass_control(gmacdev,GmacPassControl0);
	synopGMAC_broadcast_enable(gmacdev);
	//synopGMAC_src_addr_filter_disable(gmacdev);
	synopGMAC_src_addr_filter_enable(gmacdev);
	synopGMAC_multicast_disable(gmacdev);
	synopGMAC_dst_addr_filter_normal(gmacdev);
	synopGMAC_multicast_hash_filter_disable(gmacdev);
	synopGMAC_promisc_disable(gmacdev);
	synopGMAC_unicast_hash_filter_disable(gmacdev);

	/*Flow Control Configuration*/
	//synopGMAC_unicast_pause_frame_detect_disable(gmacdev);
	synopGMAC_unicast_pause_frame_detect_enable(gmacdev);
	synopGMAC_rx_flow_control_enable(gmacdev);
	//synopGMAC_rx_flow_control_disable(gmacdev);
	//synopGMAC_tx_flow_control_disable(gmacdev);
	synopGMAC_tx_flow_control_enable(gmacdev);
}
static void jz47xx_mac_configure(void)
{
	/* pbl32 incr with rxthreshold 128 and Desc is 8 Words */
	synopGMAC_dma_bus_mode_init(gmacdev,
				    DmaBurstLength32 | DmaDescriptorSkip2 |
				    DmaDescriptor8Words | DmaFixedBurstEnable |
					0x02000000);
#if (CONFIG_NET_PHY_TYPE == PHY_TYPE_DP83867)
    u32 status = synopGMACReadReg((u32 *)gmacdev->DmaBase, 0x28);
    synopGMACWriteReg((u32 *)gmacdev->DmaBase, 0x28 ,(status |1<<4));
#endif
	synopGMAC_dma_control_init(gmacdev,
				   DmaStoreAndForward | DmaTxSecondFrame |
				   DmaRxThreshCtrl128);

	/* Initialize the mac interface */
	jzmac_init();

	/* This enables the pause control in Full duplex mode of operation */
	//synopGMAC_pause_control(gmacdev);
	synopGMAC_clear_interrupt(gmacdev);

	/*
	 * Disable the interrupts generated by MMC and IPC counters.
	 * If these are not disabled ISR should be modified accordingly
	 * to handle these interrupts.
	*/
	synopGMAC_disable_mmc_tx_interrupt(gmacdev, 0xFFFFFFFF);
	synopGMAC_disable_mmc_rx_interrupt(gmacdev, 0xFFFFFFFF);
	synopGMAC_disable_mmc_ipc_rx_interrupt(gmacdev, 0xFFFFFFFF);
}

/***************************************************************************
 * ETH interface routines
 **************************************************************************/

static void jzmac_restart_tx_dma(void)
{
	u32 data;

	/* TODO: clear error status bits if any */

	data = synopGMACReadReg((u32 *)gmacdev->DmaBase, DmaControl);
	if (data & DmaTxStart) {
		synopGMAC_resume_dma_tx(gmacdev);
	} else {
		synopGMAC_enable_dma_tx(gmacdev);
	}
}

static int jz_send(struct eth_device* dev, void *packet, int length)
{
	DmaDesc *desc = tx_desc + next_tx;
	int ret = 1;
	int wait_delay = 1000;
    /*unsigned char my_packet[2048] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x11, 0x22, 0x56, 0x96, 0x69, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0x11, 0x22, 0x56, 0x96, 0x69, 0xc1, 0xa9, 0x03, 0x24, 0x00, 0x00, 0x00};*/

	if (!packet) {
		printf("jz_send: packet is NULL !\n");
		return -1;
	}

	memset(&tx_buff[next_tx * 2048], 0, 2048);
	memcpy((void *)&tx_buff[next_tx * 2048], packet, length);
	flush_dcache_all();
	flush_l2cache_all();

    //printf("jz_send\n");
	//jzmac_dump_pkt_data(&tx_buff[next_tx * 2048], length-4);
	/* prepare DMA data */
	desc->length |= (((length <<DescSize1Shift) & DescSize1Mask)
			 | ((0 <<DescSize2Shift) & DescSize2Mask));

	desc->buffer1 = virt_to_phys(&tx_buff[next_tx * 2048]);
//	desc->buffer1 = virt_to_phys(packet);
	/* ENH_DESC */
	desc->status |=  (DescTxFirst | DescTxLast | DescTxIntEnable);
	desc->status |= DescOwnByDma;

//	flush_dcache_all();

	/* start tx operation*/
	jzmac_restart_tx_dma();

	/* wait until current desc transfer done */
#if 1
	while(--wait_delay && synopGMAC_is_desc_owned_by_dma(desc)) {
		udelay(100);
	}
	/* check if there is error during transmit */
	if(wait_delay == 0)
	{
		printf("error may happen, need reload\n");
		return -1;
	}
#endif

//	printf("send data length: %d\n", length);

	//jzmac_dump_all_regs(__func__, __LINE__);
	/* if error occurs, then handle the error */

	next_tx++;
	if (next_tx >= NUM_TX_DESCS)
		next_tx = 0;

	return ret;
}

static int jz_recv(struct eth_device* dev)
{
	volatile DmaDesc *desc;
	int length = -1;

	desc = rx_desc + next_rx;

	if(!synopGMAC_is_desc_owned_by_dma(desc)) {

		/* since current desc contains the valid data, now we're going to get the data */
		length = synopGMAC_get_rx_desc_frame_length(desc->status);
		/* Pass the packet up to the protocol layers */

#if 0
		jzmac_dump_arp_reply(NetRxPackets[next_rx], length - 4);
		jzmac_dump_icmp_reply(NetRxPackets[next_rx], length - 4);
#endif

	    //printf("recv length:%d\n", length);
	    //jzmac_dump_pkt_data(NetRxPackets[next_rx], length - 4);
		//jzmac_dump_all_regs(__func__, __LINE__);
#if 1
		if(length  < 28) {
			udelay(100);
			return -1;
		}
#endif
		NetReceive(NetRxPackets[next_rx], length - 4);
		/* after got data, make sure the dma owns desc to recv data from MII */
		desc->status = DescOwnByDma;

		synopGMAC_resume_dma_rx(gmacdev);

		flush_dcache_all();
		flush_l2cache_all();

		next_rx++;
		if (next_rx >= NUM_RX_DESCS)
			next_rx = 0;
	}

	return length;
}

static int jz_init(struct eth_device* dev, bd_t * bd)
{
	int i;
	int phy_id;
	next_tx = 0;
	next_rx = 0;

	memset(tx_buff, 0, 2048 * NUM_TX_DESCS);

	printf("jz_init......\n");

	/* init global pointers */
	tx_desc = (DmaDesc *)((unsigned long)_tx_desc | 0xa0000000);
	rx_desc = (DmaDesc *)((unsigned long)_rx_desc | 0xa0000000);

#if (CONFIG_NET_GMAC_PHY_MODE == GMAC_PHY_RMII)
	u32 cpm_mphyc = 0;
	cpm_mphyc = read_cpm_mphyc();
	cpm_mphyc &= ~0x7;
	cpm_mphyc |= 0x4;
	cpm_mphyc &= ~(1<< 23);		//outside phy, need set 23 bit to zero
	write_cpm_mphyc(cpm_mphyc);
#elif (CONFIG_NET_GMAC_PHY_MODE == GMAC_PHY_RGMII)
	u32 cpm_mphyc = 0;
	cpm_mphyc = read_cpm_mphyc();
	cpm_mphyc |= 0x1<<31;
	cpm_mphyc &= ~0x7;
	cpm_mphyc |= 0x1;
	cpm_mphyc &= ~(1<< 23);		//outside phy, need set 23 bit to zero
	write_cpm_mphyc(cpm_mphyc);
#endif //CONFIG_NET_GMAC_PHY_MODE

#ifdef SYNOP_DEBUG
	printf("*(volatile unsigned int *)(0xB0000054)=%x\n", *(volatile unsigned int *)(0xB0000054));
	printf("*(volatile unsigned int *)(0xB00000E8)=%x\n", *(volatile unsigned int *)(0xB00000E8));
	printf("*(volatile unsigned int *)(0xB00000c0)=%x\n", *(volatile unsigned int *)(0xB00000c0));
	printf("*(volatile unsigned int *)(0xB00000c4)=%x\n", *(volatile unsigned int *)(0xB00000c4));
#endif

	/* reset GMAC, prepare to search phy */
	synopGMAC_reset(gmacdev);

	/* we do not process interrupts */
	synopGMAC_disable_interrupt_all(gmacdev);

	// Set MAC address
	synopGMAC_set_mac_addr(gmacdev,
			       GmacAddr0High,GmacAddr0Low,
			       eth_get_dev()->enetaddr);

	synopGMAC_set_mdc_clk_div(gmacdev,GmiiCsrClk2);
	gmacdev->ClockDivMdc = synopGMAC_get_mdc_clk_div(gmacdev);

	/* search phy */
#if 1
	phy_id = synopGMAC_search_phy(gmacdev);
	if (phy_id >= 0) {
		printf("====>found PHY %d\n", phy_id);
		gmacdev->PhyBase = phy_id;
	} else {
		printf("====>PHY not found!\n");
	}
#endif
	synopGMAC_check_phy_init(gmacdev);


	jz47xx_mac_configure();
	/* setup tx_desc */
	for (i = 0; i <  NUM_TX_DESCS; i++) {
		synopGMAC_tx_desc_init_ring(tx_desc + i, i == (NUM_TX_DESCS - 1));
	}

	synopGMACWriteReg((u32 *)gmacdev->DmaBase,DmaTxBaseAddr, virt_to_phys(_tx_desc));

	/* setup rx_desc */
	for (i = 0; i < NUM_RX_DESCS; i++) {
		DmaDesc *curr_desc = rx_desc + i;
		synopGMAC_rx_desc_init_ring(curr_desc, i == (NUM_RX_DESCS - 1));

		curr_desc->length |= ((PKTSIZE_ALIGN <<DescSize1Shift) & DescSize1Mask) |
			((0 << DescSize2Shift) & DescSize2Mask);
		curr_desc->buffer1 = virt_to_phys(NetRxPackets[i]);

		curr_desc->extstatus = 0;
		curr_desc->reserved1 = 0;
		curr_desc->timestamplow = 0;
		curr_desc->timestamphigh = 0;

		/* start transfer */
		curr_desc->status = DescOwnByDma;
	}

	synopGMACWriteReg((u32 *)gmacdev->DmaBase,DmaRxBaseAddr, virt_to_phys(_rx_desc));

	flush_dcache_all();
	flush_l2cache_all();

	//jz47xx_mac_configure();

#ifdef SYNOP_DEBUG
	jzmac_dump_all_regs(__func__, __LINE__);
#endif
	/* we only enable rx here */
	synopGMAC_enable_dma_rx(gmacdev);
//	synopGMAC_enable_dma_tx(gmacdev);

#if 1
	synopGMAC_tx_enable(gmacdev);
	synopGMAC_rx_enable(gmacdev);

#endif
	printf("GMAC init finish\n");
	return 1;
}

static void jz_halt(struct eth_device *dev)
{
	next_tx = 0;
	next_rx = 0;
	synopGMAC_rx_disable(gmacdev);
	udelay(100);
	synopGMAC_disable_dma_rx(gmacdev);
	udelay(100);
	synopGMAC_disable_dma_tx(gmacdev);
	udelay(100);
	synopGMAC_tx_enable(gmacdev);
}

extern s32 synopGMAC_read_phy_reg(u32 *RegBase,u32 PhyBase, u32 RegOffset, u16 * data);
extern s32 synopGMAC_write_phy_reg(u32 *RegBase, u32 PhyBase, u32 RegOffset, u16 data);
int jz_net_initialize(bd_t *bis)
{
	struct eth_device *dev;

	clk_set_rate(MACPHY, 50000000);
//	udelay(5000);

#if defined (CONFIG_T10) || defined (CONFIG_T20) || defined (CONFIG_T30) || defined (CONFIG_T21) || defined (CONFIG_T31) || defined (CONFIG_T40)
	/* initialize gmac gpio */
	gpio_set_func(GPIO_PORT_B, GPIO_FUNC_0, 0x1EFC0);
#endif

	gmacdev = &_gmacdev;
#define JZ_GMAC_BASE 0xb34b0000
	gmacdev->DmaBase =  JZ_GMAC_BASE + DMABASE;
	gmacdev->MacBase =  JZ_GMAC_BASE + MACBASE;


#if (CONFIG_NET_PHY_TYPE == PHY_TYPE_IP101G)
	/* gpio reset IP101G */
#ifdef CONFIG_GPIO_IP101G_RESET
	gpio_direction_output(CONFIG_GPIO_IP101G_RESET, !CONFIG_GPIO_IP101G_RESET_ENLEVEL);
	//mdelay(10);
	mdelay(2);
	gpio_direction_output(CONFIG_GPIO_IP101G_RESET, CONFIG_GPIO_IP101G_RESET_ENLEVEL);
	//mdelay(50);
	mdelay(5);
	//gpio_direction_output(32*1+13, 1);
	gpio_direction_output(CONFIG_GPIO_IP101G_RESET, !CONFIG_GPIO_IP101G_RESET_ENLEVEL);
	//mdelay(10);
	mdelay(2);
#endif/*CONFIG_GPIO_IP101G_RESET*/
#endif

#if (CONFIG_NET_PHY_TYPE == PHY_TYPE_88E1111)
	int phy_id;
	u16 data;
	s32 status = -ESYNOPGMACNOERR;

#if (CONFIG_NET_GMAC_PHY_MODE == GMAC_PHY_RGMII)

	phy_id = synopGMAC_search_phy(gmacdev);
	if (phy_id >= 0) {
		gmacdev->PhyBase = phy_id;
	} else {
		printf("====>PHY not found!\n");
	}

	/* configure 88e1111 in rgmii to copper mode
	 */
	status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, 27, &data);
	if(status) {
		TR("read mac register error\n");
	}
	data &= ~0xF;
	data |= 0xB;
	status = synopGMAC_write_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, 27, data);
	if(status) {
		TR("write mac register error\n");
	}

#endif  /* CONFIG_NET_GMAC_PHY_MODE */

	/* software reset 88e1111
	 */
	status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, 0, &data);
	if(status) {
		TR("read mac register error\n");
	}
	data |= 0x1<<15;
	status = synopGMAC_write_phy_reg((u32*)gmacdev->MacBase, phy_id, 0, data);
	if(status) {
		TR("write mac register error\n");
	}
	while(1) {
		status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, 0, &data);
		if(status) {
			TR("read mac register error\n");
		}
		if((data & (0x1<<15)) != 0) {
			continue;
		} else {
			break;
		}
	}
#endif //CONFIG_NET_PHY_TYPE

#if (CONFIG_NET_PHY_TYPE == PHY_TYPE_DP83867)

	int phy_id;
	int i;
	u16 data;
	u16 PhyModel;
	u8 PhyType;
	s32 status = -ESYNOPGMACNOERR;

	phy_id = synopGMAC_search_phy(gmacdev);
	if (phy_id >= 0) {
		gmacdev->PhyBase = phy_id;
	} else {
		printf("====>PHY not found!\n");
	}
	printf("====>PHY %d ID found!\n",phy_id);

#ifdef SYNOP_DEBUG
	status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, 0xf ,&data);
	printf("***************  line : %d PHY 0xf = 0x%x ****************\n", __LINE__,data);
	status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, 1 ,&data);
	printf("***************  line : %d PHY_R0_STATUS_REG 0x%x ****************\n", __LINE__,data);
#endif

#if 1
	/* software reset dp83867
	 */

	status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, 0, &data);
	if(status) {
		TR("read mac register error\n");
	}
	data |= 0x1<<15;
	status = synopGMAC_write_phy_reg((u32*)gmacdev->MacBase, phy_id, 0, data);
	if(status) {
		TR("write mac register error\n");
	}
	while(1) {
		status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, 0, &data);
		if(status) {
			TR("read mac register error\n");
		}
		if((data & (0x1<<15)) != 0) {
			continue;
		} else {
			break;
		}
	}
#endif
	/*loopback*/
	/*status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, PHY_R0_CTRL_REG ,&data);*/
	/*synopGMAC_write_phy_reg((u32*)gmacdev->MacBase, phy_id, PHY_R0_CTRL_REG,data | PHY_R0_LOOPBACK);*/

	status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, 0x9 ,&data);
	printf("***************  line : %d reg 0x9 = 0x%x ****************\n", __LINE__,data);
	udelay(100000);
	status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, 1 ,&data);
	printf("***************  line : %d PHY_R0_STATUS_REG 0x%x ****************\n", __LINE__,data);

	status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, 0x10 ,&data);
	printf("***************  line : %d reg 0x10 = 0x%x ****************\n", __LINE__,data);

#if 1
 /* Enable SGMII Clock */
#if 1
	synopGMAC_write_phy_reg((u32*)gmacdev->MacBase, phy_id, TI_PHY_CR, TI_PHY_CR_DEVAD_EN),
	synopGMAC_write_phy_reg((u32*)gmacdev->MacBase, phy_id, TI_PHY_ADDDR, TI_PHY_SGMIITYPE);
	synopGMAC_write_phy_reg((u32*)gmacdev->MacBase, phy_id, TI_PHY_CR, TI_PHY_CR_DEVAD_EN | TI_PHY_CR_DEVAD_DATAEN);
	synopGMAC_write_phy_reg((u32*)gmacdev->MacBase, phy_id, TI_PHY_ADDDR, TI_PHY_SGMIICLK_EN);
	status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, 1 ,&data);
	printf("***************  line : %d PHY_R0_STATUS_REG 0x%x ****************\n", __LINE__,data);
	udelay(100000);
#endif

	/* Enable SGMII */
#if 1
	synopGMAC_write_phy_reg((u32*)gmacdev->MacBase, phy_id, TI_PHY_PHYCTRL, TI_PHY_CR_SGMII_EN);
	status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, TI_PHY_CFGR2,&data);
	synopGMAC_write_phy_reg((u32*)gmacdev->MacBase, phy_id, TI_PHY_CFGR2,
			data & (~TI_PHY_CFGR2_SGMII_AUTONEG_EN));
	udelay(100000);
	/*status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, TI_PHY_CFGR2.&data);*/
#endif

	status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, PHY_R0_CTRL_REG ,&data);
	/*data &= (~PHY_R0_ANEG_ENABLE);*/
	data &= (~PHY_R0_ISOLATE);
	data |= PHY_R0_DFT_SPD_1000;

	printf("*************** file : %s, func : %s, line : %d PHY_R0_CTRL_REG %x ****************\n", __FILE__, __func__, __LINE__,data);
	synopGMAC_write_phy_reg((u32*)gmacdev->MacBase, phy_id, PHY_R0_CTRL_REG,data);

#endif

#ifdef SYNOP_DEBUG
	status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, 0xf ,&data);
	printf("***************  line : %d PHY 0xf = 0x%x ****************\n", __LINE__,data);

	status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, 0x9 ,&data);
	printf("***************  line : %d reg 0x9 = 0x%x ****************\n", __LINE__,data);

	while(0){
			status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, 3, 1 ,&data);
		/*for ( i =0 ; i<=31 ;i++) {*/
			/*status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, i, 1 ,&data);*/
			/*printf("***************  line : %d PHY_R0_STATUS_REG %x  i = %d****************\n", __LINE__,data,i);*/
			printf("***************  line : %d PHY_R0_STATUS_REG %x  ****************\n", __LINE__,data);
		/*}*/
			status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, 0xf ,&data);
			printf("***************  line : %d PHY 0xf = 0x%x ****************\n", __LINE__,data);

			status = synopGMAC_read_phy_reg((u32*)gmacdev->MacBase, phy_id, 0x9 ,&data);
			printf("***************  line : %d reg 0x9 = 0x%x ****************\n", __LINE__,data);

			/*status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,3,PHY_CONTROL_REG, &data);*/
			status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,3,PHY_SPECIFIC_STATUS_REG, &data);
			if(status)
				return status;
			int speed_bit;
			speed_bit = data & (0x3<<14);
			switch(speed_bit) {
				case 0x8000:
					gmacdev->Speed = SPEED1000;
					break;
				case 0x4000:
					gmacdev->Speed = SPEED100;
					break;
				case 0x0000:
					gmacdev->Speed = SPEED10;
					break;
			}

			if(data & 0x0100)
				gmacdev->DuplexMode = FULLDUPLEX;
			else
				gmacdev->DuplexMode = HALFDUPLEX;

			printf("Link is up in %s mode\n",(gmacdev->DuplexMode == FULLDUPLEX) ? "FULL DUPLEX": "HALF DUPLEX");
			if(gmacdev->Speed == SPEED1000)
				printf("Link is with 1000M Speed \n");
			if(gmacdev->Speed == SPEED100)
				printf("Link is with 100M Speed \n");
			if(gmacdev->Speed == SPEED10)
				printf("Link is with 10M Speed \n");
	}
#endif

#endif //CONFIG_NET_PHY_TYPE

//	udelay(100000);

	dev = (struct eth_device *)malloc(sizeof(struct eth_device));
	if(dev == NULL) {
		printf("struct eth_device malloc fail\n");
		return -1;
	}

	memset(dev, 0, sizeof(struct eth_device));

	sprintf(dev->name, "Jz4775-9161");

	dev->iobase	= 0;
	dev->priv	= 0;
	dev->init	= jz_init;
	dev->halt	= jz_halt;
	dev->send	= jz_send;
	dev->recv	= jz_recv;

	eth_register(dev);

	return 1;
}

static int do_ethphy(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *cmd;
	int ret = 0;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

    cmd = argv[1];
    --argc;
    ++argv;

    if (strcmp(cmd, "read") == 0) {
        unsigned long addr;
        char *endp;
        if (argc != 2) {
            ret = -1;
            goto done;
        }
        addr = simple_strtoul(argv[1], &endp, 16);
        if (*argv[1] == 0 || *endp != 0) {
            ret = -1;
            goto done;
        }
        u16 data;
        s32 status = -ESYNOPGMACNOERR;
        status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, addr, &data);
        if(status) {
            printf("%s,%d:read mac register error\n", __func__, __LINE__);
        }
        printf("phy read 0x%lx = 0x%x\n",
                addr, data);

    } else if (strcmp(cmd, "reset") == 0) {
#ifdef CONFIG_GPIO_IP101G_RESET
		printf("eth phy reset %d, %d\n", CONFIG_GPIO_IP101G_RESET, CONFIG_GPIO_IP101G_RESET_ENLEVEL);
		gpio_direction_output(CONFIG_GPIO_IP101G_RESET, !CONFIG_GPIO_IP101G_RESET_ENLEVEL);
		mdelay(10);
		gpio_direction_output(CONFIG_GPIO_IP101G_RESET, CONFIG_GPIO_IP101G_RESET_ENLEVEL);
		mdelay(50);
		gpio_direction_output(CONFIG_GPIO_IP101G_RESET, !CONFIG_GPIO_IP101G_RESET_ENLEVEL);
		mdelay(10);
#endif/*CONFIG_GPIO_IP101G_RESET*/
    } else if (strcmp(cmd, "write") == 0) {
        unsigned long addr;
        u16 data;
        char *endp;
        if (argc != 3) {
            ret = -1;
            goto done;
        }
        addr = simple_strtoul(argv[1], &endp, 16);
        if (*argv[1] == 0 || *endp != 0) {
            ret = -1;
            goto done;
        }
        data = simple_strtoul(argv[2], &endp, 16);
        if (*argv[2] == 0 || *endp != 0) {
            ret = -1;
            goto done;
        }

        s32 status = -ESYNOPGMACNOERR;
        printf("phy write 0x%lx = 0x%x\n",
                addr, data);
        status = synopGMAC_write_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, addr, data);
        if(status) {
            printf("%s,%d:write phy register error\n", __func__, __LINE__);
        }

    } else {
        ret = -1;
        goto done;
    }
    return ret;

done:

	if (ret != -1)
		return ret;
usage:
	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	ethphy,	4,	1,	do_ethphy,
    "ethphy contrl",
	"\nethphy read addr\n"
	"ethphy write addr data\n"
);

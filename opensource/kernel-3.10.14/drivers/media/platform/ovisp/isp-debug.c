#include <linux/debugfs.h>
#include "ovisp-isp.h"
#include "isp-debug.h"
#include "ovisp-csi.h"
#include "ovisp-base.h"

/* -------------------debugfs interface------------------- */
static struct dentry *isp_debug_dir;
static struct dentry *isp_debug_print;
unsigned int isp_print_level = PRINT_LEVEL;

int isp_printf(unsigned int level, unsigned char *fmt, ...)
{
	struct va_format vaf;
	va_list args;
	int r = 0;

	if(level >= isp_print_level){
		va_start(args, fmt);

		vaf.fmt = fmt;
		vaf.va = &args;

		r = printk("%pV",&vaf);
		va_end(args);
		if(level >= ISP_ERROR)
			dump_stack();
	}
	return r;
}
int isp_debug_init(void)
{
	int ret = 0;
	isp_debug_dir = debugfs_create_dir("isp_debug" , NULL);
	if (!isp_debug_dir) {
		ret = -ENOMEM;
		goto fail;
	}
	isp_debug_print = debugfs_create_u32("isp_print", S_IWUSR | S_IRUSR, isp_debug_dir, &isp_print_level);
	if (!isp_debug_print) {
		ret = -ENOMEM;
		goto fail_u8;
	}
	return ret;
fail_u8:
	debugfs_remove(isp_debug_dir);
fail:
	return ret;
}
int isp_debug_deinit(void)
{
	debugfs_remove(isp_debug_print);
	debugfs_remove(isp_debug_dir);
	return 0;
}
/* -------------------end debugfs interface--------------- */
void dump_isp_set_para(struct isp_device *isp, struct isp_parm *iparm, unsigned short iformat, unsigned short oformat)
{
	ISP_PRINT(ISP_INFO,"-----------------------------  dump set parameter ----------------------------------\n");
	ISP_PRINT(ISP_INFO,"%s:input format:%08x, output format, %08x\n", __func__, iformat, oformat);
	ISP_PRINT(ISP_INFO,"%s:iparm->in_width:%d, in_width:0x%x\n", __func__, iparm->input.width, iparm->input.width);
	ISP_PRINT(ISP_INFO,"%s:iparm->in_height:%d, in_height:0x%x\n", __func__, iparm->input.height, iparm->input.height);
	ISP_PRINT(ISP_INFO,"%s:iparm->out_width:%d, out_width:0x%x\n", __func__, iparm->output[0].width, iparm->output[0].width);
	ISP_PRINT(ISP_INFO,"%s:iparm->out_height:%d, out_height:0x%x\n", __func__, iparm->output[0].height, iparm->output[0].height);
	ISP_PRINT(ISP_INFO,"---------------------------------------------------------------------------\n");
}

void dump_isp_cal_zoom(struct isp_device *isp)
{
	ISP_PRINT(ISP_INFO,"----------------------------  dump cal zoom  ------------------------------\n");
	ISP_PRINT(ISP_INFO,"isp->parm.ratio_d= %d,isp->parm.ratio_dcw = %d\n",isp->parm.ratio_d,isp->parm.ratio_dcw);
	ISP_PRINT(ISP_INFO,"isp->parm.crop_width= %d,isp->parm.crop_height = %d\n",isp->parm.crop_width,isp->parm.crop_height);
	ISP_PRINT(ISP_INFO,"isp->parm.crop_x= %d,isp->parm.crop_y = %d\n",isp->parm.crop_x,isp->parm.crop_y);
	ISP_PRINT(ISP_INFO,"isp->parm.dcwFlag= %d,isp->parm.dowscaleFlag = %d\n",isp->parm.dcwFlag,isp->parm.dowscaleFlag);
	ISP_PRINT(ISP_INFO,"isp->parm.ratio_up= %d\n",isp->parm.ratio_up);
	ISP_PRINT(ISP_INFO,"---------------------------------------------------------------------------\n");
}

void dump_isp_offline_process(struct isp_device *isp)
{
	ISP_PRINT(ISP_INFO,"---------------------------  dump offline process -------------------------\n");
	ISP_PRINT(ISP_INFO,"input address:%02x\n", isp_reg_readb(isp, 0x63b20));
	ISP_PRINT(ISP_INFO,"input address:%02x\n", isp_reg_readb(isp, 0x63b21));
	ISP_PRINT(ISP_INFO,"input address:%02x\n", isp_reg_readb(isp, 0x63b22));
	ISP_PRINT(ISP_INFO,"input address:%02x\n", isp_reg_readb(isp, 0x63b23));
	ISP_PRINT(ISP_INFO,"ready:%02x\n", isp_reg_readb(isp, 0x63b30));
	ISP_PRINT(ISP_INFO,"base 0:%02x\n", isp_reg_readb(isp, 0x63b00));
	ISP_PRINT(ISP_INFO,"base 0:%02x\n", isp_reg_readb(isp, 0x63b01));
	ISP_PRINT(ISP_INFO,"base 0:%02x\n", isp_reg_readb(isp, 0x63b02));
	ISP_PRINT(ISP_INFO,"base 0:%02x\n", isp_reg_readb(isp, 0x63b03));
	ISP_PRINT(ISP_INFO,"base 10:%02x\n", isp_reg_readb(isp, 0x63b28));
	ISP_PRINT(ISP_INFO,"base 10:%02x\n", isp_reg_readb(isp, 0x63b29));
	ISP_PRINT(ISP_INFO,"base 10:%02x\n", isp_reg_readb(isp, 0x63b2a));
	ISP_PRINT(ISP_INFO,"base 10:%02x\n", isp_reg_readb(isp, 0x63b2b));
	ISP_PRINT(ISP_INFO,"---------------------------------------------------------------------------\n");
}

void __dump_isp_regs(struct isp_device * isp, int base, int end)
{
	int i;
	int num = end - base + 1;
	ISP_PRINT(ISP_INFO,"================dump_isp_regs begin============\n");
	for(i = 0; i < num; i++) {
		ISP_PRINT(ISP_INFO,"{0x%06x,0x%02x},\n", base + i, isp_reg_readb(isp, base + i));
	}
	ISP_PRINT(ISP_INFO,"================dump_isp_regs end ============\n");
}

void dump_csi_reg(void)
{

	ISP_PRINT(ISP_INFO,"****>>>>> dump csi reg <<<<<******\n");
	ISP_PRINT(ISP_INFO,"**********VERSION =%08x\n", csi_core_read(VERSION));
	ISP_PRINT(ISP_INFO,"**********N_LANES =%08x\n", csi_core_read(N_LANES));
	ISP_PRINT(ISP_INFO,"**********PHY_SHUTDOWNZ = %08x\n", csi_core_read(PHY_SHUTDOWNZ));
	ISP_PRINT(ISP_INFO,"**********DPHY_RSTZ = %08x\n", csi_core_read(DPHY_RSTZ));
	ISP_PRINT(ISP_INFO,"**********CSI2_RESETN =%08x\n", csi_core_read(CSI2_RESETN));
	ISP_PRINT(ISP_INFO,"**********PHY_STATE = %08x\n", csi_core_read(PHY_STATE));
	ISP_PRINT(ISP_INFO,"**********DATA_IDS_1 = %08x\n", csi_core_read(DATA_IDS_1));
	ISP_PRINT(ISP_INFO,"**********DATA_IDS_2 = %08x\n", csi_core_read(DATA_IDS_2));
	ISP_PRINT(ISP_INFO,"**********ERR1 = %08x\n", csi_core_read(ERR1));
	ISP_PRINT(ISP_INFO,"**********ERR2 = %08x\n", csi_core_read(ERR2));
	ISP_PRINT(ISP_INFO,"**********MASK1 =%08x\n", csi_core_read(MASK1));
	ISP_PRINT(ISP_INFO,"**********MASK2 =%08x\n", csi_core_read(MASK2));
	ISP_PRINT(ISP_INFO,"**********PHY_TST_CTRL0 = %08x\n", csi_core_read(PHY_TST_CTRL0));
	ISP_PRINT(ISP_INFO,"**********PHY_TST_CTRL1 = %08x\n", csi_core_read(PHY_TST_CTRL1));
}

void dump_isp_configuration(struct isp_device * isp)
{
	int i;

	ISP_PRINT(ISP_INFO,"input configuration\n");
	for(i = 0; i < 0xf; i++) {
		ISP_PRINT(ISP_INFO,"0%x:==>0x%x\n", 0x1f000 + i,isp_firmware_readb(isp, 0x1f000 + i));
	}
	ISP_PRINT(ISP_INFO,"output configuration\n");
	for(i = 0; i < 0x13; i++) {
		ISP_PRINT(ISP_INFO,"0%x:==>0x%x\n", 0x1f022 + i,isp_firmware_readb(isp, 0x1f022 + i));
	}
	ISP_PRINT(ISP_INFO,"ISP configuration\n");
	for(i = 0; i < 0x16; i++) {
		ISP_PRINT(ISP_INFO,"0%x:==>0x%x\n", 0x1f070 + i,isp_firmware_readb(isp, 0x1f070 + i));
	}
}

void dump_isp_firmware(struct isp_device * isp)
{
	int i;

	for(i = 0; i < 200; i++) {
		ISP_PRINT(ISP_INFO,"0x%08x,:==>0x%x\n", 0x1e000 + i, isp_firmware_readb(isp, 0x1e000 + i));
	}
}

void dump_isp_syscontrol(struct isp_device * isp)
{
	ISP_PRINT(ISP_INFO,"core_ctrl[15:8]:0x63022:0x%08x\n", isp_reg_readb(isp, 0x63022));
	ISP_PRINT(ISP_INFO,"core_ctrl[7:0]: 0x63023:0x%08x\n", isp_reg_readb(isp, 0x63023));
	ISP_PRINT(ISP_INFO,"bypass_mode?:   0x63025:0x%08x\n", isp_reg_readb(isp, 0x63025));
	ISP_PRINT(ISP_INFO,"pack1_en?	0x63c21:0x%08x\n", isp_reg_readb(isp, 0x63c21));
	ISP_PRINT(ISP_INFO,"pack2_en?	0x63c22:0x%08x\n", isp_reg_readb(isp, 0x63c22));
	ISP_PRINT(ISP_INFO,"input interface, 0x63108:%08x\n", isp_reg_readb(isp, 0x63108));
	ISP_PRINT(ISP_INFO,"clkrst 0, 0x6301a:%08x\n", isp_reg_readb(isp, 0x6301a));
	ISP_PRINT(ISP_INFO,"clkrst 0, 0x6301b:%08x\n", isp_reg_readb(isp, 0x6301b));

}
void dump_firmware_reg(struct isp_device * isp, int base, int num)
{
	int i;
	ISP_PRINT(ISP_INFO,"================dump_firmware_reg begin============\n");
	for(i = 0; i < num; i++) {
		ISP_PRINT(ISP_INFO,"0x%08x:%02x\n", base + i, isp_firmware_readb(isp, base + i));
	}
	ISP_PRINT(ISP_INFO,"================dump_firmware_reg end ============\n");
}

void dump_isp_configs(struct isp_device *isp)
{
	/*1. lens register*/
	ISP_PRINT(ISP_INFO,"lens online register\n");
	__dump_isp_regs(isp, 0x65200, 0x6523d);
	/*2. raw strecth */
	ISP_PRINT(ISP_INFO,"raw stretch register\n");
	__dump_isp_regs(isp, 0x65400, 0x65413);
	/*3. awb register*/
	ISP_PRINT(ISP_INFO,"awb register\n");
	__dump_isp_regs(isp, 0x65300, 0x65320);
	/*4. cip register*/
	ISP_PRINT(ISP_INFO,"cip register\n");
	__dump_isp_regs(isp, 0x65900, 0x65921);
	/*5. gamma*/
	ISP_PRINT(ISP_INFO,"gamma register\n");
	__dump_isp_regs(isp, 0x65c00, 0x65c49);
	/*6. sde*/
	ISP_PRINT(ISP_INFO,"sde register\n");
	__dump_isp_regs(isp, 0x65f00, 0x65f21);
	/*7.65000-65006c*/
	ISP_PRINT(ISP_INFO,"65000-6506c\n");
	__dump_isp_regs(isp, 0x65000, 0x6506c);
}

void dump_tlb_regs(struct isp_device *isp)
{
	ISP_PRINT(ISP_INFO,"TLB_ERROR:0x%08x\n", isp_reg_readl(isp, 0xF0000));
	ISP_PRINT(ISP_INFO,"TLB_CTRL:0x%08x\n", isp_reg_readl(isp, 0xF0004));
	ISP_PRINT(ISP_INFO,"TLB_BASE:0x%08x\n", isp_reg_readl(isp, 0xF0008));
	ISP_PRINT(ISP_INFO,"TLB_ENTRY:0x%08x\n", isp_reg_readl(isp, 0xF000C));
	ISP_PRINT(ISP_INFO,"TLB_EVPN:0x%08x\n", isp_reg_readl(isp, 0xF0010));
	ISP_PRINT(ISP_INFO,"AXI_WR_CMD:0x%08x\n", isp_reg_readl(isp, 0xF0014));
	ISP_PRINT(ISP_INFO,"AXI_WR_LEN:0x%08x\n", isp_reg_readl(isp, 0xF0018));
	ISP_PRINT(ISP_INFO,"AXI_WR_DATA:0x%08x\n", isp_reg_readl(isp, 0xF001c));
}

void dump_command_reg(struct isp_device *isp)
{
	ISP_PRINT(ISP_INFO,"COMMAND_REG0:%08x\n", isp_reg_readb(isp,COMMAND_REG0));
	ISP_PRINT(ISP_INFO,"COMMAND_REG1:%08x\n", isp_reg_readb(isp,COMMAND_REG1));
	ISP_PRINT(ISP_INFO,"COMMAND_REG2:%08x\n", isp_reg_readb(isp,COMMAND_REG2));
	ISP_PRINT(ISP_INFO,"COMMAND_REG3:%08x\n", isp_reg_readb(isp,COMMAND_REG3));
	ISP_PRINT(ISP_INFO,"COMMAND_REG4:%08x\n", isp_reg_readb(isp,COMMAND_REG4));
	ISP_PRINT(ISP_INFO,"COMMAND_REG5:%08x\n", isp_reg_readb(isp,COMMAND_REG5));
	ISP_PRINT(ISP_INFO,"COMMAND_REG6:%08x\n", isp_reg_readb(isp,COMMAND_REG6));
	ISP_PRINT(ISP_INFO,"COMMAND_REG7:%08x\n", isp_reg_readb(isp,COMMAND_REG7));
	ISP_PRINT(ISP_INFO,"COMMAND_REG8:%08x\n", isp_reg_readb(isp,COMMAND_REG8));
	ISP_PRINT(ISP_INFO,"COMMAND_REG9:%08x\n", isp_reg_readb(isp,COMMAND_REG9));
}

void dump_i2c_regs(struct isp_device * isp)
{
	ISP_PRINT(ISP_INFO,"sccb speed:0x%x\n", isp_reg_readb(isp, 0x63600));
	ISP_PRINT(ISP_INFO,"sccb slave id:0x%x\n", isp_reg_readb(isp, 0x63601));
	ISP_PRINT(ISP_INFO,"sccb address[15:8]:0x%x\n", isp_reg_readb(isp, 0x63602));
	ISP_PRINT(ISP_INFO,"sccb address[7:0]:0x%x\n", isp_reg_readb(isp, 0x63603));
	ISP_PRINT(ISP_INFO,"sccb output data[15:8]:0x%x\n", isp_reg_readb(isp, 0x63604));
	ISP_PRINT(ISP_INFO,"sccb output data[7:0]:0x%x\n", isp_reg_readb(isp, 0x63605));
	ISP_PRINT(ISP_INFO,"sccb 2byte control:0x%x\n", isp_reg_readb(isp, 0x63606));
	ISP_PRINT(ISP_INFO,"sccb input data h:0x%x\n", isp_reg_readb(isp, 0x63607));
	ISP_PRINT(ISP_INFO,"sccb input data l:0x%x\n", isp_reg_readb(isp, 0x63608));
	ISP_PRINT(ISP_INFO,"sccb command :%x\n", isp_reg_readb(isp, 0x63609));
}

void dump_isp_i2c_cmd(struct isp_device *isp, struct isp_i2c_cmd *cmd)
{
	ISP_PRINT(ISP_INFO,"isp_i2c_cmd >flags 0x%08x\n", cmd->flags);
	ISP_PRINT(ISP_INFO,"isp_i2c_cmd >addr 0x%08x\n", cmd->addr);
	ISP_PRINT(ISP_INFO,"isp_i2c_cmd >reg 0x%08x\n", cmd->reg);
	ISP_PRINT(ISP_INFO,"isp_i2c_cmd >data 0x%08x\n", cmd->data);
}

void dump_cmd_regs(struct isp_device * isp)
{

}

void dump_isp_range_regs(struct isp_device * isp, unsigned int addr, int count)
{
	int i;
	ISP_PRINT(ISP_INFO,"========== dump register start ==============\n");
	for(i = 0; i < count; i++)
		ISP_PRINT(ISP_INFO,"[0x%x]:	0x%x\n", addr + i,isp_firmware_readb(isp, addr + i));
	ISP_PRINT(ISP_INFO,"========== dump register end ==============\n");
}
void dump_isp_top_register(struct isp_device * isp)
{
	int i;
	ISP_PRINT(ISP_INFO,"ISP pipeline=====================================\n");
	/*0x65000-0x65007 top*/
	for(i = 0; i < 0x10; i++) {
		ISP_PRINT(ISP_INFO,"[0x%x]:	0x%x\n", 0x65000 + i, isp_reg_readb(isp,0x65000 + i));
	}
#if 0
	/*0x6500b- */
	for(i = 0; i < 19; i++) {
		ISP_PRINT(ISP_INFO,"[0x%x]:	0x%x\n", 0x6500b + i, isp_reg_readb(isp,0x6500b + i));
	}
	/*65020-65047*/
	for(i = 0; i < 40; i++) {
		ISP_PRINT(ISP_INFO,"[0x%x]:	0x%x\n", 0x65020 + i, isp_reg_readb(isp,0x65020 + i));
	}
	/*65050-6505b*/
	for(i = 0; i < 0xb; i++) {
		ISP_PRINT(ISP_INFO,"[0x%x]:	0x%x\n", 0x65050 + i, isp_reg_readb(isp,0x65050 + i));
	}
	/*65060-6506c*/
	for(i = 0; i < 0xc; i++) {
		ISP_PRINT(ISP_INFO,"[0x%x]:	0x%x\n", 0x65060 + i, isp_reg_readb(isp,0x65060 + i));
	}
#endif

}

void dump_isp_debug_regs(struct isp_device * isp)
{
	ISP_PRINT(ISP_INFO,"===========================ISP DEBUG REGISTER START===============================\n");
	ISP_PRINT(ISP_INFO,"[IDI] settings (0x63c00-0x63c13)\n");
	ISP_PRINT(ISP_INFO,"X_OFFSET_16:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c00), isp_reg_readb(isp, 0x63c01));
	ISP_PRINT(ISP_INFO,"Y_OFFSET_16:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c02), isp_reg_readb(isp, 0x63c03));
	ISP_PRINT(ISP_INFO,"WIDTH_16:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c04), isp_reg_readb(isp, 0x63c05));
	ISP_PRINT(ISP_INFO,"HEIGHT_16:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c06), isp_reg_readb(isp, 0x63c07));
	ISP_PRINT(ISP_INFO,"LINE_LENGTH16:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c10), isp_reg_readb(isp, 0x63c11));
	ISP_PRINT(ISP_INFO,"X_OFFSET_64:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c08), isp_reg_readb(isp, 0x63c09));
	ISP_PRINT(ISP_INFO,"Y_OFFSET_64:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c0a), isp_reg_readb(isp, 0x63c0b));
	ISP_PRINT(ISP_INFO,"WIDTH_64:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c0c), isp_reg_readb(isp, 0x63c0d));
	ISP_PRINT(ISP_INFO,"HEIGHT_64:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c0e), isp_reg_readb(isp, 0x63c0f));
	ISP_PRINT(ISP_INFO,"HLINE_LENGT_64:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c12), isp_reg_readb(isp, 0x63c13));
	ISP_PRINT(ISP_INFO,"scale_v_enable:			0x%02x\n", isp_reg_readb(isp, 0x63c14));
	ISP_PRINT(ISP_INFO,"[IDI] counter (0x63c62-0x63c6b)\n");
	ISP_PRINT(ISP_INFO,"csi_pcnt1:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c52), isp_reg_readb(isp, 0x63c53));
	ISP_PRINT(ISP_INFO,"csi_pcnt2:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c54), isp_reg_readb(isp, 0x63c55));
	ISP_PRINT(ISP_INFO,"pcnt_crop_16:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c42), isp_reg_readb(isp, 0x63c43));
	ISP_PRINT(ISP_INFO,"lcnt_crop_16:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c44), isp_reg_readb(isp, 0x63c45));
	ISP_PRINT(ISP_INFO,"pcnt_crop_64:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c46), isp_reg_readb(isp, 0x63c47));
	ISP_PRINT(ISP_INFO,"lcnt_crop_64:			0x%02x%02x\n", isp_reg_readb(isp, 0x63c48), isp_reg_readb(isp, 0x63c49));
	ISP_PRINT(ISP_INFO,"================================================================================\n");
	ISP_PRINT(ISP_INFO,"[ISP1(Pipeline1)] settings(0x65010~0x65013)\n");
	ISP_PRINT(ISP_INFO,"input width:			0x%02x%02x\n", isp_reg_readb(isp, 0x65010), isp_reg_readb(isp, 0x65011));
	ISP_PRINT(ISP_INFO,"input height:			0x%02x%02x\n", isp_reg_readb(isp, 0x65012), isp_reg_readb(isp, 0x65013));
	ISP_PRINT(ISP_INFO,"[ISP1(Pipeline1)] counter(0x65037~0x6503a)\n");
	ISP_PRINT(ISP_INFO,"input pix cnt:			0x%02x%02x\n", isp_reg_readb(isp, 0x65037), isp_reg_readb(isp, 0x65038));
	ISP_PRINT(ISP_INFO,"input line cnt:			0x%02x%02x\n", isp_reg_readb(isp, 0x65039), isp_reg_readb(isp, 0x6503a));
	ISP_PRINT(ISP_INFO,"[ISP1(Pipeline1) YUV Crop registers  for channel 1(scale1)(0x650f0~0x65f5)\n");
	ISP_PRINT(ISP_INFO,"crop_left:			0x%02x%02x\n", isp_reg_readb(isp, 0x650f0), isp_reg_readb(isp, 0x650f1));
	ISP_PRINT(ISP_INFO,"crop_top:			0x%02x%02x\n", isp_reg_readb(isp, 0x650f2), isp_reg_readb(isp, 0x650f3));
	ISP_PRINT(ISP_INFO,"crop_width:			0x%02x%02x\n", isp_reg_readb(isp, 0x650f4), isp_reg_readb(isp, 0x650f5));
	ISP_PRINT(ISP_INFO,"crop_height:			0x%02x%02x\n", isp_reg_readb(isp, 0x650f6), isp_reg_readb(isp, 0x650f7));
	ISP_PRINT(ISP_INFO,"counter:\n");
	ISP_PRINT(ISP_INFO,"Oput P cnt s1:			0x%02x%02x\n", isp_reg_readb(isp, 0x650f8), isp_reg_readb(isp, 0x650f9));
	ISP_PRINT(ISP_INFO,"Oput L cnt s1:			0x%02x%02x\n", isp_reg_readb(isp, 0x650fa), isp_reg_readb(isp, 0x650fb));
	ISP_PRINT(ISP_INFO,"[ISP1(Pipeline1) YUV Crop registers  for channel 2(scale2)(0x66200~0x6620b)\n");
	ISP_PRINT(ISP_INFO,"crop_left:			0x%02x%02x\n", isp_reg_readb(isp, 0x66200), isp_reg_readb(isp, 0x66201));
	ISP_PRINT(ISP_INFO,"crop_top:			0x%02x%02x\n", isp_reg_readb(isp, 0x66202), isp_reg_readb(isp, 0x66203));
	ISP_PRINT(ISP_INFO,"crop_width:			0x%02x%02x\n", isp_reg_readb(isp, 0x66204), isp_reg_readb(isp, 0x66205));
	ISP_PRINT(ISP_INFO,"crop_height:			0x%02x%02x\n", isp_reg_readb(isp, 0x66206), isp_reg_readb(isp, 0x66207));
	ISP_PRINT(ISP_INFO,"counter:\n");
	ISP_PRINT(ISP_INFO,"Oput P cnt s1:			0x%02x%02x\n", isp_reg_readb(isp, 0x66208), isp_reg_readb(isp, 0x66209));
	ISP_PRINT(ISP_INFO,"Oput L cnt s1:			0x%02x%02x\n", isp_reg_readb(isp, 0x6620a), isp_reg_readb(isp, 0x6620b));
	ISP_PRINT(ISP_INFO,"channel1:\n");
	ISP_PRINT(ISP_INFO,"scale oput w_th:			0x%02x%02x\n", isp_reg_readb(isp, 0x65014), isp_reg_readb(isp, 0x65015));
	ISP_PRINT(ISP_INFO,"scale oput h_ht:			0x%02x%02x\n", isp_reg_readb(isp, 0x65016), isp_reg_readb(isp, 0x65017));
	ISP_PRINT(ISP_INFO,"scale dwn enable[1]:		0x%02x\n", isp_reg_readb(isp, 0x65002));
	ISP_PRINT(ISP_INFO,"scale dwn w_th:			0x%02x%02x\n", isp_reg_readb(isp, 0x65032), isp_reg_readb(isp, 0x65033));
	ISP_PRINT(ISP_INFO,"scale dwn h_ht:			0x%02x%02x\n", isp_reg_readb(isp, 0x65034), isp_reg_readb(isp, 0x65035));
	ISP_PRINT(ISP_INFO,"scale dwn ratio_x:		0x%02x\n", isp_reg_readb(isp, 0x65024));
	ISP_PRINT(ISP_INFO,"scale dwn ratio_y:		0x%02x\n", isp_reg_readb(isp, 0x65025));
	ISP_PRINT(ISP_INFO,"YUV DCW:				0x%02x\n", isp_reg_readb(isp, 0x65023));
	ISP_PRINT(ISP_INFO,"scale up enable[3]:		0x%02x\n", isp_reg_readb(isp, 0x65002));
	ISP_PRINT(ISP_INFO,"scale up ratio_y:		0x%02x%02x\n", isp_reg_readb(isp, 0x65026), isp_reg_readb(isp, 0x65027));
	ISP_PRINT(ISP_INFO,"scale up ratio_x:		0x%02x%02x\n", isp_reg_readb(isp, 0x65028), isp_reg_readb(isp, 0x65029));
	ISP_PRINT(ISP_INFO,"channel2:\n");
	ISP_PRINT(ISP_INFO,"scale oput w_th:			0x%02x%02x\n", isp_reg_readb(isp, 0x65058), isp_reg_readb(isp, 0x65059));
	ISP_PRINT(ISP_INFO,"scale oput h_ht:			0x%02x%02x\n", isp_reg_readb(isp, 0x6505a), isp_reg_readb(isp, 0x6505b));
	ISP_PRINT(ISP_INFO,"scale dwn enable[0]:		0x%02x\n", isp_reg_readb(isp, 0x65050));
	ISP_PRINT(ISP_INFO,"scale dwn w_th:			0x%02x%02x\n", isp_reg_readb(isp, 0x65054), isp_reg_readb(isp, 0x65055));
	ISP_PRINT(ISP_INFO,"scale dwn h_ht:			0x%02x%02x\n", isp_reg_readb(isp, 0x65056), isp_reg_readb(isp, 0x65057));
	ISP_PRINT(ISP_INFO,"scale dwn ratio_x:		0x%02x\n", isp_reg_readb(isp, 0x65052));
	ISP_PRINT(ISP_INFO,"scale dwn ratio_y:		0x%02x\n", isp_reg_readb(isp, 0x65053));
	ISP_PRINT(ISP_INFO,"YUV DCW[3:0]:			0x%02x\n", isp_reg_readb(isp, 0x65051));
	ISP_PRINT(ISP_INFO,"===============================================================================\n");
	ISP_PRINT(ISP_INFO,"MAC(0x63b00)\n");
	ISP_PRINT(ISP_INFO,"settings:(Left Write Channel)\n");
	ISP_PRINT(ISP_INFO,"W_format_S0[2:0]:		0x%02x\n", isp_reg_readb(isp, 0x63b34));
	ISP_PRINT(ISP_INFO,"W_width_S0:			0x%02x%02x\n", isp_reg_readb(isp, 0x63b36), isp_reg_readb(isp, 0x63b37));
	ISP_PRINT(ISP_INFO,"W_MEM_widthS0:			0x%02x%02x\n", isp_reg_readb(isp, 0x63b38), isp_reg_readb(isp, 0x63b39));
	ISP_PRINT(ISP_INFO,"W_height_S0:			0x%02x%02x\n", isp_reg_readb(isp, 0x63b3a), isp_reg_readb(isp, 0x63b3b));
	ISP_PRINT(ISP_INFO,"W_MEM_wth_S0_uv:			0x%02x%02x\n", isp_reg_readb(isp, 0x63b5e), isp_reg_readb(isp, 0x63b5f));
	ISP_PRINT(ISP_INFO,"counter:(Left Write Channel)\n");
	ISP_PRINT(ISP_INFO,"x_cnt_latch:			0x%02x%02x\n", isp_reg_readb(isp, 0x63b99), isp_reg_readb(isp, 0x63b9a));
	ISP_PRINT(ISP_INFO,"y_cnt_latch:			0x%02x%02x\n", isp_reg_readb(isp, 0x63b9b), isp_reg_readb(isp, 0x63b9c));
	ISP_PRINT(ISP_INFO,"settings:(Right Write Channel)\n");
	ISP_PRINT(ISP_INFO,"W_format_S1[6:4]:		0x%02x\n", isp_reg_readb(isp, 0x63b34));
	ISP_PRINT(ISP_INFO,"W_width_S1:			0x%02x%02x\n", isp_reg_readb(isp, 0x63b36), isp_reg_readb(isp, 0x63b37));
	ISP_PRINT(ISP_INFO,"W_MEM_widthS1:			0x%02x%02x\n", isp_reg_readb(isp, 0x63b38), isp_reg_readb(isp, 0x63b39));
	ISP_PRINT(ISP_INFO,"W_height_S1:			0x%02x%02x\n", isp_reg_readb(isp, 0x63b3a), isp_reg_readb(isp, 0x63b3b));
	ISP_PRINT(ISP_INFO,"W_MEM_wth_S1_uv:			0x%02x%02x\n", isp_reg_readb(isp, 0x63b5e), isp_reg_readb(isp, 0x63b5f));
	ISP_PRINT(ISP_INFO,"counter:(Right Write Channel)\n");
	ISP_PRINT(ISP_INFO,"x_cnt_latch:			0x%02x%02x\n", isp_reg_readb(isp, 0x63b99), isp_reg_readb(isp, 0x63b9a));
	ISP_PRINT(ISP_INFO,"y_cnt_latch:			0x%02x%02x\n", isp_reg_readb(isp, 0x63b9b), isp_reg_readb(isp, 0x63b9c));

	ISP_PRINT(ISP_INFO,"Read Channel\n");
	ISP_PRINT(ISP_INFO,"R_width:				0x%02x%02x\n", isp_reg_readb(isp, 0x63b3f), isp_reg_readb(isp, 0x63b40));
	ISP_PRINT(ISP_INFO,"R_heght:				0x%02x%02x\n", isp_reg_readb(isp, 0x63b41), isp_reg_readb(isp, 0x63b42));
	ISP_PRINT(ISP_INFO,"R_MEM_width:			0x%02x%02x\n", isp_reg_readb(isp, 0x63b43), isp_reg_readb(isp, 0x63b44));
	ISP_PRINT(ISP_INFO,"===========================ISP DEBUG REGISTER END===============================\n");


}

void dump_idi(struct isp_device * isp)
{

	ISP_PRINT(ISP_INFO,"\ndump idi-----\n");

	ISP_PRINT(ISP_INFO,"[idi]reserved ,default 0x3,real:0x%08x\n", isp_reg_readb(isp, 0x63c65));
	ISP_PRINT(ISP_INFO,"[idi]fifo erro flag:0x%08x\n", isp_reg_readb(isp, 0x63c40));


	ISP_PRINT(ISP_INFO,"[idi]x_offset1:0x%08x\n", isp_reg_readb(isp, 0x63c00));
	ISP_PRINT(ISP_INFO,"[idi]x_offset1:0x%08x\n", isp_reg_readb(isp, 0x63c01));
	ISP_PRINT(ISP_INFO,"[idi]y_offset1:0x%08x\n", isp_reg_readb(isp, 0x63c02));
	ISP_PRINT(ISP_INFO,"[idi]y_offset1:0x%08x\n", isp_reg_readb(isp, 0x63c03));

	ISP_PRINT(ISP_INFO,"[idi]width1:0x%08x\n", isp_reg_readb(isp, 0x63c04));
	ISP_PRINT(ISP_INFO,"[idi]width1:0x%08x\n", isp_reg_readb(isp, 0x63c05));
	ISP_PRINT(ISP_INFO,"[idi]height1:0x%08x\n", isp_reg_readb(isp, 0x63c06));
	ISP_PRINT(ISP_INFO,"[idi]height1:0x%08x\n", isp_reg_readb(isp, 0x63c07));

	ISP_PRINT(ISP_INFO,"[idi]x_offset2:0x%08x\n", isp_reg_readb(isp, 0x63c08));
	ISP_PRINT(ISP_INFO,"[idi]x_offset2:0x%08x\n", isp_reg_readb(isp, 0x63c09));
	ISP_PRINT(ISP_INFO,"[idi]y_offset2:0x%08x\n", isp_reg_readb(isp, 0x63c0a));
	ISP_PRINT(ISP_INFO,"[idi]y_offset2:0x%08x\n", isp_reg_readb(isp, 0x63c0b));

	ISP_PRINT(ISP_INFO,"[idi]width2:0x%08x\n", isp_reg_readb(isp, 0x63c0c));
	ISP_PRINT(ISP_INFO,"[idi]width2:0x%08x\n", isp_reg_readb(isp, 0x63c0d));
	ISP_PRINT(ISP_INFO,"[idi]height2:0x%08x\n", isp_reg_readb(isp, 0x63c0e));
	ISP_PRINT(ISP_INFO,"[idi]height2:0x%08x\n", isp_reg_readb(isp, 0x63c0f));

	ISP_PRINT(ISP_INFO,"[idi]linelenth1:0x%08x\n", isp_reg_readb(isp, 0x63c10));
	ISP_PRINT(ISP_INFO,"[idi]linelenth1:0x%08x\n", isp_reg_readb(isp, 0x63c11));
	ISP_PRINT(ISP_INFO,"[idi]linelenth2:0x%08x\n", isp_reg_readb(isp, 0x63c12));
	ISP_PRINT(ISP_INFO,"[idi]linelenth2:0x%08x\n", isp_reg_readb(isp, 0x63c13));

	ISP_PRINT(ISP_INFO,"[idi]csi_pcnt[13:8]:0x%08x\n", isp_reg_readb(isp, 0x63c52));
	ISP_PRINT(ISP_INFO,"[idi]csi_pcnt[7:0]:0x%08x\n", isp_reg_readb(isp, 0x63c53));

	ISP_PRINT(ISP_INFO,"[idi]clk_cnt[15:8]:0x%08x\n", isp_reg_readb(isp, 0x63c58));
	ISP_PRINT(ISP_INFO,"[idi]clk_cnt[7:0]:0x%08x\n", isp_reg_readb(isp, 0x63c59));

	ISP_PRINT(ISP_INFO,"[idi]mem_waddr1[9:8]:0x%08x\n", isp_reg_readb(isp, 0x63c4c));
	ISP_PRINT(ISP_INFO,"[idi]mem_waddr1[7:0]:0x%08x\n", isp_reg_readb(isp, 0x63c4d));

	ISP_PRINT(ISP_INFO,"[idi]mem_waddr2[9:8]:0x%08x\n", isp_reg_readb(isp, 0x63c4e));
	ISP_PRINT(ISP_INFO,"[idi]mem_waddr2[7:0]:0x%08x\n", isp_reg_readb(isp, 0x63c4f));

	ISP_PRINT(ISP_INFO,"[idi]mem_waddr3[9:8]:0x%08x\n", isp_reg_readb(isp, 0x63c50));
	ISP_PRINT(ISP_INFO,"[idi]mem_waddr3[7:0]:0x%08x\n", isp_reg_readb(isp, 0x63c51));

	ISP_PRINT(ISP_INFO,"[idi]dump idi<<<<<<\n\n");
}

void dump_mac(struct isp_device *isp)
{
	ISP_PRINT(ISP_INFO,"\ndump mac ------\n");


	ISP_PRINT(ISP_INFO,"[mac] base address0:0x%x\n", isp_reg_readb(isp, 0x63b00));
	ISP_PRINT(ISP_INFO,"[mac] base address0:0x%x\n", isp_reg_readb(isp, 0x63b01));
	ISP_PRINT(ISP_INFO,"[mac] base address0:0x%x\n", isp_reg_readb(isp, 0x63b02));
	ISP_PRINT(ISP_INFO,"[mac] base address0:0x%x\n", isp_reg_readb(isp, 0x63b03));

	ISP_PRINT(ISP_INFO,"[mac] base address1:0x%x\n", isp_reg_readb(isp, 0x63b04));
	ISP_PRINT(ISP_INFO,"[mac] base address1:0x%x\n", isp_reg_readb(isp, 0x63b05));
	ISP_PRINT(ISP_INFO,"[mac] base address1:0x%x\n", isp_reg_readb(isp, 0x63b06));
	ISP_PRINT(ISP_INFO,"[mac] base address1:0x%x\n", isp_reg_readb(isp, 0x63b07));

	ISP_PRINT(ISP_INFO,"[mac] base address2:0x%x\n", isp_reg_readb(isp, 0x63b08));
	ISP_PRINT(ISP_INFO,"[mac] base address2:0x%x\n", isp_reg_readb(isp, 0x63b09));
	ISP_PRINT(ISP_INFO,"[mac] base address2:0x%x\n", isp_reg_readb(isp, 0x63b0a));
	ISP_PRINT(ISP_INFO,"[mac] base address2:0x%x\n", isp_reg_readb(isp, 0x63b0b));

	ISP_PRINT(ISP_INFO,"[mac] base address3:0x%x\n", isp_reg_readb(isp, 0x63b0c));
	ISP_PRINT(ISP_INFO,"[mac] base address3:0x%x\n", isp_reg_readb(isp, 0x63b0d));
	ISP_PRINT(ISP_INFO,"[mac] base address3:0x%x\n", isp_reg_readb(isp, 0x63b0e));
	ISP_PRINT(ISP_INFO,"[mac] base address3:0x%x\n", isp_reg_readb(isp, 0x63b0f));

	ISP_PRINT(ISP_INFO,"[mac]interrupt enable[9:8]:0x%08x\n", isp_reg_readb(isp, 0x63b53));
	ISP_PRINT(ISP_INFO,"[mac]interrupt enable[7:0]:0x%08x\n", isp_reg_readb(isp, 0x63b54));


	//ISP_PRINT(ISP_INFO,"[mac]mac int ctrl1:0x%08x\n", isp_reg_readb(isp, 0x63b32));
	//ISP_PRINT(ISP_INFO,"[mac]mac int ctrl0:0x%08x\n", isp_reg_readb(isp, 0x63b33));
	ISP_PRINT(ISP_INFO,"[mac] w_format_control:0x%x\n", isp_reg_readb(isp, 0x63b34));

	ISP_PRINT(ISP_INFO,"[mac] w_image_width_s0_1:0x%x\n", isp_reg_readb(isp, 0x63b36));
	ISP_PRINT(ISP_INFO,"[mac] w_image_width_s1_0:0x%x\n", isp_reg_readb(isp, 0x63b37));
	ISP_PRINT(ISP_INFO,"[mac] W_MEM_WIDTH_S0_1:0x%x\n", isp_reg_readb(isp, 0x63b38));
	ISP_PRINT(ISP_INFO,"[mac] W_MEM_WIDTH_S0_0:0x%x\n", isp_reg_readb(isp, 0x63b39));
	ISP_PRINT(ISP_INFO,"[mac] W_IMAGE_HEIGHT_S0_1:0x%x\n", isp_reg_readb(isp, 0x63b3a));
	ISP_PRINT(ISP_INFO,"[mac] W_IMAGE_HEIGHT_S0_0:0x%x\n", isp_reg_readb(isp, 0x63b3b));


	ISP_PRINT(ISP_INFO,"[mac]mac debug0: 0x%08x\n", isp_reg_readb(isp, 0x63b80));
	ISP_PRINT(ISP_INFO,"[mac]mac debug1: 0x%08x\n", isp_reg_readb(isp, 0x63b81));
	ISP_PRINT(ISP_INFO,"[mac]mac debug2: 0x%08x\n", isp_reg_readb(isp, 0x63b82));
	ISP_PRINT(ISP_INFO,"[mac]mac debug3: 0x%08x\n", isp_reg_readb(isp, 0x63b83));
	ISP_PRINT(ISP_INFO,"[mac]mac debug4: 0x%08x\n", isp_reg_readb(isp, 0x63b84));
	ISP_PRINT(ISP_INFO,"[mac]mac debug5: 0x%08x\n", isp_reg_readb(isp, 0x63b85));

	ISP_PRINT(ISP_INFO,"[mac]shadow int:0x%08x\n", isp_reg_readb(isp, 0x63b86));
	ISP_PRINT(ISP_INFO,"[mac]shadow int:0x%08x\n", isp_reg_readb(isp, 0x63b87));

	ISP_PRINT(ISP_INFO,"[mac]w_cnt[25:24]:0x%08x\n", isp_reg_readb(isp, 0x63b88));
	ISP_PRINT(ISP_INFO,"[mac]w_cnt[23:16]:0x%08x\n", isp_reg_readb(isp, 0x63b89));
	ISP_PRINT(ISP_INFO,"[mac]w_cnt[15:8]:0x%08x\n", isp_reg_readb(isp, 0x63b8a));
	ISP_PRINT(ISP_INFO,"[mac]w_cnt[7:0]:0x%08x\n", isp_reg_readb(isp, 0x63b8b));

	ISP_PRINT(ISP_INFO,"[mac]s0_xcnt[10:8]:0x%08x\n", isp_reg_readb(isp, 0x63b8c));
	ISP_PRINT(ISP_INFO,"[mac]s0_xcnt[7:0]:0x%08x\n", isp_reg_readb(isp, 0x63b8d));
	ISP_PRINT(ISP_INFO,"[mac]s0_ycnt[12:8]:0x%08x\n", isp_reg_readb(isp, 0x63b8e));
	ISP_PRINT(ISP_INFO,"[mac]s0_ynt[7:0]:0x%08x\n", isp_reg_readb(isp, 0x63b8f));

	ISP_PRINT(ISP_INFO,"[mac]s1_xcnt[10:8]:0x%08x\n", isp_reg_readb(isp, 0x63b90));
	ISP_PRINT(ISP_INFO,"[mac]s1_xcnt[7:0]:0x%08x\n", isp_reg_readb(isp, 0x63b91));
	ISP_PRINT(ISP_INFO,"[mac]s1_ycnt[12:8]:0x%08x\n", isp_reg_readb(isp, 0x63b92));
	ISP_PRINT(ISP_INFO,"[mac]s1_ycnt[7:0]:0x%08x\n", isp_reg_readb(isp, 0x63b93));

	ISP_PRINT(ISP_INFO,"[mac]w_cnt_latch:0x%08x\n", isp_reg_readb(isp, 0x63b95));
	ISP_PRINT(ISP_INFO,"[mac]w_cnt_latch:0x%08x\n", isp_reg_readb(isp, 0x63b96));
	ISP_PRINT(ISP_INFO,"[mac]w_cnt_latch:0x%08x\n", isp_reg_readb(isp, 0x63b97));
	ISP_PRINT(ISP_INFO,"[mac]w_cnt_latch:0x%08x\n", isp_reg_readb(isp, 0x63b98));

	ISP_PRINT(ISP_INFO,"[mac]s0_xcnt_latch:0x%08x\n", isp_reg_readb(isp, 0x63b99));
	ISP_PRINT(ISP_INFO,"[mac]s0_xcnt_latch:0x%08x\n", isp_reg_readb(isp, 0x63b9a));
	ISP_PRINT(ISP_INFO,"[mac]s0_ycnt_latch:0x%08x\n", isp_reg_readb(isp, 0x63b9b));
	ISP_PRINT(ISP_INFO,"[mac]s0_ycnt_latch:0x%08x\n", isp_reg_readb(isp, 0x63b9c));

	ISP_PRINT(ISP_INFO,"[mac]s1_xcnt_latch:0x%08x\n", isp_reg_readb(isp, 0x63b9d));
	ISP_PRINT(ISP_INFO,"[mac]s1_xcnt_latch:0x%08x\n", isp_reg_readb(isp, 0x63b9e));
	ISP_PRINT(ISP_INFO,"[mac]s1_ycnt_latch:0x%08x\n", isp_reg_readb(isp, 0x63b9f));
	ISP_PRINT(ISP_INFO,"[mac]s1_ycnt_latch:0x%08x\n", isp_reg_readb(isp, 0x63ba0));

	ISP_PRINT(ISP_INFO,"[mac]s0_w_image_pixel:0x%08x\n", isp_reg_readb(isp, 0x63bb4));
	ISP_PRINT(ISP_INFO,"[mac]s0_w_image_pixel:0x%08x\n", isp_reg_readb(isp, 0x63bb5));
	ISP_PRINT(ISP_INFO,"[mac]s0_w_image_pixel:0x%08x\n", isp_reg_readb(isp, 0x63bb6));
	ISP_PRINT(ISP_INFO,"[mac]s0_w_image_pixel:0x%08x\n", isp_reg_readb(isp, 0x63bb7));

	ISP_PRINT(ISP_INFO,"[mac]s1_w_image_pixel:0x%08x\n", isp_reg_readb(isp, 0x63bb8));
	ISP_PRINT(ISP_INFO,"[mac]s1_w_image_pixel:0x%08x\n", isp_reg_readb(isp, 0x63bb9));
	ISP_PRINT(ISP_INFO,"[mac]s1_w_image_pixel:0x%08x\n", isp_reg_readb(isp, 0x63bba));
	ISP_PRINT(ISP_INFO,"[mac]s1_w_image_pixel:0x%08x\n", isp_reg_readb(isp, 0x63bbb));

	ISP_PRINT(ISP_INFO,"[mac]rd_xcnt:0x%08x\n", isp_reg_readb(isp, 0x63bc0));
	ISP_PRINT(ISP_INFO,"[mac]rd_xcnt:0x%08x\n", isp_reg_readb(isp, 0x63bc1));
	ISP_PRINT(ISP_INFO,"[mac]rd_ycnt:0x%08x\n", isp_reg_readb(isp, 0x63bc2));
	ISP_PRINT(ISP_INFO,"[mac]rd_ycnt:0x%08x\n", isp_reg_readb(isp, 0x63bc3));


	ISP_PRINT(ISP_INFO,"[mac]rd_dummy_line:0x%08x\n", isp_reg_readb(isp, 0x63bd2));

	ISP_PRINT(ISP_INFO,"dump mac<<<<<<<<<<<<\n");
}
void dump_isp_exposure(struct isp_device *isp)
{
	ISP_PRINT(ISP_INFO, "1C056 IS %#x\n",isp_firmware_readb(isp, 0x1c056));
	ISP_PRINT(ISP_INFO, "1C21e IS %#x\n",isp_firmware_readb(isp, 0x1c21e));
	ISP_PRINT(ISP_INFO, "1e056 IS %#x\n",isp_firmware_readb(isp, 0x1e056));
	ISP_PRINT(ISP_INFO, "1e022 IS %#x\n",isp_firmware_readb(isp, 0x1e022));
	ISP_PRINT(ISP_INFO, "1e030 IS %#x\n",isp_firmware_readb(isp, 0x1e030));
	ISP_PRINT(ISP_INFO, "1e031 IS %#x\n",isp_firmware_readb(isp, 0x1e031));
	ISP_PRINT(ISP_INFO, "1e032 IS %#x\n",isp_firmware_readb(isp, 0x1e032));
	ISP_PRINT(ISP_INFO, "1e033 IS %#x\n",isp_firmware_readb(isp, 0x1e033));
	ISP_PRINT(ISP_INFO, "1e034 IS %#x\n",isp_firmware_readb(isp, 0x1e034));
	ISP_PRINT(ISP_INFO, "1e035 IS %#x\n",isp_firmware_readb(isp, 0x1e035));
	ISP_PRINT(ISP_INFO,"63601 IS %#x\n", isp_reg_readb(isp, 0x63601));
	ISP_PRINT(ISP_INFO,"63902 IS %#x\n", isp_reg_readb(isp, 0x63902));

	ISP_PRINT(ISP_INFO, "sensor address1 1e058 IS %#x\n",isp_firmware_readb(isp, 0x1e058));
	ISP_PRINT(ISP_INFO, "sensor address 1e059 IS %#x\n",isp_firmware_readb(isp, 0x1e059));
	ISP_PRINT(ISP_INFO, "sensor address2 1e05a IS %#x\n",isp_firmware_readb(isp, 0x1e05a));
	ISP_PRINT(ISP_INFO, "sensor address 1e05b IS %#x\n",isp_firmware_readb(isp, 0x1e05b));
	ISP_PRINT(ISP_INFO, "sensor address3 1e05c IS %#x\n",isp_firmware_readb(isp, 0x1e05c));
	ISP_PRINT(ISP_INFO, "sensor address 1e05d IS %#x\n",isp_firmware_readb(isp, 0x1e05d));
	ISP_PRINT(ISP_INFO, "sensor address4 1e05e IS %#x\n",isp_firmware_readb(isp, 0x1e05e));
	ISP_PRINT(ISP_INFO, "sensor address 1e05f IS %#x\n",isp_firmware_readb(isp, 0x1e05f));
	ISP_PRINT(ISP_INFO, "sensor address5 1e060 IS %#x\n",isp_firmware_readb(isp, 0x1e060));
	ISP_PRINT(ISP_INFO, "sensor address 1e061 IS %#x\n",isp_firmware_readb(isp, 0x1e061));
	ISP_PRINT(ISP_INFO, "sensor address6 1e062 IS %#x\n",isp_firmware_readb(isp, 0x1e062));
	ISP_PRINT(ISP_INFO, "sensor address 1e063 IS %#x\n",isp_firmware_readb(isp, 0x1e063));
	ISP_PRINT(ISP_INFO, "sensor address 1e064 IS %#x\n",isp_firmware_readb(isp, 0x1e064));
	ISP_PRINT(ISP_INFO, "sensor address 1e065 IS %#x\n",isp_firmware_readb(isp, 0x1e065));
	ISP_PRINT(ISP_INFO, "sensor address 1e066 IS %#x\n",isp_firmware_readb(isp, 0x1e066));
	ISP_PRINT(ISP_INFO, "sensor address 1e067 IS %#x\n",isp_firmware_readb(isp, 0x1e067));
	ISP_PRINT(ISP_INFO, "sensor address 1e068 IS %#x\n",isp_firmware_readb(isp, 0x1e068));
	ISP_PRINT(ISP_INFO, "sensor address 1e069 IS %#x\n",isp_firmware_readb(isp, 0x1e069));
	ISP_PRINT(ISP_INFO, "sensor address en bit 1e070 IS %#x\n",isp_firmware_readb(isp, 0x1e070));
	ISP_PRINT(ISP_INFO, "sensor address en bit 1e071 IS %#x\n",isp_firmware_readb(isp, 0x1e071));
	ISP_PRINT(ISP_INFO, "sensor address en bit 1e072 IS %#x\n",isp_firmware_readb(isp, 0x1e072));
	ISP_PRINT(ISP_INFO, "sensor address en bit 1e073 IS %#x\n",isp_firmware_readb(isp, 0x1e073));
	ISP_PRINT(ISP_INFO, "sensor address en bit 1e074 IS %#x\n",isp_firmware_readb(isp, 0x1e074));
	ISP_PRINT(ISP_INFO, "sensor address en bit 1e075 IS %#x\n",isp_firmware_readb(isp, 0x1e075));
	ISP_PRINT(ISP_INFO, "sensor address en bit 1e076 IS %#x\n",isp_firmware_readb(isp, 0x1e076));
}

void dump_isp_exposure_init(struct isp_device *isp)
{
	ISP_PRINT(ISP_INFO," 0x65000 IS %#x\n", isp_reg_readb(isp, 0x65000));
	ISP_PRINT(ISP_INFO," 0x65001 IS %#x\n", isp_reg_readb(isp, 0x65001));
	ISP_PRINT(ISP_INFO," 0x65002 IS %#x\n", isp_reg_readb(isp, 0x65002));
	ISP_PRINT(ISP_INFO," 0x65003 IS %#x\n", isp_reg_readb(isp, 0x65003));
	ISP_PRINT(ISP_INFO," 0x65004 IS %#x\n", isp_reg_readb(isp, 0x65004));
	ISP_PRINT(ISP_INFO," 0x65005 IS %#x\n", isp_reg_readb(isp, 0x65005));
	ISP_PRINT(ISP_INFO," 0x6502f IS %#x\n", isp_reg_readb(isp, 0x6502f));
	ISP_PRINT(ISP_INFO," 0x1e010 IS %#x\n", isp_firmware_readb(isp, 0x1e010));
	ISP_PRINT(ISP_INFO," 0x1e012 IS %#x\n", isp_firmware_readb(isp, 0x1e012));
	ISP_PRINT(ISP_INFO," 0x1e014 IS %#x\n", isp_firmware_readb(isp, 0x1e014));
	ISP_PRINT(ISP_INFO," 0x1e015 IS %#x\n", isp_firmware_readb(isp, 0x1e015));
	ISP_PRINT(ISP_INFO," 0x1e01a IS %#x\n", isp_firmware_readb(isp, 0x1e01a));
	ISP_PRINT(ISP_INFO," 0x1e01b IS %#x\n", isp_firmware_readb(isp, 0x1e01b));
	ISP_PRINT(ISP_INFO," 0x1e024 IS %#x\n", isp_firmware_readb(isp, 0x1e024));
	ISP_PRINT(ISP_INFO," 0x1e025 IS %#x\n", isp_firmware_readb(isp, 0x1e025));
	ISP_PRINT(ISP_INFO," 0x1e026 IS %#x\n", isp_firmware_readb(isp, 0x1e026));
	ISP_PRINT(ISP_INFO," 0x1e027 IS %#x\n",	isp_firmware_readb(isp, 0x1e027));
	ISP_PRINT(ISP_INFO," 0x1e028 IS	%#x\n",	isp_firmware_readb(isp, 0x1e028));
	ISP_PRINT(ISP_INFO," 0x1e029 IS %#x\n", isp_firmware_readb(isp, 0x1e029));
	ISP_PRINT(ISP_INFO," 0x1e02a IS %#x\n", isp_firmware_readb(isp, 0x1e02a));
	ISP_PRINT(ISP_INFO," 0x1e02b IS %#x\n", isp_firmware_readb(isp, 0x1e02b));
	ISP_PRINT(ISP_INFO," 0x1e02c IS %#x\n", isp_firmware_readb(isp, 0x1e02c));
	ISP_PRINT(ISP_INFO," 0x1e02d IS %#x\n", isp_firmware_readb(isp, 0x1e02d));
	ISP_PRINT(ISP_INFO," 0x1e02e IS %#x\n", isp_firmware_readb(isp, 0x1e02e));
	ISP_PRINT(ISP_INFO," 0x1e02f IS %#x\n", isp_firmware_readb(isp, 0x1e02f));
	ISP_PRINT(ISP_INFO," 0x1e048 IS %#x\n", isp_firmware_readb(isp, 0x1e048));
	ISP_PRINT(ISP_INFO," 0x1e049 IS %#x\n", isp_firmware_readb(isp, 0x1e049));
	ISP_PRINT(ISP_INFO," 0x1e04a IS %#x\n", isp_firmware_readb(isp, 0x1e04a));
	ISP_PRINT(ISP_INFO," 0x1e04f IS %#x\n", isp_firmware_readb(isp, 0x1e04f));
	ISP_PRINT(ISP_INFO," 0x1e050 IS %#x\n", isp_firmware_readb(isp, 0x1e050));
	ISP_PRINT(ISP_INFO," 0x1e051 IS %#x\n", isp_firmware_readb(isp, 0x1e051));
	ISP_PRINT(ISP_INFO," 0x1e04c IS %#x\n", isp_firmware_readb(isp, 0x1e04c));
	ISP_PRINT(ISP_INFO," 0x1e04d IS %#x\n", isp_firmware_readb(isp, 0x1e04d));
	ISP_PRINT(ISP_INFO," 0x1e013 IS %#x\n", isp_firmware_readb(isp, 0x1e013));
	ISP_PRINT(ISP_INFO," 0x1e056 IS %#x\n", isp_firmware_readb(isp, 0x1e056));
	ISP_PRINT(ISP_INFO," 0x1e057 IS %#x\n", isp_firmware_readb(isp,0x1e057));
	ISP_PRINT(ISP_INFO," 0x1e058 IS %#x\n", isp_firmware_readb(isp,0x1e058));
	ISP_PRINT(ISP_INFO," 0x1e059 IS %#x\n", isp_firmware_readb(isp,0x1e059));
	ISP_PRINT(ISP_INFO," 0x1e05a IS %#x\n", isp_firmware_readb(isp,0x1e05a));
	ISP_PRINT(ISP_INFO," 0x1e05b IS %#x\n", isp_firmware_readb(isp,0x1e05b));
	ISP_PRINT(ISP_INFO," 0x1e05c IS %#x\n", isp_firmware_readb(isp,0x1e05c));
	ISP_PRINT(ISP_INFO," 0x1e05d IS %#x\n", isp_firmware_readb(isp,0x1e05d));
	ISP_PRINT(ISP_INFO," 0x1e05e IS %#x\n", isp_firmware_readb(isp,0x1e05e));
	ISP_PRINT(ISP_INFO," 0x1e05f IS %#x\n", isp_firmware_readb(isp,0x1e05f));
	ISP_PRINT(ISP_INFO," 0x1e060 IS %#x\n", isp_firmware_readb(isp,0x1e060));
	ISP_PRINT(ISP_INFO," 0x1e061 IS %#x\n", isp_firmware_readb(isp,0x1e061));
	ISP_PRINT(ISP_INFO," 0x1e062 IS %#x\n", isp_firmware_readb(isp,0x1e062));
	ISP_PRINT(ISP_INFO," 0x1e063 IS %#x\n", isp_firmware_readb(isp,0x1e063));
	ISP_PRINT(ISP_INFO," 0x1e064 IS %#x\n", isp_firmware_readb(isp,0x1e064));
	ISP_PRINT(ISP_INFO," 0x1e065 IS %#x\n", isp_firmware_readb(isp,0x1e065));
	ISP_PRINT(ISP_INFO," 0x1e066 IS %#x\n", isp_firmware_readb(isp,0x1e066));
	ISP_PRINT(ISP_INFO," 0x1e067 IS %#x\n", isp_firmware_readb(isp,0x1e067));
	ISP_PRINT(ISP_INFO," 0x1e070 IS %#x\n", isp_firmware_readb(isp,0x1e070));
	ISP_PRINT(ISP_INFO," 0x1e071 IS %#x\n", isp_firmware_readb(isp,0x1e071));
	ISP_PRINT(ISP_INFO," 0x1e072 IS %#x\n", isp_firmware_readb(isp,0x1e072));
	ISP_PRINT(ISP_INFO," 0x1e073 IS %#x\n", isp_firmware_readb(isp,0x1e073));
	ISP_PRINT(ISP_INFO," 0x1e074 IS %#x\n", isp_firmware_readb(isp,0x1e074));
	ISP_PRINT(ISP_INFO," 0x1e075 IS %#x\n", isp_firmware_readb(isp,0x1e075));
	ISP_PRINT(ISP_INFO," 0x1e076 IS %#x\n", isp_firmware_readb(isp,0x1e076));
	ISP_PRINT(ISP_INFO," 0x1e077 IS %#x\n", isp_firmware_readb(isp,0x1e077));
	ISP_PRINT(ISP_INFO," 0x66501 IS %#x\n", isp_reg_readb(isp, 0x66501));
	ISP_PRINT(ISP_INFO," 0x66502 IS %#x\n", isp_reg_readb(isp, 0x66502));
	ISP_PRINT(ISP_INFO," 0x66503 IS %#x\n", isp_reg_readb(isp, 0x66503));
	ISP_PRINT(ISP_INFO," 0x66504 IS %#x\n", isp_reg_readb(isp, 0x66504));
	ISP_PRINT(ISP_INFO," 0x66505 IS %#x\n", isp_reg_readb(isp, 0x66505));
	ISP_PRINT(ISP_INFO," 0x66506 IS %#x\n", isp_reg_readb(isp, 0x66506));
	ISP_PRINT(ISP_INFO," 0x66507 IS %#x\n", isp_reg_readb(isp, 0x66507));
	ISP_PRINT(ISP_INFO," 0x66508 IS %#x\n", isp_reg_readb(isp, 0x66508));
	ISP_PRINT(ISP_INFO," 0x66509 IS %#x\n", isp_reg_readb(isp, 0x66509));
	ISP_PRINT(ISP_INFO," 0x6650a IS %#x\n", isp_reg_readb(isp, 0x6650a));
	ISP_PRINT(ISP_INFO," 0x6650b IS %#x\n", isp_reg_readb(isp, 0x6650b));
	ISP_PRINT(ISP_INFO," 0x6650c IS %#x\n", isp_reg_readb(isp, 0x6650c));
	ISP_PRINT(ISP_INFO," 0x6650d IS %#x\n", isp_reg_readb(isp, 0x6650d));
	ISP_PRINT(ISP_INFO," 0x6650e IS %#x\n", isp_reg_readb(isp, 0x6650e));
	ISP_PRINT(ISP_INFO," 0x6650f IS %#x\n", isp_reg_readb(isp, 0x6650f));
	ISP_PRINT(ISP_INFO," 0x66510 IS %#x\n", isp_reg_readb(isp, 0x66510));
	ISP_PRINT(ISP_INFO," 0x6651c IS %#x\n", isp_reg_readb(isp, 0x6651c));
	ISP_PRINT(ISP_INFO," 0x6651d IS %#x\n", isp_reg_readb(isp, 0x6651d));
	ISP_PRINT(ISP_INFO," 0x6651e IS %#x\n", isp_reg_readb(isp, 0x6651e));
	ISP_PRINT(ISP_INFO," 0x6651f IS %#x\n", isp_reg_readb(isp, 0x6651f));
	ISP_PRINT(ISP_INFO," 0x66520 IS %#x\n", isp_reg_readb(isp, 0x66520));
	ISP_PRINT(ISP_INFO," 0x66521 IS %#x\n", isp_reg_readb(isp, 0x66521));
	ISP_PRINT(ISP_INFO," 0x66522 IS %#x\n", isp_reg_readb(isp, 0x66522));
	ISP_PRINT(ISP_INFO," 0x66523 IS %#x\n", isp_reg_readb(isp, 0x66523));
	ISP_PRINT(ISP_INFO," 0x66524 IS %#x\n", isp_reg_readb(isp, 0x66524));
	ISP_PRINT(ISP_INFO," 0x66525 IS %#x\n", isp_reg_readb(isp, 0x66525));
	ISP_PRINT(ISP_INFO," 0x66526 IS %#x\n", isp_reg_readb(isp, 0x66526));
	ISP_PRINT(ISP_INFO," 0x66527 IS %#x\n", isp_reg_readb(isp, 0x66527));
	ISP_PRINT(ISP_INFO," 0x66528 IS %#x\n", isp_reg_readb(isp, 0x66528));
	ISP_PRINT(ISP_INFO," 0x66529 IS %#x\n", isp_reg_readb(isp, 0x66529));
	ISP_PRINT(ISP_INFO," 0x6652a IS %#x\n", isp_reg_readb(isp, 0x6652a));
	ISP_PRINT(ISP_INFO," 0x6652c IS %#x\n", isp_reg_readb(isp, 0x6652c));
	ISP_PRINT(ISP_INFO," 0x6652c IS %#x\n", isp_reg_readb(isp, 0x6652c));
	ISP_PRINT(ISP_INFO," 0x6652d IS %#x\n", isp_reg_readb(isp, 0x6652d));
	ISP_PRINT(ISP_INFO," 0x6652e IS %#x\n", isp_reg_readb(isp, 0x6652e));
	ISP_PRINT(ISP_INFO," 0x6652f IS %#x\n", isp_reg_readb(isp, 0x6652f));
	ISP_PRINT(ISP_INFO," 0x66530 IS %#x\n", isp_reg_readb(isp, 0x66530));
	ISP_PRINT(ISP_INFO," 0x66531 IS %#x\n", isp_reg_readb(isp, 0x66531));
	ISP_PRINT(ISP_INFO," 0x66532 IS %#x\n", isp_reg_readb(isp, 0x66532));
	ISP_PRINT(ISP_INFO," 0x66533 IS %#x\n", isp_reg_readb(isp, 0x66533));
	ISP_PRINT(ISP_INFO," 0x1e022 IS %#x\n", isp_firmware_readb(isp, 0x1e022));
	ISP_PRINT(ISP_INFO," 0x1e030 IS %#x\n", isp_firmware_readb(isp, 0x1e030));
	ISP_PRINT(ISP_INFO," 0x1e031 IS %#x\n", isp_firmware_readb(isp, 0x1e031));
	ISP_PRINT(ISP_INFO," 0x1e032 IS %#x\n", isp_firmware_readb(isp, 0x1e032));
	ISP_PRINT(ISP_INFO," 0x1e033 IS %#x\n", isp_firmware_readb(isp, 0x1e033));
	ISP_PRINT(ISP_INFO," 0x1e034 IS %#x\n", isp_firmware_readb(isp, 0x1e034));
	ISP_PRINT(ISP_INFO," 0x1e035 IS %#x\n",	isp_firmware_readb(isp, 0x1e035));
}
#if 0
void dump_sensor_exposure(struct v4l2_subdev *sd)
{
	int ret;
	unsigned char val;

	ret = ov5645_read(sd, 0x3500, &val);
	ISP_PRINT(ISP_INFO, "OV5645 manual en 0x3500 is %#x\n", val);
	ret = ov5645_read(sd, 0x3501, &val);
	ISP_PRINT(ISP_INFO, "OV5645 manual en 0x3501 is %#x\n", val);
	ret = ov5645_read(sd, 0x3502, &val);
	ISP_PRINT(ISP_INFO, "OV5645 manual en 0x3502 is %#x\n", val);
	ret = ov5645_read(sd, 0x3503, &val);
	ISP_PRINT(ISP_INFO, "OV5645 manual en 0x3503 is %#x\n", val);
	ret = ov5645_read(sd, 0x3504, &val);
	ISP_PRINT(ISP_INFO, "OV5645 manual en 0x3504 is %#x\n", val);
	ret = ov5645_read(sd, 0x3505, &val);
	ISP_PRINT(ISP_INFO, "OV5645 manual en 0x3505 is %#x\n", val);
	ret = ov5645_read(sd, 0x3506, &val);
	ISP_PRINT(ISP_INFO, "OV5645 manual en 0x3506 is %#x\n", val);
	ret = ov5645_read(sd, 0x3507, &val);
	ISP_PRINT(ISP_INFO, "OV5645 manual en 0x3507 is %#x\n", val);
	ret = ov5645_read(sd, 0x3508, &val);
	ISP_PRINT(ISP_INFO, "OV5645 manual en 0x3508 is %#x\n", val);
	ret = ov5645_read(sd, 0x3509, &val);
	ISP_PRINT(ISP_INFO, "OV5645 manual en 0x3509 is %#x\n", val);
	ret = ov5645_read(sd, 0x350a, &val);
	ISP_PRINT(ISP_INFO, "OV5645 manual en 0x350a is %#x\n", val);
	ret = ov5645_read(sd, 0x350b, &val);
	ISP_PRINT(ISP_INFO, "OV5645 manual en 0x350b is %#x\n", val);
}
#else
void dump_sensor_exposure(struct v4l2_subdev *sd)
{}
#endif

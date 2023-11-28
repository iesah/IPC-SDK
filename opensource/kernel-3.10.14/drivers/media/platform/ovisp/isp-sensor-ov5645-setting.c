#include "ovisp-base.h"

void isp_setting_init(struct isp_device *isp)
{
	isp_reg_writeb(isp, 0x3f, 0x65000);
#if 0
	isp_reg_writeb(isp, 0xef, 0x65001);
	isp_reg_writeb(isp, 0x25, 0x65002);
	isp_reg_writeb(isp, 0xff, 0x65003);
	isp_reg_writeb(isp, 0x30, 0x65004);
	isp_reg_writeb(isp, 0x14, 0x65005);
	isp_reg_writeb(isp, 0xc0, 0x6502f);

	isp_firmware_writeb(isp, 0x01, 0x1e010);
	isp_firmware_writeb(isp, 0x02, 0x1e012);
	isp_firmware_writeb(isp, 0x30, 0x1e014);
	isp_firmware_writeb(isp, 0x35, 0x1e015);
	isp_firmware_writeb(isp, 0x0a, 0x1e01a);
	isp_firmware_writeb(isp, 0x08, 0x1e01b);
	isp_firmware_writeb(isp, 0x00, 0x1e024);
	isp_firmware_writeb(isp, 0xff, 0x1e025);
	isp_firmware_writeb(isp, 0x00, 0x1e026);
	isp_firmware_writeb(isp, 0x10, 0x1e027);
	isp_firmware_writeb(isp, 0x00, 0x1e028);
	isp_firmware_writeb(isp, 0x00, 0x1e029);
	isp_firmware_writeb(isp, 0x03, 0x1e02a);
	isp_firmware_writeb(isp, 0xd8, 0x1e02b);
	isp_firmware_writeb(isp, 0x00, 0x1e02c);
	isp_firmware_writeb(isp, 0x00, 0x1e02d);
	isp_firmware_writeb(isp, 0x00, 0x1e02e);
	isp_firmware_writeb(isp, 0x10, 0x1e02f);
	isp_firmware_writeb(isp, 0x00, 0x1e048);
	isp_firmware_writeb(isp, 0x01, 0x1e049);
	isp_firmware_writeb(isp, 0x01, 0x1e04a);
	isp_firmware_writeb(isp, 0xe8, 0x1e04f);
	isp_firmware_writeb(isp, 0x18, 0x1e050);
	isp_firmware_writeb(isp, 0x1e, 0x1e051);
	isp_firmware_writeb(isp, 0x03, 0x1e04c);
	isp_firmware_writeb(isp, 0xd8, 0x1e04d);
	isp_firmware_writeb(isp, 0x08, 0x1e013);

	isp_firmware_writeb(isp, 0x78, 0x1e056);

	isp_firmware_writeb(isp, 0x09, 0x1e057);
	isp_firmware_writeb(isp, 0x35, 0x1e058);
	isp_firmware_writeb(isp, 0x00, 0x1e059);
	isp_firmware_writeb(isp, 0x35, 0x1e05a);
	isp_firmware_writeb(isp, 0x01, 0x1e05b);
	isp_firmware_writeb(isp, 0x35, 0x1e05c);
	isp_firmware_writeb(isp, 0x02, 0x1e05d);
	isp_firmware_writeb(isp, 0x00, 0x1e05e);
	isp_firmware_writeb(isp, 0x00, 0x1e05f);
	isp_firmware_writeb(isp, 0x00, 0x1e060);
	isp_firmware_writeb(isp, 0x00, 0x1e061);
	isp_firmware_writeb(isp, 0x00, 0x1e062);
	isp_firmware_writeb(isp, 0x00, 0x1e063);
	isp_firmware_writeb(isp, 0x35, 0x1e064);
	isp_firmware_writeb(isp, 0x0a, 0x1e065);
	isp_firmware_writeb(isp, 0x35, 0x1e066);
	isp_firmware_writeb(isp, 0x0b, 0x1e067);
	isp_firmware_writeb(isp, 0xff, 0x1e070);
	isp_firmware_writeb(isp, 0xff, 0x1e071);
	isp_firmware_writeb(isp, 0xff, 0x1e072);
	isp_firmware_writeb(isp, 0x00, 0x1e073);
	isp_firmware_writeb(isp, 0x00, 0x1e074);
	isp_firmware_writeb(isp, 0x00, 0x1e075);
	isp_firmware_writeb(isp, 0xff, 0x1e076);
	isp_firmware_writeb(isp, 0xff, 0x1e077);

	isp_reg_writeb(isp, 0x00, 0x66501);
	isp_reg_writeb(isp, 0x00, 0x66502);
	isp_reg_writeb(isp, 0x00, 0x66503);
	isp_reg_writeb(isp, 0x00, 0x66504);
	isp_reg_writeb(isp, 0x00, 0x66505);
	isp_reg_writeb(isp, 0x00, 0x66506);
	isp_reg_writeb(isp, 0x00, 0x66507);
	isp_reg_writeb(isp, 0x80, 0x66508);
	isp_reg_writeb(isp, 0x00, 0x66509);
	isp_reg_writeb(isp, 0xc8, 0x6650a);
	isp_reg_writeb(isp, 0x00, 0x6650b);
	isp_reg_writeb(isp, 0x96, 0x6650c);
	isp_reg_writeb(isp, 0x04, 0x6650d);
	isp_reg_writeb(isp, 0xb0, 0x6650e);
	isp_reg_writeb(isp, 0x01, 0x6650f);
	isp_reg_writeb(isp, 0x00, 0x66510);
	isp_reg_writeb(isp, 0x01, 0x6651c);
	isp_reg_writeb(isp, 0x01, 0x6651d);
	isp_reg_writeb(isp, 0x01, 0x6651e);
	isp_reg_writeb(isp, 0x01, 0x6651f);
	isp_reg_writeb(isp, 0x02, 0x66520);
	isp_reg_writeb(isp, 0x02, 0x66521);
	isp_reg_writeb(isp, 0x02, 0x66522);
	isp_reg_writeb(isp, 0x02, 0x66523);
	isp_reg_writeb(isp, 0x04, 0x66524);
	isp_reg_writeb(isp, 0x02, 0x66525);
	isp_reg_writeb(isp, 0x02, 0x66526);
	isp_reg_writeb(isp, 0x02, 0x66527);
	isp_reg_writeb(isp, 0x02, 0x66528);
	isp_reg_writeb(isp, 0x04, 0x66529);
	isp_reg_writeb(isp, 0xf0, 0x6652a);
	isp_reg_writeb(isp, 0x2b, 0x6652c);

	isp_reg_writeb(isp, 0x2b, 0x6652c);

	isp_reg_writeb(isp, 0x2c, 0x6652d);
	isp_reg_writeb(isp, 0x2d, 0x6652e);
	isp_reg_writeb(isp, 0x2e, 0x6652f);
	isp_reg_writeb(isp, 0x2f, 0x66530);
	isp_reg_writeb(isp, 0x0a, 0x66531);
	isp_reg_writeb(isp, 0x14, 0x66532);
	isp_reg_writeb(isp, 0x14, 0x66533);

	isp_firmware_writeb(isp, 0x00, 0x1e022);
	isp_firmware_writeb(isp, 0x00, 0x1e030);
	isp_firmware_writeb(isp, 0x00, 0x1e031);
	isp_firmware_writeb(isp, 0x34, 0x1e032);
	isp_firmware_writeb(isp, 0x60, 0x1e033);
	isp_firmware_writeb(isp, 0x00, 0x1e034);
	isp_firmware_writeb(isp, 0x40, 0x1e035);
#endif

}



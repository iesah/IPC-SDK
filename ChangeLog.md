Attension please:
        when you update the ISP driver, you should also update the SDK synchronously.

#ChangeLog ISVP-T40-1.2.0

---
## [doc]
* Update function doc
* Update T40开发指南.doc
* Update T40码流控制说明.pdf
* Add T系列GPIO寄存器操作说明.pdf
* Add 调试手册.pdf
* Add Ingenic T40算法多进程使用指南.pdf

## [tools]
* Update sensor bin file
* Add some new bin file
* Update USB clone tool
* Update Carrier-V5.0.0
* Update T40 image
* Add image make script
* Update T40 toolchain to toolchain720_r511 versioni，Compatible with toolchain720_ r400 version
* Update HDK to T40_HDK_20220413.rar

## [u-boot]
* Optimization some code
* Repair uboot password can't set mistake
* Repair nor flash quad mode bug
* Add nand flash F50L1G41LB   support
* Optimize system stability
* Add  LCD display funcition
* Add msc1 compilation options

## [Drivers]
* Update ISP driver
* ISP add get_raw interface to the ISP driver
* Modify isp osd func to the ISP driver
* Add IMP_ISP_SetPreDqtime Interfaces to the ISP driver
* Add some debug infomation to the ISP driver
* Fixed LCE function bug to the ISP driver
* Add API Set sclaer level to the ISP
* Optimization AE function to the ISP driver
* Add a defog interface to the ISP driver
* Optimize the three camera and bypass processes
* Add a HLDC interface to the ISP driver
* Fix the invalid problem of calling the antiflicker interface immediately after day and night vision switching
* Reapir AVPU clk bug
* Fix ae and defog params switch issue on wdr mode
* Fixed the ae mannual bug on dual mode
* Fixed the day or night mode bug
* Repair the camera WDR Compression curve of isp
* Optimize AF interface :IMP_ISP_Tuning_GetAFMetricesInfo and IMP_ISP_Tuning_GetStatisConfig
* Add ae stable interface
* Optimization code of ISP driver
* Modify some bug of ISP driver
* Fixed soc-nna memory mistake
* Update sensor drivers
* Support sensor list: bf20a1 bf2253 c2399 c4390 gc2053 gc2093 gc4023 gc4653 hi556 imx307 imx327 imx334 imx335 imx415 jxf23 jxf23s jxf23-s0 jxf23-s1 jxf32 jxf35
                       imx415 jxf37 jxh63 jxk04 jxk08 jxq03 mis2006 mis2008 mis2031 mis8001 os04b10 os05a10 os08a10 ov01a1s ov2740 ov9732 ps5258 ps5260 sc200ai
                       sc201cs sc2232h sc2239 sc223a sc2310 sc2335 sc301IoT sc3235 sc3335 sc3336 sc401ai sc4236 sc4335 sc500ai sc5235 sc5239 sc8235 sc8238 sc830ai

## [Kernel]
* Repair some bug of IPU
* Repair Nand drivers bug
* Optimization nor flash 64k erase option
* Fixed rootfs bug of ranmdisk
* Optimization Kernel code

## [SDK]
* Modify AE interface to the ISP
* Add a new interface of defog to ths isp
* Add a new interface of HLDC to ths isp
* Add a new interface of DRC or DPC to ths isp
* fixed the alloc memory leak of ISP
* Optimize AE interfaces:IMP_ISP_Tuning_GetAFMetricesInfo or IMP_ISP_Tuning_GetStatisConfig
* Add ae stable api
* Optimization code of ISP
* Optimization ISP code
* Optimize sample
* Modify ivs_base_move bug
* Modify H265 sps_num_recoder_picIs
* Resolve the deadlock caused by destroying the encoding channel
* Add OSD anti color interface
* Modify the problem that the coding code rate is low for a long time during scene switching, dynamic switching to static or night vision switching
* Fix the problem of incomplete system de initialization
* Add multi process stream fetching / releasing interface
* Resolve code assertion errors caused by frequent switching of iminqp and imaxqp
* Solve the problem of incomplete code de initialization
* Add encoding alcodec to process user data without binding sample
* Add isp osd function
* Repair bug of encoder
* Repair osd memory manger bug
* Optimization software of encoder
* Fix I2D bug


#ChangeLog ISVP-T40-1.1.1

---
## [doc]
* Update function doc
* Update T40开发指南.doc
* Update T40码流控制说明.pdf
* Add new functions doc
* Add 调试手册.pdf

## [tools]
* Update sensor bin file
* Add some new bin file
* Add USB cloner tool
* Update T40 HDK
* Update T40 image
* Update T40 wifi drivers
* Add image make script

## [u-boot]
* Optimization some code
* Repair uboot password can't set mistake
* Repair nor flash quad mode bug
* Add nand flash GD5F2GM7UE XT26G01C support
* Add nor flash W25Q512JV XM25QU128C XM25Q256C GD25Q256D/E MX25L25645G/35F MX25L51245G XM25QH***C support
* Modify fw_printenv can't read nor env mistake
* Modify some bug
* Optimize system stability

## [Drivers]
* Update ISP driver
* Fixed sample_motor3 mistake
* Add API Set sclaer level of ISP
* Fixed the ae mannual bug on dual mode
* Fixed the day or night mode bug
* Repair the camera ADR exception of isp
* Added ISP debug for the second camera
* Added ISP parent clock switchover
* Added ISP option isp_memopt for memory clipping
* Fixed mirror and flip mode for isp
* Add isp osd function
* Optimization code of ISP driver
* Modify some bug of ISP driver
* Add some new functions of isp driver
* Fixed Audio AI recording volume bug
* Fixed DES driver dma type mistake
* Repair PWM driver
* Fixed soc-nna memory mistake
* Update sensor drivers
* Support sensor list: bf20a1 bf2253 c2399 c4390 gc2053 gc2093 gc4653 imx327 imx335 imx415 jxf23 jxf23s jxf23-s0 jxf23-s1 jxf32 jxf35 jxf37 jxh63 jxk04 jxq03
                       mis2006 mis2008 mis2031 mis8001 os04b10 os05a10 os08a10 ov9732 ps5258 ps5260 sc200ai sc201cs sc2232h sc2239 sc223a sc2310 sc2335 sc3235 
                       sc3335 sc4236 sc4335 sc500ai sc5235 sc5239 sc8235 sc8238 sc830ai

## [Kernel]
* Add nand flash GD5F2GM7UE XT26G01C support
* Add nor flash W25Q512JV XM25QU128C XM25Q256C GD25Q256D/E MX25L25645G/35F MX25L51245G XM25QH***C support
* isvp_shark_nand_defconfig and hardware LZMA config
* Add gmac ethtool function
* Fixed rootfs bug of randdisk
* Reapir overlay overrun problem of ipu driver
* The MMC power control function is turned off by default
* Repair i2d bug of argb8888
* Repair i2d driver mistake
* Optimization Kernel code

## [SDK]
* Modify the switch bin interface of ISP
* Add a new interface of scaler lervel of isp
* Optimization code of ISP
* Fixed the following interfaces of ISP
* Add IMP_ISP_GetCameraInputMode interface
* Optimization the Mask function of ISP
* Added start night mode of ISP
* Added the Sensor mirror and flip mode of ISP
* Add sample_soft_photosensitive_ctrl demo in sample-comon.c
* Optimization ISP code
* add error code for sdk of isp
* Repair i2d mistake
* Repair bug of encoder
* Repair bug of IPU truncation
* Repair osd memory manger bug
* Optimization software of encoder
* Remove ROI function of encoder
* Added custom SEI set of encoder
* Fix the problem of 16 alignment. At present, only 8 alignment is required for coding and ISP
* Optimize encoder bug
* Add API int IMP_Encoder_SetFrameRelease(int encChn, int num, int den) of encoder
* Repair memory is released in advance of aduio
* Modify audio sample
* Optimization framesource debug log
* Delete useless code


#ChangeLog ISVP-T40-1.1.0

---
## [doc]
* Update function doc
* Update T40开发指南-20210831.doc
* Update T40码流控制说明-20210617.pdf

## [tools]
* Update sensor bin file
* Add some new bin file
* Update rootfs to support udhcpc
* Update T40 HDK
* Update T40 image
* Modify init script
* Add opensource library of mtd_utils

## [u-boot]
* Optimization some code
* Add nand flash F50L2G41XA F35SQA001G MT29F2G01A MT29F2G01A support
* Add nor flash M25W128 EN25QH256A GM25Q64A PY25Q128HA support
* Modify some bug
* Add device tree separate complier,open marc CONFIG_OF_LIBFDT in isvp_t40.h
* Optimize system stability

## [Drivers]
* Update ISP driver
* Add new interface can set bin file path of isp
* Fixed the ae exposure by min it step issue of isp
* Fixed the oom issue caused by ydns and adr of isp
* Add debug of ISP
* Add RGB color input for the Mask of isp
* Fixed the ae and awb algo func issue when stop process
* Modify ae/awb algo functions of isp
* Modify the ae/awb/clm/ccm reg write trig of isp
* Fixed the low fps issue on dual sensor mode of isp
* Fixed the secondary start AWB exception of isp
* Optimization code of ISP driver
* Modify some bug of ISP driver
* Add some new functions of isp driver
* Fix pwm bug of can set the duty cycle is 0%
* Add the protection mechanism,avoid the spk sync error when the DMA get adnormal address of the current transmissing of audio
* Repair t40 can't rmmod bug
* Repair rmmod soc-nna mistake
* Update sensor drivers
* Support sensor list: c4390 gc2053 gc2093 gc4653 imx327 imx335 imx415 jxf23s jxf23-s0 jxf23-s1 jxf32 jxf35 jxf37 jxh63 jxk04 jxq03
                       mis2006 os04b10 os05a10 os08a10 ov9732 ps5258 ps5260 sc200ai sc201cs sc2232h sc2239 sc2310 sc2335 sc3235 sc3335
                       sc4236 sc4335 sc500ai sc5235 sc8238

## [Kernel]
* Add nand flash F50L2G41XA F35SQA001G MT29F2G01A MT29F2G01A support
* Add nor flash M25W128 EN25QH256A GM25Q64A PY25Q128HA support
* Add network multicast
* Add gmac ethtool function
* Add ma0060 panel driver of LCD
* Reapir GPIO shadow smp operation bug
* Delete useless config file
* Optimization Kernel code
* Add J40 support of LCD

## [SDK]
* Add set bin file path API
* Modify the switch bin interface
* Add RGB color input for the Mask of isp
* Modify ae/awb algo functionb
* Add error code for SDK of isp
* Repair bug of ISP
* Optimization code of ISP
* Support drawing slash of OSD
* Modify OSD code of line and rect beyond boundary
* Add gopmode IMP_ENC_GOP_CTRL_MODE_SMARTP of encoder
* Add soft scaler function of encoder
* Add ROI function of encoder
* Add some encoder parameters set API of encoder
* Fix memory leak in encoding shared channel
* Optimize encoder bug
* Add audio impdbg
* Repair memory is released in advance of aduio
* Modify audio sample
* Add sample-Fcrop
* Delete useless code
* Optimization ISP code


#ChangeLog ISVP-T40-1.0.4

---
## [doc]
* Add some function doc
* Update T40开发指南-20210617.doc
* Update Encoder 编码 API 参考

## [tools]
* Update sensor bin file
* Add some new bin file
* Update WDR effect
* Update T40 HDK
* Update T40 image
* Modify init script
* Add yaffs2 filesystem tools

## [u-boot]
* Optimization some code
* Modify PLL set
* Modify some bug
* Repair mbr-gpt error
* Add nor flash NM25Q128EVB
* Repair sz18201 link bug
* Support usb ethernet device(Asix)
* Add device tree separate complier,open marc CONFIG_OF_LIBFDT in isvp_t40.h
* Support mtdparts partition table of nand
* Modify GD5F nand flash code

## [Drivers]
* Update ISP driver
* Fix some pwm bug
* Fix bug adr deflash of isp driver
* Add 3th custom 3A library support
* Fix the awb set array issue
* Fix the cdns/ydns have no effects on wdr mode
* T40 add single frame scheduling, support support any number nrvbs buffer
* Optimization sc8238 sensor driver
* Optimization code of ISP driver
* Modify some bug of ISP driver
* Add some new functions of isp driver
* Support sensor list: c4390 gc2053 gc2093 gc4653 imx327 imx335 imx415 jxf23-s0 jxf23-s1 jxf32 jxf35 jxf37 jxh63 jxk04 jxq03
                       mis2006 os04b10 os05a10 os08a10 ov9732 ps5258 ps5260 sc2232h sc2239 sc2310 sc2335 sc3235 sc3335
                       sc4236 sc4335 sc500ai sc5235 sc8238

## [Kernel]
* Add nor flash NM25Q128EVB support
* Add I2D driver
* Update dpu driver
* Add nand defconfig of isvp_shark_nand_defconfig
* Repair OSD bug when drawing a four-corner rect.
* Repair spi board device set frequency failed
* MMC driver add power control
* Repair ifconfig eth0 down  bug
* Can separate complier kernel and dtb
* Add filesystem support of yaffs2
* Add configuration of rmmod driver
* Add kernel mtdpards partition table support
* Midify GD5F nand flash code
* Delete some useless code
* Optimization Kernel code

## [SDK]
* Add raw8 bypass function
* Add smart ae api
* Add encodering process for encoder sample
* Repair bug of ISP
* Add sample-I2D
* Delete useless code
* Add face ae api
* Add face awb api
* Update sample_LCD.c
* Add some encoder parameters set API
* Add awb global statictis
* Add watchdog  and AutoZoom sample
* Optimization code to Solve snap framesource picture segment default
* Chang copy frame to fsdepth->frame's size,for better use mem
* Fix single frame scheduling bug
* Optimize audio sample,solve the segment fault while using AEC
* Add 3a custom library support
* Optimize encoder bug
* Modify some bug of ISP
* Optimization ISP code
* Add audio impdbg function
* Modify the sample of Audio


#ChangeLog ISVP-T40-1.0.3

---
## [doc]
* Update Audio API doc
* Add IPU API doc

## [tools]
* Update sensor bin file
* Add some new bin file
* Update rootfs of sensor cache
* Update T40 HDK
* Update T40 image
* Update webrtc profile doc
* Add T40 rmem calculate excel

## [u-boot]
* Optimization some code
* Modify PLL set
* Modify some bug
* Add nor flash IS25LP064D IS25LP128F SK25P128 ZD25Q64B EN25QX128A support
* Add nand flash GD5F1GQ5UE DS35Q1GA support
* Support card up and nand burning,support uboot command to modify partition table
* Optimization the code of nand partition table in uboot

## [Drivers]
* Update ISP driver
* Update VPU driver
* Update Audio driver
* Update wifi rtl8188ftv/rtl8189ftv drivers
* Add wifi Hi3881V100 and RTL8188GTV drivers
* Fix some audio bug
* Fix bug gamma of isp driver
* Fix bug awb of isp driver
* Fix bug af static of isp driver
* Fix bug second camera zoom in of isp driver
* Fix some bug of sensor
* Optimization motor driver of T40
* Delete some useless code of nna driver
* Optimization code of ISP driver
* Modify some bug of ISP driver
* Add some new functions of isp driver
* Add some new sensor drivers
* Optimization sensor driver
* Support sensor list: gc2053 gc2093 gc4653 imx327 imx335 imx415 jxf23-s0 jxf23-s1 jxf32 jxf35 jxf37 jxh63 jxk04 jxq03
                       mis2006 os04b10 os05a10 os08a10 ov9732 ps5258 ps5260 sc2232h sc2239 sc2310 sc2335 sc3235 sc3335
                       sc4236 sc4335 sc500ai sc5235 sc8238

## [Kernel]
* Add nor flash IS25LP064D IS25LP128F SK25P128 ZD25Q64B EN25QX128A support
* Add nand flash GD5F1GQ5UE DS35Q1GA support
* Modify msc0 clk from 50M to 25M
* Modify IPU drawbox driver
* Modify ADC driver
* Fix GD nand use spi standard mode
* Modify the clock of i2s for audio
* Modify gmac driver for can read gamc phy register
* Add node for watchdog to reset system
* Modify Kernel version from 4.4.94+ to 4.4.94
* Add configuration of rmmod driver
* Update t40 LCD driver
* Optimization Kernel deveice tree
* Delete some useless code
* Optimization Kernel code

## [SDK]
* Optimization isp interrupt
* Modify the command of audio sample
* Add encodering process for encoder sample
* Repair bug of audio
* Add ircut function code in sample-common.c
* Modify code of getcpuinfo
* Optimization encode code
* Add some encoder parameters set API
* Solve the problem caused by the starting point coordinate out of bounds when the OSD draw rect
* Add watchdog  and AutoZoom sample
* Optimization sample-comon.h and sample-comon.c to support up to 6 framesource channel
* Add snap raw and lcd sample
* Add rmem manger of IPU
* Optimize audio sample
* Optimize adc sample
* Optimize ISP-filp sample
* Optimize encoder bug
* Modify the ivs_move
* Modify the dual mode: add select mode and remove main cache mode
* Modify some bug of ISP
* Optimization ISP code
* Delete some useless code
* Modify Ivs_move and ivs_base_move bug


#ChangeLog ISVP-T40-1.0.2

---
## [doc]
* Add ISP API doc
* Update T40开发指南-20210202.doc

## [tools]
* Update sensor bin file
* Add some new bin file
* Update rootfs
* Update debug tools
* Update T40 image

## [u-boot]
* Optimization some code
* Modify PLL set
* Modify some bug
* Delete some useless code

## [Drivers]
* Update ISP driver
* Update VPU driver
* Update Audio driver
* Fix some audio bug
* Audio add excodec of ak7755
* Fix some bug of isp driver(clk)
* Fix some bug of sensor
* Update rsa driver
* Update PWM drivers
* Modify AVPU driver of pll set
* Add some new functions of isp driver
* Add some new sensor drivers
* Optimization sensor driver
* Support sensor list: gc2053 gc2093 gc4653 imx327 imx335 imx415 jxf23-s0 jxf23-s1 jxf32 jxf35 jxf37 jxk04 jxq03
                       mis2006 os04b10 os05a10 os08a10 ov9732 ps5258 ps5260 sc2232h sc2239 sc2310 sc2335 sc3235 
                       sc3335 sc4236 sc4335 sc5235 sc8238

## [Kernel]
* Add SLCD configuration
* Add drawbox function
* Optimization Kernel deveice tree
* Add  SPI driver
* Repair gmac tx timeout
* modify uart config
* Optimization tcu driver
* Delete some useless code
* Modify mmc code
* Repair PLL set mistake
* Modify clock code
* Optimization Kernel code

## [SDK]
* Optimization isp api definition
* Optimization sample-comon.h and sample-comon.c to support Monophoto and biphoto
* Optimize audio sample
* Optimize encoder bug
* Add drawbox use for ipu
* Modify the ivs_move
* Delete some useless code


#ChangeLog ISVP-T40-1.0.1

---
## [doc]
* Add API doc

## [tools]
* Update Carrier to V4.0.0
* Add jxk04 os05a10 bin file
* Update rootfs
* Update toolchain
* Update T40 image
* Update Carrier to V4.0.0
* Add wifi tools

## [u-boot]
* Add nor flash support
* Update mmc driver
* Repair sfc nand not support cmd_tftpdownload
* Modify some bug
* Solve set env error
* Delete some useless code

## [Drivers]
* Update ISP driver
* Update VPU driver
* Update Audio driver
* Fix some bug of isp driver(wdr mirror)
* Fix some bug of sensor
* Update hash driver
* Update DES,AES,RSA,LZMA,DTRNG,HASH drivers
* Modify AVPU driver
* Add some new functions of isp driver
* Add some new sensor drivers
* Support sensor list: gc2053 imx415 imx327 imx335 sc8238 os08a10 jxf23-s0 jxf35-s1 jxf37
* Optimization sensor driver

## [Kernel]
* Add nor flash
* Add gpio configuration function
* Modify Kernel deveice tree
* Repair ADC driver
* Add logger to support sdk tool logcat
* modify uart config
* Modify dma code
* Modify sfc code
* Modify mmc code
* Modify watchdog code
* Modify clock code
* Optimization Kernel code

## [SDK]
* Optimization isp api definition
* Add sample-watchdog sample
* Optimize sample-adc sample
* Optimize encoder bug
* Optimize isp parameters
* Update audio code
* Delete some useless code
* Optimize encoder parameters


#ChangeLog ISVP-T40-1.0.0

---
## First version


Attension please:
        when you update the ISP driver, you should also update the SDK synchronously.

#ChangeLog ISVP-T41-1.1.1

---
## [doc]
* Update all doc

## [tools]
* Update rootfs to fix TOP problems
* Add ubi rootfs
* Add gc2063 gc4663 gc4023 sc301IoT mis4001 imx327 sc500ai sc8238 bin file
* Add uclibc toolchain
* Update T41 image
* Update HDK
* update debug tools

## [u-boot]
* Add nor flash XT25Q128D
* Add nand flash IS37SML01G1 W25N01GW GD5F1GM7UE FM25S01A S35ML01G3 XT26G11C F35SQA002G FM25S02A and XT26G02C
* Solve watchdog bug
* Optimization MSC1 config
* Solve kernel-3.10 clk set bug
* Solve some msc0 bug
* Delete some useless code

## [Drivers]
* Modify the max gain for gc2607 sensor
* Modify the cim and alloc again issue for gc4023
* odify the init setting and gain lut to fix some bugs for gc4023
* Fix the problem that the ISP OSD function block number is greater than 7 and does not take effect
* Modify the pass through function to use the rmem memory
* Correct the Bayer format of imx415 to rggb
* Change the maximum exposure timestamp
* Add scene AE interface
* Add swicth_bin interface
* Update ISP driver
* Fix ISP driver memory leaks and unload exceptions
* Repairing LSC shading will flip with sensor mirror and flip
* Repair IMP_ISP_Tuning_GetAeExprInfo The range of ExposeValue increases
* ISP add min/max fps limit
* ISP add Ae expo list
* ISP fixed the read bin issue
* ISP add AWB function
* Fixed the imx415 black steak on the top
* Fix ISP driver unload crash
* Fix the right vertical bar problem after GC3003/GC3003A mirror
* Fix the problem that ISP OSD draws lines without color
* Fix some bug of WDR
* Add IMP_ISP_Tuning_SetModule_Ratio interface
* Add IMP_ISP_SetDefaultBinPath interface
* Fix AE re convergence problem caused by backlight compensation interface
* Add IMP_ISP_SetCsccrMode and IMP_ISP_GetCsccrMode interfaces
* Fix the ambiguity problem in the switching resolution of the main code stream
* Fix the third channel code stream ambiguity of ISP
* Fix the flicker problem caused by the conflict between AE strong light suppression and backlight compensation of ISP
* Fix some bug of isp driver
* Fix some bug of sensor
* Fix the offset problem of recording data for Audio
* Update VPU driver
* Instructions for optimizing PWM functions
* Optimization soc-nna driver
* Update Audio driver
* Optimization mpsys-driver
* Optimization bus_monitor sample
* Add I2C sample
* Add some new sensor drivers
* Support sensor list:gc2063 gc2607 gc3003 gc3003a gc4023 gc4663 gc5603 imx327 imx386 imx415 jxf37 jxk06 os04c10 os08a10 ov01a1s sc401ai sc4210 sc430ai
                      sc500ai sc8238 sc830ai

## [Kernel-3.10]
* Add nor flash XT25Q128D
* Add nand flash IS37SML01G1 W25N01GW GD5F1GM7UE FM25S01A S35ML01G3 XT26G11C F35SQA002G FM25S02A and XT26G02C
* Modify IPU driver
* Modify the mmc clock frequency division calculation method and fix the first line problems
* Modify I2D driver
* Add nor flash erase size config
* Modify MXU3 bug
* Adapt TCU driver, delete invalid data structure
* Adapt to WDT driver
* Repair I2D bug
* Optimization Gmac driver
* Optimize SD card power control logic
* Optimization SFC driver
* Optimization AIP driver
* Optimization reboot code
* Optimization Kernel code

## [Kernel-4.4.94]
* Add nor flash XT25Q128D
* Add nand flash IS37SML01G1 GD5F1GM7UE W25N01GW GD5F1GM7UE FM25S01A S35ML01G3 XT26G11C F35SQA002G FM25S02A and XT26G02C
* Solve the problem of grabbing MJPEG image screen after turning on I2D
* Modify OST bug
* Modify the mmc clock frequency division calculation method and fix the first line problems
* Fix jzgpio_func_set function
* Optimization Gmac driver
* Add nor flash erase size config
* Add logcat dynamic debugging level control
* Modify MXU3 bug
* Add ubi Kernel defconfig
* Modify USB disconnect bug
* Add nor flash erase block size config
* Add kernel mini config
* Optimization SFC driver
* Optimization AIP driver
* Optimization reboot code
* Optimization Kernel code

## [SDK]
* Optimization isp api definition
* Repair switch_bin set bug
* Add ISP Defog ratio function
* Add IMP_ISP_SetCsccrMode and IMP_ISP_GetCsccrMode interfaces
* Solve segment errors caused by frequent reboots
* Repair the abnormal use of some ISP interfaces caused by cmd dislocation
* Optimize soft photosensitive sample and add luma judgment
* Modify the pass through function to use the rmem memory
* Repairing LSC shading will flip with sensor mirror and flip
* Repair IMP_ISP_Tuning_GetAeExprInfo The range of ExposeValue increases
* Optimize isp parameters
* Modify isp API functions
* Optimization isp osd functions
* Optimize encoder parameters
* Optimize the de initialization of the sdk layer code stream buffer
* Support external input NV12 encoding AVC/HEVC/JPEG
* Optimization encoder code
* Add IMP_Encoder_SetChnMaxPictureSize interface for I frame and P frame MaxPictureSize Set
* Add the api for dynamically modifying E200Jpeg quantization Qp
* Fix floating point errors caused by frequent switching of frame rate, code rate and GOP
* Fix the problem that long reference frames are out of sequence after forced I frames
* Add virtual I frame mark
* Resolve assertion errors for iRefCount
* Solve using IMP_Encode_SetGopParam problem that the structure h265NalType I frame type is not updated when SetGopParam dynamically modifies GOP
* Solve the problem that the coding is not coded when the resolution is frequently switched
* Add logcat dynamic debugging level control
* Solve some Encoder bug
* The IPU drawbox cannot be drawn to the extreme boundary
* Update audio code
* Algorithm for adding audio howling
* Add IMP_AI_Set_WebrtcProfileIni_Path interface
* Modify the sample OSD timestamp display exception
* Optimization Audio timestamp
* Add sample sample-Encoder-yuv.c
* Optimization sample code
* Delete some useless code
* Optimization sdk sample


#ChangeLog ISVP-T41-1.1.0

---
## [doc]
* Update all doc

## [tools]
* Update rootfs
* Add ubi rootfs
* Add gc2063 gc4663 imx327 sc500ai sc8238 bin file
* Add uclibc toolchain
* Update T41 image
* Update HDK
* update debug tools
* Update Audio AEC function config file

## [u-boot]
* Add nor flash support
* Nand support one or two environment variable partitions
* Optimization some functions
* Update gmac to support two built-in etherent phy
* Solve kernel-3.10 clk set bug
* Solve some bug
* Optimization fw_setenv tools
* Delete some useless code

## [Drivers]
* Isp driver support kernel-3.10
* Update ISP driver
* Modify mipi bug
* ISP add LDC Support
* ISP add BLC Support
* ISP add ISP bypass functions
* ISP add ir function
* ISP add AF function
* ISP add AWB function
* Fix some bug of isp driver
* Optimization isp code
* Update VPU driver
* VPU driver support kernel-3.10
* Fix some bug of sensor
* Optimization gc2063 driver
* Optimization soc-nna driver
* Modify sensor_sinfo bug
* Solve motor driver
* Solve IVDC 4K H265 bug
* Add PWM test script
* Update Audio driver
* Audio driver Support kernel-3.10
* Optimization Audio timestamp
* Update soc-nna driver
* Add some new sensor drivers
* Support sensor list:gc2063 gc2607 gc4663 imx327 imx415 jxf37 jxk06 os04c10 os08a10 ov01a1s sc430ai sc500ai sc5235 sc8238

## [Kernel-3.10]
* SOC T41 support kernel-3.10

## [Kernel-4.4.94]
* Solve the problem of grabbing MJPEG image screen after turning on I2D
* Add nand flash FM25S01A and TX25G01
* Add nor flash
* Add gpio configuratio
* Add ubi Kernel defconfig
* Modify clk set bug
* Fix the bug that the SFC driver crashes when the mtd partition information is too long
* Optimized Ethernet driver supports Ethernet phy with two different clocks
* Modify JPEG code
* Modify PWM same Bug
* Modify Kernel deveice tree
* Optimization SFC functions
* Add nor flash erase block size config
* Add kernel mini config
* Optimization Kernel code

## [SDK]
* Optimization isp api definition
* Optimize isp parameters
* Modify isp API functions
* Isp add some API
* Optimization isp osd functions
* Fix the bug of code startup and shutdown
* Optimize encoder parameters
* Optimization encoder code
* Solve some Encoder bug
* Update audio code
* Optimization Audio timestamp
* Solve the problem that H264 encoding fails to stream in IVDC 4K
* Solve the problem of grabbing MJPEG image screen after turning on I2D
* Process the code stream interruption caused by shared code stream buf in the pass through mode
* Modify the usage of JPEG shared stream buffer. The sdk supports both shared and unshared stream buffers
* Modify Audio bug
* Optimization sample code
* Delete some useless code
* Optimization sdk sample

#ChangeLog ISVP-T41-1.0.1

---
## [doc]
* Update all doc

## [tools]
* Update Carrier to V5.0.1
* Add imx327 gc4663 bin file
* Update toolchain
* Add T41 image
* Update HDK
* update debug tools

## [u-boot]
* Add nor flash support
* Update mmc driver
* Update the SFC driver to support the use of two SFCS together
* Modify some bug
* Solve set env error
* Delete some useless code

## [Drivers]
* Update ISP driver
* Update VPU driver
* Update Audio driver
* Update soc-nna driver
* Fix some bug of isp driver
* Fix some bug of sensor
* Update DES,AES,RSA,LZMA,DTRNG,HASH drivers
* Add some new functions of isp driver
* Add some new sensor drivers
* Support sensor list: gc4663 imx327 jxf37 ov01a1s sc430ai sc500ai sc5235 sc8238
* Optimization sensor driver

## [Kernel]
* Add nor flash
* Add gpio configuration function
* Modify Kernel deveice tree
* Update the SFC driver to support the use of two SFCS together
* Modify mmc code
* Modify watchdog code
* Modify clock code
* Optimization Kernel code
* Add slcd support
* Add AIP driver
* Upfate T41 default gpio configuration
* Add VO driver
* Update rtc driver
* Update pwm driver

## [SDK]
* Optimization isp api definition
* Optimize isp parameters
* Optimization isp osd functions
* Modify encoder bug
* Optimize encoder parameters
* Optimization encoder code
* Update audio code
* Modify Audio bug
* Optimization sample code
* Add sample-Soft-photosensitive
* Add sample-Encoder-video-direct
* Add sample-Encoder-h265-ivpu-jpeg
* Delete some useless code


#ChangeLog ISVP-T41-1.0.0

---
## First version
GetAeExprInfoI

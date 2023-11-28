EXTRA_CFLAGS += $(USER_EXTRA_CFLAGS) -O1 -Wno-unused-variable -Wno-unused-value -Wno-unused-label \
				-Wno-unused-parameter -Wno-unused-function -Wno-unused  -I$(src)/include \
				-I$(src)/hal/phydm  -I$(src)/platform -I$(src)/hal/btc -DCONFIG_RTL8723D \
				-DCONFIG_MP_INCLUDED -DCONFIG_BT_COEXIST -DCONFIG_EFUSE_CONFIG_FILE \
				-DCONFIG_TRAFFIC_PROTECT -DCONFIG_LOAD_PHY_PARA_FROM_FILE -DREALTEK_CONFIG_PATH=\"\" \
				-DCONFIG_TXPWR_BY_RATE_EN=1 -DCONFIG_RTW_ADAPTIVITY_EN=0 -DCONFIG_RTW_ADAPTIVITY_MODE=0 \
				-DWAKEUP_GPIO_IDX=default -DCONFIG_RTW_SDIO_PM_KEEP_POWER \
				-DCONFIG_BR_EXT '-DCONFIG_BR_EXT_BRNAME="'$(BR_NAME)'"' -DCONFIG_RTW_NAPI \
				-DCONFIG_RTW_GRO -DCONFIG_RTW_DEBUG -DRTW_LOG_LEVEL=4 -DDM_ODM_SUPPORT_TYPE=0x04 \
				-DCONFIG_LITTLE_ENDIAN -DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT -DCONFIG_REDUCE_TX_CPU_LOADING

EXTRA_LDFLAGS += --strip-debug

EXTRA_CFLAGS += -DCONFIG_CONCURRENT_MODE -Wno-error=date-time

CONFIG_RTL8723D = y

export TopDIR ?= $(shell pwd)

HCI_NAME = sdio
_OS_INTFS_FILES :=	os_dep/osdep_service.o os_dep/linux/os_intfs.o os_dep/linux/$(HCI_NAME)_intf.o \
			os_dep/linux/$(HCI_NAME)_ops_linux.o os_dep/linux/ioctl_linux.o os_dep/linux/xmit_linux.o \
			os_dep/linux/mlme_linux.o os_dep/linux/recv_linux.o os_dep/linux/ioctl_cfg80211.o \
			os_dep/linux/rtw_cfgvendor.o os_dep/linux/wifi_regd.o os_dep/linux/rtw_android.o os_dep/linux/rtw_proc.o

_OS_INTFS_FILES += os_dep/linux/ioctl_mp.o

_OS_INTFS_FILES += os_dep/linux/custom_gpio_linux.o
_OS_INTFS_FILES += os_dep/linux/$(HCI_NAME)_ops_linux.o

_HAL_INTFS_FILES :=	hal/hal_intf.o hal/hal_com.o hal/hal_com_phycfg.o hal/hal_phy.o \
			hal/hal_dm.o hal/hal_btcoex.o hal/hal_mp.o hal/hal_mcc.o \
			hal/hal_hci/hal_$(HCI_NAME).o hal/led/hal_$(HCI_NAME)_led.o

_OUTSRC_FILES := hal/phydm/phydm_debug.o hal/phydm/phydm_antdiv.o hal/phydm/phydm_antdect.o \
				 hal/phydm/phydm_interface.o hal/phydm/phydm_hwconfig.o hal/phydm/phydm.o\
				 hal/phydm/halphyrf_ce.o hal/phydm/phydm_edcaturbocheck.o hal/phydm/phydm_dig.o\
				 hal/phydm/phydm_pathdiv.o hal/phydm/phydm_rainfo.o hal/phydm/phydm_dynamicbbpowersaving.o\
				 hal/phydm/phydm_powertracking_ce.o hal/phydm/phydm_dynamictxpower.o hal/phydm/phydm_adaptivity.o\
				 hal/phydm/phydm_cfotracking.o hal/phydm/phydm_noisemonitor.o hal/phydm/phydm_acs.o\
			     hal/phydm/phydm_dfs.o hal/phydm/txbf/halcomtxbf.o hal/phydm/txbf/haltxbfinterface.o\
				 hal/phydm/txbf/phydm_hal_txbf_api.o hal/phydm/phydm_kfree.o hal/phydm/phydm_ccx.o \
				 hal/phydm/phydm_beamforming.o

_PLATFORM_FILES := platform/platform_ops.o

_OUTSRC_FILES += hal/btc/HalBtc8192e1Ant.o hal/btc/HalBtc8192e2Ant.o hal/btc/HalBtc8723b1Ant.o \
				 hal/btc/HalBtc8723b2Ant.o hal/btc/HalBtc8812a1Ant.o hal/btc/HalBtc8812a2Ant.o \
				 hal/btc/HalBtc8821a1Ant.o hal/btc/HalBtc8821a2Ant.o hal/btc/HalBtc8821aCsr2Ant.o \
				 hal/btc/HalBtc8703b1Ant.o hal/btc/halbtc8723d1ant.o hal/btc/halbtc8723d2ant.o \
				 hal/btc/HalBtc8822b1Ant.o hal/btc/halbtc8821c1ant.o hal/btc/halbtc8821c2ant.o

RTL871X = rtl8723d
MODULE_NAME = 8723ds
MODULE_SUB_NAME = 8723ds

_HAL_INTFS_FILES += hal/HalPwrSeqCmd.o hal/$(RTL871X)/Hal8723DPwrSeq.o hal/$(RTL871X)/$(RTL871X)_sreset.o

_HAL_INTFS_FILES +=	hal/$(RTL871X)/$(RTL871X)_hal_init.o hal/$(RTL871X)/$(RTL871X)_phycfg.o hal/$(RTL871X)/$(RTL871X)_rf6052.o \
			hal/$(RTL871X)/$(RTL871X)_dm.o hal/$(RTL871X)/$(RTL871X)_rxdesc.o hal/$(RTL871X)/$(RTL871X)_cmd.o \
			hal/$(RTL871X)/hal8723d_fw.o hal/$(RTL871X)/$(RTL871X)_lps_poff.o

_HAL_INTFS_FILES +=	hal/$(RTL871X)/$(HCI_NAME)/$(HCI_NAME)_halinit.o \
			hal/$(RTL871X)/$(HCI_NAME)/rtl$(MODULE_SUB_NAME)_led.o \
			hal/$(RTL871X)/$(HCI_NAME)/rtl$(MODULE_SUB_NAME)_xmit.o \
			hal/$(RTL871X)/$(HCI_NAME)/rtl$(MODULE_SUB_NAME)_recv.o

_HAL_INTFS_FILES += hal/$(RTL871X)/$(HCI_NAME)/$(HCI_NAME)_ops.o

_OUTSRC_FILES += hal/phydm/$(RTL871X)/halhwimg8723d_bb.o hal/phydm/$(RTL871X)/halhwimg8723d_mac.o\
				 hal/phydm/$(RTL871X)/halhwimg8723d_rf.o hal/phydm/$(RTL871X)/halhwimg8723d_fw.o\
				 hal/phydm/$(RTL871X)/phydm_rtl8723d.o hal/phydm/$(RTL871X)/halphyrf_8723d.o \
				 hal/phydm/$(RTL871X)/phydm_regconfig8723d.o
#EFUSE_MAP_PATH
USER_EFUSE_MAP_PATH ?=
ifneq ($(USER_EFUSE_MAP_PATH),)
EXTRA_CFLAGS += -DEFUSE_MAP_PATH=\"$(USER_EFUSE_MAP_PATH)\"
else
EXTRA_CFLAGS += -DEFUSE_MAP_PATH=\"/system/etc/wifi/wifi_efuse_$(MODULE_NAME).map\"
endif

#WIFIMAC_PATH
USER_WIFIMAC_PATH ?=
ifneq ($(USER_WIFIMAC_PATH),)
EXTRA_CFLAGS += -DWIFIMAC_PATH=\"$(USER_WIFIMAC_PATH)\"
else
EXTRA_CFLAGS += -DWIFIMAC_PATH=\"/data/wifimac.txt\"
endif

BR_NAME = br0
ARCH := mips
CROSS_COMPILE := /home/ingenic/work/manhattan/prebuilts/toolchains/mips-gcc472-glibc216/bin/mips-linux-gnu-
KSRC := /home/ingenic/work/audio/kernel
MODULE_NAME := wlan

USER_MODULE_NAME ?=
ifneq ($(USER_MODULE_NAME),)
MODULE_NAME := $(USER_MODULE_NAME)
endif

ifneq ($(KERNELRELEASE),)
rtk_core :=	core/rtw_cmd.o core/rtw_security.o core/rtw_debug.o core/rtw_io.o \
		core/rtw_ioctl_query.o core/rtw_ioctl_set.o core/rtw_ieee80211.o \
		core/rtw_mlme.o core/rtw_mlme_ext.o core/rtw_mi.o core/rtw_wlan_util.o \
		core/rtw_vht.o core/rtw_pwrctrl.o core/rtw_rf.o core/rtw_recv.o \
		core/rtw_sta_mgt.o core/rtw_ap.o core/rtw_xmit.o core/rtw_p2p.o \
		core/rtw_tdls.o core/rtw_br_ext.o core/rtw_iol.o core/rtw_sreset.o \
		core/rtw_btcoex.o core/rtw_beamforming.o core/rtw_odm.o core/efuse/rtw_efuse.o core/rtw_sdio.o

$(MODULE_NAME)-y += $(rtk_core)

$(MODULE_NAME)-$(CONFIG_INTEL_WIDI) += core/rtw_intel_widi.o
$(MODULE_NAME)-$(CONFIG_WAPI_SUPPORT) += core/rtw_wapi.o core/rtw_wapi_sms4.o

$(MODULE_NAME)-y += $(_OS_INTFS_FILES)
$(MODULE_NAME)-y += $(_HAL_INTFS_FILES)
$(MODULE_NAME)-y += $(_OUTSRC_FILES)
$(MODULE_NAME)-y += $(_PLATFORM_FILES)

$(MODULE_NAME)-y += core/rtw_mp.o

obj-$(CONFIG_RTL8723DS) := $(MODULE_NAME).o
else
export CONFIG_RTL8723DS = m

all: modules
modules:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(shell pwd)  modules
strip:
	$(CROSS_COMPILE)strip $(MODULE_NAME).ko --strip-unneeded

install:
	install -p -m 644 $(MODULE_NAME).ko  $(MODDESTDIR)
	/sbin/depmod -a ${KVER}

uninstall:
	rm -f $(MODDESTDIR)/$(MODULE_NAME).ko
	/sbin/depmod -a ${KVER}

config_r:
	@echo "make config"
	/bin/bash script/Configure script/config.in

.PHONY: modules clean
clean:
	#$(MAKE) -C $(KSRC) M=$(shell pwd) clean
	cd hal ; rm -fr */*/*/*.mod.c */*/*/*.mod */*/*/*.o */*/*/.*.cmd */*/*/*.ko
	cd hal ; rm -fr */*/*.mod.c */*/*.mod */*/*.o */*/.*.cmd */*/*.ko
	cd hal ; rm -fr */*.mod.c */*.mod */*.o */.*.cmd */*.ko
	cd hal ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd core/efuse ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd core ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd os_dep/linux ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd os_dep ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd platform ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	rm -fr Module.symvers ; rm -fr Module.markers ; rm -fr modules.order
	rm -fr *.mod.c *.mod *.o .*.cmd *.ko *~
	rm -fr .tmp_versions
endif


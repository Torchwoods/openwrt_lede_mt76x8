include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/package.mk

PKG_NAME:=wifiSniffer
PKG_VERSION:=1.0

define Package/wifiSniffer
	CATEGORY:= UltraIot
	DEPENDS:= +libc +libcurl  +libpcap  +libubox  +libuci
	TITLE:= wifiSniffer 
	MAINTAINER:=UltraIot <UltraIot@163.com>
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/wifiSniffer/install
	$(INSTALL_DIR) $(1)/bin 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifiSniffer $(1)/bin/
endef

define Package/wifiSniffer/preinst
	#!/bin/sh
	echo 'before install $(PKG_NAME)'
endef

$(eval $(call BuildPackage,wifiSniffer))


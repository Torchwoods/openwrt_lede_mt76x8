#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/UltraIot-PR1
	NAME:=Ultraiot Profile
	PRIORITY:=1
	PACKAGES:= kmod-usb-core kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport \
	kmod-ledtrig-netdev mountd mjpg-streamer kmod-video-core kmod-video-uvc \
	kmod-fs-vfat kmod-fs-exfat kmod-fs-ext4 kmod-fs-ntfs block-mount e2fsprogs \
	kmod-i2c-core kmod-i2c-ralink kmod-nls-base kmod-nls-cp437 kmod-nls-iso8859-1 kmod-nls-utf8 \
	kmod-usb-storage kmod-usb-storage-extras kmod-usb-uhci \
	kmod-sound-core kmod-sound-mtk madplay-alsa alsa-utils \
	luci tcpdump
endef

define Profile/UltraIot-PR1/Description
	Default package set compatible with most boards.
endef
$(eval $(call Profile,UltraIot-PR1))

include $(TOPDIR)/rules.mk

PKG_NAME:=msqt
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/msqt
	DEPENDS:=+libuci +libmosquitto
	CATEGORY:=Base system
	TITLE:=msqt
endef

define Package/msqt/description
	MQTT subscriber
endef

define Package/msqt/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/msqt $(1)/usr/bin/msqt
	$(INSTALL_BIN) ./files/msqt.init $(1)/etc/init.d/msqt
	$(INSTALL_CONF) ./files/msqt.config $(1)/etc/config/msqt
endef
$(eval $(call BuildPackage,msqt))

/*
 *  GL_ar150 board support
 *
 *  Copyright (C) 2011 dongyuqi <729650915@qq.com>
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2013 alzhao <alzhao@gmail.com>
 *  Copyright (C) 2014 Michel Stempin <michel.stempin@wanadoo.fr>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
*/

#include <linux/gpio.h>
#include <linux/mmc/host.h>
#include <linux/spi/spi.h>
#include <linux/spi/mmc_spi.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-spi.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define GL_AR150_GPIO_LED_WLAN		   0
#define GL_AR150_GPIO_LED_LAN		   13
#define GL_AR150_GPIO_LED_WAN		   15 

#define GL_AR150_GPIO_BIN_USB         6
#define GL_AR150_GPIO_BTN_MANUAL      7
#define GL_AR150_GPIO_BTN_AUTO	   	   8
#define GL_AR150_GPIO_BTN_RESET	   11

// PGS 20170201 - Fictional CS line for 2nd SPI device
// (used for microSD card). In reality, we are simply
// inverting CS0 and feeding that to the microSD slot,
// however, I don't know how to specify that here.
#define GL_AR150_GPIO_CS1_MMC 23

#define GL_AR150_KEYS_POLL_INTERVAL   20	/* msecs */
#define GL_AR150_KEYS_DEBOUNCE_INTERVAL	(3 * GL_AR150_KEYS_POLL_INTERVAL)

#define GL_AR150_MAC0_OFFSET	0x0000
#define GL_AR150_MAC1_OFFSET	0x0000
#define GL_AR150_CALDATA_OFFSET	0x1000
#define GL_AR150_WMAC_MAC_OFFSET	0x0000


static struct mmc_spi_platform_data ath79_mmc_data = {
       .detect_delay = 100, /* msecs */ 
	.ocr_mask	= MMC_VDD_32_33 | MMC_VDD_33_34,
};

static struct ath79_spi_controller_data ath79_spi1_cdata = {
	.cs_type = ATH79_SPI_CS_TYPE_GPIO,
	.cs_line = GL_AR150_GPIO_CS1_MMC,
};

static struct spi_board_info ath79_spi_info[] = {
	{
		.bus_num	= 0,
		.chip_select	= 1,
		.max_speed_hz	= 25000000,
		.modalias	= "mmc_spi",
		.platform_data	= &ath79_mmc_data,
		.controller_data = &ath79_spi1_cdata,
	}
};


static struct gpio_led gl_ar150_leds_gpio[] __initdata = {
	{
		.name = "gl_ar150:wlan",
		.gpio = GL_AR150_GPIO_LED_WLAN,
		.active_low = 0,
	},
	{
		.name = "gl_ar150:lan",
		.gpio = GL_AR150_GPIO_LED_LAN,
		.active_low = 0,
	},
	{
		.name = "gl_ar150:wan",
		.gpio = GL_AR150_GPIO_LED_WAN,
		.active_low = 0,
 		.default_state = 1,
	},
};

static struct gpio_keys_button gl_ar150_gpio_keys[] __initdata = {
	{
		.desc = "BTN_7",
		.type = EV_KEY,
		.code = BTN_7,
		.debounce_interval = GL_AR150_KEYS_DEBOUNCE_INTERVAL,
		.gpio = GL_AR150_GPIO_BTN_MANUAL,
		.active_low = 0,
	},
	{
		.desc = "BTN_8",
		.type = EV_KEY,
		.code = BTN_8,
		.debounce_interval = GL_AR150_KEYS_DEBOUNCE_INTERVAL,
		.gpio = GL_AR150_GPIO_BTN_AUTO,
		.active_low = 0,
	},
	{
		.desc = "reset",
		.type = EV_KEY,
		.code = KEY_RESTART,
		.debounce_interval = GL_AR150_KEYS_DEBOUNCE_INTERVAL,
		.gpio = GL_AR150_GPIO_BTN_RESET,
		.active_low = 0,
	},
};

static void __init gl_ar150_setup(void)
{

	/* ART base address */
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	/* disable PHY_SWAP and PHY_ADDR_SWAP bits */
	ath79_setup_ar933x_phy4_switch(false, false);

	/* register flash. */
	ath79_register_m25p80(NULL);

	/* And SPI-connected microSD/MMC */
	spi_register_board_info(ath79_spi_info,
				ARRAY_SIZE(ath79_spi_info));

	/* register gpio LEDs and keys */
	ath79_register_leds_gpio(-1, ARRAY_SIZE(gl_ar150_leds_gpio),
				 gl_ar150_leds_gpio);
	ath79_register_gpio_keys_polled(-1, GL_AR150_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(gl_ar150_gpio_keys),
					gl_ar150_gpio_keys);

	/* enable usb */
	gpio_request_one(GL_AR150_GPIO_BIN_USB,
				 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
	 			 "USB power");
	ath79_register_usb();
	
	/* register eth0 as WAN, eth1 as LAN */
	ath79_init_mac(ath79_eth0_data.mac_addr, art+GL_AR150_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, art+GL_AR150_MAC1_OFFSET, 0);
	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);
	ath79_register_eth(1);

	/* register wireless mac with cal data */
	ath79_register_wmac(art + GL_AR150_CALDATA_OFFSET, art + GL_AR150_WMAC_MAC_OFFSET);

}

MIPS_MACHINE(ATH79_MACH_GL_AR150, "GL-AR150", "GL AR150",gl_ar150_setup);

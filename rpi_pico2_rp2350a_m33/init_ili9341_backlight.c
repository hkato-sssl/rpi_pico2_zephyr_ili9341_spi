/*
 * Copyright (c) 2026 Hidekazu Kato <hkato.sssl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>

static int init_ili9341_backlight(void)
{
	const struct gpio_dt_spec bl = GPIO_DT_SPEC_GET(DT_ALIAS(backlight0), gpios);
	
	gpio_pin_configure_dt(&bl, GPIO_OUTPUT_ACTIVE);
	gpio_pin_set_dt(&bl, 1);

	return 0;
}

SYS_INIT(init_ili9341_backlight, APPLICATION, 0);

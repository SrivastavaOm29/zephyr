/*
 * Copyright 2025 Om Srivastava <srivastavaom97714@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/emul.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/emul_sensor.h>
#include <zephyr/drivers/i2c_emul.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/drivers/emul.h>

#include "checks.h"
// #include "opt3001.h"

static int mock_i2c_transfer_fail_reg_number = -1;

static int mock_i2c_transfer(const struct emul *target, struct i2c_msg *msgs, int num_msgs,
			     int addr)
{
	ARG_UNUSED(target);
	ARG_UNUSED(addr);
	if (mock_i2c_transfer_fail_reg_number >= 0 && i2c_is_read_op(&msgs[1]) &&
	    opt3001_i2c_is_touching_reg(msgs, num_msgs, mock_i2c_transfer_fail_reg_number)) {
		return -EIO;
	}
	return -ENOSYS;
}

ZTEST_USER_F(opt3001, test_opt3001_i2c_get_offset_fail_to_read_offset_acc)
{
	struct i2c_emul_api mock_bus_api;
	struct sensor_value value;

	fixture->emul_i2c->bus.i2c->mock_api = &mock_bus_api;
	mock_bus_api.transfer = mock_i2c_transfer;

	enum sensor_channel channels[] = {SENSOR_CHAN_LIGHT, SENSOR_CHAN_LIGHT};
	int fail_registers[] = {0x45, 0x46,
				0x47, 0x48,
				0x49, 0x4A,
				0x4B};

	for (int fail_reg_idx = 0; fail_reg_idx < ARRAY_SIZE(fail_registers); ++fail_reg_idx) {
		mock_i2c_transfer_fail_reg_number = fail_registers[fail_reg_idx];
		for (int chan_idx = 0; chan_idx < ARRAY_SIZE(channels); ++chan_idx) {
			zassert_equal(-EIO, sensor_attr_get(fixture->dev_i2c, channels[chan_idx],
							    SENSOR_ATTR_OFFSET, &value));
		}
	}
}

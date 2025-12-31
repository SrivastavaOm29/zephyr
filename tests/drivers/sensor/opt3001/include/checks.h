/*
 * Copyright 2025 Bhainsa
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TEST_DRIVERS_SENSOR_ACCEL_INCLUDE_CHECKS_H_
#define TEST_DRIVERS_SENSOR_ACCEL_INCLUDE_CHECKS_H_

#include <zephyr/drivers/i2c.h>

#include "opt3001.h"

static inline bool opt3001_i2c_is_touching_reg(struct i2c_msg *msgs, int num_msgs, int reg)
{
	__ASSERT_NO_MSG(num_msgs == 2);
	__ASSERT_NO_MSG(msgs[0].len == 1);

	uint8_t start_reg = msgs[0].buf[0];

	return (start_reg <= reg) && (reg < start_reg + msgs[1].len);
}

#endif /* TEST_DRIVERS_SENSOR_ACCEL_INCLUDE_CHECKS_H_ */

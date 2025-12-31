/*
 * Copyright 2025 Om Srivastava <srivastavaom97714@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// #include "fixture.h"

#include <zephyr/drivers/emul_sensor.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/drivers/emul.h>

struct opt3001_fixture {
	const struct device *dev_i2c;
	const struct emul *emul_i2c;
};

static void sensor_opt3001_setup_emulator(const struct device *dev, const struct emul *emulator)
{
	static struct {
		enum sensor_channel channel;
		q31_t value;
	} values[] = {
		{SENSOR_CHAN_LIGHT, 0},       {SENSOR_CHAN_LIGHT, 1 << 28},
		{SENSOR_CHAN_LIGHT, 2 << 28}, {SENSOR_CHAN_LIGHT, 3 << 28},
		{SENSOR_CHAN_LIGHT, 4 << 28},  {SENSOR_CHAN_LIGHT, 5 << 28},
	};
	static struct sensor_value scale;

	/* 4g */
	scale.val1 = 39;
	scale.val2 = 226600;
	zassert_ok(sensor_attr_set(dev, SENSOR_CHAN_LIGHT, SENSOR_ATTR_FULL_SCALE, &scale));

	/* 125 deg/s */
	scale.val1 = 2;
	scale.val2 = 181661;
	zassert_ok(sensor_attr_set(dev, SENSOR_CHAN_LIGHT, SENSOR_ATTR_FULL_SCALE, &scale));

	for (size_t i = 0; i < ARRAY_SIZE(values); ++i) {
		struct sensor_chan_spec chan_spec = {.chan_type = values[i].channel, .chan_idx = 0};

		zassert_ok(emul_sensor_backend_set_channel(emulator, chan_spec,
							   &values[i].value, 3));
	}
}

static void *opt3001_setup(void)
{
	static struct opt3001_fixture fixture = {
		.dev_i2c = DEVICE_DT_GET(DT_ALIAS(accel)),
		.emul_i2c = EMUL_DT_GET(DT_ALIAS(accel)),
	};

	sensor_opt3001_setup_emulator(fixture.dev_i2c, fixture.emul_i2c);

	return &fixture;
}

static void opt3001_before(void *f)
{
	struct opt3001_fixture *fixture = (struct opt3001_fixture *)f;

	zassert_true(device_is_ready(fixture->dev_i2c), "'%s' device is not ready",
		     fixture->dev_i2c->name);

	k_object_access_grant(fixture->dev_i2c, k_current_get());
}

ZTEST_SUITE(opt3001, NULL, opt3001_setup, opt3001_before, NULL, NULL);

#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c_emul.h>
#include <zephyr/devicetree.h>
// #include <zephyr/drivers/emul.h>

#define OPT3001_NODE DT_INST(0, ti_opt3001)

struct opt3001_emul_data {
	uint16_t result;
	uint16_t config;
	uint8_t reg_ptr;
};

struct opt3001_fixture {
	const struct device *dev;
	struct opt3001_emul_data *emul;
};

#include <zephyr/drivers/emul.h>

#include <zephyr/drivers/emul.h>

// const struct emul *emul =
// 	EMUL_DT_GET(DT_INST(0, ti_opt3001));

// struct opt3001_emul_data *edata = emul->data;

static void *opt3001_setup(void)
{
  static struct opt3001_fixture fixture;

  fixture.dev = DEVICE_DT_GET(DT_INST(0, ti_opt3001));
  zassert_true(device_is_ready(fixture.dev),
                "OPT3001 device not ready");

  const struct emul *emul =
      EMUL_DT_GET(DT_INST(0, ti_opt3001));

  zassert_not_null(emul, "OPT3001 emulator not found");

  fixture.emul = emul->data;

  return &fixture;
}


ZTEST_SUITE(opt3001, NULL, opt3001_setup, NULL, NULL, NULL);

/* ================= Tests ================= */

ZTEST_F(opt3001, test_i2c_error_free)
{
	zassert_ok(sensor_sample_fetch(fixture->dev));
}

ZTEST_F(opt3001, test_lux_10)
{
	struct sensor_value val;

	/* EXP=1, MANT=500 → 10 lux */
	fixture->emul->result = (1 << 12) | 500;

	zassert_ok(sensor_sample_fetch(fixture->dev));
	zassert_ok(sensor_channel_get(fixture->dev,
				      SENSOR_CHAN_LIGHT, &val));

	zassert_equal(10, val.val1);
	zassert_equal(0, val.val2);
}

ZTEST_F(opt3001, test_lux_fractional)
{
	struct sensor_value val;

	/* EXP=0, MANT=123 → 1.23 lux */
	fixture->emul->result = 123;

	zassert_ok(sensor_sample_fetch(fixture->dev));
	zassert_ok(sensor_channel_get(fixture->dev,
				      SENSOR_CHAN_LIGHT, &val));

	zassert_equal(1, val.val1);
	zassert_equal(230000, val.val2);
}

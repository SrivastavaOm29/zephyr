#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/i2c_emul.h>
#include <zephyr/drivers/emul.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>

#if !DT_NODE_HAS_STATUS(DT_INST(0, ti_opt3001), okay)
#error "OPT3001 not enabled in devicetree"
#endif

#define OPT3001_NODE DT_INST(0, ti_opt3001)
#define OPT3001_I2C_BUS DT_PARENT(OPT3001_NODE)
#define OPT3001_I2C_ADDR DT_REG_ADDR(OPT3001_NODE)

#define OPT3001_REG_RESULT  0x00
#define OPT3001_REG_CONFIG  0x01

/* ===================== Emulator private data ===================== */

struct opt3001_emul {
	const struct emul emul;
	uint16_t result_reg;
	uint16_t config_reg;
	uint8_t current_reg;
	bool fail_i2c;
};

/* ===================== I2C transfer handler ===================== */

static int opt3001_emul_transfer(
	const struct emul *emul,
	struct i2c_msg *msgs,
	int num_msgs,
	int addr)
{
	struct opt3001_emul *data = emul->data;

	if (data->fail_i2c) {
		return -EIO;
	}

	/* Write: register address */
	if (num_msgs == 1 && !(msgs[0].flags & I2C_MSG_READ)) {
		data->current_reg = msgs[0].buf[0];
		return 0;
	}

	/* Write register + Read data */
	if (num_msgs == 2 &&
	    !(msgs[0].flags & I2C_MSG_READ) &&
	    (msgs[1].flags & I2C_MSG_READ)) {

		uint16_t val = 0;

		data->current_reg = msgs[0].buf[0];

		if (data->current_reg == OPT3001_REG_RESULT) {
			val = data->result_reg;
		} else if (data->current_reg == OPT3001_REG_CONFIG) {
			val = data->config_reg;
		}

		if (msgs[1].len < 2) {
			return -EINVAL;
		}

		msgs[1].buf[0] = val >> 8;
		msgs[1].buf[1] = val & 0xff;
		return 0;
	}

	return -ENOTSUP;
}

/* ===================== Emulator instances ===================== */

const struct device device_name = {
	.name = "OPT3001",
};

static struct opt3001_emul opt3001_emul_data = {
	.emul = {
		.dev = &device_name,
		.data = &opt3001_emul_data,
	},
	.result_reg = 0,
	.config_reg = 0,
	.current_reg = 0,
	.fail_i2c = false,
};

static struct i2c_emul_api opt3001_emul_api = {
	.transfer = opt3001_emul_transfer,
};

static struct i2c_emul opt3001_i2c_emul = {
	.target = &opt3001_emul_data.emul,
	.addr = OPT3001_I2C_ADDR,
	.mock_api = &opt3001_emul_api,
};

/* ===================== Test fixture ===================== */

struct opt3001_fixture {
	const struct device *dev;
	struct opt3001_emul *emul;
};

static void *opt3001_setup(void)
{
	static struct opt3001_fixture fixture = {
		.dev = DEVICE_DT_GET(OPT3001_NODE),
		.emul = &opt3001_emul_data,
	};

	const struct device *i2c =
		DEVICE_DT_GET(OPT3001_I2C_BUS);

	zassert_true(device_is_ready(i2c), "I2C bus not ready");

	i2c_emul_register(i2c, &opt3001_i2c_emul);
		printk("This is a simple printk debug message2.\n");


	zassert_true(device_is_ready(fixture.dev),
		     "OPT3001 device not ready");

	return &fixture;
}

static void opt3001_before(void *f)
{
	printk("This is a simple printk debug message.\n");
	struct opt3001_fixture *fixture = f;

	fixture->emul->fail_i2c = false;
	fixture->emul->current_reg = 0;
}

/* ===================== Test suite ===================== */

ZTEST_SUITE(opt3001, NULL,
	    opt3001_setup,
	    opt3001_before,
	    NULL, NULL);

/* ===================== Tests ===================== */

ZTEST_F(opt3001, test_sample_fetch_i2c_fail)
{

	fixture->emul->fail_i2c = true;

	zassert_equal(-EIO,
		sensor_sample_fetch(fixture->dev));
}

ZTEST_F(opt3001, test_wrong_channel)
{
	struct sensor_value val;

	zassert_equal(-ENOTSUP,
		sensor_channel_get(fixture->dev,
				   SENSOR_CHAN_ACCEL_X, &val));
}

ZTEST_F(opt3001, test_lux_10)
{
	struct sensor_value val;

	/* EXP = 1, MANT = 500 → 10 lux */
	fixture->emul->result_reg = (1 << 12) | 500;

	zassert_equal(0,
		sensor_sample_fetch(fixture->dev));

	zassert_equal(0,
		sensor_channel_get(fixture->dev,
				   SENSOR_CHAN_LIGHT, &val));

	zassert_equal(10, val.val1);
	zassert_equal(0, val.val2);
}

ZTEST_F(opt3001, test_lux_fractional)
{
	struct sensor_value val;

	/* EXP = 0, MANT = 123 → 1.23 lux */
	fixture->emul->result_reg = 123;

	zassert_equal(0,
		sensor_sample_fetch(fixture->dev));

	zassert_equal(0,
		sensor_channel_get(fixture->dev,
				   SENSOR_CHAN_LIGHT, &val));

	zassert_equal(1, val.val1);
	zassert_equal(230000, val.val2);
}

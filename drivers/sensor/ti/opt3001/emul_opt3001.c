#define DT_DRV_COMPAT ti_opt3001

#include <zephyr/drivers/i2c_emul.h>
#include <zephyr/drivers/emul.h>
#include <zephyr/sys/byteorder.h>

struct opt3001_emul_data {
	uint16_t result;
	uint16_t config;
	uint8_t reg;
};

static int opt3001_emul_transfer(const struct i2c_emul *emul,
				 struct i2c_msg *msgs,
				 int num_msgs,
				 int addr)
{
	struct opt3001_emul_data *d = emul->data;

	if (num_msgs == 1 && !(msgs[0].flags & I2C_MSG_READ)) {
		d->reg = msgs[0].buf[0];
		return 0;
	}

	if (num_msgs == 2 &&
	    !(msgs[0].flags & I2C_MSG_READ) &&
	    (msgs[1].flags & I2C_MSG_READ)) {

		uint16_t val = (d->reg == 0x00) ? d->result : d->config;
		sys_put_be16(val, msgs[1].buf);
		return 0;
	}

	return -ENOTSUP;
}

static const struct i2c_emul_api opt3001_emul_api = {
	.transfer = opt3001_emul_transfer,
};

#define OPT3001_EMUL_INIT(inst)                          \
	static struct opt3001_emul_data data_##inst;     \
	static struct i2c_emul emul_##inst = {           \
		.api = &opt3001_emul_api,                 \
		.addr = DT_INST_REG_ADDR(inst),           \
		.data = &data_##inst,                     \
	};                                                 \
	EMUL_DT_INST_DEFINE(inst, NULL, &data_##inst, &emul_##inst);

DT_INST_FOREACH_STATUS_OKAY(OPT3001_EMUL_INIT)

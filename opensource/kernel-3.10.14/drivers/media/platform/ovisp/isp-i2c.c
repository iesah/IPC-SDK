
#include "isp-i2c.h"
#include "isp-debug.h"

static inline unsigned short isp_i2c_to_word(unsigned char *buf)
{
	return (((unsigned short)buf[0] << 8) | (unsigned short)buf[1]);
}

static int isp_i2c_fill_cmd(struct isp_device *isp, struct isp_i2c_cmd *cmd,
				struct i2c_msg *msg, int num)
{
	memset(cmd, 0, sizeof(*cmd));

	cmd->addr = msg->addr << 1;

	if (num == 1) {
		if (msg[0].flags & I2C_M_RD)
			return -EINVAL;

		/* Write. */
		if (msg[0].len == 2) {
			cmd->reg = msg[0].buf[0];
			cmd->data = msg[0].buf[1];
		} else if (msg[0].len == 3) {
			cmd->flags |= I2C_CMD_ADDR_16BIT;
			cmd->reg = isp_i2c_to_word(&msg[0].buf[0]);
			cmd->data = msg[0].buf[2];
		} else if (msg[0].len == 4) {
			cmd->flags |= I2C_CMD_ADDR_16BIT | I2C_CMD_DATA_16BIT;
			cmd->reg = isp_i2c_to_word(&msg[0].buf[0]);
			cmd->data = isp_i2c_to_word(&msg[0].buf[2]);
		} else
			return -EINVAL;
	} else if (num == 2) {

		if (msg[0].len == 1) {
			cmd->reg = msg[0].buf[0];
		} else if (msg[0].len == 2) {
			cmd->flags |= I2C_CMD_ADDR_16BIT;
			cmd->reg = isp_i2c_to_word(&msg[0].buf[0]);
		} else
			return -EINVAL;

		if (!(msg[1].flags & I2C_M_RD)) {
			if (msg[1].len == 1) {
				cmd->data = msg[0].buf[0];
			} else if (msg[1].len == 2) {
				cmd->flags |= I2C_CMD_DATA_16BIT;
				cmd->data = isp_i2c_to_word(&msg[0].buf[0]);
			} else
				return -EINVAL;
		} else
			cmd->flags |= I2C_CMD_READ;
	} else
		return -EINVAL;

	return 0;
}

static int isp_i2c_get_result(struct isp_device *isp, struct isp_i2c_cmd *cmd,
				struct i2c_msg *msg, int num)
{
	if (msg[num - 1].flags & I2C_M_RD) {
		if (msg[num - 1].len == 1) {
			msg[num - 1].buf[0] = (unsigned char)cmd->data;
		} else if (msg[num - 1].len == 2) {
			msg[num - 1].buf[0] = (cmd->data >> 8);
			msg[num - 1].buf[1] = (cmd->data & 0xff);
		}
	/*ISP_PRINT(ISP_INFO,"%s:msg[%d]:%x\n", __func__, num -1, msg[num-1].buf[0]);*/
	}

	return 0;
}

static int isp_i2c_do_xfer(struct isp_device *isp, struct i2c_msg *msg,
					int num)
{
	struct isp_i2c *i2c = &isp->i2c;
	struct isp_i2c_cmd cmd;
	int ret = 0;

	ret = isp_i2c_fill_cmd(isp, &cmd, msg, num);
	if (ret)
		return ret;

	mutex_lock(&i2c->lock);

	if (i2c->xfer_cmd) {
		ret = i2c->xfer_cmd(isp, &cmd);
		if (!ret)
			isp_i2c_get_result(isp, &cmd, msg, num);
	}

	mutex_unlock(&i2c->lock);

	return ret;
}

static int isp_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[],
					int num)
{
	struct isp_device *isp = adap->algo_data;

	if (!num)
		return 0;

	return isp_i2c_do_xfer(isp, msgs, num);
}

static u32 isp_i2c_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm isp_i2c_algorithm = {
	.master_xfer = isp_i2c_xfer,
	.functionality = isp_i2c_functionality,
};

int isp_i2c_register(struct isp_device *isp)
{
	struct i2c_adapter *adap = &isp->i2c.adap;
	int ret;

	mutex_init(&isp->i2c.lock);

	adap->owner = THIS_MODULE;
	adap->retries = 5;
	adap->nr = isp->pdata->i2c_adapter_id;
	snprintf(adap->name, sizeof(adap->name),
		"ovisp_i2c-i2c.%u", adap->nr);

	adap->algo = &isp_i2c_algorithm;
	adap->algo_data = isp;
	adap->dev.parent = isp->dev;

	ret = i2c_add_numbered_adapter(adap);
	if (ret < 0)
		return ret;

	return 0;
}
EXPORT_SYMBOL(isp_i2c_register);

int isp_i2c_unregister(struct isp_device *isp)
{
	struct i2c_adapter *adap = &isp->i2c.adap;

	i2c_del_adapter(adap);

	return 0;
}
EXPORT_SYMBOL(isp_i2c_unregister);


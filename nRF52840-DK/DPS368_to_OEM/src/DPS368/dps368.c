/*
 * dps368.c
 *
 *  Created on: 26 May 2022
 *      Author: A.Ascher
 */

 #include <dps368.h>
 #include <logging/log.h>

LOG_MODULE_REGISTER(DPS368, LOG_LEVEL_INF);

dps368_data_t data;
dps368_cfg_t dps368_config;

/*
 * Convert the bytes from calibration memory to calibration coefficients
 * structure
 */
void dps368_calib_coeff_creation(const uint8_t raw_coef[18],
				 struct dps368_cal_coeff *comp)
{
	/* Temperature sensor compensation values */
	comp->c0 = (((uint16_t)raw_coef[0]) << 4) + (raw_coef[1] >> 4);
	/* coefficient is 2nd compliment */
	if (comp->c0 > POW_2_11_MINUS_1) {
		comp->c0 = comp->c0 - POW_2_12;
	}

	comp->c1 = (((uint16_t)(raw_coef[1] & 0x0F)) << 8) + raw_coef[2];
	/* coefficient is 2nd compliment */
	if (comp->c1 > POW_2_11_MINUS_1) {
		comp->c1 = comp->c1 - POW_2_12;
	}

	/* Pressure sensor compensation values */
	comp->c00 = (((uint32_t)raw_coef[3]) << 12) + (((uint16_t)raw_coef[4]) << 4) +
		    (raw_coef[5] >> 4);
	/* coefficient is 2nd compliment */
	if (comp->c00 > POW_2_19_MINUS_1) {
		comp->c00 = comp->c00 - POW_2_20;
	}

	comp->c10 = (((uint32_t)(raw_coef[5] & 0x0F)) << 16) +
		    (((uint16_t)raw_coef[6]) << 8) + raw_coef[7];
	/* coefficient is 2nd compliment */
	if (comp->c10 > POW_2_19_MINUS_1) {
		comp->c10 = comp->c10 - POW_2_20;
	}

	comp->c01 = (int16_t) sys_get_be16(&raw_coef[8]);
	comp->c11 = (int16_t) sys_get_be16(&raw_coef[10]);
	comp->c20 = (int16_t) sys_get_be16(&raw_coef[12]);
	comp->c21 = (int16_t) sys_get_be16(&raw_coef[14]);
	comp->c30 = (int16_t) sys_get_be16(&raw_coef[16]);
}

/* Poll one or multiple bits given by ready_mask in reg_addr */
bool poll_rdy(uint8_t reg_addr, uint8_t ready_mask)
{
	/* Try only a finite number of times */
	for (int i = 0; i < POLL_TRIES; i++) {
		uint8_t reg = 0;
		int res = i2c_reg_read_byte(data.i2c_master, dps368_config.i2c_addr,
					 reg_addr, &reg);
		if (res < 0) {
			LOG_WRN("I2C error: %d", res);
			return false;
		}

		if ((reg & ready_mask) == ready_mask) {
			/* measurement is ready */
			return true;
		}

		/* give the sensor more time */
		k_sleep(POLL_TIME_MS);
	}

	return false;
}

/* Trigger a temperature measurement and wait until the result is stored */
bool dps368_trigger_temperature()
{
	/* command to start temperature measurement */
	static const uint8_t tmp_meas_cmd[] = {
		IFX_DPS368_REG_ADDR_MEAS_CFG,
		IFX_DPS368_MODE_COMMAND_TEMPERATURE
	};

	/* trigger temperature measurement */
	int res = i2c_write(data.i2c_master, tmp_meas_cmd,
			    sizeof(tmp_meas_cmd), dps368_config.i2c_addr);
	if (res < 0) {
		LOG_WRN("I2C error: %d", res);
		return false;
	}

	/* give the sensor time to store measured values internally */
	k_msleep(IFX_DPS368_TMP_MEAS_TIME);

	if (!poll_rdy(IFX_DPS368_REG_ADDR_MEAS_CFG,
		      IFX_DPS368_REG_ADDR_MEAS_CFG_TMP_RDY)) {
		LOG_DBG("Poll timeout for temperature");
		return false;
	}

	return true;
}

/* Trigger a pressure measurement and wait until the result is stored */
bool dps368_trigger_pressure()
{
	/* command to start pressure measurement */
	static const uint8_t psr_meas_cmd[] = {
		IFX_DPS368_REG_ADDR_MEAS_CFG,
		IFX_DPS368_MODE_COMMAND_PRESSURE
	};

	/* trigger pressure measurement */
	int res = i2c_write(data.i2c_master, psr_meas_cmd,
			    sizeof(psr_meas_cmd), dps368_config.i2c_addr);
	if (res < 0) {
		LOG_WRN("I2C error: %d", res);
		return false;
	}

	/* give the sensor time to store measured values internally */
	k_msleep(IFX_DPS368_PSR_MEAS_TIME);

	if (!poll_rdy(IFX_DPS368_REG_ADDR_MEAS_CFG,
		      IFX_DPS368_REG_ADDR_MEAS_CFG_PRS_RDY)) {
		LOG_DBG("Poll timeout for pressure");
		return false;
	}

	return true;
}

/*
 * function to fix a hardware problem on some devices
 * you have this bug if you measure around 60°C when temperature is around 20°C
 * call dps368_hw_bug_fix() directly in the init() function to fix this issue
 */
void dps368_hw_bug_fix()
{
	/* setup the necessary 5 sequences to fix the hw bug */
	static const uint8_t hw_bug_fix_sequence[HW_BUG_FIX_SEQUENCE_LEN][2] = {
		/*
		 * First write valid signature on 0x0e and 0x0f
		 * to unlock address 0x62
		 */
		{ 0x0E, 0xA5 },
		{ 0x0F, 0x96 },
		/* Then update high gain value for Temperature */
		{ 0x62, 0x02 },
		/* Finally lock back the location 0x62 */
		{ 0x0E, 0x00 },
		{ 0x0F, 0x00 }
	};

	/* execute sequence for hw bug fix */
	for (int i = 0; i < HW_BUG_FIX_SEQUENCE_LEN; i++) {
		int res = i2c_write(data.i2c_master, hw_bug_fix_sequence[i], 2,
				    dps368_config.i2c_addr);
		if (res < 0) {
			LOG_WRN("I2C error: %d", res);
			return;
		}
	}
}

/*
 * Scale and compensate the raw temperature measurement value to micro °C
 * The formula is based on the Chapter 4.9.2 in the datasheet.
 */
void dps368_scale_temperature(int32_t tmp_raw)
{
	const struct dps368_cal_coeff *comp = &data.comp;

	/* first term, rescaled to micro °C */
	int32_t tmp_p0 = (1000000 / 2) * comp->c0;

	/* second term, rescaled to mirco °C */
	int32_t tmp_p1 =
		(((int64_t)1000000) * comp->c1 * tmp_raw) / IFX_DPS368_SF_TMP;

	/* calculate the temperature corresponding to the datasheet */
	int32_t tmp_final = tmp_p0 + tmp_p1; /* value is in micro °C */

	/* store calculated value */
	data.tmp_val1 = tmp_final / 1000000;
	data.tmp_val2 = tmp_final % 1000000;
        data.tmp_val = (float)tmp_final / 1000000;
}

/*
 * Scale and temperature compensate the raw pressure measurement value to
 * Kilopascal. The formula is based on the Chapter 4.9.1 in the datasheet.
 */
void dps368_scale_pressure(int32_t tmp_raw, int32_t psr_raw)
{
	const struct dps368_cal_coeff *comp = &data.comp;

	float psr = ((float)psr_raw) / IFX_DPS368_SF_PSR;
	float tmp = ((float)tmp_raw) / IFX_DPS368_SF_TMP;

	/* scale according to formula from datasheet */
	float psr_final = comp->c00;

	psr_final += psr * (comp->c10 + psr * (comp->c20 + psr * comp->c30));
	psr_final += tmp * comp->c01;
	psr_final += tmp * psr * (comp->c11 + psr * comp->c21);

	/* rescale from Pascal to Kilopascal */
	psr_final /= 1000;

	/* store calculated value */
	data.psr_val1 = psr_final;
	data.psr_val2 = (psr_final - data.psr_val1) * 1000000;
        data.psr_val = psr_final;
}

/* Convert the raw sensor data to int32_t */
int32_t raw_to_int24(const uint8_t raw[3])
{
	/* convert from twos complement */
	int32_t res = (int32_t) sys_get_be24(raw);

	if (res > POW_2_23_MINUS_1) {
		res -= POW_2_24;
	}

	return res;
}

/* perform a single measurement of temperature and pressure */
bool dps368_measure_tmp_psr()
{
	if (!dps368_trigger_temperature()) {
		return false;
	}

	if (!dps368_trigger_pressure()) {
		return false;
	}

	/* memory for pressure and temperature raw values */
	uint8_t value_raw[6];

	/* read pressure and temperature raw values in one continuous read */
	int res = i2c_write_read(data.i2c_master, dps368_config.i2c_addr,
				 &REG_ADDR_PSR_B2, 1, &value_raw,
				 sizeof(value_raw));
	if (res < 0) {
		LOG_WRN("I2C error: %d", res);
		return false;
	}

	/* convert raw data to int */
	int32_t psr_raw = raw_to_int24(&value_raw[0]);

	data.raw_tmp = raw_to_int24(&value_raw[3]);

	dps368_scale_temperature(data.raw_tmp);
	dps368_scale_pressure(data.raw_tmp, psr_raw);

	return true;
}

/*
 * perform a single pressure measurement
 * uses the stored temperature value for sensor temperature compensation
 * temperature must be measured regularly for good temperature compensation
 */
bool dps368_measure_psr()
{
	/* measure pressure */
	if (!dps368_trigger_pressure(dps368_config)) {
		return false;
	}

	/* memory for pressure raw value */
	uint8_t value_raw[3];

	/* read pressure raw values in one continuous read */
	int res = i2c_write_read(data.i2c_master, dps368_config.i2c_addr,
				 &REG_ADDR_PSR_B2, 1, &value_raw,
				 sizeof(value_raw));
	if (res < 0) {
		LOG_WRN("I2C error: %d", res);
		return false;
	}

	/* convert raw data to int */
	int32_t psr_raw = raw_to_int24(&value_raw[0]);

	dps368_scale_pressure(data.raw_tmp, psr_raw);

	return true;
}

/* perform a single temperature measurement */
bool dps368_measure_tmp()
{
	/* measure temperature */
	if (!dps368_trigger_temperature(dps368_config)) {
		return false;
	}

	/* memory for temperature raw value */
	uint8_t value_raw[3];

	/* read temperature raw values in one continuous read */
	int res = i2c_write_read(data.i2c_master, dps368_config.i2c_addr,
				 &REG_ADDR_TMP_B2, 1, &value_raw,
				 sizeof(value_raw));
	if (res < 0) {
		LOG_WRN("I2C error: %d", res);
		return false;
	}

	/* convert raw data to int */
	data.raw_tmp = raw_to_int24(&value_raw[0]);

	dps368_scale_temperature(data.raw_tmp);

	return true;
}

/* Initialize the sensor and apply the configuration */
int dps368_init()
{
	dps368_config.i2c_addr = 119;  //0x77 device adress
        dps368_config.i2c_bus_name = "I2C_0";

	data.i2c_master = device_get_binding(dps368_config.i2c_bus_name);
	if (data.i2c_master == NULL) {
		LOG_ERR("Failed to get I2C device");
		return -EINVAL;
	}

	uint8_t product_id = 0;
	int res = i2c_reg_read_byte(data.i2c_master, dps368_config.i2c_addr,
				 IFX_DPS368_REG_ADDR_PRODUCT_ID, &product_id);

	if (res < 0) {
		LOG_ERR("No device found");
		return -EINVAL;
	}

	if (product_id != IFX_DPS368_PRODUCT_ID) {
		LOG_ERR("Device is not a DPS368");
		return -EINVAL;
	}

	LOG_DBG("Init DPS368");
	/* give the sensor time to load the calibration data */
	k_sleep(K_MSEC(40));

	/* wait for the sensor to load the calibration data */
	if (!poll_rdy(REG_ADDR_MEAS_CFG,
		      IFX_DPS368_REG_ADDR_MEAS_CFG_SELF_INIT_OK)) {
		LOG_DBG("Sensor not ready");
		return -EIO;
	}

	/* read calibration coefficients */
	uint8_t raw_coef[18] = { 0 };

	res = i2c_write_read(data.i2c_master, dps368_config.i2c_addr,
			     &REG_ADDR_CALIB_COEFF_0, 1, &raw_coef, 18);
	if (res < 0) {
		LOG_WRN("I2C error: %d", res);
		return -EIO;
	}

	/* convert calibration coefficients */
	dps368_calib_coeff_creation(raw_coef, &data.comp);

	/*
	 * check which temperature sensor was used for calibration and use it
	 * for measurements.
	 */
	uint8_t tmp_coef_srce = 0;

	res = i2c_write_read(data.i2c_master, dps368_config.i2c_addr,
			     &REG_ADDR_COEF_SRCE, 1, &tmp_coef_srce, sizeof(tmp_coef_srce));
	if (res < 0) {
		LOG_WRN("I2C error: %d", res);
		return -EIO;
	}

	/* clear all other bits */
	tmp_coef_srce &= IFX_DPS368_COEF_SRCE_MASK;

	/* merge with temperature measurement configuration */
	tmp_coef_srce |= IFX_DPS368_TMP_CFG;

	/* set complete configuration in one write */
	const uint8_t config_seq[] = {
		IFX_DPS368_REG_ADDR_PRS_CFG,	/* start register address */
		IFX_DPS368_PSR_CFG,		/* PSR_CFG */
		tmp_coef_srce,			/* TMP_CFG */
		0x00,				/* MEAS_CFG */
		DPS368_CFG_REG			/* CFG_REG */
	};

	res = i2c_write(data.i2c_master, config_seq, sizeof(config_seq),
			dps368_config.i2c_addr);
	if (res < 0) {
		LOG_WRN("I2C error: %d", res);
		return -EIO;
	}

	dps368_hw_bug_fix(dps368_config);
	dps368_measure_tmp_psr(dps368_config);

	LOG_DBG("Init OK");
	return 0;
}
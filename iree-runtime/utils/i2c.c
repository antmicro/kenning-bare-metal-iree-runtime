/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "i2c.h"

ut_static i2c_t g_i2c = {.initialized = false};

static uint16_t round_up_divide(uint32_t a, uint32_t b) {
    const uint32_t result = ((a - 1) / b) + 1;
    return (uint16_t)result;
}

static i2c_timing_config_t get_default_config_for_speed(I2C_SPEED speed, uint32_t clock_period_nanos)
{
    switch (speed) {
        case I2C_SPEED_STANDARD:
            return (i2c_timing_config_t) {
                .scl_time_high_cycles = round_up_divide(4000, clock_period_nanos),
                .scl_time_low_cycles = round_up_divide(4700, clock_period_nanos),
                .start_signal_setup_cycles = round_up_divide(4700, clock_period_nanos),
                .start_signal_hold_cycles = round_up_divide(4000, clock_period_nanos),
                .data_signal_setup_cycles = round_up_divide(250, clock_period_nanos),
                .data_signal_hold_cycles = 1,
                .stop_signal_setup_cycles = round_up_divide(4000, clock_period_nanos),
                .stop_signal_hold_cycles = round_up_divide(4700, clock_period_nanos),
            };
        case I2C_SPEED_FAST:
            return (i2c_timing_config_t){
                .scl_time_high_cycles = round_up_divide(600, clock_period_nanos),
                .scl_time_low_cycles = round_up_divide(1300, clock_period_nanos),
                .start_signal_setup_cycles = round_up_divide(600, clock_period_nanos),
                .start_signal_hold_cycles = round_up_divide(600, clock_period_nanos),
                .data_signal_setup_cycles = round_up_divide(100, clock_period_nanos),
                .data_signal_hold_cycles = 1,
                .stop_signal_setup_cycles = round_up_divide(600, clock_period_nanos),
                .stop_signal_hold_cycles = round_up_divide(1300, clock_period_nanos),
            };
        case I2C_SPEED_FAST_PLUS:
            return (i2c_timing_config_t){
                .scl_time_high_cycles = round_up_divide(260, clock_period_nanos),
                .scl_time_low_cycles = round_up_divide(500, clock_period_nanos),
                .start_signal_setup_cycles = round_up_divide(260, clock_period_nanos),
                .start_signal_hold_cycles = round_up_divide(260, clock_period_nanos),
                .data_signal_setup_cycles = round_up_divide(50, clock_period_nanos),
                .data_signal_hold_cycles = 1,
                .stop_signal_setup_cycles = round_up_divide(260, clock_period_nanos),
                .stop_signal_hold_cycles = round_up_divide(500, clock_period_nanos),
            };
        default:
            return (i2c_timing_config_t){0};
    }
}

static I2C_STATUS i2c_set_timings(const i2c_timing_config_t *config)
{
    VALIDATE_POINTER(config, I2C_STATUS_INVALID_POINTER);
    if (!g_i2c.initialized)
    {
        return I2C_STATUS_UNINITIALIZED;
    }

    uint32_t timing0 = 0;
    timing0 = SET_REG_FIELD(timing0, TIMING0_THIGH, config->scl_time_high_cycles);
    timing0 = SET_REG_FIELD(timing0, TIMING0_TLOW, config->scl_time_low_cycles);
    g_i2c.registers->TIMING0 = timing0;

    uint32_t timing1 = 0;
    timing1 = SET_REG_FIELD(timing1, TIMING1_T_R, config->rise_cycles);
    timing1 = SET_REG_FIELD(timing1, TIMING1_T_F, config->fall_cycles);
    g_i2c.registers->TIMING1 = timing1;

    uint32_t timing2 = 0;
    timing2 = SET_REG_FIELD(timing2, TIMING2_TSU_STA, config->start_signal_setup_cycles);
    timing2 = SET_REG_FIELD(timing2, TIMING2_THD_STA, config->start_signal_hold_cycles);
    g_i2c.registers->TIMING2 = timing2;

    uint32_t timing3 = 0;
    timing3 = SET_REG_FIELD(timing3, TIMING3_TSU_DAT, config->data_signal_setup_cycles);
    timing3 = SET_REG_FIELD(timing3, TIMING3_THD_DAT, config->data_signal_hold_cycles);
    g_i2c.registers->TIMING3 = timing3;

    uint32_t timing4 = 0;
    timing4 = SET_REG_FIELD(timing4, TIMING4_TSU_STO, config->stop_signal_setup_cycles);
    timing4 = SET_REG_FIELD(timing4, TIMING4_T_BUF, config->stop_signal_hold_cycles);
    g_i2c.registers->TIMING4 = timing4;

    return I2C_STATUS_OK;
}

static I2C_STATUS i2c_reset_rx_fifo() {
    if (!g_i2c.initialized)
    {
        return I2C_STATUS_UNINITIALIZED;
    }

    g_i2c.registers->FIFO_CTRL = SET_REG_FIELD(g_i2c.registers->FIFO_CTRL, FIFO_CTRL_RXRST, 1);

    return I2C_STATUS_OK;
}

static I2C_STATUS i2c_reset_fmt_fifo() {
    if (!g_i2c.initialized)
    {
        return I2C_STATUS_UNINITIALIZED;
    }

    g_i2c.registers->FIFO_CTRL = SET_REG_FIELD(g_i2c.registers->FIFO_CTRL, FIFO_CTRL_FMTRST, 1);

    return I2C_STATUS_OK;
}

static I2C_STATUS i2c_reset_acq_fifo() {
    if (!g_i2c.initialized)
    {
        return I2C_STATUS_UNINITIALIZED;
    }

    g_i2c.registers->FIFO_CTRL = SET_REG_FIELD(g_i2c.registers->FIFO_CTRL, FIFO_CTRL_ACQRST, 1);

    return I2C_STATUS_OK;
}

static I2C_STATUS i2c_reset_tx_fifo() {
    if (!g_i2c.initialized)
    {
        return I2C_STATUS_UNINITIALIZED;
    }

    g_i2c.registers->FIFO_CTRL = SET_REG_FIELD(g_i2c.registers->FIFO_CTRL, FIFO_CTRL_TXRST, 1);

    return I2C_STATUS_OK;
}

I2C_STATUS i2c_init(const i2c_config_t *config)
{
    VALIDATE_POINTER(config, I2C_STATUS_INVALID_POINTER);

    g_i2c.registers = (i2c_registers *)I2C_ADDRESS;

    // Timing parameter initialization
    i2c_timing_config_t timing_config = get_default_config_for_speed(I2C_SPEED_STANDARD, config->clock_period_nanos);
    i2c_set_timings(&timing_config);

    // FIFO reset and configuration
    i2c_reset_rx_fifo();
    i2c_reset_fmt_fifo();
    i2c_reset_acq_fifo();
    i2c_reset_tx_fifo();

    // Interrupt configuration (no interrupts enabled)

    // Enable I2C Host or Target functionality
    g_i2c.registers->CTRL = CTRL_ENABLEHOST;

    g_i2c.initialized = true;

    LOG_INFO("INTR_STATE: %x", g_i2c.registers->INTR_STATE);
    LOG_INFO("INTR_ENABLE: %x", g_i2c.registers->INTR_ENABLE);
    LOG_INFO("INTR_TEST: %x", g_i2c.registers->INTR_TEST);
    LOG_INFO("ALERT_TEST: %x", g_i2c.registers->ALERT_TEST);
    LOG_INFO("CTRL: %x", g_i2c.registers->CTRL);
    LOG_INFO("STATUS: %x", g_i2c.registers->STATUS);
    LOG_INFO("RDATA: %x", g_i2c.registers->RDATA);
    LOG_INFO("FDATA: %x", g_i2c.registers->FDATA);
    LOG_INFO("FIFO_CTRL: %x", g_i2c.registers->FIFO_CTRL);
    LOG_INFO("FIFO_STATUS: %x", g_i2c.registers->FIFO_STATUS);
    LOG_INFO("OVRD: %x", g_i2c.registers->OVRD);
    LOG_INFO("VAL: %x", g_i2c.registers->VAL);
    LOG_INFO("TIMING0: %x", g_i2c.registers->TIMING0);
    LOG_INFO("TIMING1: %x", g_i2c.registers->TIMING1);
    LOG_INFO("TIMING2: %x", g_i2c.registers->TIMING2);
    LOG_INFO("TIMING3: %x", g_i2c.registers->TIMING3);
    LOG_INFO("TIMING4: %x", g_i2c.registers->TIMING4);
    LOG_INFO("TIMEOUT_CTRL: %x", g_i2c.registers->TIMEOUT_CTRL);
    LOG_INFO("TARGET_ID: %x", g_i2c.registers->TARGET_ID);
    LOG_INFO("ACQDATA: %x", g_i2c.registers->ACQDATA);
    LOG_INFO("TXDATA: %x", g_i2c.registers->TXDATA);
    LOG_INFO("HOST_TIMEOUT_CTRL: %x", g_i2c.registers->HOST_TIMEOUT_CTRL);

    return I2C_STATUS_OK;
}

I2C_STATUS i2c_write_byte(const uint8_t byte)
{
    if (!g_i2c.initialized)
    {
        return I2C_STATUS_UNINITIALIZED;
    }

    return I2C_STATUS_OK;
}

I2C_STATUS i2c_write(const uint8_t *data, size_t data_length)
{
    VALIDATE_POINTER(data, I2C_STATUS_INVALID_POINTER);

    if (!g_i2c.initialized)
    {
        return I2C_STATUS_UNINITIALIZED;
    }

    return I2C_STATUS_OK;
}

I2C_STATUS i2c_read_byte(uint8_t *byte)
{
    VALIDATE_POINTER(byte, I2C_STATUS_INVALID_POINTER);

    if (!g_i2c.initialized)
    {
        return I2C_STATUS_UNINITIALIZED;
    }

    *byte = GET_REG_FIELD(g_i2c.registers->RDATA, RDATA_RDATA);

    return I2C_STATUS_OK;
}

I2C_STATUS i2c_read(const uint8_t *data, size_t data_length)
{
    VALIDATE_POINTER(data, I2C_STATUS_INVALID_POINTER);

    if (!g_i2c.initialized)
    {
        return I2C_STATUS_UNINITIALIZED;
    }

    return I2C_STATUS_OK;
}

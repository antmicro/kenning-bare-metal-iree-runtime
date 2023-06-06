/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "i2c.h"

GENERATE_MODULE_STATUSES_STR(I2C);

ut_static i2c_t g_i2c = {.initialized = false};

static uint16_t round_up_divide(uint32_t a, uint32_t b)
{
    const uint32_t result = ((a - 1) / b) + 1;
    return (uint16_t)result;
}

static I2C_STATUS get_default_config_for_speed(I2C_SPEED speed, uint32_t clock_period_nanos,
                                               i2c_timing_config_t *config)
{
    VALIDATE_POINTER(config, I2C_STATUS_INV_PTR);

    switch (speed)
    {
    case I2C_SPEED_STANDARD:
        config->scl_time_high_cycles = round_up_divide(4000, clock_period_nanos);
        config->scl_time_low_cycles = round_up_divide(4700, clock_period_nanos);
        config->start_signal_setup_cycles = round_up_divide(4700, clock_period_nanos);
        config->start_signal_hold_cycles = round_up_divide(4000, clock_period_nanos);
        config->data_signal_setup_cycles = round_up_divide(250, clock_period_nanos);
        config->data_signal_hold_cycles = 1;
        config->stop_signal_setup_cycles = round_up_divide(4000, clock_period_nanos);
        config->stop_signal_hold_cycles = round_up_divide(4700, clock_period_nanos);
        break;
    case I2C_SPEED_FAST:
        config->scl_time_high_cycles = round_up_divide(600, clock_period_nanos);
        config->scl_time_low_cycles = round_up_divide(1300, clock_period_nanos);
        config->start_signal_setup_cycles = round_up_divide(600, clock_period_nanos);
        config->start_signal_hold_cycles = round_up_divide(600, clock_period_nanos);
        config->data_signal_setup_cycles = round_up_divide(100, clock_period_nanos);
        config->data_signal_hold_cycles = 1;
        config->stop_signal_setup_cycles = round_up_divide(600, clock_period_nanos);
        config->stop_signal_hold_cycles = round_up_divide(1300, clock_period_nanos);
        break;
    case I2C_SPEED_FAST_PLUS:
        config->scl_time_high_cycles = round_up_divide(260, clock_period_nanos);
        config->scl_time_low_cycles = round_up_divide(500, clock_period_nanos);
        config->start_signal_setup_cycles = round_up_divide(260, clock_period_nanos);
        config->start_signal_hold_cycles = round_up_divide(260, clock_period_nanos);
        config->data_signal_setup_cycles = round_up_divide(50, clock_period_nanos);
        config->data_signal_hold_cycles = 1;
        config->stop_signal_setup_cycles = round_up_divide(260, clock_period_nanos);
        config->stop_signal_hold_cycles = round_up_divide(500, clock_period_nanos);
        break;
    default:
        return I2C_STATUS_INV_ARG;
    }

    return STATUS_OK;
}

static I2C_STATUS i2c_set_timings(const i2c_timing_config_t *config)
{
    VALIDATE_POINTER(config, I2C_STATUS_INV_PTR);

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

    return STATUS_OK;
}

void i2c_reset_rx_fifo() { g_i2c.registers->FIFO_CTRL = SET_REG_FIELD(g_i2c.registers->FIFO_CTRL, FIFO_CTRL_RXRST, 1); }

void i2c_reset_fmt_fifo()
{
    g_i2c.registers->FIFO_CTRL = SET_REG_FIELD(g_i2c.registers->FIFO_CTRL, FIFO_CTRL_FMTRST, 1);
}

void i2c_reset_acq_fifo()
{
    g_i2c.registers->FIFO_CTRL = SET_REG_FIELD(g_i2c.registers->FIFO_CTRL, FIFO_CTRL_ACQRST, 1);
}

void i2c_reset_tx_fifo() { g_i2c.registers->FIFO_CTRL = SET_REG_FIELD(g_i2c.registers->FIFO_CTRL, FIFO_CTRL_TXRST, 1); }

status_t i2c_init(const i2c_config_t *config)
{
    status_t status = STATUS_OK;

    VALIDATE_POINTER(config, I2C_STATUS_INV_PTR);

    g_i2c.registers = (i2c_registers_t *)I2C_ADDRESS;

    // Timing parameter initialization
    i2c_timing_config_t timing_config;
    status = get_default_config_for_speed(config->speed, config->clock_period_nanos, &timing_config);
    RETURN_ON_ERROR(status, status);
    i2c_set_timings(&timing_config);

    // FIFO reset
    i2c_reset_rx_fifo();
    i2c_reset_fmt_fifo();
    i2c_reset_acq_fifo();
    i2c_reset_tx_fifo();

    // Enable I2C Host or Target functionality
    g_i2c.registers->CTRL = CTRL_ENABLEHOST;

    g_i2c.initialized = true;

    return STATUS_OK;
}

status_t i2c_get_status(i2c_status_t *status)
{
    VALIDATE_POINTER(status, I2C_STATUS_INV_PTR);

    if (!g_i2c.initialized)
    {
        return I2C_STATUS_UNINIT;
    }

    memset(status, 0, sizeof(i2c_status_t));
    uint32_t status_reg = g_i2c.registers->STATUS;

    status->fmt_full = GET_REG_FIELD(status_reg, STATUS_FMT_FULL);
    status->rx_full = GET_REG_FIELD(status_reg, STATUS_RX_FULL);
    status->fmt_empty = GET_REG_FIELD(status_reg, STATUS_FMT_EMPTY);
    status->host_idle = GET_REG_FIELD(status_reg, STATUS_HOST_IDLE);
    status->target_idle = GET_REG_FIELD(status_reg, STATUS_TARGET_IDLE);
    status->rx_empty = GET_REG_FIELD(status_reg, STATUS_RX_EMPTY);
    status->tx_full = GET_REG_FIELD(status_reg, STATUS_TX_FULL);
    status->acq_full = GET_REG_FIELD(status_reg, STATUS_ACQ_FULL);
    status->tx_empty = GET_REG_FIELD(status_reg, STATUS_TX_EMPTY);
    status->acq_empty = GET_REG_FIELD(status_reg, STATUS_ACQ_EMPTY);

    return STATUS_OK;
}

status_t i2c_write_byte(const uint8_t byte, I2C_FORMAT format)
{
    if (!g_i2c.initialized)
    {
        return I2C_STATUS_UNINIT;
    }

    i2c_format_flags_t flags = {.no_ack_ok = true};

    switch (format)
    {
    case I2C_FORMAT_START:
        flags.start = true;
        break;
    case I2C_FORMAT_TX:
        break;
    case I2C_FORMAT_TX_STOP:
        flags.stop = true;
        break;
    case I2C_FORMAT_RX:
        flags.read = true;
        break;
    case I2C_FORMAT_RX_CONT:
        flags.read = true;
        flags.read_cont = true;
        break;
    case I2C_FORMAT_RX_STOP:
        flags.read = true;
        flags.stop = true;
        break;
    default:
        return I2C_STATUS_INV_ARG;
    }

    uint32_t fdata = 0;
    fdata = SET_REG_FIELD(fdata, FDATA_FBYTE, byte);
    fdata = SET_REG_FIELD(fdata, FDATA_START, flags.start);
    fdata = SET_REG_FIELD(fdata, FDATA_STOP, flags.stop);
    fdata = SET_REG_FIELD(fdata, FDATA_READ, flags.read);
    fdata = SET_REG_FIELD(fdata, FDATA_RCONT, flags.read_cont);
    fdata = SET_REG_FIELD(fdata, FDATA_NAKOK, flags.no_ack_ok);

    g_i2c.registers->FDATA = fdata;

    return STATUS_OK;
}

status_t i2c_read_byte(uint8_t *byte)
{
    VALIDATE_POINTER(byte, I2C_STATUS_INV_PTR);

    if (!g_i2c.initialized)
    {
        return I2C_STATUS_UNINIT;
    }

    *byte = GET_REG_FIELD(g_i2c.registers->RDATA, RDATA_RDATA);

    return STATUS_OK;
}

status_t i2c_write_target_register(uint8_t target_id, uint8_t address, uint8_t data)
{
    status_t status;

    if (!g_i2c.initialized)
    {
        return I2C_STATUS_UNINIT;
    }

    status = i2c_write_byte(target_id, I2C_FORMAT_START);
    RETURN_ON_ERROR(status, status);
    status = i2c_write_byte(address, I2C_FORMAT_TX);
    RETURN_ON_ERROR(status, status);
    status = i2c_write_byte(data, I2C_FORMAT_TX);
    RETURN_ON_ERROR(status, status);
    status = i2c_write_byte(1, I2C_FORMAT_RX_STOP);
    RETURN_ON_ERROR(status, status);

    return STATUS_OK;
}

status_t i2c_read_target_register(uint8_t target_id, uint8_t address, uint8_t *data)
{
    return i2c_read_target_registers(target_id, address, 1, data);
}

status_t i2c_read_target_registers(uint8_t target_id, uint8_t address, size_t count, uint8_t *data)
{
    status_t status;

    VALIDATE_POINTER(data, I2C_STATUS_INV_PTR);

    if (!g_i2c.initialized)
    {
        return I2C_STATUS_UNINIT;
    }

    status = i2c_write_byte(target_id, I2C_FORMAT_START);
    RETURN_ON_ERROR(status, status);
    status = i2c_write_byte(address, I2C_FORMAT_TX);
    RETURN_ON_ERROR(status, status);
    status = i2c_write_byte(count, I2C_FORMAT_RX_STOP);
    RETURN_ON_ERROR(status, status);

    i2c_status_t i2c_status;
    size_t i = 0;
    while (1)
    {
        status = i2c_get_status(&i2c_status);
        RETURN_ON_ERROR(status, status);
        if (i2c_status.rx_empty || i >= count)
        {
            break;
        }
        status = i2c_read_byte(data + i);
        RETURN_ON_ERROR(status, status);
        ++i;
    }

    return STATUS_OK;
}

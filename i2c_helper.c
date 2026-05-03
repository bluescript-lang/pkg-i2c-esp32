#include <stdint.h>
#include <stdbool.h>
#include "driver/i2c_master.h"
#include "c-runtime.h"

void* get_native_ptr(value_t obj) {
    return &(value_to_ptr(obj))->body[2];
}

typedef enum {
    I2C_RESULT_OK = 0,
    I2C_RESULT_FAIL = 1,
    I2C_RESULT_INVALID_ARG = 2,
    I2C_RESULT_NO_MEM = 3,
    I2C_RESULT_NOT_FOUND = 4,
    I2C_RESULT_TIMEOUT = 5,
    I2C_RESULT_INVALID_RESPONSE = 6,
} I2C_RESULT;

I2C_RESULT esp_err_to_i2c_result(esp_err_t err) {
    switch (err) {
        case ESP_OK:
            return I2C_RESULT_OK;
        case ESP_ERR_INVALID_ARG:
            return I2C_RESULT_INVALID_ARG;
        case ESP_ERR_NO_MEM:
            return I2C_RESULT_NO_MEM;
        case ESP_ERR_NOT_FOUND: 
            return I2C_RESULT_NOT_FOUND;
        case ESP_ERR_INVALID_RESPONSE:
            return I2C_RESULT_INVALID_RESPONSE;
        case ESP_ERR_TIMEOUT:
            return I2C_RESULT_TIMEOUT;
        default:
            return I2C_RESULT_FAIL;
    }
}

// Bus

I2C_RESULT bus_new(int32_t port, int32_t sda, int32_t scl, int32_t *ret_handle) {
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = port,
        .scl_io_num = scl,
        .sda_io_num = sda,
        .glitch_ignore_cnt = 7,
    };
    i2c_master_bus_handle_t bus_handle = NULL;
    esp_err_t err = i2c_new_master_bus(&i2c_mst_config, &bus_handle);
    *ret_handle = (int32_t)bus_handle;
    return esp_err_to_i2c_result(err);
}

bool bus_probe(int32_t bus_handle, int32_t address, int32_t timeout_ms) {
    esp_err_t err = i2c_master_probe((i2c_master_bus_handle_t)bus_handle, address, timeout_ms);
    return err == ESP_OK;
}

I2C_RESULT bus_delete(int32_t bus_handle) {
    esp_err_t err = i2c_del_master_bus((i2c_master_bus_handle_t)bus_handle);
    return esp_err_to_i2c_result(err);
}


// Device

I2C_RESULT device_new(int32_t bus_handle, int32_t address, int32_t frequency, int32_t *ret_handle) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = frequency,
    };
    i2c_master_dev_handle_t dev_handle = NULL;
    esp_err_t err = i2c_master_bus_add_device((i2c_master_bus_handle_t)bus_handle, &dev_cfg, &dev_handle);
    *ret_handle = (int32_t)dev_handle;
    return esp_err_to_i2c_result(err);
}

I2C_RESULT device_write(int32_t dev_handle, uint8_t* buf, int32_t length, int32_t timeout_ms) {
    esp_err_t err = i2c_master_transmit((i2c_master_dev_handle_t)dev_handle, buf, length, timeout_ms);
    return esp_err_to_i2c_result(err);
}

I2C_RESULT device_read(int32_t dev_handle, uint8_t* buf, int32_t length, int32_t timeout_ms) {
    esp_err_t err = i2c_master_receive((i2c_master_dev_handle_t)dev_handle, buf, length, timeout_ms);
    return esp_err_to_i2c_result(err);
}

I2C_RESULT device_write_read(
    int32_t dev_handle, 
    uint8_t* write_buf, int32_t write_length,
    uint8_t* read_buf, int32_t read_length,
    int32_t timeout_ms
) {
    esp_err_t err = i2c_master_transmit_receive((i2c_master_dev_handle_t)dev_handle, write_buf, write_length, read_buf, read_length, timeout_ms);
    return esp_err_to_i2c_result(err);
}

I2C_RESULT device_delete(int32_t dev_handle) {
    esp_err_t err = i2c_master_bus_rm_device((i2c_master_dev_handle_t)dev_handle);
    return esp_err_to_i2c_result(err);
}


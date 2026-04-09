#include <stdint.h>
#include <stdbool.h>
#include "driver/i2c_master.h"
#include "c-runtime.h"

void* get_native_ptr(value_t obj) {
    return &(value_to_ptr(obj))->body[2];
}

i2c_master_bus_handle_t* get_bus_handle(value_t handle_obj) {
    return (i2c_master_bus_handle_t*)get_native_ptr(handle_obj);
}

i2c_master_dev_handle_t* get_dev_handle(value_t handle_obj) {
    return (i2c_master_dev_handle_t*)get_native_ptr(handle_obj);
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

I2C_RESULT bus_init(i2c_master_bus_handle_t* bus_handle, int32_t port, int32_t sda, int32_t scl) {
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = port,
        .scl_io_num = scl,
        .sda_io_num = sda,
        .glitch_ignore_cnt = 7,
    };
    esp_err_t err = i2c_new_master_bus(&i2c_mst_config, bus_handle);
    return esp_err_to_i2c_result(err);
}

bool bus_probe(i2c_master_bus_handle_t* bus_handle, int32_t address, int32_t timeout_ms) {
    esp_err_t err = i2c_master_probe(*bus_handle, address, timeout_ms);
    return err == ESP_OK;
}

I2C_RESULT bus_delete(i2c_master_bus_handle_t* bus_handle) {
    esp_err_t err = i2c_del_master_bus(*bus_handle);
    return esp_err_to_i2c_result(err);
}


// Device

I2C_RESULT device_init(i2c_master_bus_handle_t* bus_handle, i2c_master_dev_handle_t* dev_handle, int32_t address, int32_t frequency) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = frequency,
    };
    esp_err_t err = i2c_master_bus_add_device(*bus_handle, &dev_cfg, dev_handle);
    return esp_err_to_i2c_result(err);
}

I2C_RESULT device_write(i2c_master_dev_handle_t* dev_handle, uint8_t* buf, int32_t length, int32_t timeout_ms) {
    esp_err_t err = i2c_master_transmit(*dev_handle, buf, length, timeout_ms);
    return esp_err_to_i2c_result(err);
}

I2C_RESULT device_read(i2c_master_dev_handle_t* dev_handle, uint8_t* buf, int32_t length, int32_t timeout_ms) {
    esp_err_t err = i2c_master_receive(*dev_handle, buf, length, timeout_ms);
    return esp_err_to_i2c_result(err);
}

I2C_RESULT device_write_read(
    i2c_master_dev_handle_t* dev_handle, 
    uint8_t* write_buf, int32_t write_length,
    uint8_t* read_buf, int32_t read_length,
    int32_t timeout_ms
) {
    esp_err_t err = i2c_master_transmit_receive(*dev_handle, write_buf, write_length, read_buf, read_length, timeout_ms);
    return esp_err_to_i2c_result(err);
}

I2C_RESULT device_delete(i2c_master_dev_handle_t* dev_handle) {
    esp_err_t err = i2c_master_bus_rm_device(*dev_handle);
    return esp_err_to_i2c_result(err);
}


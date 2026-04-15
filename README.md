# I2C Library for ESP32

A high-level I2C (Inter-Integrated Circuit) master library for **BlueScript** on ESP32.
This package provides an easy-to-use interface for initializing I2C buses, scanning for devices, and performing read/write operations (including specific register operations) with connected I2C devices.

## Installation

Install this package in your BlueScript project:

```bash
bscript project install https://github.com/bluescript-lang/pkg-i2c-esp32.git
```

## Usage

### Scan I2C Devices

Initialize an I2C bus and scan for connected devices.

```typescript
import { I2CMasterBus, I2CPort } from "i2c";

// Configure I2C Bus on Port 0 (SDA: Pin 21, SCL: Pin 22)
const bus = new I2CMasterBus(I2CPort.I2C0, 21, 22);
const timeoutMs = 100;

console.log("Scanning I2C bus...");
const devices = bus.scan(timeoutMs);

for (let i = 0; i < devices.length; i++) {
    console.log("Found device at address: 0x" + devices[i].toString(16));
}

bus.close();
```

### Read/Write Device Registers

Communicate with a specific I2C device (e.g., an MPU6050 sensor).

```typescript
import { I2CMasterBus, I2CDevice, I2CPort } from "i2c";

const bus = new I2CMasterBus(I2CPort.I2C0, 21, 22);

// Target device at address 0x68, 400kHz frequency, 1000ms timeout
const sensor = new I2CDevice(bus, 0x68, 400000, 1000);

if (sensor.probe()) {
    console.log("Sensor detected!");

    // Wake up the sensor by writing 0x00 to the power management register (0x6B)
    sensor.writeRegisterByte(0x6B, 0x00);
    
    // Read 1 byte from the WHO_AM_I register (0x75)
    const data = sensor.readRegister(0x75, 1);
    console.log("Device ID:" + data[0]);
} else {
    console.log("Sensor not found.");
}

sensor.close();
bus.close();
```

## API Reference

### Class: `I2CMasterBus`

Represents an I2C master bus hardware controller.

#### `constructor(port: I2CPort, sda: integer, scl: integer)`
Initializes the I2C master bus.
- **port**: The hardware I2C port to use (`I2CPort.I2C0`, `I2CPort.I2C1`, or `I2CPort.Auto`).
- **sda**: The GPIO pin number for SDA (Data).
- **scl**: The GPIO pin number for SCL (Clock).

#### `probe(address: integer, timeoutMs: integer): boolean`
Checks if a device acknowledges communication at the given address.
- **Returns**: `true` if a device is found, `false` otherwise.

#### `scan(timeoutMs: integer): integer[]`
Scans standard 7-bit addresses (0x08 to 0x77) to find connected devices.
- **Returns**: An array of integer addresses where devices were detected.

#### `close(): void`
Deletes the bus instance and releases hardware resources.

#### **Properties**
- `lastOperationResult: I2CResult` - The result of the last bus operation.


### Class: `I2CDevice`

Represents a specific I2C slave device connected to a master bus.

#### `constructor(bus: I2CMasterBus, address: integer, frequency: integer, timeoutMs: integer)`
Initializes communication with a specific I2C device.
- **bus**: The initialized `I2CMasterBus` instance.
- **address**: The 7-bit I2C address of the device.
- **frequency**: Clock frequency in Hz (e.g., `100000` for standard, `400000` for fast mode).
- **timeoutMs**: Default timeout in milliseconds for operations on this device.

#### `write(data: Uint8Array): void`
Writes an array of bytes to the device.

#### `read(length: integer): Uint8Array`
Reads the specified number of bytes from the device.
- **Returns**: A `Uint8Array` containing the read data.

#### `writeRead(writeData: Uint8Array, readLength: integer): Uint8Array`
Performs a combined write-then-read operation without a stop condition in between (Repeated Start).

#### `writeRegister(register: integer, data: integer[]): void`
Writes multiple bytes to a specific register address.

#### `writeRegisterByte(register: integer, value: integer): void`
Writes a single byte to a specific register address.

#### `readRegister(register: integer, length: integer): Uint8Array`
Reads the specified number of bytes from a specific register address.

#### `probe(): boolean`
Checks if the device is currently responding on the bus.

#### `close(): void`
Frees resources associated with the device.

#### **Properties**
- `lastOperationResult: I2CResult` - The result of the last device operation.


## Enums

### `I2CPort`
Defines the I2C hardware controller port.

| Name | Value | Description |
| :--- | :--- | :--- |
| `I2C0` | 0 | I2C Port 0 |
| `I2C1` | 1 | I2C Port 1 |
| `Auto` | 2 | Automatically select an available port |

### `I2CResult`
Return values representing the status of the last I2C operation.

| Name | Value | Description |
| :--- | :--- | :--- |
| `OK` | 0 | Operation succeeded |
| `Fail` | 1 | Generic failure |
| `InvalidArg` | 2 | Invalid argument provided |
| `NoMemory` | 3 | Memory allocation failed |
| `NotFound` | 4 | Device or bus not found |
| `Timeout` | 5 | Operation timed out |
| `InvalidResponse` | 6 | Invalid response from device |


## Error Handling (`lastOperationResult`)

Because many methods in this library (such as `write`, `read`, `writeRegister`, and `close`) do not return success or error codes directly, both `I2CMasterBus` and `I2CDevice` classes maintain a `lastOperationResult` property. 

This property is overwritten every time an internal I2C function is called. You can inspect this property immediately after performing an operation or initializing an object to verify whether it succeeded (`I2CResult.OK`) or failed due to reasons like a timeout or missing device.

### Example: Checking for Errors

```typescript
import { I2CMasterBus, I2CDevice, I2CPort, I2CResult } from "i2c";

// 1. Check Bus Initialization
const bus = new I2CMasterBus(I2CPort.I2C0, 21, 22);
if (bus.lastOperationResult !== I2CResult.OK) {
    console.log("Failed to initialize I2C bus!");
}

const sensor = new I2CDevice(bus, 0x68, 400000, 1000);

// 2. Perform an operation
sensor.writeRegisterByte(0x6B, 0x00);

// 3. Check the result
if (sensor.lastOperationResult === I2CResult.OK) {
    console.log("Write operation successful.");
} else if (sensor.lastOperationResult === I2CResult.Timeout) {
    console.log("Device did not respond in time (Timeout).");
} else {
    console.log("Operation failed with error code:" + sensor.lastOperationResult);
}

sensor.close();
bus.close();
```

By checking `lastOperationResult`, you can gracefully handle hardware disconnections, noisy lines, or communication glitches without crashing your BlueScript application.

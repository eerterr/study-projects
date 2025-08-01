# Assigment 3. Smart Home system

**Time limit per test:** 1 second  
**Memory limit per test:** 256 megabytes

---

## Smart Home System

### Description:

The smart home system is designed to manage various smart devices, including Lights, Cameras, and Heaters. Each device has capabilities for being activated or deactivated, alongside particular features and settings that can be manipulated when the devices are in the ON state.

Here is the UML class diagram that represents the structure of the code:

---

### Device States:

- **Lights:** Each light can be in one of two states: ON or OFF. When the lights are ON, their brightness and color can be modified. When they are OFF, these attributes cannot be changed.
- **Cameras:** Each camera can also be in one of two states: ON or OFF. When ON, cameras can record, and their angles can be adjusted. When OFF, cameras cannot record, and their angles cannot be changed.
- **Heaters:** Each heater has two states: ON or OFF. When ON, the temperature can be set within the specified range. When OFF, the temperature cannot be modified.

---

### Initial Status of the Home:

- **Lights:** There are four lights, all of which are initially turned ON. They have a LOW brightness setting and are configured to emit a YELLOW color. They are not charging.
- **Cameras:** The system includes two cameras, both initially turned ON. They are not recording, not charging, and are positioned with a camera angle of 45°.
- **Heaters:** The system has four heaters, all turned ON and set to a temperature of 20°C.

---

### Rules:

- When any device is turned OFF, adjustments to any attributes are restricted except the charging status attribute for Lights and Cameras.
- The lights can only be set to either YELLOW or WHITE colors.
- Brightness levels can be categorized as LOW, MEDIUM, or HIGH.
- Each camera's angle can be adjusted independently within a vertical integer range from -60° to +60°.
- The temperature settings for heaters must remain within the integer range from 15°C to 30°C.
- The Lights and Cameras are chargeable, while the Heaters are not.

---

### Initial Device IDs and Names:

- **Lights:** The lights are identified by device IDs ranging from 0 to 3.
- **Cameras:** The cameras are identified by device IDs 4 and 5.
- **Heaters:** The heaters are identified by device IDs ranging from 6 to 9.
- **Names:** The name of any device should be from the following set of names: {Camera, Heater, Light}.

---

### Commands:

| Command | Description |
|--------|-------------|
| `DisplayAllStatus` | Displays the current status of all devices in the system. |
| `TurnOn {deviceName} {deviceId}` | Turns a specified device ON. |
| `TurnOff {deviceName} {deviceId}` | Turns a specified device OFF. |
| `StartCharging {deviceName} {deviceId}` | Starts charging a specified device, if chargeable. |
| `StopCharging {deviceName} {deviceId}` | Stops charging a specified device, if chargeable. |
| `SetTemperature {deviceName} {deviceId} {temperature}` | Sets the temperature for a specified heater device within its allowable range [15, 30]. |
| `SetBrightness {deviceName} {deviceId} {brightnessLevel}` | Sets the brightness level (LOW, MEDIUM, HIGH) for a specified light device. |
| `SetColor {deviceName} {deviceId} {color}` | Sets the color (YELLOW or WHITE) for a specified light device. |
| `SetAngle {deviceName} {deviceId} {angle}` | Adjusts the camera angle within the range [-60, 60] for a specified camera device. |
| `StartRecording {deviceName} {deviceId}` | Starts recording for a specified camera device. |
| `StopRecording {deviceName} {deviceId}` | Stops recording for a specified camera device. |
| `end` | It is the end of the input, after which the program should stop. |

---

### Status Display:

- **Lights:**  
  `Light {deviceId} is {status}, the color is {color}, the charging status is {chargingStatus}, and the brightness level is {brightnessLevel}.`

- **Cameras:**  
  `Camera {deviceId} is {status}, the angle is {angle}, the charging status is {chargingStatus}, and the recording status is {recordingStatus}.`

- **Heaters:**  
  `Heater {deviceId} is {status} and the temperature is {temperature}.`

---

### Errors:

| Condition | Error Message |
|----------|----------------|
| The command doesn't follow the structure of a command or has incorrect values | `Invalid command` |
| Device with specified name and ID not found | `The smart device was not found` |
| Charging a non-chargeable device | `{deviceName} {deviceId} is not chargeable` |
| Setting temperature for non-heater | `{deviceName} {deviceId} is not a heater` |
| Brightness out of scope | `The brightness can only be one of "LOW", "MEDIUM", or "HIGH"` |
| Brightness for non-light | `{deviceName} {deviceId} is not a light` |
| Invalid light color | `The light color can only be "YELLOW" or "WHITE"` |
| Color for non-light | `{deviceName} {deviceId} is not a light` |
| Camera angle for non-camera | `{deviceName} {deviceId} is not a camera` |
| Recording on non-camera | `{deviceName} {deviceId} is not a camera` |
| Device already OFF | `{deviceName} {deviceId} is already off` |
| Device already ON | `{deviceName} {deviceId} is already on` |
| Changing attributes while OFF (except charging) | `You can't change the status of the {deviceName} {deviceId} while it is off` |
| Camera angle out of range | `Camera {deviceId} angle should be in the range [-60, 60]` |
| Start recording when already recording | `{deviceName} {deviceId} is already recording` |
| Stop recording when not recording | `{deviceName} {deviceId} is not recording` |
| Charging a device already charging | `{deviceName} {deviceId} is already charging` |
| Stop charging on device not charging | `{deviceName} {deviceId} is not charging` |
| Heater temperature out of range | `Heater {deviceId} temperature should be in the range [15, 30]` |

---

### ✅ Success Outputs:

| Condition | Success Message |
|----------|-----------------|
| TurnOn | `{deviceName} {deviceId} is on` |
| TurnOff | `{deviceName} {deviceId} is off` |
| StartCharging | `{deviceName} {deviceId} is charging` |
| StopCharging | `{deviceName} {deviceId} stopped charging` |
| SetTemperature | `{deviceName} {deviceId} temperature is set to {temperature}` |
| SetBrightness | `{deviceName} {deviceId} brightness level is set to {brightness}` |
| SetColor | `{deviceName} {deviceId} color is set to {color}` |
| SetAngle | `{deviceName} {deviceId} angle is set to {angle}` |
| StartRecording | `{deviceName} {deviceId} started recording` |
| StopRecording | `{deviceName} {deviceId} stopped recording` |

---

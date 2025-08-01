import java.util.Scanner;

/**
 * The main class that implements the work of SmartHomeManagementSystem.
 * @author Daria Evdokimova
 * @version 23
 */
public class SmartHomeManagementSystem {
    /**{@value} is the number of devices*/
    static final int NUMBER_OF_DEVICES = 10;
    /**{@value} is the minimal value of light ID*/
    static final int MIN_LIGHT_ID = 0;
    /**{@value} is the maximum value of light ID*/
    static final int MAX_LIGHT_ID = 3;
    /**{@value} is the minimal value of camera ID*/
    static final int MIN_CAMERA_ID = 4;
    /**{@value} is the minimal value of camera ID*/
    static final int MAX_CAMERA_ID = 5;
    /**{@value} is the minimal value of heater ID*/
    static final int MIN_HEATER_ID = 6;
    /**{@value} is the maximum value of heater ID*/
    static final int MAX_HEATER_ID = 9;
    /**{@value} is the default value of camera angle*/
    static final int DEFAULT_ANGLE = 45;
    /**{@value} is the default value of heater temperature*/
    static final int DEFAULT_TEMPERATURE = 20;
    /**{@value} is the minimal length of command*/
    static final int MIN_COMMAND_LENGTH = 1;
    /**{@value} is the average length of command*/
    static final int AVERAGE_COMMAND_LENGTH = 3;
    /**{@value} is the maximum length of command*/
    static final int MAX_COMMAND_LENGTH = 4;
    /**{@value} is the index under which the command is written in the received line*/
    static final int COMMAND_INDEX = 0;
    /**{@value} is the index under which the device name is written in the received line*/
    static final int NAME_INDEX = 1;
    /**{@value} is the index under which the device ID is written in the received line*/
    static final int ID_INDEX = 2;
    /**{@value} is the index under which the parameter is written in the received line*/
    static final int PARAMETER_INDEX = 3;

    /**
     * Method that checks whether the resulting string can be converted to a number or not.
     * @param strId line that needs to be checked
     * @return true if it is possible to convert string into integer and false otherwise
     */
    public static boolean isNumber(String strId) {
        try {
            int number = Integer.parseInt(strId);
            return true;
        } catch (NumberFormatException e) {
            return false;
        }
    }

    /**
     * Method that check does the device name match the command.
     * @param command command
     * @param deviceName name of device
     * @param deviceId ID of device
     */
    static void isValidDevice(String command, String deviceName, int deviceId) {
        //check is device name is valid for any command
        if (deviceName.equals("Light") || deviceName.equals("Camera") || deviceName.equals("Heater")) {
            //check is device is light if we get a one of the commands, which specific for light
            if (!deviceName.equals("Light") && (command.equals("SetColor") || command.equals("SetBrightness"))) {
                System.out.println(deviceName + " " + deviceId + " is not a light");
                return;
                //check is device is camera if we get a one of the commands, which specific for camera
            } else if ((command.equals("SetAngle") || command.equals("StartRecording")
                    || command.equals("StopRecording")) && !deviceName.equals("Camera")) {
                System.out.println(deviceName + " " + deviceId + " is not a camera");
                return;
                //check is device is heater if we get a one of the commands, which specific for heater
            } else if (!deviceName.equals("Heater") && command.equals("SetTemperature")) {
                System.out.println(deviceName + " " + deviceId + " is not a heater");
                return;
            }
        }
    }
    /**
     * Method to turn on the device.
     * @param deviceName name of device
     * @param deviceId ID of device
     * @param device type of device
     */
    static void turnOnDevice(String deviceName, int deviceId, SmartDevice device) {
        if (!device.isOn()) {
            device.turnOn();
            System.out.println(deviceName + " " + deviceId + " is on");
        } else {
            System.out.println(deviceName + " " + deviceId + " is already on");
        }
    }

    /**
     * Method to turn off the device.
     * @param deviceName name of device
     * @param deviceId ID of device
     * @param device type of device
     */
    static void turnOffDevice(String deviceName, int deviceId, SmartDevice device) {
        if (device.isOn()) {
            device.turnOff();
            if (deviceName.equals("Camera")) {
                Camera camera = (Camera) device;
                camera.stopRecording();
            }
            System.out.println(deviceName + " " + deviceId + " is off");
        } else {
            System.out.println(deviceName + " " + deviceId + " is already off");
        }
    }

    /**
     * Method which contains all the methods and outputs all the errors for the Light class.
     * @param smartDevices array of all smart devices
     * @param command command
     * @param deviceName name of device
     * @param deviceId ID of device
     * @param parameter variable that is used in some methods
     */
    public static void light(SmartDevice[] smartDevices, String command, String deviceName,
                             int deviceId, String parameter) {
        //check if ID of device is valid for lights
        if (deviceId < MIN_LIGHT_ID || deviceId > MAX_LIGHT_ID) {
            System.out.println("The smart device was not found");
            return;
        }
        //convert smart device into light (for using specific commands)
        Light light = (Light) smartDevices[deviceId];

        //depending on the command, we perform certain methods
        switch (command) {
            //if command is "turnOn" or "turnOff" perform methods "turnOnDevice" or "turnOffDevice"
            case "TurnOn" -> {
                turnOnDevice(deviceName, deviceId, light);
                return;
            }
            case "TurnOff" -> {
                turnOffDevice(deviceName, deviceId, light);
                return;
            }
            //if command is "StartCharging" or "StopCharging" starting or stopping the device charging
            case "StartCharging" -> {

                if (!light.isCharging()) {
                    light.startCharging();
                    System.out.println(deviceName + " " + deviceId + " is charging");
                } else {
                    System.out.println(deviceName + " " + deviceId + " is already charging");
                }
                return;
            }
            case "StopCharging" -> {

                if (light.isCharging()) {
                    light.stopCharging();
                    System.out.println(deviceName + " " + deviceId + " stopped charging");
                } else {
                    System.out.println(deviceName + " " + deviceId + " is not charging");
                }
                return;
            }
            default -> {
                //for commands that work only on enabled devices, check the device status
                if (light.checkStatusAccess()) {
                    if (command.equals("SetBrightness")) {
                        //to set the brightness level, convert the resulting string into an BrightnessLevel enum element
                        BrightnessLevel brightness;
                        switch (parameter) {
                            case "LOW" -> brightness = BrightnessLevel.LOW;
                            case "MEDIUM" -> brightness = BrightnessLevel.MEDIUM;
                            case "HIGH" -> brightness = BrightnessLevel.HIGH;
                            //if the string value does not match any of the enum values, we output an error message
                            default -> {
                                System.out.println("The brightness can only be one of \"LOW\", "
                                        + "\"MEDIUM\", or \"HIGH\"");
                                return;
                            }
                        }
                        //setting the converted value for the device
                        light.setBrightnessLevel(brightness);
                        System.out.println(deviceName + " " + deviceId + " brightness level is set to " + parameter);
                    }

                    if (command.equals("SetColor")) {
                        //to set the light color, we convert the resulting string into an LightColor enum element.
                        LightColor color;
                        switch (parameter) {
                            case "YELLOW" -> color = LightColor.YELLOW;
                            case "WHITE" -> color = LightColor.WHITE;
                            //if the string value does not match any of the enum values, output an error message
                            default -> {
                                System.out.println("The light color can only be \"YELLOW\" or \"WHITE\"");
                                return;
                            }
                        }
                        //setting the converted value for the device
                        light.setLightColor(color);
                        System.out.println(deviceName + " " + deviceId + " color is set to " + parameter);
                    }
                } else {
                    System.out.println("You can't change the status of the " + deviceName + " "
                            + deviceId + " while it is off");
                    return;
                }
            }
        }
    }

    /**
     * Method which contains all the methods and outputs all the errors for the Camera class.
     * @param smartDevices array of all smart devices
     * @param command command
     * @param deviceName name of device
     * @param deviceId ID of device
     * @param parameter variable that is used in some methods
     */
    public static void camera(SmartDevice[] smartDevices, String command, String deviceName,
                              int deviceId, String parameter) {
        //check if ID of device is valid for cameras
        if (deviceId != MIN_CAMERA_ID && deviceId != MAX_CAMERA_ID) {
            System.out.println("The smart device was not found");
            return;
        }
        //convert smart device into camera (for using specific commands)
        Camera camera = (Camera) smartDevices[deviceId];

        //depending on the command, we perform certain methods
        switch (command) {
            //if command is "turnOn" or "turnOff" perform methods "turnOnDevice" or "turnOffDevice"
            case "TurnOn" -> {
                turnOnDevice(deviceName, deviceId, camera);
                return;
            }
            case "TurnOff" -> {
                turnOffDevice(deviceName, deviceId, camera);
                return;
            }
            //if command is "StartCharging" or "StopCharging" starting or stopping the device charging
            case "StartCharging" -> {
                if (camera.isCharging()) {
                    System.out.println(deviceName + " " + deviceId + " is already charging");
                } else {
                    camera.startCharging();
                    System.out.println(deviceName + " " + deviceId + " is charging");
                }
                return;
            }
            case "StopCharging" -> {
                if (!camera.isCharging()) {
                    System.out.println(deviceName + " " + deviceId + " is not charging");
                } else {
                    camera.stopCharging();
                    System.out.println(deviceName + " " + deviceId + " stopped charging");
                }
                return;
            }
            default -> {
                //for commands that work only on enabled devices, check the device status
                if (camera.checkStatusAccess()) {
                    if (command.equals("SetAngle")) {
                        //to set the angle, convert the string into an integer.
                        //if value of angle not in the needed range, output an error message.
                        if (Integer.parseInt(parameter) < Camera.MIN_CAMERA_ANGLE || Integer.parseInt(parameter)
                                > Camera.MAX_CAMERA_ANGLE) {
                            System.out.println("Camera " + deviceId + " angle should be in the range [-60, 60]");
                            return;
                        }
                        //set new angle to device
                        camera.setCameraAngle(Integer.parseInt(parameter));
                        System.out.println(deviceName + " " + deviceId + " angle is set to " + parameter);
                    }

                    //if command is "StartRecording" or "StopRecording" starting or stopping the device recording
                    if (command.equals("StartRecording")) {
                        if (camera.isRecording()) {
                            System.out.println(deviceName + " " + deviceId + " is already recording");
                        } else {
                            camera.startRecording();
                            System.out.println(deviceName + " " + deviceId + " started recording");
                        }
                        return;
                    }
                    if (command.equals("StopRecording")) {
                        if (!camera.isRecording()) {
                            System.out.println(deviceName + " " + deviceId + " is not recording");
                        } else {
                            camera.stopRecording();
                            System.out.println(deviceName + " " + deviceId + " stopped recording");
                        }
                        return;
                    }
                } else {
                    System.out.println("You can't change the status of the " + deviceName + " "
                            + deviceId  + " while it is off");
                    return;
                }
            }
        }
    }

    /**
     * Method which contains all the methods and outputs all the errors for the Heater class.
     * @param smartDevices array of all smart devices
     * @param command command
     * @param deviceName name of device
     * @param deviceId ID of device
     * @param parameter variable that is used in some methods
     */
    public static void heater(SmartDevice[] smartDevices, String command, String deviceName,
                              int deviceId, String parameter) {
        //check if ID of device is valid for heaters
        if (deviceId < MIN_HEATER_ID || deviceId > MAX_HEATER_ID) {
            System.out.println("The smart device was not found");
            return;
        }

        //convert smart device into heater (for using specific commands)
        Heater heater = (Heater) smartDevices[deviceId];
        //set the device ID
        //heater.setDeviceId(deviceId);

        //depending on the command, we perform certain methods
        switch (command) {
            //if command is "turnOn" or "turnOff" perform methods "turnOnDevice" or "turnOffDevice"
            case "TurnOn" -> {
                turnOnDevice(deviceName, deviceId, heater);
                return;
            }
            case "TurnOff" -> {
                turnOffDevice(deviceName, deviceId, heater);
                return;
            }
            //if command is "StartCharging" or "StopCharging", output an error message.
            case "StartCharging", "StopCharging" -> {
                System.out.println(deviceName + " " + deviceId + " is not chargeable");
                return;
            }
            default -> {
                //for commands that work only on enabled devices, check the device status
                if (heater.checkStatusAccess()) {
                    if (command.equals("SetTemperature")) {
                        //to set the temperature, convert the string into an integer.
                        //if value of temperature not in the needed range, output an error message.
                        if (Integer.parseInt(parameter) < Heater.MIN_HEATER_TEMPERATURE
                                || Integer.parseInt(parameter) > Heater.MAX_HEATER_TEMPERATURE) {
                            System.out.println("Heater " + deviceId + " temperature should be "
                                    + "in the range [15, 30]");
                            return;
                            }
                            //set new temperature to device
                            heater.setTemperature(Integer.parseInt(parameter));
                            System.out.println(deviceName + " " + deviceId + " temperature is set to " + parameter);
                    }
                } else {
                    System.out.println("You can't change the status of the " + deviceName + " "
                            + deviceId + " while it is off");
                    return;
                }
            }
        }
    }

    /**
     * Main method in which program is running
     * @param args
     */
    public static void main(String[] args) {

        SmartDevice[] smartDevices = new SmartDevice[NUMBER_OF_DEVICES];

        //writing devices to an array of smart devices according to indexes
        for (int i = MIN_LIGHT_ID; i <= MAX_LIGHT_ID; i++) {
            smartDevices[i] = new Light(Status.ON, false, BrightnessLevel.LOW, LightColor.YELLOW);
            smartDevices[i].setDeviceId(i);
        }
        for (int i = MIN_CAMERA_ID; i <= MAX_CAMERA_ID; i++) {
            smartDevices[i] = new Camera(Status.ON, false, false, DEFAULT_ANGLE);
            smartDevices[i].setDeviceId(i);
        }
        for (int i = MIN_HEATER_ID; i <= MAX_HEATER_ID; i++) {
            smartDevices[i] = new Heater(Status.ON, DEFAULT_TEMPERATURE);
            smartDevices[i].setDeviceId(i);
        }
        Scanner scan = new Scanner(System.in);

        //scanning string with commands while command is not "end".
        while (!scan.hasNext("end")) {

            String command = " ";
            String  deviceName = " ";
            String parameter = " ";
            int deviceId = 0;

            //scanning each string and split it to an array "splitString"
            String string = scan.nextLine();
            String[] splitString = string.split(" ");

            //check length splitString (string with command) is valid
            if (splitString.length < MIN_COMMAND_LENGTH || splitString.length > MAX_COMMAND_LENGTH
                    || splitString.length == 2) {
                System.out.println("Invalid command");
                continue;
            }
            //writting first element of array splitSting into command
            command = splitString[COMMAND_INDEX];
            //check is command respond to lenght of splitString (string with command)
            if ((!command.equals("DisplayAllStatus") && !command.equals("end"))
                    && (splitString.length == MIN_COMMAND_LENGTH)) {
                System.out.println("Invalid command");
                continue;
            }
            //if command equals to "DisplayAllStatus", display status of each array element
            if (command.equals("DisplayAllStatus") && splitString.length == 1) {
                for (int i = 0; i < smartDevices.length; i++) {
                    System.out.println(smartDevices[i].displayStatus());
                }
                continue;
            }
            //depends on length of splitString (string with command)
            //check if command is valid and then write needed elements (three or four)
            if (splitString.length == AVERAGE_COMMAND_LENGTH) {

                if (!command.equals("TurnOn") && !command.equals("StartCharging") && !command.equals("TurnOff")
                        && !command.equals("StopCharging") && !command.equals("StartRecording")
                        && !command.equals("StopRecording")) {
                    System.out.println("Invalid command");
                    continue;
                } else {

                    deviceName = splitString[NAME_INDEX];

                    if (isNumber(splitString[ID_INDEX])) {
                        deviceId = Integer.parseInt(splitString[ID_INDEX]);
                    } else {
                        System.out.println("Invalid command");
                        continue;
                    }
                }
            }

            if (splitString.length == MAX_COMMAND_LENGTH) {

                if (!command.equals("SetTemperature") && !command.equals("SetBrightness")
                        && !command.equals("SetColor") && !command.equals("SetAngle")) {
                    System.out.println("Invalid command");
                    continue;
                } else {
                    deviceName = splitString[NAME_INDEX];

                    if (isNumber(splitString[ID_INDEX])) {
                        deviceId = Integer.parseInt(splitString[ID_INDEX]);
                    } else {
                        System.out.println("Invalid command");
                        continue;
                    }
                    parameter = splitString[PARAMETER_INDEX];
                }
            }
            //check is device respond to the command
            isValidDevice(command, deviceName, deviceId);
            //check is it possible to convert parameter to integer to command where it is needed
            if ((command.equals("SetAngle") || command.equals("SetTemperature")) && !isNumber(parameter)) {
                System.out.println("Invalid command");
                continue;
            }
            //check if device name is valid
            if (!deviceName.equals("Camera") && !deviceName.equals("Light") && !deviceName.equals("Heater")) {
                System.out.println("The smart device was not found");
                continue;
            }
            //do method for camera if name of device is "camera"
            if (deviceName.equals("Camera")) {
                camera(smartDevices, command, deviceName, deviceId, parameter);
            }
            //do method for light if name of device is "light"
            if (deviceName.equals("Light")) {
                light(smartDevices, command, deviceName, deviceId, parameter);
            }
            //do method for heater if name of device is "heater"
            if (deviceName.equals("Heater")) {
                heater(smartDevices, command, deviceName, deviceId, parameter);
            }
        }
    }
}
/**enum with possible light colors*/
enum LightColor {
    WHITE,
    YELLOW;
}
/**enum with possible status values*/
enum Status {
    ON,
    OFF;
}
/**enum with possible brightness level*/
enum BrightnessLevel {
    HIGH,
    MEDIUM,
    LOW;
}
/**interface, which consist all methods, which using charging status*/
interface Chargeable {
    public boolean isCharging();

    public boolean startCharging();

    public boolean stopCharging();
}
/**interface, which consist main methods, which using device status (ON or OFF)*/
interface Controllable {
    public boolean turnOff();

    public boolean turnOn();

    public boolean isOn();
}

/**
 * Parent class for all smart devices
 */
abstract class SmartDevice implements Controllable {
    /**value of status of device (ON or OFF) */
    private Status status;
    /**ID of device */
    private int deviceId;
    /**number of  devices*/
    private int numberOfDevices;
    public SmartDevice(Status status) {
        this.status = status;
    }

    /**
     * Method to convert status of device to string.
     * @return string with status value
     */
    public String displayStatus() {
        return status.toString();
    }

    /**
     * Method to get ID of device.
     * @return
     */
    public int getDeviceId() {
        return deviceId;
    }

    /**
     * Method to set ID of device.
     * @param deviceId
     */
    public void setDeviceId(int deviceId) {
        this.deviceId = deviceId;
    }

    /**
     * Method to get status of device.
     * @return status of device
     */
    public Status getStatus() {
        return status;
    }

    /**
     * Method to set status of device (ON or OFF).
     * @param status
     */
    public void setStatus(Status status) {
        this.status = status;
    }

    /**
     * Method to turn off the device.
     * @return true
     */
    @Override
    public boolean turnOff() {
        this.status = Status.OFF;
        return true;
    }

    /**
     * Method to turn on the device.
     * @return true
     */
    @Override
    public boolean turnOn() {
        this.status = Status.ON;
        return true;
    }

    /**
     * Method to check status off the device.
     * @return true if status ON and false otherwise
     */
    @Override
    public boolean isOn() {
        return status == Status.ON;
    }

    /**
     * Method to check status access.
     * @return true if status ON and false otherwise
     */
    public boolean checkStatusAccess() {
        return status == Status.ON;
    }
}

/**
 * Child class Light
 */
class Light extends SmartDevice implements Controllable, Chargeable {

    /**charging status of device */
    private boolean charging;
    /**brightness level of device */
    private BrightnessLevel brightnessLevel;
    /**color of light of device */
    private LightColor lightColor;

    public Light(Status status, boolean charging, BrightnessLevel brightnessLevel, LightColor lightColor) {
        super(status);
        this.charging = charging;
        this.brightnessLevel = brightnessLevel;
        this.lightColor = lightColor;
    }

    /**
     * Method to get light color of device.
     * @return
     */
    public LightColor getLightColor() {
        return lightColor;
    }

    /**
     * Method to set light color for device.
     * @param lightColor
     */
    public void setLightColor(LightColor lightColor) {
        this.lightColor = lightColor;
    }

    /**
     * Method to get brightness level of device.
     * @return brightnessLevel
     */
    public BrightnessLevel getBrightnessLevel() {
        return brightnessLevel;
    }

    /**
     * Method to set brightness level for device.
     * @param brightnessLevel
     */
    public void setBrightnessLevel(BrightnessLevel brightnessLevel) {
        this.brightnessLevel = brightnessLevel;
    }

    /**
     * Method to get charging status of the device.
     * @return charging status
     */
    @Override
    public boolean isCharging() {
        return charging;
    }

    /**
     * Method to start charging of the device.
     * @return true
     */
    @Override
    public boolean startCharging() {
        this.charging = true;
        return true;
    }

    /**
     * Method to stop charging of the device.
     * @return true
     */
    @Override
    public boolean stopCharging() {
        this.charging = false;
        return true;
    }

    /**
     * Method which display ID, status, angle, and charging and statuses.
     * @return format string with information
     */
    @Override
    public String displayStatus() {
        return String.format("Light %d is %s, the color is %s, "
                        + "the charging status is %s, and the brightness level is %s.",
                getDeviceId(), getStatus(), getLightColor(), isCharging(), getBrightnessLevel());
    }
}

/**
 * Child class Camera
 */
class Camera extends SmartDevice implements Controllable, Chargeable {

    /**{@value} is maximum value of camera angle*/
    static final int MAX_CAMERA_ANGLE = 60;
    /**{@value} is minimal value of camera angle*/
    static final int MIN_CAMERA_ANGLE = -60;

    /**charging status*/
    private boolean charging;
    /**recording status*/
    private boolean recording;
    /**value of the camera angle */
    private int angle;

    public Camera(Status status, boolean charging, boolean recording, int angle) {
        super(status);
        this.charging = charging;
        this.recording = recording;
        this.angle = angle;
    }

    /**
     * Method to get set camera angle.
     * @return angle
     */
    public int getAngle() {
        return angle;
    }

    /**
     * Method to set camera angle
     * @param angle
     * @return true
     */
    public boolean setCameraAngle(int angle) {
        this.angle = angle;
        return true;
    }

    /**
     * Method to get recording status of the device
     * @return device recording status
     */
    public boolean isRecording() {
        return recording;
    }

    /**
     * Method to start device recording.
     * @return true
     */
    public boolean startRecording() {
        this.recording = true;
        return true;
    }

    /**
     * Method to stop device recording.
     * @return true
     */
    public boolean stopRecording() {
        this.recording = false;
        return true;
    }

    /**
     * Method to get charging status of the device.
     * @return charging status
     */
    @Override
    public boolean isCharging() {
        return charging;
    }

    /**
     * Method to start charging of the device.
     * @return true
     */
    @Override
    public boolean startCharging() {
        this.charging = true;
        return true;
    }

    /**
     * Method to stop charging of the device.
     * @return true
     */
    @Override
    public boolean stopCharging() {
        this.charging = false;
        return true;
    }

    /**
     * Method which display ID, status, angle, and charging and statuses.
     * @return format string with information
     */
    @Override
    public String displayStatus() {
        return String.format("Camera %d is %s, the angle is %d, "
                        + "the charging status is %s, and the recording status is %s.",
                getDeviceId(), getStatus(), getAngle(), isCharging(), isRecording());

    }
}

/**
 * Child class Heater
 */
class Heater extends SmartDevice implements Controllable {

    /**{@value} is temperature of the heater*/
    private int temperature;
    /**{@value} is maximum possible value of the heater temperature*/
    static final int MAX_HEATER_TEMPERATURE = 30;
    /**{@value} is minimum possible value of the heater temperature*/
    static final int MIN_HEATER_TEMPERATURE = 15;
    public Heater(Status status, int temperature) {
        super(status);
        this.temperature = temperature;
    }

    /**
     * Method to get temperature of the heater
     * @return temperature of device
     */
    public int getTemperature() {
        return temperature;
    }

    /**
     * Method to set temperature of the heater
     * @param temperature
     * @return true
     */
    public boolean setTemperature(int temperature) {
        this.temperature = temperature;
        return true;
    }
    /**
     * Method to get info about ID, Status of device and its temperature
     * @return format string with information
     */
    @Override
    public String displayStatus() {
        return String.format("Heater %d is %s and the temperature is %d.",
                getDeviceId(), getStatus(), getTemperature());

    }
}

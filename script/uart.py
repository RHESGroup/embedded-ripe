import sys
from time import time
import serial


def isLinux() -> bool:
    # Returns True is the script is running on Linux, False otherwise
    if sys.platform == "linux" or sys.platform == "linux2":
        return True
    return False


def getSerial(port=None, baudrate=9600, timeout=5000) -> serial.Serial:
    # Open a serial connection on the specified port

    # default port setup
    if port is None:
        if isLinux():
            port = "/dev/ttyUSB1"
        else:
            port = "COM3"

    if baudrate is None:
        baudrate = 9600
    if timeout is None:
        timeout = 5000

    return serial.Serial(port=port, baudrate=baudrate, timeout=timeout)


def listPorts():
    # List serial ports detected and print them

    print("Serial Ports:")

    ports = sys.list_ports.comports()

    if not ports:
        print("\tNo serial port detected")
    else:
        for port in ports:
            print("\t" + str(port))


def sendCommand(port, baudrate, command):
    # Send a command through serial port specified and print device response
    ser = serial.Serial(port, baudrate)
    ser.write(bytes(command, 'utf-8'))

    line = ser.read(20)
    resp = line.decode('utf-8').split('\0')

    print("Device response:")
    print(resp[0])


def readDeviceMessage(ser: serial.Serial, len: int, expected: str = None) -> bool:
    # Read device message sent on serial connection and print it

    rawMessage = ser.read(len)
    message = rawMessage.decode('utf-8')

    print("[{}] {}".format("RX", message))

    if expected is None:
        return True

    if (message == expected):
        return True

    return False


def writeDeviceMessage(ser: serial.Serial, message: str):
    # Write the given message into the serial connection and print it

    rawMessage = bytes(message, 'utf-8')
    print("[{}] {}".format("TX", rawMessage))
    ser.write(rawMessage)

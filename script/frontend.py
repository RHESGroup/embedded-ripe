from uart import *
from terminal import *

code_ptr = [
    "ret",
    "funcptr",
    "structfuncptr"
]

funcs = [
    "memcpy",
    "strcpy",
    "strncpy",
    "memmove",
    "homebrew"
]

locations = [
    "stack",
    "bss",
    "data"
]

attacks = [
    "nonop",
    "nop",
    "indirectcall"
]

techniques = [
    "direct"
]

results = {
    "score": 0,
    "details": []
}

# -------------------------------------- @main

printTitle()
parser = createArgParser()
args = parser.parse_args()

if args.ports:
    listPorts()
    exit(0)

attack_code = 0
ser = getSerial("COM3", args.baudrate, args.timeout)

for technique in techniques:
    for location in locations:
        for ptr in code_ptr:
            for attack in attacks:
                for function in funcs:

                    if readDeviceMessage(ser, "DEVICE->ONLINE", 14):
                        writeDeviceMessage(
                            ser, "HOST->"+str(attack_code).zfill(4))
                    if readDeviceMessage(ser, "DEVICE->DONE", 12):
                        results["score"] += 1

                    attack_code += 1

ser.close()
print(results)

import argparse


def createArgParser() -> argparse.ArgumentParser:
    # Create an argument parser using argparse lib

    parser = argparse.ArgumentParser('Embedded RIPE usage manual')

    parser.add_argument("ports", default=False, action="store_true")
    parser.add_argument("--port", type=str, help="Serial port (i.e. COM3)")
    parser.add_argument("--baudrate", type=int, help="Baudrate (i.e. 9600)")

    return parser


def printTitle():
    print("*"*5 + " Embedded RIPE " + "*"*5 + "\n")

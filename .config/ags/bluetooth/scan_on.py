#!/bin/python3

import subprocess
import os

if __name__ == "__main__":
    process = subprocess.Popen(['bluetoothctl'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    with open('bluetoothctl.pid', 'w') as pid_file:
        pid_file.write(str(process.pid))

    if process.stdin == None:
        print("bluetoothctl didn't start :(")
        exit(0)

    process.stdin.write("scan on" + '\n')
    process.stdin.flush()

    process.wait()

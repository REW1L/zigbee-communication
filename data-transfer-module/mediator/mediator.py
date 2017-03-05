#!/bin/python2.7

import sys
import argparse
import threading
import io
import os
from random import randint
from time import sleep
from datetime import datetime

def process_file(input_file, output_files):
    EUI = ''.join([chr(randint(0,9)+48), chr(randint(0,9)+48), chr(randint(0,9)+48), chr(randint(0,9)+48), chr(randint(0,9)+48), chr(randint(0,9)+48), chr(randint(0,9)+48), chr(randint(0,9)+48)])
    fout = []
    io.open(file=input_file, mode="w").close()
    for f in output_files:
        is_ok = False
        while not is_ok:
            try:
                temp = io.open(file=f, mode="w+b")
                is_ok = True
            except Exception as e:
                is_ok = False
        fout.append(temp)
    fin = io.open(file=input_file, mode="r+b")
    fin.seek(0, io.SEEK_END)
    while True:
        arrbyte = fin.read()
        line = ""
        if arrbyte != "":
            if "AT" in arrbyte:
                fin.seek(0, io.SEEK_END)
                fin.write(bytearray("\r\nOK\r\n"))
            if "AT+BCASTB:4A,00" in arrbyte:
                packets = arrbyte.split("AT+BCASTB:4A,00\r")
                for packet in packets:
                    if packet != '':
                        pckt = "\r\nBCAST:" + EUI + ",4A=" + packet + "\r\n"
                        for file in fout:
                            file.seek(0, io.SEEK_END)
                            file.write(bytearray(pckt))
                            file.flush()
                        sleep(0.2)

            print("[{}] Received packet with length: {}".format(datetime.utcnow(), str(len(arrbyte))))
        sleep(0.01)

def parse_commands(files):
    while True:
        try:
            cmd = raw_input()
            if cmd == "exit" or cmd == "^C":
                os._exit(0)
        except EOFError:
            exit()
        except KeyboardInterrupt:
            exit()

def exit():
    print("")
    os._exit(0)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Tool for implementing sharing information in ZigBee.')

    parser.add_argument('-f', '--files', dest="files", required=False, help='files for serial interface', default=["device1", "device2"], nargs="*")
    # parser.add_argument('-s', '--sampleapp', dest="bin", required=True, help='Sampleapp path', default=["device1", "device2"], nargs=1)

    args = parser.parse_args()

    threads = []

    for file in args.files:
        threads.append(threading.Thread(group=None, target=process_file, name=file, 
            args=(file, [x for x in args.files if x != file])))
        threads[-1].start()

    # threads.append(threading.Thread(group=None, target=parse_commands, name="console", 
    #         args=([args.files])))
    # threads[-1].start()
    parse_commands([args.files]);

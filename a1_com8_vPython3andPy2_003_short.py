#!/usr/bin/env python3
# -*- coding: UTF-8 -*-
# win32.SetupComm(self._port_handle, 4096, 4096)
# from pip._internal.commands import install
# zipped pyserial, next form command line: python setup.py install
# pip3 install pyserial
# https://stackoverflow.com/questions/33267070/no-module-named-serial
import serial
import struct
import array
from datetime import datetime

# verbose = 0 # silent
# verbose = 1 # print variables in memory to check
verbose = 0

# python -m serial.tools.list_ports
ser = serial.Serial()
ser.port = "COM8"
ser.baudrate = 9600
ser.open()
# ser.close()
ser.flushInput()

byte = lastbyte = chr(0)
line_count = 0
excel_format = "{};{};{}\r\n"


def f2a(one_float):
    return str(one_float).replace('.', ',')


result_excel_ls = []
global pm_25_end
pm_25_end = 0
try:
    excel_line = excel_format.format('Time', 'PM 2.5 [ug/m^3]', 'PM 10 [ug/m^3]')
    result_excel_ls.append(excel_line)
    while True:
        lastbyte = byte
        byte = ser.read(size=1)
        if verbose:
            print(("Got byte %x") % ord(byte))
        # We got a valid packet header

# Python2 vs Python3
# https://www.delftstack.com/howto/python/how-to-convert-int-to-bytes-in-python-2-and-python-3/#python-3-only-int-to-bytes-conversion-methods
#        if lastbyte == chr(170) and byte == chr(192):  # OK if Python 2
#        if lastbyte == bytes([170]) and byte == bytes([192]):  # OK if Python 3
        if lastbyte == bytes([170]) and byte == bytes([192]):  # OK if Python 3
            sentence = ser.read(size=8)  # Read 8 more bytes
            if verbose:
                print("Sentence size {}".format(len(sentence)))
                print(array.array('B', sentence))
            # Decode the packet - big endian, 2 shorts for pm2.5 and pm10, 2 reserved bytes, checksum, message tail
            reading_ls = struct.unpack('<hhxxcc', sentence)
            pm_25 = reading_ls[0] / 10.0
            pm_10 = reading_ls[1] / 10.0
            pm_25_end = pm_25
            # ignoring the checksum and message tail

            if line_count == 0:
                line = ("PM 2.5: {} ug/m^3  PM 10: {} ug/m^3".format(pm_25, pm_10))
                the_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                excel_line = excel_format.format(the_time, f2a(pm_25), f2a(pm_10))
                result_excel_ls.append(excel_line)

                print(the_time + ' ' + line)

            line_count += 1
            if line_count == 5:
                line_count = 0
finally:
    the_time = datetime.now().strftime("%Y.%m.%d_%H.%M.%S")
    file_name = 'PM2.5_{}__{}.csv'.format(the_time, str(int(pm_25_end)).zfill(3))

# from: https://steemit.com/python/@weaming/string-in-python2-and-python3
# Python 2 vs Python 3
#    fd = open(file_name, 'wb') # OK if Python 2
#    fd = open(file_name, 'w')   # OK if Python 3
# P3
    fd = open(file_name, 'w')   # OK if Python 3
    one_big_text = ''.join(result_excel_ls)
    type(one_big_text)
    fd.writelines([one_big_text])
    fd.write(one_big_text)
    fd.close()
    print("Written {} lines to file {}".format(len(result_excel_ls), file_name))

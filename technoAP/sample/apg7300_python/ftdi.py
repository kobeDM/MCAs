#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import platform
import ctypes
from ctypes import *
import logging
import re

os_type = platform.system()
arch = platform.architecture()[0]

############################################################################
#   Class for communicating with FTDI devices
############################################################################
class FTDI(object):
    # ======================================================================
    #   Set the library path
    # ======================================================================
    if  os_type == "Windows":
        if arch == "64bit":
            system_path = os.path.join(os.environ["SystemRoot"], "System32")
        elif arch == "32bit":
            system_path = os.path.join(os.environ["SystemRoot"], "SysWOW64")

        # Load ftd2xx.dll if it was installed according to FTDI's official instructions
        libpath = os.path.join(system_path, "ftd2xx.dll")
        # Load ftd2xx.dll if it was installed according to TechnoAP's instructions
        if not os.path.exists(libpath):
            subfolder_name = "TechnoAP"
            libpath = os.path.join(system_path, subfolder_name, "ftd2xx.dll")
        LIBHANDLE = ctypes.WinDLL(libpath)

    elif os_type == "Linux":
        __dir = os.path.dirname(__file__)
        if arch == "64bit":
            libpath = "%s/lib/linux64/%s" % (__dir, "libftd2xx.so.1.4.33")
        elif arch == "32bit":
            libpath = "%s/lib/linux32/%s" % (__dir, "libftd2xx.so.1.4.33")
        LIBHANDLE = ctypes.cdll.LoadLibrary(libpath)

    # ======================================================================
    #   Set vendor ID and product ID
    # ======================================================================    
    def SetVIDPID(self, vendorID, productID):
        isSuccess = True # Always true if os_type is Windows
        if os_type == "Linux":                
            value1 = ctypes.c_uint(vendorID)
            value2 = ctypes.c_uint(productID)
            result = FTDI.LIBHANDLE.FT_SetVIDPID(value1, value2)
            if result != 0: # 0: FT_OK
                isSuccess = False
                self.logger.info(f"[!] Failed to set VID and PID. Error code: {result}")
            else:
            	self.logger.info("... Successfully set VID and PID. VID: 0x%04x, PID: 0x%04x" % (value1.value, value2.value))
        return isSuccess
    
    # ======================================================================
    #   Get vendor ID and product ID
    # ======================================================================  
    def GetVIDPID(self):
        isSuccess = True # Always true if os_type is Windows
        vid = ctypes.c_uint(0)
        pid = ctypes.c_uint(0)
        if os_type == "Linux":
            result = FTDI.LIBHANDLE.FT_GetVIDPID(byref(vid), byref(pid))
            if result != 0: # 0: FT_OK
                isSuccess = False
                self.logger.info(f"[!] Failed to get VID and PID. Error code: {result}")
            else:    
                self.logger.info("... Successfully got VID and PID. VID: 0x%04x, PID: 0x%04x" % (vid.value, pid.value))
        vendorID = int(vid.value)
        productID = int(pid.value)
        return isSuccess, vendorID, productID
    
    # ======================================================================
    #   Initialize the class
    # ======================================================================  
    def __init__(self):
        self.logger = logging.getLogger(__name__)

        self.mHandle = None
        self.mDeviceCounts = 0
        self.mDeviceList = []
        self.Close()
        self.logger.info("... Initialization of the class FTDI completed.")

    # ======================================================================
    #   Destructor method called when the object is deleted
    # ====================================================================== 
    def __del__(self):
        self.Close()
        self.logger.info("... FTDI instance has been deleted.")

    # ======================================================================
    #   Open connection by serial number
    # ======================================================================     
    def OpenBySerialNumber(self, serialnum):
        isSuccess = False
        handle = ctypes.c_void_p()
        param = ctypes.c_uint(1) # 1: FT_OPEN_BY_SERIAL_NUMBER
        result = FTDI.LIBHANDLE.FT_OpenEx(serialnum, param, byref(handle))
        if result == 0: # 0: FT_OK
            self.mHandle = handle
            isSuccess = True
            self.logger.info("... Successfully opened the device by serial number")
        else:
            self.logger.error(f"[!] Failed to open the device. Error code: {result}")
        return isSuccess
    
    # ======================================================================
    #   Open connection by device index
    # ====================================================================== 
    def OpenByIndex(self, index):
        isSuccess = False
        handle = ctypes.c_void_p()
        result = FTDI.LIBHANDLE.FT_Open(index, byref(handle))
        if result == 0: # 0: FT_OK
            self.mHandle = handle
            isSuccess = True
        return isSuccess
    
    # ======================================================================
    #   Close connection
    # ====================================================================== 
    def Close(self):
        isSuccess = False
        if self.mHandle is not None:
            result = FTDI.LIBHANDLE.FT_Close(self.mHandle)
            self.mHandle = None
            if result == 0: # 0: FT_OK
                isSuccess = True
                self.logger.info("... Successfully closed the connection")
            else:
                self.logger.error(f"[!] Failed to close the connection. Error code: {result}")
        else:
            isSuccess = True
            self.logger.info("... No active connection. Nothing to close.")
        
        return isSuccess
        
    # ======================================================================
    #   Get information about all connected FTDI devices
    # ====================================================================== 
    def GetDeviceInfoList(self):

        isSuccess = False
        counts = ctypes.c_uint()
        result1 = FTDI.LIBHANDLE.FT_CreateDeviceInfoList(byref(counts))

        if result1 == 0 and counts.value > 0: # 0: FT_OK
            template0 = FT_DEVICE_LIST_INFO_NODE * counts.value # The class FT_DEVICE_LIST_INFO_NODE is defined below.
            info = template0()
            result2 = FTDI.LIBHANDLE.FT_GetDeviceInfoList(info, byref(counts))
            
            if result2 == 0: # 0: FT_OK
                self.mDeviceCounts = int(counts.value)
                for i in range(self.mDeviceCounts):
                    self.mDeviceList.append(info[i])
                    self.logger.info("... Device index: %s" % i)
                    self.logger.info("...... Flags: %s, Type: %s, ID: %s, LocID: %s" % (info[i].Flags, info[i].Type, info[i].ID, info[i].LocID))
                    
                    serialnum_str = bytes(info[i].SerialNumber).decode("utf-8", errors="ignore").strip("\x00")
                    serialnum_clean = re.sub(r"[\x00-\x1F\x7F]", "", serialnum_str) # To avoid mojibake    
                    self.logger.info(f"...... SerialNumber: {serialnum_clean}")
                    
                    desc_str = bytes(info[i].Description).decode("utf-8", errors="ignore").strip("\x00")
                    desc_clean = re.sub(r"[\x00-\x1F\x7F]", "", desc_str) # To avoid mojibake    
                    self.logger.info(f"...... Description: {desc_clean}")

                    self.logger.info("...... Handle: %s" % info[i].ftHandle)
                isSuccess = True
            else:
            	self.logger.error(f"[!] Failed to get device information. Error code: {result2}")

        else:
            self.logger.error(f"[!] Failed to create device info list. Error code: {result1}, number of devices: {counts.value}")	
        return isSuccess, self.mDeviceList, self.mDeviceCounts

    # ======================================================================
    #   Read data from FTDI device
    # ====================================================================== 
    def Read(self, rxbytes):
        isSuccess = False
        rxdata = None

        template0 = ctypes.c_ubyte * rxbytes
        buffer = template0()
        bufferbytes = ctypes.c_uint(rxbytes)
        readBytes = ctypes.c_uint(0)
        result = FTDI.LIBHANDLE.FT_Read(self.mHandle, buffer, bufferbytes, byref(readBytes))

        if result == 0: # 0: FT_OK
            rxdata = bytearray(readBytes.value)
            for i in range(readBytes.value):
                rxdata[i] = buffer[i]
            isSuccess = True

        return isSuccess, rxdata, int(readBytes.value)

    # ======================================================================
    #   Write data to FTDI device
    # ====================================================================== 
    def Write(self, buffer, txbytes):
        isSuccess = False
        templete0 = ctypes.c_ubyte * txbytes
        txdata = templete0()

        for i in range(txbytes):
            txdata[i] = buffer[i]

        bufferbytes = ctypes.c_uint(txbytes)
        writtenBytes = ctypes.c_byte(0)
        result = FTDI.LIBHANDLE.FT_Write(self.mHandle, txdata, bufferbytes, byref(writtenBytes))

        if result == 0: # 0: FT_OK
            isSuccess = True
        return isSuccess, int(writtenBytes.value)

    # ======================================================================
    #   Reset FTDI device
    # ====================================================================== 
    def ResetDevice(self):
        isSuccess = False
        result = FTDI.LIBHANDLE.FT_ResetDevice(self.mHandle)
        if result == 0: # 0: FT_OK
            isSuccess = True
        return isSuccess
    
    # ======================================================================
    #   Set bit mode of FTDI device
    # ====================================================================== 
    def SetBitMode(self, mask, enable):
        isSuccess = False
        value1 = ctypes.c_byte(mask)
        value2 = ctypes.c_byte(enable)
        result = FTDI.LIBHANDLE.FT_SetBitMode(self.mHandle, value1, value2)
        if result == 0: # 0: FT_OK
            isSuccess = True
        return isSuccess        
    
    # ======================================================================
    #   Set latency timer of FTDI device
    # ====================================================================== 
    def SetLatencyTimer(self, latency):
        isSuccess = False
        value = ctypes.c_byte(latency)
        result = FTDI.LIBHANDLE.FT_SetLatencyTimer(self.mHandle, value)
        if result == 0: # 0: FT_OK
            isSuccess = True
        return isSuccess  
    
    # ======================================================================
    #   Set time outs of FTDI device
    # ====================================================================== 
    def SetTimeouts(self, readTimeouts, writeTimeouts):
        isSuccess = False
        value1 = ctypes.c_uint(readTimeouts)
        value2 = ctypes.c_uint(writeTimeouts)
        result = FTDI.LIBHANDLE.FT_SetTimeouts(self.mHandle, value1, value2)
        if result == 0: # 0: FT_OK
            isSuccess = True
        return isSuccess 

    # ======================================================================
    #   Set flow control of FTDI device
    # ====================================================================== 
    def SetFlowControl(self, flowControl, xOn, xOff):
        isSuccess = False
        value1 = ctypes.c_ushort(flowControl)
        value2 = ctypes.c_byte(xOn)
        value3 = ctypes.c_byte(xOff)
        result = FTDI.LIBHANDLE.FT_SetFlowControl(self.mHandle, value1, value2, value3)
        if result == 0: # 0: FT_OK
            isSuccess = True
        return isSuccess 

    # ======================================================================
    #   Purge FTDI device buffers
    # ====================================================================== 
    def Purge(self, mask):
        isSuccess = False
        value = ctypes.c_uint(mask)
        result = FTDI.LIBHANDLE.FT_Purge(self.mHandle, value)
        if result == 0: # 0: FT_OK
            isSuccess = True
        return isSuccess 

############################################################################
#   Class used in the method GetDeviceInfoList
############################################################################
class FT_DEVICE_LIST_INFO_NODE(Structure, object):
     _fields_ = [
        ("Flags", c_uint),
        ("Type", c_uint),
        ("ID", c_uint),
        ("LocID", c_uint),
        ("SerialNumber", c_byte * 16),
        ("Description", c_byte * 64),
        ("ftHandle", c_void_p)
     ]
     


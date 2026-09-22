#!/usr/bin/python
# -*- coding: utf-8 -*-

import time
import array
import struct
import copy
import logging
import sys

import ftdi

DIGIT_PER_SEC = 25000000.0
SEC_PER_DIGIT = 1.0 / DIGIT_PER_SEC

APG7400A_VENDORID  = 0x1ca6		# vendor  ID
APG7400A_PRODUCTID = 0x0004		# product ID
#APG7400A_PRODUCTID = 0x0001		# product ID

STATUS_LIST = [
	#common
	["RLT", 6],
	#CH1
	["LVT1", 6],	["DDT1", 6],	["TCR1", 3],	["TCT1", 4],	["ICR1", 3],
	#CH2
	["LVT2", 6],	["DDT2", 6],	["TCR2", 3],	["TCT2", 4],	["ICR2", 3],
	#CH3
	["LVT3", 6],	["DDT3", 6],	["TCR3", 3],	["TCT3", 4],	["ICR3", 3],
	#CH4
	["LVT4", 6],	["DDT4", 6],	["TCR4", 3],	["TCT4", 4],	["ICR4", 3]
]

############################################################################
#   Class for device-specific operations of the device APG7400A
############################################################################
class APG7400A(ftdi.FTDI):
    # ======================================================================
    #   Initialize the class
    # ======================================================================  
    def __init__(self): 
        self.SetLogger()       
        super(APG7400A, self).__init__()    # Execute __init__ of class FTDI (Necessary to use parameters defined in ftdi.py)
        self.mOldVendorID = None
        self.mOldProductID = None
        isSuccess, self.mOldVendorID, self.mOldProductID = super(APG7400A, self).GetVIDPID()
        isSuccess =  super(APG7400A, self).SetVIDPID(APG7400A_VENDORID, APG7400A_PRODUCTID)
        self.logger.info("... Initialization of the class APG7400A completed.")

    # ======================================================================
    #   Set up a logger for logging messages
    # ======================================================================
    def SetLogger(self):
        logger = logging.getLogger()
        logger.setLevel(logging.DEBUG)
        
        if not logger.hasHandlers():
            file_handler = logging.FileHandler("debug.log", mode = "w", encoding="utf-8")
            file_formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
            file_handler.setFormatter(file_formatter)
            file_handler.setLevel(logging.DEBUG)
            logger.addHandler(file_handler)

            stdout_handler = logging.StreamHandler(stream=sys.stdout)
            stdout_formatter = logging.Formatter("%(levelname)s - %(message)s")
            stdout_handler.setFormatter(stdout_formatter)
            stdout_handler.setLevel(logging.ERROR)
            logger.addHandler(stdout_handler)

        self.logger = logging.getLogger(__name__)

    # ======================================================================
    #   Initialize the device APG7400A
    # ======================================================================
    def InitializeDevice(self, usbftdi, waitTime = 0.5):
        isSuccess = True  
        resultList = [False, False, False, False, False, False]
        resultList[0] = usbftdi.ResetDevice()
        resultList[1] = usbftdi.SetBitMode(255, 0x00)         # 0x00: FT_BITMODE_RESET
        resultList[2] = usbftdi.SetLatencyTimer(2)
        resultList[3] = usbftdi.SetTimeouts(100, 100)
        resultList[4] = usbftdi.SetFlowControl(0x0100, 0, 0)  # 0x0100: FT_FLOW_RTS_CTS
        resultList[5] = usbftdi.Purge(1 | 2)                  # 1: FT_PURGE_RX, 2: FT_PURGE_TX
        time.sleep(waitTime)
		
        for i in range(len(resultList)):
            if resultList[i] != True: # True: Success
                isSuccess = False
                self.logger.error("[!] Failed to initialize the device APG7400A.")   
                break
        self.logger.info("... Initialization of the device APG7400A completed.")        

        return isSuccess
    
    # ======================================================================
    #   Performs cleanup without invoking the parent class destructor
    # ====================================================================== 
    def __del__(self):
        isSuccess =  super(APG7400A, self).SetVIDPID(self.mOldVendorID, self.mOldProductID) 
        self.logger.info("... APG7400A instance has been deleted.")

    # ======================================================================
    #   Write commands to and read responses from the device APG7400A
    # ====================================================================== 
    def WriteReadCommand(self, usbftdi, command, value) :
        isSuccess = False      
        value_4bytes = value.to_bytes(4, byteorder="big")       # 4-byte array in Big Endian
        txcommand = bytearray(command, "utf-8") + value_4bytes  # UTF-8 includes all ASCII characters
        isSuccess1, txbytes = usbftdi.Write(txcommand, len(txcommand))
        self.logger.info(f"... [Write command] Is success: {isSuccess1}, Command: {txcommand}, Data size: {txbytes}")

        if isSuccess1 == True:
            isSuccess2, rxdata, rxbytes = usbftdi.Read(txbytes)
            self.logger.info(f"... [Read command] Is success: {isSuccess2}, Command: {rxdata}, Data size: {rxbytes}")
            if isSuccess2 == True:
                isSuccess = True
        return isSuccess, txbytes

    # ======================================================================
    #   Write commands to the device APG7400A (Reading response is unnecessary)
    # ====================================================================== 
    def WriteCommand(self, usbftdi, command, value) :
        value_4bytes = value.to_bytes(4, byteorder="big")       # 4-byte array in Big Endian
        txcommand = bytearray(command, "utf-8") + value_4bytes  # UTF-8 includes all ASCII characters
        isSuccess, txbytes = usbftdi.Write(txcommand, len(txcommand))
        self.logger.info(f"... [Write command] Is success: {isSuccess}, Command: {txcommand}, Data size: {txbytes}")
        return isSuccess, txbytes

    # ======================================================================
    #   Read the status from the device APG7400A
    # ====================================================================== 
    def ReadStatus(self, usbftdi):
        isSuccess = False
        self.logger.info("... Clear status")
        self.mStatus = {}
        self.mSumBytes= 0
        for item0 in STATUS_LIST:
            status_name = item0[0]
            status_data_size = item0[1]
            self.mStatus[status_name,] = 0
            self.mSumBytes = self.mSumBytes + item0[1]

        self.logger.info("... Read status")
        isSuccess1, txbytes = self.WriteCommand(usbftdi, "STUW", 0)
        if isSuccess1 == True:
            isSuccess2, rxdata, rxbytes = usbftdi.Read(94) # Read 94 bytes of data
            if isSuccess2 == True and rxbytes == 94:
                index = 0
                temp0 = bytearray(8)

                for item0 in STATUS_LIST:
                    value = copy.copy(temp0)
                    status_name = item0[0]
                    status_data_size = item0[1]
                    if 0 < status_data_size < 8:
                        zlength = 8 - status_data_size
                        nlast = index + status_data_size
                        value = temp0[0:zlength] + rxdata[index:nlast]
                    
                    value1 = int.from_bytes(value[0:8], byteorder="big") # Convert to UInt64 (big-endian)
                    self.mStatus[status_name,] = value1

                    index = index + status_data_size
                isSuccess = True
        return isSuccess

    # ======================================================================
    #   Display the status of the device APG7400A
    # ====================================================================== 
    def DisplayStatus(self):
        print("live time (CH1, CH2, CH3, CH4):    %.2f, %.2f, %.2f, %.2f (sec)" % (
			float(self.mStatus["LVT1",]) * SEC_PER_DIGIT,	 #CH1
			float(self.mStatus["LVT2",]) * SEC_PER_DIGIT,	 #CH2
			float(self.mStatus["LVT3",]) * SEC_PER_DIGIT,	 #CH3
			float(self.mStatus["LVT4",]) * SEC_PER_DIGIT)) #CH4

        print ("dead time (CH1, CH2, CH3, CH4):    %.2f, %.2f, %.2f, %.2f (sec)" % (
			float(self.mStatus["DDT1",]) * SEC_PER_DIGIT,	 #CH1
			float(self.mStatus["DDT2",]) * SEC_PER_DIGIT,	 #CH2
			float(self.mStatus["DDT3",]) * SEC_PER_DIGIT,	 #CH3
			float(self.mStatus["DDT4",]) * SEC_PER_DIGIT)) #CH4
        
        print ("throughput count rate (CH1, CH2, CH3, CH4):  %d, %d, %d, %d" % (
			self.mStatus["TCR1",], self.mStatus["TCR2",],	#CH1, CH2
			self.mStatus["TCR3",], self.mStatus["TCR4",]))	#CH3, CH4
        print ("throughput total count (CH1, CH2, CH3, CH4): %d, %d, %d, %d" % (
			self.mStatus["TCT1",], self.mStatus["TCT2",],	#CH1, CH2
			self.mStatus["TCT3",], self.mStatus["TCT4",]))	#CH3, CH4
        print ("input count rate (CH1, CH2, CH3, CH4):       %d, %d, %d, %d" % (
			self.mStatus["ICR1",], self.mStatus["ICR2",],	#CH1, CH2
			self.mStatus["ICR3",], self.mStatus["ICR4",]))	#CH3, CH4
        return

    # ======================================================================
    #   Read histogram data from the device APG7400A
    # ======================================================================                   
    def ReadHistogram(self, usbftdi, chn):
        self.logger.info("... Read histogram")
        isSuccess = False
        histdata = None
        try:
            isSuccess1, dummy = self.WriteReadCommand(usbftdi, "HCHW", (chn - 1))
            
            if isSuccess1 == True:
                rxdata = bytearray()
                for i in range(32): # 32: 16384 / 512 
                    command = "HI%02X" % (i)
                    isSuccess2, dummy = self.WriteCommand(usbftdi, command, 0)
   
                    if isSuccess2 == True:
                        isSuccess3, rxdata0, rxbyte = usbftdi.Read(2048) # Read 2048 bytes    
                        if isSuccess3 != True or rxbyte != 2048:
                            break

                        rxdata += rxdata0

                    histdata = array.array("I") # I: Unsigned integer (4 bytes)
                    itemsize = histdata.itemsize # The number of bytes of each item (4 bytes)

                    for j in range(0, len(rxdata), itemsize):
                        work = 0
                        for k in range(itemsize):
                            work |= (rxdata[j+k] << ((itemsize - 1 - k) * 8)) # Shift each byte and combine them (Big Endian)
                        histdata.append(work)
            isSuccess = True
            self.logger.info("... Successfully read histogram from the device APG7400A.")                   
  
        except Exception as e:
            self.logger.error("[!] An error has occurred in reading histogram: ", e)
        
        return isSuccess, histdata

    # ======================================================================
    #   Read list data from the device APG7400A
    # ======================================================================                     
    def ReadListData(self, usbftdi):
        self.logger.info("... Read list data")
        isSuccess = False
        rxdata = None
        rxbyte = 0

        try:
            isSuccess1, dummy = self.WriteCommand(usbftdi, "LISR", 0)

            if isSuccess1 == True:
                isSuccess2, rxdata, rxbyte = usbftdi.Read(8) # Read 8 bytes
                command = rxdata[0:4]
                rxvalue = rxdata[4:8]
                rxvalue = struct.unpack(">I", rxdata[4:8])[0] # ">I" means Big Endian

                if isSuccess2 == True and rxbyte == 8:
                    rxblocks = rxvalue & 0x0000ffff
                    rxbytes = rxblocks * 8

                    if rxbytes > 0:
                        isSuccess, rxdata, rxbyte0 = usbftdi.Read(rxbytes) 
                    else:
                        isSuccess = True
                        rxbyte0 = 0
                        self.logger.info("... Successfully read list data from the device APG7400A.")     
        except Exception as e:
            self.logger.error("[!] An error has occurred in reading list data: ", e) 

        return isSuccess, rxdata, rxbyte0


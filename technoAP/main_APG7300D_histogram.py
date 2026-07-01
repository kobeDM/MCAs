#!/usr/bin/python
# -*- coding: utf-8 -*-

import time
import os
os.chdir(os.path.dirname(__file__)) # Current directory

import ftdi
import apg7300d

DIGIT_PER_SEC = 25000000.0
SEC_PER_DIGIT = 1.0 / DIGIT_PER_SEC


# ======================================================================
#   Main program
# ====================================================================== 
def main():    
    try:
        serialnum = ("APG7300AA").encode("utf-8")
        usbmca = apg7300d.APG7300D()                                        # Create an instance of class APG7300D and run its initialization
        usbftdi = ftdi.FTDI()                                               # create an instance of class FTDI and run its initialization
        isSuccess, deviceList, deviceCounts = usbftdi.GetDeviceInfoList()   # Display information for all connected devices

        isSuccess = usbftdi.OpenBySerialNumber(serialnum)
        if isSuccess == True:
            isSuccess = usbmca.InitializeDevice(usbftdi)
            print("\n###### configure device ######")
            configure_device(usbmca, usbftdi)
            print("\n###### acquire data ######")
            acquire_data(usbmca, usbftdi, 5.0)	# acq_sec = 5.0 sec
            print("\n###### data acquisition complete ######")
    
    except Exception as e:
        print("An error has occurred: ", e)

# ======================================================================
#   Acquire data from the device APG7300D
# ====================================================================== 
def acquire_data(usbmca, usbftdi, acq_sec):
    usbmca.WriteReadCommand(usbftdi, "AQEW", 1) # Stop data acquisition: 1 --> execute
    usbmca.WriteReadCommand(usbftdi, "CLRW", 0) # Cear data: 0 --> clear

    usbmca.WriteReadCommand(usbftdi, "MODW", 0) # Data acquisition mode: 0 --> histogram
    usbmca.WriteReadCommand(usbftdi, "MMDW", 0) # Measurement mode: 0 --> real time

    acq_digit = int(acq_sec * DIGIT_PER_SEC)
    usbmca.WriteReadCommand(usbftdi, "MT0W", (acq_digit >> 32) & 0x0fff)        # Upper 12 bits of measurement time (44 bits in total)
    usbmca.WriteReadCommand(usbftdi, "MT1W", (acq_digit >> 0) & 0x0ffffffff)    # Lower 32 bits of measurement time (44 bits in total)

    elapsed_sec = -0.001                        # Initialize elapsed time
    usbmca.WriteReadCommand(usbftdi, "AQSW", 1) # Start data acquisition: 1 --> execute
    while acq_sec >= elapsed_sec:
        usbmca.ReadStatus(usbftdi)              # Get device status
        elapsed_sec  = float(usbmca.mStatus["RLT",]) * SEC_PER_DIGIT
        print("\n##### elapsed_sec/acq_sec (sec): %.2f/%.2f #####" % (elapsed_sec, acq_sec))
        usbmca.DisplayStatus()                  # Display device status
        time.sleep(0.5)		# delay

    usbmca.WriteReadCommand(usbftdi, "AQEW", 1) # Stop data acquisition: 1 --> execute

	#-------------------- read histogram --> CSV file --------------------#
    histdata = []
    isSuccess, hist0 = usbmca.ReadHistogram(usbftdi, 1)	# read histogram data, 1 --> CH1       
    histdata.append(hist0)

    with open("./histogram.csv", "w") as csv:
        csv.writelines("[header]\n")
        csv.writelines("real time(s),%f\n" % (elapsed_sec))
        csv.writelines("[data]\n")
        csv.writelines("#bin\n")

        for i in range(16384):
            textline = "%d,%d\n" % (i,
				histdata[0][i]		    # CH1
			    )
            csv.writelines(textline)
    
    return

# ======================================================================
#   Send configuration command to the device APG7300D
# ====================================================================== 
def configure_device(usbmca, usbftdi):
    usbmca.WriteReadCommand(usbftdi, "PDSW", 0) # Peak detection mode: 0 --> absolute
    usbmca.WriteReadCommand(usbftdi, "ADGW", 0) # CH1 ADC gain: 0 --> 16384 bins
    usbmca.WriteReadCommand(usbftdi, "THRW", 40) # CH1 threshold: 40 ch
    usbmca.WriteReadCommand(usbftdi, "LLDW", 40) # CH1 lower level discrimination: 40 ch
    usbmca.WriteReadCommand(usbftdi, "ULDW", 16383) # CH1 upper level discrimination: 16383 ch
    usbmca.WriteReadCommand(usbftdi, "OFSW", 0) # CH1 offset: 0 ch

    return

# ======================================================================
#   Run main program
# ====================================================================== 
if __name__ == "__main__":
    main()


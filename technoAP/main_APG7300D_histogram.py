#!/usr/bin/python3
# -*- coding: utf-8 -*-
#  2025 Oct, Kentaro Miuchi
#  
#  Original version:
#  http://www.techno-ap.com/support_sample.html

import time
import os
import sys
import subprocess
#os.chdir(os.path.dirname(__file__)) # Current directory

import ftdi
import apg7300d
import common
import argparse
import threading
import termios


DIGIT_PER_SEC = 25000000.0
SEC_PER_DIGIT = 1.0 / DIGIT_PER_SEC

maxMCAs=8
CONFIG = "MCA_config.json"
TMP_FILE = "tmp.mca"

fd = sys.stdin.fileno()
old = termios.tcgetattr(fd)
new = termios.tcgetattr(fd)
new[3] &= ~termios.ICANON
new[3] &= ~termios.ECHO
termios.tcsetattr(fd, termios.TCSANOW, new)

quit_flag = False
stop_flag = False
#verbose = False
verbose = True

def key_monitor():
    global quit_flag,stop_flag
    while True:
        ch = sys.stdin.read(1)
        if ch == 'q':
            quit_flag = True
            sys.stdout.write("q command was issued. Quitting the DAQ.")
            break
        elif ch == 's':
            stop_flag = True
            sys.stdout.write("s command was issued. Stopping the DAQ at the end of this file.")
            break


# ======================================================================
#   Main program
# ====================================================================== 
def main_APG7300D_histgram():
    #print(os.getcwd())
    global quit_flag,stop_flag
    parser = argparse.ArgumentParser()
    parser.add_argument("-c", help="config file name", default=CONFIG)
    parser.add_argument("-v","--verbose", help="verbose mode (control only)", action='store_true')
    parser.add_argument("-p","--presettime", help="preset time for one file", default=60)
    parser.add_argument("-f", help="num of files per period", default=100)
    #parser.add_argument('-S', '--serialnumber',help='S/N', default=718,type=int)    
    parser.add_argument("-t", help="temporary file name", default=TMP_FILE)
    args = parser.parse_args()
    config_filename = args.c
    presettime = int(args.presettime)
    num_file_per_period = int(args.f)
    verbose=args.verbose
    tmpfile="../"+args.t

    #sys.stdout.write('Preset time: '+str(presettime)+' \n')
    sys.stdout.write('### press "s" to stop after this file.\t Press "q" to quit.###\n')
    
    try:
        mcacommon=common.COMMON
        print("\n###### read configure file ######")
        configs=mcacommon.readConfig(CONFIG)
        ID=0
        for i in range(maxMCAs):
            if configs[i].MCA_type == "APG73000D":
                print("APG73000D was found. (ID=",configs[i].ID,")")
                ID=configs[i].ID
        mcacommon.showConfig(configs[ID])
        serialnum = (configs[ID].SN).encode("utf-8")
        usbmca = apg7300d.APG7300D()     # Create an instance of class APG7300D and run its initialization
        usbftdi = ftdi.FTDI()                                               # create an instance of class FTDI and run its initialization
        isSuccess, deviceList, deviceCounts = usbftdi.GetDeviceInfoList()   # Display information for all connected devices
        isSuccess = usbftdi.OpenBySerialNumber(serialnum)
        fileID=0
        #filename=tmpfile
        if isSuccess == True:
            isSuccess = usbmca.InitializeDevice(usbftdi)
            configure_device(usbmca, usbftdi,configs[ID])

        print("\n###### data acquisiion started ######")
        sys.stdout.write('### press "s" to stop after this file.\t Press "q" to quit.###\n')
        while(fileID < num_file_per_period):
            starttime = time.time()
            #if isSuccess == True:
            #isSuccess = usbmca.InitializeDevice(usbftdi)
            thisfile=configs[ID].SN+'_'+str(fileID)+'.mca'
            print(" file:",fileID,"/",num_file_per_period,"filename:",thisfile,end="\n")
            cmd="unlink "+tmpfile
            cp=subprocess.run(cmd, shell=True)
            cmd="touch "+thisfile
            cp=subprocess.run(cmd, shell=True)
            path=os.getcwd()+"/"+thisfile
            path=path.replace(' ','')
            cmd="ln -s "+path+" "+tmpfile
            #print(cmd)
            subprocess.run(cmd, shell=True)

            start_acquisition(usbmca, usbftdi, presettime,configs[ID])
            elapsed_sec = -0.001 
            while presettime >= elapsed_sec:
                if quit_flag:
                    #sys.stdout.write("q command was issued. Quitting the DAQ.")
                    sys.stdout.flush()
                    break
                usbmca.ReadStatus(usbftdi)              # Get device status
                elapsed_sec  = float(usbmca.mStatus["RLT",]) * SEC_PER_DIGIT
                spec=read_data(usbmca, usbftdi, presettime,configs[ID])
                status=0
                mcacommon.saveSpectrum(thisfile, spec,status,starttime,presettime)  
                if(int(elapsed_sec)%1==0):
                    #print("\n##### elapsed_sec/acq_sec (sec): %.2f/%.2f #####" % (elapsed_sec, acq_sec))            
                    print(" time:",str(int(elapsed_sec)),"/",str(presettime),end="\r")
                    #usbmca.DisplayStatus()                  # Display device status
                time.sleep(1)		# delay
            
            usbmca.WriteReadCommand(usbftdi, "AQEW", 1) # Stop data acquisition: 1 --> execute
            #spec=acquire_data(usbmca, usbftdi, presettime,configs[ID])
            #print("\n###### data acquisition complete ######")
            spec=read_data(usbmca, usbftdi, presettime,configs[ID])
            status=0
            mcacommon.saveSpectrum(thisfile, spec,status,starttime,presettime)  
            if(quit_flag or stop_flag):
                return(1)            
            fileID=fileID+1
            #return(0)
    except Exception as e:
        print("An error has occurred: ", e)

# ======================================================================
#   start data acquisition
# ====================================================================== 
def start_acquisition(usbmca, usbftdi, acq_sec,config):
    global quit_flag,stop_flag
    usbmca.WriteReadCommand(usbftdi, "AQEW", 1) # Stop data acquisition: 1 --> execute
    usbmca.WriteReadCommand(usbftdi, "CLRW", 0) # Cear data: 0 --> clear

    usbmca.WriteReadCommand(usbftdi, "MODW", 0) # Data acquisition mode: 0 --> histogram
    usbmca.WriteReadCommand(usbftdi, "MMDW", 0) # Measurement mode: 0 --> real time
    acq_digit = int(acq_sec * DIGIT_PER_SEC)
    usbmca.WriteReadCommand(usbftdi, "MT0W", (acq_digit >> 32) & 0x0fff)        # Upper 12 bits of measurement time (44 bits in total)
    usbmca.WriteReadCommand(usbftdi, "MT1W", (acq_digit >> 0) & 0x0ffffffff)    # Lower 32 bits of measurement time (44 bits in total)
    usbmca.WriteReadCommand(usbftdi, "AQSW", 1) # Start data acquisition: 1 --> execute

def read_data(usbmca, usbftdi, acq_sec,config):    
    histdata = []
    isSuccess, hist0 = usbmca.ReadHistogram(usbftdi, 1)	# read histogram data, 1 --> CH1       
    histdata.append(hist0)
    spectrum = []      
    MCAmax=pow(2,14-config.MCAchannel)
    for indx in range(MCAmax):
        #if histdata[0][indx] >0:
            #print(indx,"\t",histdata[0][indx])
        spectrum.append(histdata[0][indx])
    return spectrum

    
# ======================================================================
#   Acquire data from the device APG7300D
# ====================================================================== 
def acquire_data(usbmca, usbftdi, acq_sec,config):
    global quit_flag,stop_flag
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
        if quit_flag:
            #sys.stdout.write("q command was issued. Quitting the DAQ.")
            sys.stdout.flush()
            break
        usbmca.ReadStatus(usbftdi)              # Get device status
        elapsed_sec  = float(usbmca.mStatus["RLT",]) * SEC_PER_DIGIT
        if(int(elapsed_sec)%1==0):
            #print("\n##### elapsed_sec/acq_sec (sec): %.2f/%.2f #####" % (elapsed_sec, acq_sec))            
            print(" time:",str(int(elapsed_sec)),"/",str(acq_sec),end="\r")
            #usbmca.DisplayStatus()                  # Display device status
        time.sleep(1)		# delay

    usbmca.WriteReadCommand(usbftdi, "AQEW", 1) # Stop data acquisition: 1 --> execute

	#-------------------- read histogram --> CSV file --------------------#
    histdata = []
    isSuccess, hist0 = usbmca.ReadHistogram(usbftdi, 1)	# read histogram data, 1 --> CH1       
    histdata.append(hist0)

    spectrum = []      
    MCAmax=pow(2,14-config.MCAchannel)
    for indx in range(MCAmax):
        #if histdata[0][indx] >0:
            #print(indx,"\t",histdata[0][indx])
        spectrum.append(histdata[0][indx])
    return spectrum

# ======================================================================
#   Send configuration command to the device APG7300D
# ====================================================================== 
def configure_device(usbmca, usbftdi,config):
    print("\n###### configure device ######")
    MCAmax=pow(2,14-config.MCAchannel)
    uldw=MCAmax-1
    th=int(config.threshold*MCAmax/100)
    #th=5
    print("MCAmax=",MCAmax)
    print("threshold=",th)
    usbmca.WriteReadCommand(usbftdi, "PDSW", 0) # Peak detection mode: 0 --> absolute
    #usbmca.WriteReadCommand(usbftdi, "ADGW", 0) # CH1 ADC gain: 0 --> 16384 bins
    usbmca.WriteReadCommand(usbftdi, "ADGW", config.MCAchannel) # ADC fullscale
    usbmca.WriteReadCommand(usbftdi, "THRW", th) # CH1 threshold 
    usbmca.WriteReadCommand(usbftdi, "LLDW", th) # CH1 lower level discrimination
    usbmca.WriteReadCommand(usbftdi, "ULDW", uldw) # CH1 upper level discrimination
    #usbmca.WriteReadCommand(usbftdi, "THRW", 40) # CH1 threshold: 40 ch
    #usbmca.WriteReadCommand(usbftdi, "LLDW", 40) # CH1 lower level discrimination: 40 ch
    #usbmca.WriteReadCommand(usbftdi, "ULDW", 16383) # CH1 upper level discrimination: 16383 ch
    usbmca.WriteReadCommand(usbftdi, "OFSW", 0) # CH1 offset: 0 ch

    return

# ======================================================================
#   Run main program
# ====================================================================== 
if __name__ ==  '__main__':
    #print(os.getcwd())
    monitor_thread = threading.Thread(target=key_monitor)
    monitor_thread.daemon = True  
    monitor_thread.start()
    exit_code=main_APG7300D_histgram()
    print("DAQ stopped.")
    termios.tcsetattr(fd, termios.TCSANOW, old)
    sys.exit(exit_code)


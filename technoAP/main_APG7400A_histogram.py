#!/usr/bin/python3
# -*- coding: utf-8 -*-
#  2026 Sep, Kentaro Miuchi
#  
#  Original version:
#  http://www.techno-ap.com/support_sample.html

import time
import os
import sys
import subprocess
#os.chdir(os.path.dirname(__file__)) # Current directory

import ftdi
import apg7400a
import common
import argparse
import threading
import termios


DIGIT_PER_SEC = 25000000.0
SEC_PER_DIGIT = 1.0 / DIGIT_PER_SEC

prescale=1

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

def make_links(thisfile,tmpfile):
    if os.path.isfile(tmpfile):
        cmd="unlink "+tmpfile
        cp=subprocess.run(cmd, shell=True)
    cmd="touch "+thisfile
    cp=subprocess.run(cmd, shell=True)
    path=os.getcwd()+"/"+thisfile
    path=path.replace(' ','')
    cmd="ln -s "+path+" "+tmpfile
    #print(cmd)
    subprocess.run(cmd, shell=True)

# ======================================================================
#   Main program
# ====================================================================== 
def main_APG7400A_histgram(): 
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
        #print("\n###### read configure file: done.######")
        #status=mcacommon.initStatus()
        status=common.STATUS
        ID=0
        status.presettime =presettime
        for i in range(maxMCAs):
            if configs[i].MCA_type == "APG7400A":
                print("APG7400A was found. (ID=",configs[i].ID,")")
                ID=configs[i].ID
        serialnum = (configs[ID].SN).encode("utf-8")
        usbmca = apg7400a.APG7400A()                # Create an instance of class APG7400A and run its initialization
        if verbose:
            print("usbmca=",usbmca)
        if verbose:
            print("init OK.")
        usbftdi = ftdi.FTDI()    # create an instance of class FTDI and run its initialization
        if verbose:
            print("ftdi OK.")
        isSuccess, deviceList, deviceCounts = usbftdi.GetDeviceInfoList()   # Display information for all connected devices
        if verbose:
            print("deviceList=",deviceList)  
            print("SN=",configs[ID].SN.encode("utf-8"))
        isSuccess = usbftdi.OpenBySerialNumber((configs[ID].SN).encode("utf-8"))
        fileID=0 
        #isSuccess = usbftdi.OpenBySerialNumber(serialnum)
        #fileID=0
        if isSuccess == True:
            isSuccess = usbmca.InitializeDevice(usbftdi)
            print("\n###### configure device ######")
            configure_device(usbmca, usbftdi,configs[ID])
            #configure_device(usbmca, usbftdi)
            #print("\n###### acquire data ######")
            #acquire_data(usbmca, usbftdi, 5.0)	# acq_sec = 5.0 sec
            #print("\n###### data acquisition complete ######")

        print("\n###### data acquisiion started ######")
        sys.stdout.write('### press "s" to stop after this file.\t Press "q" to quit.###\n')
        
        while(fileID < num_file_per_period):
            status.starttime = time.time()
            thisfile=configs[ID].SN+'_'+str(fileID)+'.mca'
            #print(" file:",fileID,"/",num_file_per_period,"filename:",thisfile,end="\n")
            make_links(thisfile,tmpfile)
            start_acquisition(usbmca, usbftdi, status.presettime,configs[ID])
            status.realtime = -0.001
            while status.presettime >= status.realtime:
                if quit_flag:
                    break
                usbmca.ReadStatus(usbftdi)              # Get device status
                status.realtime= float(usbmca.mStatus["RLT",]) * SEC_PER_DIGIT
                status.livetime= float(usbmca.mStatus["LVT1",]) * SEC_PER_DIGIT
                spec=read_data(usbmca, usbftdi, status.presettime,configs[ID])
                #status=0

                #mcacommon.saveSpectrum(thisfile, spec,status,starttime,presettime,elapsed_sec)  
                mcacommon.saveSpectrum(thisfile, spec,configs[ID],status)  
                if(int(status.realtime)%prescale==0):
                    #print("\n##### elapsed_sec/acq_sec (sec): %.2f/%.2f #####" % (elapsed_sec, acq_sec))
                    print(" time:",str(int(status.realtime)),"/",str(presettime),end="\t")
                    print(" file:",fileID,"/",num_file_per_period,"filename:",thisfile,end="\r")

  
                    #usbmca.DisplayStatus()                  # Display device status
                time.sleep(1)		# delay

            print("                                      ",end="\r")
            status.stoptime = time.time()
            usbmca.WriteReadCommand(usbftdi, "AQEW", 1) # Stop data acquisition: 1 --> execute
            #spec=acquire_data(usbmca, usbftdi, presettime,configs[ID])
            #print("\n###### data acquisition complete ######")
            spec=read_data(usbmca, usbftdi, status.presettime,configs[ID])
            #status=0
            
            status.realtime  = float(usbmca.mStatus["RLT",]) * SEC_PER_DIGIT
            status.livetime  = float(usbmca.mStatus["LVT1",]) * SEC_PER_DIGIT
            status.deadtime  = float(usbmca.mStatus["DDT1",]) * SEC_PER_DIGIT
            #mcacommon.saveSpectrum(thisfile, spec,status,starttime,presettime,elapsed_sec)
            mcacommon.saveSpectrum(thisfile, spec,configs[ID],status)  
                            
            if(quit_flag or stop_flag):
                return(1)            
            fileID=fileID+1
            #return(0)
        
    except Exception as e:
        print("An error has occurred: ", e)

        
    #except Exception as e:
    #    print("An error has occurred: ", e)

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
#   Acquire data from the device APG7400A
# ====================================================================== 
def acquire_data(usbmca, usbftdi, acq_sec):
    usbmca.WriteReadCommand(usbftdi, "AQEW", 1) # Stop data acquisition: 1 --> execute
    usbmca.WriteReadCommand(usbftdi, "CLRW", 0) # Clear data: 0 --> clear

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
    USED_CHANNEL_LIST = [1, 2, 3, 4] # CH1, CH2, CH3, CH4
    histdata = []

    for chn in USED_CHANNEL_LIST:
        isSuccess, hist0 = usbmca.ReadHistogram(usbftdi, chn)	# read histogram data
        histdata.append(hist0)

    with open("./histogram.csv", "w") as csv:
        csv.writelines("[header]\n")
        csv.writelines("real time(s),%f\n" % (elapsed_sec))
        csv.writelines("[data]\n")
        csv.writelines("#bin,CH1,CH2,CH3,CH4\n")

        for i in range(8192):
            textline = "%d,%d,%d,%d,%d\n" % (i,
				histdata[0][i], histdata[1][i],		# CH1, CH2
				histdata[2][i], histdata[3][i])		# CH3, CH4
            csv.writelines(textline)
    
    return

# ======================================================================
#   Send configuration command to the device APG7400A
# ====================================================================== 
#def configure_device(usbmca, usbftdi):
def configure_device(usbmca, usbftdi,config):    
    MCAmax=pow(2,14-config.MCAchannel)
    uldw=MCAmax-1
    th=int(config.threshold*MCAmax/100)
    th_nonactive=MCAmax
        
    usbmca.WriteReadCommand(usbftdi, "PDSW", 0) # Peak detection mode: 0 --> absolute
    usbmca.WriteReadCommand(usbftdi, "ADGW", config.MCAchannel) # CH1 ADC gain: 1 --> 8192 bins
    usbmca.WriteReadCommand(usbftdi, "ADG1", config.MCAchannel) # CH2 ADC gain: 1 --> 8192 bins
    usbmca.WriteReadCommand(usbftdi, "ADG2", config.MCAchannel) # CH3 ADC gain: 1 --> 8192 bins
    usbmca.WriteReadCommand(usbftdi, "ADG3", config.MCAchannel) # CH4 ADC gain: 1 --> 8192 bins
    usbmca.WriteReadCommand(usbftdi, "THRW", th) # CH1 threshold: 80 ch
    usbmca.WriteReadCommand(usbftdi, "THR1", th_nonactive) # CH2 threshold: 80 ch
    usbmca.WriteReadCommand(usbftdi, "THR2", th_nonactive) # CH3 threshold: 80 ch
    usbmca.WriteReadCommand(usbftdi, "THR3", th_nonactive) # CH4 threshold: 80 ch
    usbmca.WriteReadCommand(usbftdi, "LLDW", th) # CH1 lower level discrimination: 95 ch
    usbmca.WriteReadCommand(usbftdi, "LLD1", th_nonactive) # CH2 lower level discrimination: 95 ch
    usbmca.WriteReadCommand(usbftdi, "LLD2", th_nonactive) # CH3 lower level discrimination: 95 ch
    usbmca.WriteReadCommand(usbftdi, "LLD3", th_nonactive) # CH4 lower level discrimination: 95 ch
    usbmca.WriteReadCommand(usbftdi, "ULDW", MCAmax) # CH1 upper level discrimination: 8191 ch
    usbmca.WriteReadCommand(usbftdi, "ULD1", MCAmax) # CH2 upper level discrimination: 8191 ch
    usbmca.WriteReadCommand(usbftdi, "ULD2", MCAmax) # CH3 upper level discrimination: 8191 ch
    usbmca.WriteReadCommand(usbftdi, "ULD3", MCAmax) # CH4 upper level discrimination: 8191 ch
    usbmca.WriteReadCommand(usbftdi, "OFSW", 0) # CH1 offset: 0 ch
    usbmca.WriteReadCommand(usbftdi, "OFS1", 0) # CH2 offset: 0 ch
    usbmca.WriteReadCommand(usbftdi, "OFS2", 0) # CH3 offset: 0 ch
    usbmca.WriteReadCommand(usbftdi, "OFS3", 0) # CH4 offset: 0 ch

    return

# ======================================================================
#   Run main program
# ====================================================================== 
if __name__ == "__main__":
    #main()
    monitor_thread = threading.Thread(target=key_monitor)
    monitor_thread.daemon = True  
    monitor_thread.start()
    exit_code=main_APG7400A_histgram()
    print("DAQ stopped.")
    termios.tcsetattr(fd, termios.TCSANOW, old)
    sys.exit(exit_code)


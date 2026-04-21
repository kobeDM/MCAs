#! /usr/bin/python3
#  2026 Apr, Kentaro Miuchi

import os,subprocess,psutil,signal
import time
import datetime
import sys
import json
from datetime import timezone, timedelta
JST = timezone(timedelta(hours=+9), 'JST')
import argparse
CONFIG = "MCA_config.json"

detector=['','','']
MCAchannel=[0,0,0]
threshold=[0,0,0]
dynamicrange=[0,0,0]
polarity=[1,1,1]

ROI=[[[0,0],[0,0],[0,0],[0,0],[0,0]],[[0,0],[0,0],[0,0],[0,0],[0,0]],[[0,0],[0,0],[0,0],[0,0],[0,0]]]
SN=[0,0,0]
active=[False,False,False]

HOME = os.environ["HOME"]+"/"
MCAdir=HOME+"MCAs/"
KROMEKdir=MCAdir+"kromek/"
exe=KROMEKdir+"K102/K102"

CONFIG = "MCA_config.json"
TMP_FILE = "../tmp.mca"

#quit_flag = False
stop_flag = False
verbose = False

def key_monitor():
    global stop_flag
    while True:
        ch = sys.stdin.read(1)
        #if ch == 'q':
        #    quit_flag = True
        #    sys.stdout.write("q command was issued. Quitting the DAQ.")
        #    break
        if ch == 's':
            stop_flag = True
            sys.stdout.write("s command was issued. Stopping the DAQ at the end of this file.")
            break


def readConfig(filename):
    global SN,detector,threshold,detector,MCAchannel,dynamicrange,ROI,host,port,database
    print("Reading config file ",filename)
    ID=0
    with open(filename) as f:
        d = json.load(f)
        host=d['INFLUXDB']['host']
        port=d['INFLUXDB']['port']
        database=d['INFLUXDB']['database']
        for MCAid in d['MCA']:
            active[ID]=d['MCA'][MCAid]['active']
            print(" MCA ID:",MCAid,"(active:",active[ID],")")
            if (active[ID]):
                detector[ID]=d['MCA'][MCAid]['detector']
                MCA_type=d['MCA'][MCAid]['MCA_type']
                SN[ID]=d['MCA'][MCAid]['SN']
                threshold[ID]=d['MCA'][MCAid]['threshold']
                polarity[ID]=d['MCA'][MCAid]['polarity']
                MCAchannel[ID]=d['MCA'][MCAid]['MCAchannel']
                dynamicrange[ID]=d['MCA'][MCAid]['dynamicrange']
                ROI[ID][0][0]=d['MCA'][MCAid]['ROI0_min']
                ROI[ID][0][1]=d['MCA'][MCAid]['ROI0_max']
                ROI[ID][1][0]=d['MCA'][MCAid]['ROI1_min']
                ROI[ID][1][1]=d['MCA'][MCAid]['ROI1_max']
                ROI[ID][2][0]=d['MCA'][MCAid]['ROI2_min']
                ROI[ID][2][1]=d['MCA'][MCAid]['ROI2_max']
                ROI[ID][3][0]=d['MCA'][MCAid]['ROI3_min']
                ROI[ID][3][1]=d['MCA'][MCAid]['ROI3_max']
                ROI[ID][4][0]=d['MCA'][MCAid]['ROI4_min']
                ROI[ID][4][1]=d['MCA'][MCAid]['ROI4_max']
                print("  detector:",detector[ID],end="")
                print(", Serial Number:",SN[ID])
                print("  threshold:", threshold[ID],end="")
                print("  polarity:", polarity[ID],end="")
                print(", MCA channel:", MCAchannel[ID],end="")                
                print(", dynamic range:",dynamicrange[ID])                 
                print("  ROIs:",end="")
                for ROIid in range(5):
                    print("(",ROI[ID][ROIid][0],":",ROI[ID][ROIid][1],"), ",end="")
            #ID=ID+1
            
        #rate_filename='K102_rate.dat'    
    
        
def K102():    
    global stop_flag
    global MCAchannel, threhshold
    parser = argparse.ArgumentParser()
    parser.add_argument("-c", help="config file name", default=CONFIG)
    parser.add_argument("-v","--verbose", help="verbose mode (control only)", action='store_true')
    parser.add_argument("-p","--presettime", help="preset time for one file", default=60)
    parser.add_argument("-f", help="num of files per period", default=60)
    parser.add_argument("-t", help="temporary file name", default=TMP_FILE)
    args = parser.parse_args()

    config_filename = args.c
    presettime = int(args.presettime)
    num_file_per_period = int(args.f)
    verbose=args.verbose
    tmpfile="../"+args.t

    filename=['','']
    #th=(int)(MCAchannel[0]*threshold[0]/100)
    th=(int)(threshold[0])
    pol=polarity[0]

    sys.stdout.write('Preset time: '+str(presettime)+' \n')
    #sys.stdout.write('taking data.\tpress "s" to stop after this file.\t Press "q" to quit.\n')
    sys.stdout.write('taking data.\tctrl+c to quit.\n')    
    fileID=0
    while(fileID < num_file_per_period):
        starttime = time.time()        
        print(" file:",fileID,"/",num_file_per_period,end="")
        #while ((currentrealtime)<presettime ):
        #if quit_flag:
            #sys.stdout.write("q command was issued. Quitting the DAQ.")
            #sys.stdout.flush()
            #break
        cmd="unlink "+tmpfile
        cp=subprocess.run(cmd, shell=True)
        thisfile=" K102_"+str(fileID)+".mca"
        cmd="touch "+thisfile
        cp=subprocess.run(cmd, shell=True)
        path=os.getcwd()+"/"+thisfile
        path=path.replace(' ','')
        cmd="ln -s "+path+" "+tmpfile
        #print(cmd)
        subprocess.run(cmd, shell=True)
        cmd=exe+" "+str(presettime)+" "+str(th)+" "+str(pol)+" "+thisfile
        print(cmd)
        cp=subprocess.run(cmd, shell=True)
        runend_flag=cp.returncode;
        print("runend_flag:"+str(stop_flag))        
        if(runend_flag==0):
            #cmd="cp "+tmpfile+" K102_"+str(fileID)+".mca"
            #subprocess.run(cmd, shell=True)  
            fileID=fileID+1
            #return(1)
            #else:

    return(0)
    
if __name__ == '__main__':
    readConfig(CONFIG)
    exit_code=K102()
    print(str(exit_code))

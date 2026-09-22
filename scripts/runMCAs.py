#!/usr/bin/python3
import os,subprocess,psutil,signal

import sys
from datetime import timezone, timedelta

import argparse
import threading
import json


import csv
import termios

import common


#PATHs
HOME = os.environ["HOME"]+"/"

MCAdir=HOME+"MCAs/"
MCAbin=MCAdir+"bin/"
MCAConfigs=MCAdir+"MCAconfigfiles/"
MCAScripts=MCAdir+"scripts/"
MCARootMacros=MCAdir+"root_macros/"
runMCA=MCAdir+"scripts/runMCA.py"

# configs
CONFIG = "MCA_config.json"

print("**********************************************************")
print("   runMCAs.py  ")
print("   MCAs DAQ for MIRACLUE-AIST2026  " )
print("   2026 Sep by K. Miuchi")
print("**********************************************************")

maxMCAs=8
active=[False]*maxMCAs

#def run_MCA():
#    runMCA
    
def kill_runMCA():
    exe= "runMCA.py"
    processes = []
    for proc in psutil.process_iter(['pid', 'name', 'cmdline']):
        try:
            processes.append(proc.info)
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            pass
    
    for proc in processes:
        if proc['name'] ==exe:
            for cmdline in proc['cmdline']:
                if TMP_FILE  in cmdline: 
                    os.kill(proc['pid'],signal.SIGKILL)

def copy_configfile(config_filename):
    # copy config file        
    if (not os.path.exists(config_filename)):
        print(config_filename+" does not exist.")
        cmd="cp "+MCAConfigs+CONFIG+" "+config_filename
        print(cmd)
        subprocess.run(cmd, shell=True)
    

def make_configfile(config_filename,IDtorun):
    # copy config file        
    if (not os.path.exists(config_filename)):
        print(config_filename+" does not exist.")
        print("MCAid to run:",IDtorun)
        ID=0
        with open("../"+config_filename) as f:
            d = json.load(f)
            for MCAid in d['MCA']:
                if MCAid != IDtorun:
                    d['MCA'][MCAid]['active']=0 #deactivate
                ID=ID+1
        
        with open(config_filename, 'w') as f:
            json.dump(d, f, indent=2)

def main():    
    parser = argparse.ArgumentParser()
    parser.add_argument("-c", help="config file name", default=CONFIG)
    parser.add_argument("-v","--verbose", help="verbose mode (control only)", action='store_true')
    parser.add_argument("-p","--presettime", help="preset time for one file", default=60)
    parser.add_argument("-f", help="num of files per period", default=60)
    args = parser.parse_args()
    config_filename = args.c
    copy_configfile(config_filename)
    
    mcacommon=common.COMMON
    print("\n###### read configure file ######")
    configs=mcacommon.readConfig(config_filename)
    i=0
    activeMCAs=0

    for i in range(maxMCAs):
        if (configs[i].active):
            datadir=configs[i].SN
            os.makedirs(datadir, exist_ok=True)
            os.chdir(datadir)
            print("run MCAID",configs[i].ID," (",configs[i].MCA_type,")")
            print("  data dir: ",os.getcwd())
            make_configfile(config_filename,configs[i].MCAid)
            cmd="xterm -e "+runMCA+" &"
            subprocess.run(cmd, shell=True)
            os.chdir("../")
            activeMCAs=activeMCAs+1
        i=i+1
    print(str(activeMCAs)+" MCA(s) is(are) active.")
        



# ======================================================================
#   Run main program
# ====================================================================== 
if __name__ ==  '__main__':
    main()


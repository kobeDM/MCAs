#! /usr/bin/python3
#  2025 Oct, Kentaro Miuchi
#  
#  Original version:
#  Copyright 2019 Henning Follmann <hfollmann@itcfollmann.com>

"""A USB interface to AMPTEK's MCA8000d"""  

import usb.core
import usb.util
import struct
import sys
import os
import csv
import subprocess
import time
import datetime
import threading
import json
import argparse
import termios
import errno
#import rettest
from datetime import timezone, timedelta
JST = timezone(timedelta(hours=+9), 'JST')


# global parameters
HEADER_SIZE=12
FOOTTER_SIZE=70

CONFIG = "MCA_config.json"
TMP_FILE = "../tmp.mca"

detector=['','']
MCAchannel=[0,0]
threshold=[0,0]
dynamicrange=[0,0]
ROI=[[[0,0],[0,0],[0,0],[0,0],[0,0]],[[0,0],[0,0],[0,0],[0,0],[0,0]]]
SN=[0,0]
active=[False,False]

rate_filename=['','']
presettime=0
#for keyboard interrupt
fd = sys.stdin.fileno()
old = termios.tcgetattr(fd)
new = termios.tcgetattr(fd)
new[3] &= ~termios.ICANON
new[3] &= ~termios.ECHO
termios.tcsetattr(fd, termios.TCSANOW, new)


quit_flag = False
stop_flag = False
verbose = False

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

def post_to_influx(file,measurement,daemon):
    from influxdb import InfluxDBClient
    client = InfluxDBClient( host= host,port= port,database= database)
    if(not os.path.isfile(file)):
        cmd="touch "+file
        subprocess.run(cmd, shell=True)
    print(" influxdb server: ",host,":",port," database:",database," measurement:",measurement)
    while(1):
        with open(file,'r') as f:
            reader=csv.reader(f,delimiter='\t')
            for data in reader:
                json_data = [
                    {
                        'measurement' : measurement,
                        'fields' : {
                            'time_stamped'  : float(data[0]),
                            'event_rate_live'  : float(data[1]),
                            'rate_ROI1'  : float(data[2]),
                            'rate_ROI2'  : float(data[3]),
                            'rate_ROI3'  : float(data[4]),
                            'rate_ROI4'  : float(data[5])
                        },
                        'time': datetime.datetime.fromtimestamp(float(data[0])).astimezone(tz=JST).replace(tzinfo=JST).astimezone(tz=timezone.utc),
                        'tags' : {
                            'device' : measurement
                            #'device' :'3He'
                        }
                    }
                ]
                #print(json_data)
                result = client.write_points(json_data)
        time.sleep(1)
        
def chksum(data):
    checksum = 0;
    for b in bytearray(data):
        checksum += int(b)
    return (((checksum & 0xffff) ^ 0xffff) + 1)


# pack and unpack integer
# device expects big endian
def packint(unsint):
    """convert integer to 2 bytes (big endian)"""
    ba = struct.pack('>H', unsint)
    return (ba)

def unpackint(twobytes):
    """convert two bytes to integer (from big endian)"""
    res = struct.unpack('>H',twobytes)
    return (res[0])

# unpack status fields
# here it is little endian
def fourbytes2float(ba):
    """convert four bytes to float"""
    nums = struct.unpack('<I', ba)
    return (1.0 * nums[0])

def fourbytes2long(ba):
    """convert four bytes to integer"""
    nums = struct.unpack('<I', ba)
    return nums[0]

def threebytes2long(ba):
    """convert three bytes to integer"""
    num = int(ba[0]) + (int(ba[1]) * 256) + (int(ba[2]) * 65536)
    return num

# pack message
# message format:
# msg[0] = header[0] # this is sync1
# msg[1] = header[1] #         sync2
# msg[2] = header[2] #         requestid1
# msg[3] = header[3] #         requestid2
# msg[4] = high byte len(data)
# msg[5] = low byte len(data)
# msg[6] = data[0]
# ...
# msg[len(data)+5]
# msg[len(data)+6]=high byte checksum # msg[-2]
# msg[len(data)+7]=low byte checksum  # msg[-1]
def packmsg(header, data):
    """packmsg prepares a msg to send it to the device"""
    length = len(data)
    ba = header + packint(length) + data.encode()
    cs = chksum(ba)
    return (ba + packint(cs))


class status:
    """status of a mca8000d device"""
    def __init__(self, raw):
        """parse mca8000d status msg into status class"""
        self.DEVICE_ID = raw[39]
        self.FastCount= fourbytes2long(raw[0:4])
        self.SlowCount= fourbytes2long(raw[4:8])
        self.GP_COUNTER= fourbytes2long(raw[8:12])
        self.AccumulationTime = int(raw[12]) + (threebytes2long(raw[13:16]) * 100)  # in msec
        self.RealTime = fourbytes2long(raw[20:24]) # in msec
        self.Firmware=raw[24]
        self.FPGA=raw[25]
        if self.Firmware > 0x65 :
            self.Build = raw[37] & 0xF
        else:
            self.Build = 0
        self.bDMCA_LiveTime =  (self.DEVICE_ID == 3) and (self.Firmware >= 0x67)
        if self.bDMCA_LiveTime:
            self.LiveTime = fourbytes2long(raw[16:20]) # in msec
        else:
            self.LiveTime = 0
        if raw[29] < 128:
            self.SerialNumber = fourbytes2long(raw[26:30])
        else:
            self.SerialNumber = -1
        self.PresetRtDone = (raw[35] & 128) == 128
        self.PresetLtDone = False
        self.AFAST_LOCKED = False
        if self.bDMCA_LiveTime :
            self.PresetLtDone = (raw[35] & 64) == 64
        else:
            self.AFAST_LOCKED = (raw[35] & 64) == 64
        self.MCA_EN = (raw[35] & 32) == 32
        self.PRECNT_REACHED = (raw[35] & 16) == 16
        self.SCOPE_DR = (raw[35] & 4) == 4
        self.DP5_CONFIGURED = (raw[35] & 2) == 2
        self.AOFFSET_LOCKED = (raw[36] & 128) == 128
        self.MCS_DONE = (raw[36] & 64) == 64
        self.b80MHzMode = (raw[36] & 2) == 2
        self.bFPGAAutoClock = (raw[36] & 1) == 1
        self.PC5_PRESENT = (raw[38] & 128) == 128
        if self.PC5_PRESENT:
            self.PC5_HV_POL = (raw[38] & 64) == 64
            self.PC5_8_5V = (raw[38] & 32) == 32
        else:
            self.PC5_HV_POL = False
            self.PC5_8_5V = False
        self.DPP_ECO = raw[49]

        

def getSN(status):
    #print(str(status.SerialNumber))
    return int(status.SerialNumber)

def printStatus(status):
    sys.stdout.write('================ MCA8000D status ===================\n')
    sys.stdout.write('Device Id       : ' + str(status.DEVICE_ID) +'\n')
    sys.stdout.write('Firmware        : ' + str(status.Firmware) +'\n')
    sys.stdout.write('FPGA            : ' + str(status.FPGA) +'\n')
    sys.stdout.write('SerialNumber    : ' + str(status.SerialNumber) +'\n')
    sys.stdout.write('AccumulationTime: ' + str(status.AccumulationTime) +' msec\n')        
    sys.stdout.write('RealTime        : ' + str(status.RealTime) +' msec\n')
    sys.stdout.write('FastCount       : ' + str(status.FastCount) +'\n')
    sys.stdout.write('SlowCount       : ' + str(status.SlowCount) +'\n')
    sys.stdout.write('GP Counter      : ' + str(status.GP_COUNTER) +'\n')
    sys.stdout.write('MCA_EN          : ' )
    if status.MCA_EN:
        sys.stdout.write('Yes\n')
    else:
        sys.stdout.write('No\n')
    sys.stdout.write('MCS_DONE        : ' )
    if status.MCS_DONE:
        sys.stdout.write('Yes\n')
    else:
        sys.stdout.write('No\n')
    sys.stdout.write('PRECNT_REACHED  : ' )
    if status.PRECNT_REACHED:
        sys.stdout.write('Yes\n')
    else:
        sys.stdout.write('No\n')
    sys.stdout.write('PC5_PRESENT     : ' )
    if status.PC5_PRESENT:
        sys.stdout.write('Yes\n')
    else:
        sys.stdout.write('No\n')
    sys.stdout.write('DPP_ECO         : ' + str(status.DPP_ECO) +'\n')
    sys.stdout.write('====================================================\n')
        

#############################################################################
# mca8000d config
configParameters = {"RESC" : "Reset Configuration",\
                    "PURE" : "PUR Interval on/off",\
                    "MCAS" : "MCA Source",\
                    "MCAC" : "MCA/MCS Channels",\
                    "SOFF" : "Set Spectrum Offset",\
                    "GAIA" : "Analog Gain Index",\
                    "PDMD" : "Peak Detect Mode (min/max)",\
                    "THSL" : "Slow Threshold",\
                    "TLLD" : "LLD Threshold",\
                    "GATE" : "Gate Control",\
                    "AUO1" : "AUX OUT Selection",\
                    "PRER" : "Preset Real Time",\
                    "PREL" : "Preset Live Time",\
                    "PREC" : "Preset Counts",\
                    "PRCL" : "Preset Counts Low Threshold",\
                    "PRCH" : "Preset Counts High Threshold",\
                    "SCOE" : "Scope Trigger Edge",\
                    "SCOT" : "Scope Trigger Position",\
                    "SCOG" : "Digital Scope Gain",\
                    "MCSL" : "MCS Low Threshold",\
                    "MCSH" : "MCS High Threshold",\
                    "MCST" : "MCS Timebase",\
                    "AUO2" : "AUX OUT 2 Selection",\
                    "GPED" : "G.P.Counter Edge",\
                    "GPIN" : "G.P. Counter Input",\
                    "GPME" : "G.P. Counter Uses MCA_EN",\
                    "GPGA" : "G.P. Counter Uses Gate",\
                    "GPMC" : "G.P. Counter Cleared With MCA",\
                    "MCAE" : "MCA/MCS Enable",\
                    "TPEA" : "peaking time",\
                    "TFLA" : "Flat Top Width",\
                    "TPFA" : "Fast channel peaking time",\
                    "AINP" : "Analog input pos/neg"}


def printConfig(cfg):
    sys.stdout.write('================ MCA8000D CFG ===================\n')
    for k in cfg.keys():
        sys.stdout.write(configParameters[k] + ' : ' + cfg[k] +'\n')
    sys.stdout.write('============================= ===================\n')



def writeConfig(filename,cfg):
    """save hardware configuration to file"""
    fh = open(filename, "w")
    for k in cfg.keys():
        fh.write(k + "=" +cfg[k] +';\n')
    fh.close()


def createCfgString(cfg):
    cfgstring = ""
    for k in cfg.keys():
        cfgstring += k + "=" +cfg[k] +';'
    return(cfgstring)

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
                SN[ID]=d['MCA'][MCAid]['SN']
                threshold[ID]=d['MCA'][MCAid]['threshold']
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
                print(", MCA channel:", MCAchannel[ID],end="")                
                print(", dynamic range:",dynamicrange[ID])                 
                print("  ROIs:",end="")
                for ROIid in range(5):
                    print("(",ROI[ID][ROIid][0],":",ROI[ID][ROIid][1],"), ",end="")
            ID=ID+1
    for i in range (2):
        rate_filename[i]='SN'+str(SN[i])+'_rate.dat'    
    
#def readConfig(filename):
#    """load hardware configuration from file"""
#    fh = open(filename, "r")
#    cfg = {}
#    for line in fh:
#        last = line.find(";")
#        pv= line[:last].split('=')
#        cfg.setdefault(pv[0], pv[1])
#    fh.close()
#    return(cfg)

    
spectrumSize ={ 1 : 255,\
                2 : 255,\
                3 : 511,\
                4 : 511,\
                5 : 1023,\
                6 : 1023,\
                7 : 2047,\
                8 : 2047,\
                9 : 4095,\
                10 : 4095,\
                11 : 8191,\
                12 : 8191}  # max channel number zero indexed

    
class device:
    """device provides all communications to a mca8000d device"""
    def __init__(self,mcaID):        
        device_all=usb.core.find(find_all=True,idVendor=0x10c4, idProduct=0x842a)
        #mcaID=0
        if (verbose):
            print("vendor\tproduct\tbus\taddress")
        #print("mcaID:",mcaID)
        ID=0
        if (verbose):
            print(device_all)
        for devscan in device_all:
            if (verbose):
                print(f"{devscan.idVendor:04x}\t{devscan.idProduct:04x}\t{devscan.bus}\t{devscan.address}")
            if(ID==mcaID):            
                break
            #else:
            ID=ID+1
        self.dev=devscan
        if self.dev is None:
            raise ValueError('Device not found')
        #self.dev.set_configuration()    
        try:
            self.dev.set_configuration()
            if (verbose):
                print("Configuration set.")
        except usb.core.USBError as e:
            if e.errno != errno.EBUSY:  # 'Resource busy' is acceptable
                print(f"Could not set configuration: {e}")
                #sys.exit(1)    
        self.eout=2
        self.ein=129
        self.timeout=500
        #print("message test:")
        #status = self.dev.reqStatus()
        #printStatus(status)
        #statusmsg = self.recvCmd()
        
    def __del__(self):
        self.dev.reset()
        usb.util.dispose_resources(self.dev)
        
    def sendCmd(self, req_pid1, req_pid2, data):
        """sends raw cmd over usb"""
        header = bytearray(4)
        header[0]=0xF5        # SYNC1
        header[1]=0xFA        # SYNC2
        header[2]=req_pid1
        header[3]=req_pid2
        pout = packmsg(header, data)
        # send to device
        res=self.dev.write(self.eout, pout, self.timeout)
        return res
    
        
    def recvCmd(self):
        """receives raw cmd over usb"""
        devmsg = self.dev.read(self.ein, 65535, self.timeout)
        chksm = unpackint(devmsg[-2:])
        cntrl = chksum(devmsg[:-2])
        assert chksm == cntrl
        return ((devmsg[2], devmsg[3], devmsg[6:-2]))

    def reqStatus(self):
        """get status of  mca8000d device"""
        #print("req status start")
        data=''
        self.sendCmd(1,1,data)
        #print("send cmd OK")
        statusmsg = self.recvCmd()
        #print( statusmsg)
        return (status(statusmsg[2]))
    
    def reqHWConfig(self):
        """get hardware configuration from device"""
        data=''
        for confp in configParameters.keys():
            data += confp + '=?;'
        # pid1 = 0x20
        # pid2 = 0x03
        self.sendCmd(0x20,0x03,data)
        cfgmsg = self.recvCmd()
        l = len(cfgmsg[2])
        fmt = str(l) + 's'
        lcfgstr = struct.unpack(fmt, cfgmsg[2])
        #print(lcfgstr[0])
        #print(lcfgstr[0].decode('utf-8'))
        cfg = {}
        for param in lcfgstr[0].decode('utf-8').split(';'):
            pv = param.split('=')
            if len(pv) == 2:
                cfg.setdefault(pv[0], pv[1])
        return(cfg)

    def sendCmdConfig(self, cmd):
        """sends a configuration string to device"""
        # pid1 = 0x20
        # pid2 = 0x02
        self.sendCmd(0x20, 0x02, cmd)
        cfgmsg = self.recvCmd()
        return (cfgmsg)

    def setPresetTime(self, time):
        """set preset (real) time"""
        cmd = 'PRER='
        if (time < 0):
            raise ValueError('Negative time not allowed\n')
        if (time == 0):
            cmd = cmd + "OFF;"
        else:
            cmd = cmd + str(time) + ";"
        self.sendCmdConfig(cmd)
        cmd = 'PREL=' + str(time*2) + ";"
        self.sendCmdConfig(cmd)

    def setMCAchannel(self, value):
        cmd = 'MCAC='+ str(value) + ";"
        self.sendCmdConfig(cmd)

    def setthreshold(self, value):
        cmd = 'THSL='+ str(value) + ";"
        self.sendCmdConfig(cmd)


    def setdynamicrange(self, value):
        cmd = 'GAIA='+ str(value) + ";"
        #cmd = 'GAIA=0.1;'
        #print("set dynamicrange:",cmd)
        self.sendCmdConfig(cmd)
        
        
    # start MCA MCS scan
    def enable_MCA_MCS(self):
        """start data acquisition"""
        data = ''
        # pid1 = 0xF0
        # pid2 = 0x02
        self.sendCmd(0xF0, 0x02, data)
        res = self.recvCmd()
        return (res)

    # stop MCA MCS scan
    def disable_MCA_MCS(self):
        """stop data acquistion"""
        data = ''
        # pid1 = 0xF0
        # pid2 = 0x03
        self.sendCmd(0xF0, 0x03, data)
        res = self.recvCmd()
        return (res)

    def spectrum(self, bStatus, bClear):
        """get spectrum data 
           if bStatus is True it will get status too
           if bClear is True spectrum data (and status) will be cleared"""
        data = ''
        # pid1 = 0x02
        # pid2 = 0x01...0x04
        Pid2 = 0x01
        if bStatus:
            Pid2 += 2
        if bClear:
            Pid2 += 1
        self.sendCmd(0x02, Pid2, data)
        res = self.recvCmd()
        #maxChan = spectrumSize[res[1]]
        #print("maxChan com:",res[1])
        maxChan = spectrumSize[res[1]]
        spectrum = []
        sta = None
        for indx in range(0, maxChan*3, 3):
            spectrum.append( threebytes2long(res[2][indx:(indx+3)]) )
            
        if bStatus:
             sta = status(res[2][-64:])
        
        return ([spectrum, sta])

def ratecheck(spectrum,time,ID):
    ch=0
    rate=[0,0,0,0,0]
    for chan in spectrum:
        rate[0]+=chan
        #if chan > 0 :
            #print(str(ch)," ",str(chan))
        for i in range (1,5):
            if ch > ROI[ID][i][0] and  ch< ROI[ID][i][1]:
                rate[i]+=chan
        ch+=1
    for i in range (5):
        rate[i] /= time
    #return rate[0]
    return rate
        
def saveSpectrum(filename, spectrum,status,starttime,i):
    global presettime
    """write spectrum to file, one channel per line"""
    fh = open(filename, "w")
    fh.write("<<PMCA SPECTRUM>>\n")
    fh.write("TAG - live_data\n")    
    fh.write("DESCRIPTION - \n")    
    fh.write("GAIN - 5\n")
    s="THRESHOLD - "+str(threshold[i])+"\n"    
    fh.write(s)
    fh.write("LIVE_MODE - 0\n")    
    s="PRESET_TIME - "+str(presettime)+"\n"    
    fh.write(s)
    s="LIVE_TIME - "+str(status.AccumulationTime/1000.)+"\n"
    fh.write(s)
    s="REAL_TIME - "+str(status.RealTime/1000.)+"\n"    
    fh.write(s)
    s="START_TIME - "+str(starttime)+"\n"    
    fh.write(s)
    s="SERIAL_NUMBER - "+str(SN[i])+"\n"    
    fh.write(s)
    fh.write("<<DATA>>\n")
    for chan in spectrum:
        fh.write("{}\n".format( str(chan)))
    fh.write("0\n")
    fh.write("<<END>>\n")
    fh.write("<<DP5 CONFIGURATION>>\n")
    s="MCAC="+str(MCAchannel[i])+";    MCA/MCS Channels\n"    
    fh.write(s)
    s="GAIN="+str(dynamicrange[i])+";    Total Gain (Analog * Fine)\n"
    fh.write(s)
    fh.write("GAIA=1;    Analog Gain Index\n")
    for line in range (FOOTTER_SIZE-21):
        fh.write("FOOTTER\n")
    fh.write("<<DP5 CONFIGURATION END>>\n")
    fh.write("<<DPP STATUS>>\n")
    s="Accumulation Time: "+str(status.AccumulationTime/1000.)+"\n"
    fh.write(s)
    s="Real Time:  "+str(status.RealTime/1000.)+"\n"    
    fh.write(s)
    for line in range (11):
        fh.write("FOOTTER\n")
    fh.write("<<DPP STATUS END>>\n")
    fh.close()
    
def saveRates(filename,starttime, rate):
    fh = open(filename, "a")
    #fh.write(starttime,"\t")
    fh.write("{}\t".format(str(starttime)))
    #print(starttime,"\t")
    for i in range(5):
        #print(str(rate[i]),end="\t")
        fh.write("{}\t".format(str(rate[i])))
    fh.write("\n")
    
def mca8000d():
    global quit_flag,stop_flag
    global SN,detector,threshold,detector,MCAchannel,dynamicrange,ROI,presettime
    global rate_filename
    global active
    rate=[[0,0,0,0,0],[0,0,0,0,0]]

    #sys.stdout.write('Find MCA8000D device\n')
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

    #SN=args.serialnumber

    filename=['','']

    #scan
    #print("activity",active)
    if active[0]:
        #        print("device 0 check...")
        dev = device(0)
        status = dev.reqStatus()
        thisSN=getSN(status)
        #printStatus(status)
        if thisSN==SN[0]:
            dev0=dev
        #elif thisSN==SN[1]:
            #dev1=dev

    if active[1]:
        #print("device 1 check...")
        dev = device(1)
        status = dev.reqStatus()
        thisSN=getSN(status)
        if thisSN==SN[1]:
            dev1=dev
        #elif thisSN==SN[0]:
            #dev0=dev

    if active[0]:
        status0 = dev0.reqStatus()
        thisSN=getSN(status0)
        print(" device 0: SN=",thisSN)
    if active[1]:
        status1 = dev1.reqStatus()
        thisSN=getSN(status1)
        print(" device 1: SN=",thisSN)
    
    #printStatus(status)
    #if verbose:
    #    print("\nHardware settings")
    #    print(" device 0: SN=",getSN(status0))
    #    print(" device 1: SN=",getSN(status1))

    
    # if still running
    if active[0]:
        if status0.MCA_EN :
            sys.stdout.write('MCA8000D currently running... stopping now\n')
            dev0.disable_MCA_MCS()
    if active[1]:
        if status1.MCA_EN :
            sys.stdout.write('MCA8000D currently running... stopping now\n')
            dev1.disable_MCA_MCS()
    #sys.stdout.write('MCA8000D clearing\n')
    #dev.spectrum(True, True)
    sys.stdout.write('Preset time: '+str(presettime)+' \n')
    #sys.stdout.write('MCA8000D start scan\n')
    sys.stdout.write('taking data.\tpress "s" to stop after this file.\t Press "q" to quit.\n')
    #print(quit_flag,"in DAQ")

    if active[0]:
        #print("device setup 0")
        dev0.spectrum(True, True)
        dev0.setPresetTime(presettime)
        dev0.setMCAchannel(MCAchannel[0]+1)
        dev0.setthreshold(threshold[0])
        dev0.setdynamicrange(dynamicrange[0])


    if active[1]:
        dev1.spectrum(True, True)
        dev1.setPresetTime(presettime)
        dev1.setMCAchannel(MCAchannel[1]+1)
        dev1.setthreshold(threshold[1])
        dev1.setdynamicrange(dynamicrange[1])
    fileID=0
    while(fileID < num_file_per_period):
        #print(" file",fileID,"/",num_file_per_period,end="")
        starttime = time.time()
        if active[0]:
            dev0.enable_MCA_MCS()
            status0=dev0.reqStatus()
        if active[1]:
            dev1.enable_MCA_MCS()
            status1=dev1.reqStatus()
        if active[0]:
            currentrealtime=status0.RealTime/1000
        elif  active[1]:
            currentrealtime=status1.RealTime/1000
        while ((currentrealtime)<presettime ):
            if quit_flag:
                #sys.stdout.write("q command was issued. Quitting the DAQ.")
                sys.stdout.flush()
                break
            time.sleep(1)
            if active[0]:
                status0=dev0.reqStatus()
                currentrealtime=status0.RealTime/1000
                if(int(status0.RealTime/1000)%10==0):
                    print(" file:",fileID,"/",num_file_per_period,end="")
                    print(" time:",str(int(status0.RealTime/1000)),"/",str(presettime),end="\r")
                    #sys.stdout.flush()
                spec0=dev0.spectrum(True, False) #keep the running data
                saveSpectrum(tmpfile, spec0[0],status0,starttime,0)
                    
            elif  active[1]:
                status1=dev1.reqStatus()
                currentrealtime=status1.RealTime/1000
                if(int(status1.RealTime/1000)%10==0):
                    print(" file:",fileID,"/",num_file_per_period,end="")
                    print(" time:",str(int(status1.RealTime/1000)),"/",str(presettime),end="\r")

        if active[0]:
            status0=dev0.reqStatus()
            dev0.disable_MCA_MCS()
        if active[1]:
            status1=dev1.reqStatus()
            dev1.disable_MCA_MCS()
        
        for i in range(2):
            if active[i]:
                filename[i]='SN'+str(SN[i])+'_'+str(fileID)+'.mca'
                print("  saved in ",filename[i])
                
        #print("  saved in ",filename[0],"and",filename[1],end="\r")
        #print("  saved in ",filename[0],"and",filename[1])
        if active[0]:
            spec0=dev0.spectrum(True, True)
            rate[0]=ratecheck(spec0[0],presettime,0)
            saveSpectrum(filename[0], spec0[0],status0,starttime,0)
            saveRates(rate_filename[0], starttime,rate[0])
                    
        if active[1]:            
            spec1=dev1.spectrum(True, True)
            rate[1]=ratecheck(spec1[0],presettime,1)
            saveSpectrum(filename[1], spec1[0],status1,starttime,1)
            saveRates(rate_filename[1], starttime,rate[1])
        
        
        #for i in range(2):
            #saveRates(rate_filename[i], starttime,rate[i])
        
        if verbose:
            if active[0]:
                config=dev0.reqHWConfig()
                printConfig(config)
            if active[1]:
                config=dev1.reqHWConfig()
                printConfig(config)
        dev=None
        if(quit_flag or stop_flag):
            return(1)
            #done
        fileID=fileID+1
    return(0)
    
if __name__ == '__main__':
    readConfig(CONFIG)
    monitor_thread = threading.Thread(target=key_monitor)
    monitor_thread.daemon = True  
    monitor_thread.start()
    
    if active[0]:
        influx_thread0=threading.Thread(target=post_to_influx,args=(rate_filename[0],"he3_mca","daemon"),daemon=True)
        influx_thread0.start()
    if active[1]:
        influx_thread1=threading.Thread(target=post_to_influx,args=(rate_filename[1],"NaI","daemon"),daemon=True)
        influx_thread1.start()
    
    exit_code=mca8000d()
    print("DAQ stopped.")

    if (verbose):
        print("exit_code",exit_code)
    termios.tcsetattr(fd, termios.TCSANOW, old)

    sys.exit(exit_code)
    
    

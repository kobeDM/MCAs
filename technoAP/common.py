import os
import json
class CONFIG:
    def __init__(self):
        self.MCAchannel = 0
        self.threshold= 0
        self.dynamicrange= 0
        self.ID= 0
        self.SN=""
        self.active=False
        self.detector = ""
        self.MCA_type = ""
        self.ratefilename = ""
        self.ROI = [[0,0],[0,0],[0,0],[0,0],[0,0]]

  
class STATUS:
    def __init__(self):
        self.presettime=0
        self.realtime=0
        self.livetime=0
        self.deadtime=0
        self.startime=0
        self.stoptime=0
        self.evnum=0

        
class COMMON():
    def readConfig(filename):
        maxMCAs=8
        configs=[]
        MCAchannel=[]
        threshold=[]
        dynamicrange=[]
        SN=[]
        active=[]
        ROI=[]
        rate_filename=[]
        ID=[]
        MCA_type=[]
        for i in range (maxMCAs):
            det=CONFIG()
            configs.append(det)
        print("Reading config file ",filename)        
        ID=0
        with open(filename) as f:
            d = json.load(f)
            host=d['INFLUXDB']['host']
            port=d['INFLUXDB']['port']
            database=d['INFLUXDB']['database']
            for MCAid in d['MCA']:
                configs[ID].active=d['MCA'][MCAid]['active']
                if (configs[ID].active):
                    configs[ID].detector=d['MCA'][MCAid]['detector']
                    configs[ID].SN=d['MCA'][MCAid]['SN']
                    configs[ID].threshold=d['MCA'][MCAid]['threshold']
                    configs[ID].MCAchannel=d['MCA'][MCAid]['MCAchannel']
                    configs[ID].dynamicrange=d['MCA'][MCAid]['dynamicrange']
                    configs[ID].MCA_type=d['MCA'][MCAid]['MCA_type']
                    configs[ID].ID=ID
                    configs[ID].ROI[0][0]=d['MCA'][MCAid]['ROI0_min']
                    configs[ID].ROI[0][1]=d['MCA'][MCAid]['ROI0_max']
                    configs[ID].ROI[1][0]=d['MCA'][MCAid]['ROI1_min']
                    configs[ID].ROI[1][1]=d['MCA'][MCAid]['ROI1_max']
                    configs[ID].ROI[2][0]=d['MCA'][MCAid]['ROI2_min']
                    configs[ID].ROI[2][1]=d['MCA'][MCAid]['ROI2_max']
                    configs[ID].ROI[3][0]=d['MCA'][MCAid]['ROI3_min']
                    configs[ID].ROI[3][1]=d['MCA'][MCAid]['ROI3_max']
                    configs[ID].ROI[4][0]=d['MCA'][MCAid]['ROI4_min']
                    configs[ID].ROI[4][1]=d['MCA'][MCAid]['ROI4_max']
                    print(" MCA ID:",MCAid,"(active:", configs[ID].active,")")
                    for ROIid in range(5):
                        print("(",configs[ID].ROI[ROIid][0],":",configs[ID].ROI[ROIid][1],")",end="")
                        if(ROIid < 4):
                            print(", ",end="")
                    print("")
                #for i in range (2):
                 #   rate_filename[i]='SN'+str(configs[ID].SN[i])+'_rate.dat' 
                ID=ID+1
        return(configs)

    def showConfig(config):
        print("  detector:",config.detector,end="")
        print(", Serial Number:",config.SN,end="")
        print("  threshold:", config.threshold,end="")
        print(", MCA channel:", config.MCAchannel,end="")                
        print(", dynamic range:",config.dynamicrange,end="")                    
        print(", threshold:",config.threshold)                 
        print(",  ROIs:",end="")
        for ROIid in range(5):
            print("(",config.ROI[ROIid][0],":",config.ROI[ROIid][1],")",end="")
            if(ROIid < 4):
                print(", ",end="")
            
        print("")
        
    #def saveSpectrum(filename, spectrum,status,starttime,presettime):
    def saveSpectrum(filename, spectrum,config,status):
        HEADER_SIZE=12
        FOOTTER_SIZE=70
        #print("saving spectrum")
        #print(" file name:",filename)

        """write spectrum to file, one channel per line"""
        fh = open(filename, "w")
        fh.write("<<PMCA SPECTRUM>>\n")
        fh.write("TAG - live_data\n")    
        fh.write("DESCRIPTION - \n")    
        fh.write("GAIN - "+str(6-config.MCAchannel)+"\n")

        fh.write("THRESHOLD - "+str(config.threshold)+"%\n")
        fh.write("LIVE_MODE - 0\n")    
        fh.write("PRESET_TIME - "+str(status.presettime)+"\n")
        fh.write("LIVE_TIME - {:.2f}\n".format(status.livetime))
        fh.write("REAL_TIME - {:.2f}\n".format(status.realtime))
        fh.write("START_TIME - "+str(status.starttime)+"\n")
        fh.write("SERIAL_NUMBER - "+config.SN+" \n")
        fh.write("<<DATA>>\n")
        for chan in spectrum:
            fh.write("{}\n".format( str(chan)))
        fh.write("0\n")
        fh.write("<<END>>\n")
        fh.write("<<MCA CONFIGURATION>>\n")
        fh.write("MCAC= "+str(config.MCAchannel)+"\n")
        fh.write("GAIN= \n")
        fh.write("GAIA=1;    Analog Gain Index\n")
        for line in range (FOOTTER_SIZE-21):
            fh.write("FOOTTER\n")
        fh.write("<<MCA CONFIGURATION END>>\n")
        fh.write("<<MCA STATUS>>\n")
        fh.write("Live Time:  "+str(status.livetime)+"\n")    
        fh.write("Real Time:  "+str(status.realtime)+"\n")    
        for line in range (11):
            fh.write("FOOTTER\n")
        fh.write("<<MCA STATUS END>>\n")
        fh.close()

#include "SpectrometerDriver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <vector>
#define DEBUG 1
void initerrorhandling(){
  fprintf(stderr,"error\n");
}
void writeheader(){
  printf("header\n");
}
void writefooter(){
  printf("footer\n");
}


int main(int argc, char *argv[])
{
  char nameBuffer[200];
  int nameLength = 200;
  char SNBuffer[200];
  int SNLength = 200;
  const int MAX_NUM_OF_DETECTORS=8;
  int num_of_detectors;
  int det_index=0;
  int SN;
  int status=0;
  int livetime,realtime;//in sec
  double livetime_taken=0;
  double realtime_taken=0;//in sec
  int livetime_ms_taken,realtime_ms_taken;//in m sec
  unsigned int totalCounts;
  unsigned int detectorID[MAX_NUM_OF_DETECTORS];
  unsigned int spectrumData[TOTAL_RESULT_CHANNELS];
  int *initerrordata;
  fprintf (stderr,"***DAQ for kromek MCA***\n");

  //read command line parameters
  if(argc<2){
    printf("usage: K102 livetime\n");
    exit(1);
      }
  else{
    livetime=realtime=atoi(argv[1]);
    fprintf (stderr," DAQ time to run: %d sec.",livetime);
  }

  kr_Initialise(NULL, NULL);
  //device scan
  fprintf (stderr," scanning the devices...\n");
  detectorID[det_index] = kr_GetNextDetector(0);//to get the first ID
  while(detectorID[det_index]){    
    detectorID[det_index+1] = kr_GetNextDetector(detectorID[det_index]);
    det_index++;
  }
  num_of_detectors=det_index;
  //fprintf(stderr,," number of detectors: %d\n",num_of_detectors);
  
  //kr_GetVersionInformation(&pProduct,&pMajor,&pMinor,&pBuild);
  //not reading from the device
  //printf (" Versions: Product %d, Major %d, Minor %d, Build %d\n", pProduct,pMajor,pMinor,pBuild);
  
  for(det_index=0;det_index<num_of_detectors;det_index++){
    kr_GetDeviceName(detectorID[det_index], nameBuffer, nameLength, &nameLength);
    kr_GetDeviceSerial(detectorID[det_index], SNBuffer, SNLength, &SNLength);
    printf (" Detector properties (index=%d): ID %u, Name %s, SN %s %d\n", det_index,detectorID[det_index], nameBuffer,SNBuffer,SNLength);
  }
  
  det_index=0;
  if(kr_IsAcquiringData(detectorID[det_index])){
    kr_StopDataAcquisition(detectorID[det_index]);
  }
  kr_ClearAcquiredData(detectorID[det_index]);
  //fprintf(stderr," Data cleared.\n");
  status=kr_IsAcquiringData(detectorID[det_index]);
  //fprintf(stderr," status: %d\n",status);

  kr_SendInt16ConfigurationCommand(detectorID[det_index], HIDREPORTNUMBER_SETLLD, 32);


  kr_BeginDataAcquisition(detectorID[det_index], 0, 0);
  fprintf(stderr,"DAQ started...\n");
  while(1){

    kr_GetAcquiredData(detectorID[det_index], spectrumData, &totalCounts, &realtime_ms_taken, &livetime_ms_taken);
    livetime_taken=livetime_ms_taken/1000.;
    realtime_taken=realtime_ms_taken/1000.;
    printf("todal %d counts in %.1lf sec (live) and %.1lf sec (real)\n\t",totalCounts,livetime_taken,realtime_taken);
    for(int ch=0;ch<TOTAL_RESULT_CHANNELS;ch++){
      if(spectrumData[ch])
	printf("(%d,%u) ",ch,spectrumData[ch]);
    }
    printf("\n");
    sleep(1);
    if(livetime_taken>livetime) break;
  }
  printf("DAQ stopped...\n");
  return 0;
}

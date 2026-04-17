#include "stdafx.h"
#include "Detector.h"
#include "DriverMgr.h"
#include "assert.h"
#include "CriticalSection.h"
#include "Lock.h"
#include "kmkTime.h"
#include <memory.h>
#include "stdio.h"
#include <fstream>
#include <iostream>

using namespace std;

const int c_reportSize = 63;

Detector::Detector(kmk::IDevice *pDevice, DataReceivedCallbackFunc dataReceivedCallback, void *pCallbackArg, const kmk::DetectorProperties &detectorProperties)
: m_acquiringData(false)
, m_targetRealTime(0)
, m_targetLiveTime(0)
, m_accumilatedRealTime(0)
, m_acquiringDataStartTime(0)
, m_fileName()
, m_calibA0(0)
, m_calibA1(0)
, m_calibA2(0)
, m_totalCounts(0)
, m_detectorProperties(detectorProperties)
, m_dataReceivedCallbackFunc(dataReceivedCallback)
, m_dataReceivedCallbackArg(pCallbackArg)
{
	m_pDevice = pDevice;

	// Set a callback raised everytime data is received and processed
	m_pDevice->SetCountEventCallback(OnDataRecievedProc, this);

	// Allocate space for a full Spectrum counts array
	m_pData.resize(TOTAL_RESULT_CHANNELS);
}



Detector::Detector(const Detector &)
{
	assert("Invalid copy constructor called!");
}

Detector::~Detector()
{
	EndDataAcquisition();
}

const std::string Detector::convert_mstime_ts(time_t ts) 
{
  uint64_t msecRemainder = ts % 1000;
  time_t sec_time = ts/ 1000.0;
  tm *timeInfo= localtime(&sec_time);
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(4) << timeInfo->tm_year+1900 << "_";
  ss << std::setfill('0') << std::setw(2) << timeInfo->tm_mon+1     << "_";
  ss << std::setfill('0') << std::setw(2) << timeInfo->tm_mday      << "_";
  ss << std::setfill('0') << std::setw(2) << timeInfo->tm_hour      << "_";
  ss << std::setfill('0') << std::setw(2) << timeInfo->tm_min       << "_";
  ss << std::setfill('0') << std::setw(2) << timeInfo->tm_sec       << "_";
  ss << std::setfill('0') << std::setw(3) << msecRemainder;
  
  //std::cout<<"\n"<<sec_time<<std::endl;
  //std::cout<<"\n"<<ts<<" time:"<<ss.str()<<" "<<msecRemainder<<std::endl;
  return ss.str();
}

const std::string Detector::convert_ts_time(time_t ts) 
{
  uint64_t msecRemainder = ts % 1000;
  tm *timeInfo= localtime(&ts);
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(4) << timeInfo->tm_year+1900 << "_";
  ss << std::setfill('0') << std::setw(2) << timeInfo->tm_mon+1     << "_";
  ss << std::setfill('0') << std::setw(2) << timeInfo->tm_mday      << "_";
  ss << std::setfill('0') << std::setw(2) << timeInfo->tm_hour      << "_";
  ss << std::setfill('0') << std::setw(2) << timeInfo->tm_min       << "_";
  ss << std::setfill('0') << std::setw(2) << timeInfo->tm_sec       << "_";
  ss << std::setfill('0') << std::setw(3) << msecRemainder;
  
  //std::cout<<"\n"<<ts<<" time:"<<ss.str()<<" "<<msecRemainder<<std::endl;

  return ss.str();
}

const std::string Detector::currentDateTime() {
    time_t     now = time(0);
    char       buf[80];
    uint64_t msecRemainder = now % 1000;
    tm *timeInfo= localtime(&now);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << timeInfo->tm_year+1900 << "_";
    //ss << std::setfill('0') << std::setw(4) << timeInfo->tm_year+1601 << "_";
    ss << std::setfill('0') << std::setw(2) << timeInfo->tm_mon+1     << "_";
    ss << std::setfill('0') << std::setw(2) << timeInfo->tm_mday      << "_";
    ss << std::setfill('0') << std::setw(2) << timeInfo->tm_hour      << "_";
    ss << std::setfill('0') << std::setw(2) << timeInfo->tm_min       << "_";
    ss << std::setfill('0') << std::setw(2) << timeInfo->tm_sec       << "_";
    ss << std::setfill('0') << std::setw(3) << msecRemainder;
    //std::cout<<"\n"<<now<<" current time:"<<ss.str()<<" "<<msecRemainder<<std::endl;
    return ss.str();
}


// Calculate the live time using the default deadtime of the detector
double Detector::CalculateLiveTime(double realTimeMs, unsigned int totalCounts) const
{			
	// LiveTimeIncrement = RealTimeIncrement * (1.0  (EVENT_DEADTIME * countrate)) 
	if (realTimeMs == 0.0) 
		return 0.0;

	double countRate = totalCounts / (realTimeMs / 1000.0);
	return realTimeMs * (1.0 - (m_detectorProperties.defaultDeadTime * countRate));
}

// Start acquiring data from the detector with an optional limit to the acquisition time
bool Detector::BeginDataAcquisition(unsigned int realTime, unsigned int liveTime)
{
	if (m_acquiringData)
		return true; // Already acquiring!
	
	m_targetRealTime = realTime;
	m_targetLiveTime = liveTime;
	
    // We are not clearing data from a previous scan but the device will only report
    // real time for the current/previous acquisition. Take a note of the realtime length of the previous scan
    m_accumilatedRealTime += m_pDevice->GetRealTime();
	
	if (!m_pDevice->Start())
		return false;
	
	m_acquiringData = true;
	return true;
}

bool Detector::BeginDataAcquisitionF(unsigned int realTime, unsigned int liveTime,int64_t *timestamp,double A0, double A1, double A2,char fileName[])
{
  bool bStartOut = false;
  //time_t   nowt = time(0);
  int64_t stt=kmk::Time::GetCurrentTime();
  *timestamp=stt;
  //std::cout<<"DetectorDriver: "<<stt<<" "<<convert_mstime_ts(stt)<<std::endl;

 	if (m_acquiringData)
		return true; // Already acquiring!
	else
	  {
	    bStartOut=true;
	  }
	
	m_targetRealTime = realTime;
	m_targetLiveTime = liveTime;
	m_acquiringDataStartTime=stt;
	m_fileName = fileName;
	m_calibA0=A0;
	m_calibA1=A1;
	m_calibA2=A2;
	fpo = fopen(m_fileName.c_str(),"w");//Output data with timestamps
	std::stringstream ss;
	ss <<stt<<" "<<convert_mstime_ts(stt);
	fprintf(fpo,"%s %8.6f %8.6f  %10.9f\n", ss.str().c_str(),A0,A1,A2);
    // We are not clearing data from a previous scan but the device will only report
    // real time for the current/previous acquisition. Take a note of the realtime length of the previous scan
    m_accumilatedRealTime += m_pDevice->GetRealTime();
	
	if (!m_pDevice->Start())
		return false;
	
	m_acquiringData = true;
	return true;
}
// Stop acquiring data from the detector
void Detector::EndDataAcquisition()
{
	m_pDevice->Stop(false);	
	m_acquiringData = false;
	fclose(fpo);
}

// Callback routine called as data comes in
void Detector::OnDataRecievedProc(kmk::IDevice * /*pDetector*/, int64_t timestamp, int channel, int numCounts, void *pArg)
{
	Detector *pThis = (Detector*)pArg;

	kmk::Lock lock(pThis->m_dataCS);

	// Add the the spectrum array and counts
	pThis->m_pData[channel] += numCounts;
	pThis->m_totalCounts += numCounts;
	// Pass on the callback
	if (pThis->m_dataReceivedCallbackFunc != NULL)
	{
		pThis->m_dataReceivedCallbackFunc(pThis, timestamp, channel, numCounts, pThis->m_dataReceivedCallbackArg);
		int64_t tstart_msec = pThis->m_acquiringDataStartTime;
		std::string tstart_ts= pThis->convert_mstime_ts(tstart_msec);
		int64_t rT=pThis->m_pDevice->GetRealTime() + pThis->m_accumilatedRealTime;//real time
		unsigned int lT = (unsigned int)pThis->CalculateLiveTime((double)rT, pThis->m_totalCounts);//live time
		int64_t tcurrent_msec=(tstart_msec + rT);
		double energy = (float_t)channel*(float_t)channel*pThis->m_calibA2 + (float_t)channel*pThis->m_calibA1  + pThis->m_calibA0;
		std::string tcurrent_ts =pThis->convert_mstime_ts(tcurrent_msec);
		std::stringstream ss;
		ss <<tcurrent_ts<<" "<<tcurrent_msec<<" "<<rT<<" "<<lT<<" "<<channel<<" "<<energy<<" ";
		fprintf(pThis->fpo,"%s \n", ss.str().c_str());
	}
}

void Detector::ClearAcquiredData()
{
    kmk::Lock lock (m_dataCS);

    m_pDevice->ResetRealTime();
    m_accumilatedRealTime = 0;
    m_totalCounts = 0;
    std::fill(m_pData.begin(), m_pData.end(), 0);
}

// Return the acquired data including a real time. This is either the latest data of an active acquisition or the last completed
// acquisition of the acquisition has already ended
bool Detector::GetAcquiredData(unsigned int *pBuffer, unsigned int *pTotalCounts, unsigned int *pRealTime, unsigned int *pLiveTime, unsigned int flags)
{
	kmk::Lock lock (m_dataCS);
	
    int64_t realTime = m_accumilatedRealTime + m_pDevice->GetRealTime();

	if (pBuffer)
	{
		memcpy(pBuffer, &m_pData[0], sizeof(unsigned int) * TOTAL_RESULT_CHANNELS);
    }

	if (pTotalCounts)
		*pTotalCounts = m_totalCounts;
		
	if (pRealTime)
		*pRealTime = (unsigned int)realTime;
	
	if (pLiveTime)
		*pLiveTime = (unsigned int)CalculateLiveTime((double)realTime, m_totalCounts);

	if (flags & GAD_CLEAR_COUNTS)
	{
       ClearAcquiredData();
	}

	return true;
}

// Send the LLD value to the detector
bool Detector::SendLLDConfigurationCommand(int channelLLD)
{	
	// When setting the LLD there is the new channel lld report type and the older deprecated lld report type.
	// Try setting via channel lld and fall back to old lld if that fails
	if (m_pDevice->SetConfigurationSettingUInt16(kmk::CONFIGURATION_SETLLD_CHANNEL, channelLLD))
	{
		return true;
	}

	return m_pDevice->SetConfigurationSettingUInt16(kmk::CONFIGURATION_SETLLD, channelLLD << 4);
}

// Send a configuration command to the detector
bool Detector::SendInt16ConfigurationCommand(kmk::ConfigurationID configurationID, unsigned short command)
{
	bool validType = false;
	
	switch (configurationID)
	{
	// Valid 16 bit values
	case kmk::CONFIGURATION_SETLLD:
	case kmk::CONFIGURATION_SETBIAS2:
		validType = true;
		break;

	// Non 16 but values
	case kmk::CONFIGURATION_SETGAIN:
	case kmk::CONFIGURATION_SETPOLARITY:
	default:
		validType = false;
		break;
	}
	
	if (!validType)
		return false;

	// LLD is special
	if (HIDREPORTNUMBER_SETLLD)
	{
		return SendLLDConfigurationCommand(command);
	}

	// Send value
	return m_pDevice->SetConfigurationSettingUInt16(configurationID, command);
}

// Send a configuration command to the detector
bool Detector::SendInt8ConfigurationCommand(kmk::ConfigurationID configurationID, unsigned char command)
{
	bool validType = false;
	
	switch(configurationID)
	{
		// Valid 8 bit values
	case kmk::CONFIGURATION_SETGAIN:
	case kmk::CONFIGURATION_SETPOLARITY:
		validType = true;
		break;

		// Non 8 bit values
	case kmk::CONFIGURATION_SETLLD:
	case kmk::CONFIGURATION_SETBIAS2:
	default:
		validType = false;
		break;
	}
	
	if (!validType)
		return false;

	return m_pDevice->SetConfigurationSettingUInt8(configurationID, command);	
}

// Called regularly to update the detectors state. Check if the target realtime / live time has been reached and stop acquisition if
// necessary
void Detector::Update()
{
	if (m_acquiringData)
	{
		if (m_targetRealTime > 0)
		{
            int64_t realTime = m_pDevice->GetRealTime() + m_accumilatedRealTime;
			if (realTime >= m_targetRealTime)
			{
				EndDataAcquisition();
			}
		}

		if (m_targetLiveTime > 0 && m_acquiringData)
		{
            int64_t realTime = m_pDevice->GetRealTime() + m_accumilatedRealTime;
            unsigned int liveTime = (unsigned int)CalculateLiveTime((double)realTime, m_totalCounts);
			if (liveTime >= m_targetLiveTime)
			{
				EndDataAcquisition();
			}
		}
	}
}

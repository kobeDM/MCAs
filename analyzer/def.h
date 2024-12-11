#ifndef MCA_DEF_H
#define MCA_DEF_H

// include ShUtil
#include "inc/shinclude.h"

struct GainCurveParams {
    String m_gasMixture;
    double m_gainCurveConst;
    double m_gainCurveSlope;
};

struct CalParams {
    double m_gasGain;
    double m_gasGainErr;
    double m_eneReso;
    double m_eneResoErr;
};

struct RunParams {
    String m_dirPath;
    double m_anodeVoltage;
    String m_gasMixture;
    int    m_totEvt;
    double m_livetime;
};

#endif // MCA_DEF_H

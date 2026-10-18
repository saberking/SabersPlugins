#ifndef PARAMETERS_HPP
#define PARAMETERS_HPP
#include "src/DistrhoDefines.h"
#include "Defines.hpp"

START_NAMESPACE_DISTRHO
enum Parameters {
#define Y_REAL(i,j) kParamCrossLvl##i##j,kParamCrossAngle##i##j,kParamCrossPan##i##j,kParamCrossAngleOffset##i##j,
#define X(i) kParamInLvl##i, kParamInAngle##i, kParamInPan##i, kParamInAngleOffset##i,\
    kParamFeed##i, kParamFreq##i,\
    BIQUAD_INNER_LIST(i)\
    kParamOutPan##i, kParamOutAngleOffset##i, \
    kParamOutLvl##i, kParamOutAngle##i,
    BIQUAD_LIST
#undef X
#undef Y_REAL
    kParamCount
};

#define OUTER_PARAM_DECL(i)\
float fInLvl##i=0.0f;\
    float fInAngle##i=0.0f;\
    float fInPan##i=0.0f;\
    float fInAngleOffset##i=0.f;\
    float fFeed##i=0.1f;\
    float fFreq##i=0.f;\
    BIQUAD_INNER_LIST(i)\
    float fOutPan##i=0.f;\
    float fOutAngleOffset##i=0.f;\
    float fOutLvl##i=1.f/25.f;\
    float fOutAngle##i=0.f;

#define CROSS_PARAM_DECL(i,j)\
float fCrossLvl##i##j=0.f, fCrossPan##i##j=0.f,fCrossAngle##i##j=0.f,fCrossAngleOffset##i##j=0.f;

#define SET_CROSS_PARAM(i,j)\
if(index == kParamCrossLvl##i##j) { \
        fCrossLvl##i##j=value; \
} \
    if(index==kParamCrossAngle##i##j){\
        fCrossAngle##i##j=value;\
}\
    if(index == kParamCrossPan##i##j) { \
        fCrossPan##i##j=value; \
} \
    if(index==kParamCrossAngleOffset##i##j){\
        fCrossAngleOffset##i##j=value;\
}
#define SET_PARAM(i) \
if(index==kParamInLvl##i){\
        fInLvl##i=value;\
}\
    if(index==kParamInAngle##i){\
        fInAngle##i=value;\
}\
    if(index==kParamInPan##i){\
        fInPan##i=value;\
}\
    if(index==kParamInAngleOffset##i){\
        fInAngleOffset##i=value;\
}\
    if(index==kParamFeed##i){\
        fFeed##i=value;\
}\
    if(index==kParamFreq##i){\
        fFreq##i=value;\
}\
    BIQUAD_INNER_LIST(i)\
    if(index==kParamOutPan##i){\
        fOutPan##i=value;\
}\
    if(index==kParamOutAngleOffset##i){\
        fOutAngleOffset##i=value;\
}\
    if(index==kParamOutLvl##i){\
        fOutLvl##i=value;\
}\
    if(index==kParamOutAngle##i){\
        fOutAngle##i=value;\
}


END_NAMESPACE_DISTRHO
#endif // PARAMETERS_HPP

#ifndef PARAMETERS_HPP
#define PARAMETERS_HPP
#include "src/DistrhoDefines.h"
#include "Defines.hpp"

START_NAMESPACE_DISTRHO
enum Parameters {
#define Y_REAL(i,j) kParamCrossLvl##i##j,kParamCrossPhs##i##j,kParamCrossPan##i##j,kParamCrossPOff##i##j,
#define X(i) kParamInLvl##i, kParamInPhs##i, kParamInPan##i, kParamInPOff##i,\
    kParamPoleCentre##i, kParamDiscriminant##i,\
    BIQUAD_INNER_LIST(i)\
    kParamOutPan##i, kParamOutPOff##i, \
    kParamOutLvl##i, kParamOutPhs##i,
    BIQUAD_LIST
#undef X
#undef Y_REAL
    kParamCount
};

#define OUTER_PARAM_DECL(i)\
float fInLvl##i=0.0f;\
    float fInPhs##i=0.0f;\
    float fInPan##i=0.0f;\
    float fInPOff##i=0.f;\
    float fPoleCentre##i=0.f;\
    float fDiscriminant##i=0.f;\
    BIQUAD_INNER_LIST(i)\
    float fOutPan##i=0.f;\
    float fOutPOff##i=0.f;\
    float fOutLvl##i=1.f/25.f;\
    float fOutPhs##i=0.f;

#define CROSS_PARAM_DECL(i,j)\
float fCrossLvl##i##j=0.f, fCrossPan##i##j=0.f,fCrossPhs##i##j=0.f,fCrossPOff##i##j=0.f;

#define SET_CROSS_PARAM(i,j)\
if(index == kParamCrossLvl##i##j) { \
        fCrossLvl##i##j=value; \
} \
    if(index==kParamCrossPhs##i##j){\
        fCrossPhs##i##j=value;\
}\
    if(index == kParamCrossPan##i##j) { \
        fCrossPan##i##j=value; \
} \
    if(index==kParamCrossPOff##i##j){\
        fCrossPOff##i##j=value;\
}
#define SET_PARAM(i) \
if(index==kParamInLvl##i){\
        fInLvl##i=value;\
}\
    if(index==kParamInPhs##i){\
        fInPhs##i=value;\
}\
    if(index==kParamInPan##i){\
        fInPan##i=value;\
}\
    if(index==kParamInPOff##i){\
        fInPOff##i=value;\
}\
    if(index==kParamPoleCentre##i){\
        fPoleCentre##i=value;\
}\
    if(index==kParamDiscriminant##i){\
        fDiscriminant##i=value;\
}\
    BIQUAD_INNER_LIST(i)\
    if(index==kParamOutPan##i){\
        fOutPan##i=value;\
}\
    if(index==kParamOutPOff##i){\
        fOutPOff##i=value;\
}\
    if(index==kParamOutLvl##i){\
        fOutLvl##i=value;\
}\
    if(index==kParamOutPhs##i){\
        fOutPhs##i=value;\
}


END_NAMESPACE_DISTRHO
#endif // PARAMETERS_HPP

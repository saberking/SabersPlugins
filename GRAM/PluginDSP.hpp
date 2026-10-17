#ifndef PLUGINDSP_HPP
#define PLUGINDSP_HPP

#include "DistrhoPlugin.hpp"
#include "Parameters.hpp"
#include "WinConsoleOutput.hpp"
#include "external/base64.h"
#include "external/Eigen/Dense"
#include <atomic>
#include <ctime>
#include "external/Eigen/SVD" // Make sure to include the SVD header at the top of your file
#include <random> // <-- ADD THIS LINE HERE
#include <chrono>

START_NAMESPACE_DISTRHO
    enum ActivationFunctionType{
    activationFunctionClip,
    activationFunctionTanh,
    activationFunctionNone,
    activationFunctionCount
};
static const char* activationFunctionNames[]={
        "Clip",
        "Tanh",
        "None"
    };
#define MAX_DELAY 10000

class ImGuiPluginDSP : public Plugin
{
#define Y_REAL(i,j) CROSS_PARAM_DECL(i,j)

#define X(i) OUTER_PARAM_DECL(i)

    BIQUAD_LIST
#undef X
    bool consoleAttached=false;
    std::mt19937 gen;
    std::normal_distribution<float> d;
    float note=1.f,lastNote=1.f;
public:

    std::atomic<ActivationFunctionType> activation=activationFunctionClip;

    Eigen::Array<float,BIQUAD_COUNT*2,1>state[MAX_DELAY+1];
    Eigen::Array<float,BIQUAD_COUNT*2,1>    inL, inR, outL, outR;
    Eigen::Array<float,BIQUAD_COUNT*2,BIQUAD_COUNT*2>weights;

    int inputBufferIndex=0;
    ImGuiPluginDSP()
        : Plugin(kParamCount, 0, 1) // parameters, programs, states
    {
        if (DEBUG&&!GetConsoleWindow()) {
            initConsoleOutput();
            consoleAttached=true;
        }

        // for(int i=0;i<MAX_UNDO_DEPTH;i++)
        // {
        //     undoItems[i]=NULL;
        // }


        std::srand(std::time(nullptr));
        std::random_device rd;
        gen.seed(rd()); // Seed this specific instance with a hardware random
        std::cout<<"finished constructor"<<std::endl;
        calculateMatrix();
    }






    // void printEigen(float *outPointer)
    // {
    //     Eigen::Map<Eigen::Matrix<float, OUT_SIZE, OUT_SIZE, Eigen::RowMajor>> W(outPointer);

    //     Eigen::EigenSolver<Eigen::Matrix<float, OUT_SIZE-2, OUT_SIZE-2>> solver(W.block<OUT_SIZE-2, OUT_SIZE-2>(2, 2), false);
    //     float maxMagnitude = 0.0f;
    //     for (int i = 0; i < OUT_SIZE-2; ++i) {
    //         maxMagnitude = std::max(maxMagnitude, std::abs(solver.eigenvalues()[i]));
    //     }
    //     std::cout<<"maxMAgnitude "<<maxMagnitude<<std::endl;
    //     //float mult=1/findMaxAmplification(outPointer);

    // }




    ~ImGuiPluginDSP(){
        // for(int i=0;i<MAX_UNDO_DEPTH;i++)
        // {
        //     if(undoItems[i])delete undoItems[i];
        // }
    }

protected:




    float clip(float a, float abs=1.f)
    {
        return std::max(-1.f*abs,std::min(1.f*abs,a));
    }
    inline float fastTanh(float x) {
        // Clamp input to prevent polynomial divergence at high values
        float x_clamped = std::max(-4.5f, std::min(4.5f, x));
        float x2 = x_clamped * x_clamped;

        // Pade approximation: highly accurate, no division loops, zero hardware stalls
        return x_clamped * (135135.0f + x2 * (17325.0f + x2 * (378.0f + x2))) /
               (135135.0f + x2 * (62370.0f + x2 * (3150.0f + x2 * 28.0f)));
    }
    void noteOn(int midiNote, float velocity){
        lastNote=note;
        note=std::pow(2.f, -(float)midiNote/12.f);
    }

    void noteOff(int midiNote){

    }

    void handleMidi(const MidiEvent *midiEvent){
        int status = midiEvent->data[0]; // midi status
        int midi_message = status & 0xF0;
        int midi_data1 = midiEvent->data[1];
        int midi_data2 = midiEvent->data[2];
        float velocity=(float)midi_data2;
        velocity/=128.f;
        switch ( midi_message )
        {
        case 0x80: // note_off
            noteOff(midi_data1);
            break;
        case 0x90: // note_on
            noteOn(midi_data1, velocity);
            break;
        }
    }
    void calculateMatrix()
    {
        std::cout<<"calculate"<<std::endl;

        float panAngle;
        Eigen::Array<float,2,2> inMatrix, outMatrix;
        Eigen::Rotation2Df rot2d;
        weights.matrix().setZero();
//pan: zero is 45 degrees, -M_PI/2 is zero degrees, M_PI/2 is 90 degrees
//left magnitude is cos(pan), right is sin(pan)


#define X(i) \
        \
        panAngle=M_PI/4.f+fInPan##i/2.f;\
        if(panAngle>M_PI/2||panAngle<0)std::cout<<"ERRORERRORREREOEREOR  "<<panAngle<<std::endl;\
        inMatrix.matrix().setIdentity();\
        inMatrix*=fInLvl##i*sqrt(2);\
        inMatrix.col(0)*=cos(panAngle);\
        rot2d=Eigen::Rotation2D(fInAngle##i-fInAngleOffset##i);\
        inMatrix.col(0)=rot2d.toRotationMatrix()*inMatrix.col(1).matrix();\
        inMatrix.col(1)*=sin(panAngle);\
        rot2d=Eigen::Rotation2D(fInAngle##i+fInAngleOffset##i);\
        inMatrix.col(0)=rot2d.toRotationMatrix()*inMatrix.col(0).matrix();\
        inL(2*i)=inMatrix(0,0);\
        inL(2*i+1)=inMatrix(1,0);\
        inR(2*i)=inMatrix(0,1);\
        inR(2*i+1)=inMatrix(1,1);\
        weights(2*i, 2*i)=2*fPoleCentre##i;\
        weights(2*i,2*i+1)=-(fPoleCentre##i*fPoleCentre##i-fDiscriminant##i*fDiscriminant##i);\
        weights(2*i+1,2*i)=1;\
        panAngle=M_PI/4.f+fOutPan##i/2.f;\
        if(panAngle>M_PI/2||panAngle<0)std::cout<<"ERRORERRORREREOEREOR  "<<panAngle<<std::endl;\
        outMatrix.matrix().setIdentity();\
        outMatrix*=fOutLvl##i*sqrt(2);\
        outMatrix.col(0)*=cos(panAngle);\
        rot2d=Eigen::Rotation2D(fOutAngle##i-fOutAngleOffset##i);\
        outMatrix.col(0)=rot2d.toRotationMatrix()*outMatrix.col(1).matrix();\
        outMatrix.col(1)*=sin(panAngle);\
        rot2d=Eigen::Rotation2D(fOutAngle##i+fOutAngleOffset##i);\
        outMatrix.col(0)=rot2d.toRotationMatrix()*outMatrix.col(0).matrix();\
        outL(2*i)=inMatrix(0,0);\
            outL(2*i+1)=inMatrix(1,0);\
            outR(2*i)=inMatrix(0,1);\
            outR(2*i+1)=inMatrix(1,1);\


        BIQUAD_LIST
#undef X
#undef Y_REAL

        std::cout<<"Finsihed calcultee"<<std::endl;
    }


    std::chrono::steady_clock::time_point start=std::chrono::steady_clock::now();
    float time1=0, time2=0, time3=0, time4=0, time5=0, time6=0, time7=0, time8=0;
    int framecounter=0;

    void printTime()
    {
        std::cout<<"Time"<<std::endl;
        std::cout<<time1<<"    "<<time2<<"   "<<time3<<"    "<<time4<<"   "<<time5<<"    "<<time6<<"   "<<time7<<"    "<<time8<<std::endl;
        time1=time2=time3=time4=time5=time6=time7=time8=0.f;
    }
    void startTimer(){
        start= std::chrono::steady_clock::now();
    }
    float getTimeInterval(){
        std::chrono::steady_clock::time_point end;

        end= std::chrono::steady_clock::now();
        // 1. Get seconds as a float (e.g., 0.01234 seconds)
        float seconds = std::chrono::duration<float>(end - start).count();
        float milliseconds = std::chrono::duration<float, std::milli>(end - start).count();
        startTimer();
        return milliseconds;
    }



    void run ( const float **inputs, float **outputs, uint32_t frames,
             const MidiEvent *midiEvents, // MIDI pointer
             uint32_t midiEventCount      // Number of MIDI events in block
             ) override
    {
        startTimer();

        int curEventIndex =0;

        ActivationFunctionType activationFunction=activation.load(std::memory_order_release);




        time1+=getTimeInterval();

        for (uint32_t sample = 0; sample < frames; ++sample) {
            while ( curEventIndex < midiEventCount && sample == midiEvents[curEventIndex].frame )
            {
                handleMidi(&(midiEvents[curEventIndex++]));

            }


            int newIndex=(inputBufferIndex+1)%(MAX_DELAY+1);
            state[newIndex]=state[inputBufferIndex];
            inputBufferIndex=newIndex;
            time2+=getTimeInterval();
            state[inputBufferIndex]+=inL*inputs[0][sample]+inR*inputs[1][sample];
            time3+=getTimeInterval();
            state[inputBufferIndex].matrix()=weights.matrix()*state[inputBufferIndex].matrix();
            time4+=getTimeInterval();
            if(activationFunction==activationFunctionClip)
            {
                state[inputBufferIndex] = state[inputBufferIndex].cwiseMax(-1.0f).cwiseMin(1.0f);
            }

            if(activationFunction==activationFunctionTanh)
            {
                for(int i=0;i<state[inputBufferIndex].size();i++)
                {
                    state[inputBufferIndex][i]=fastTanh(state[inputBufferIndex][i]);
                }

            }
            time5+=getTimeInterval();

            outputs[0][sample] = clip(2.f*std::real(outL.matrix().dot(state[inputBufferIndex].matrix())));
            outputs[1][sample] = clip(2.f*std::real(outR.matrix().dot(state[inputBufferIndex].matrix())));

            time6+=getTimeInterval();


            if(!(framecounter++%48000)) printTime();
        }


    }

    // ----------------------------------------------------------------------------------------------------------------
    // Information
    void initParameter(uint32_t index, Parameter& parameter) override
    {
        parameter.hints = kParameterIsAutomatable;
#define Y_REAL(i,j) \
        if(index == kParamCrossLvl##i##j) { \
                parameter.ranges = ParameterRanges(0.f, -1.f, 1.f); \
                parameter.name = "Biquad " #i  " CrossLvl" #j; \
                parameter.symbol = "biquad_" #i"_Cross_lvl"  #j ; \
                return; \
        } \
            if(index == kParamCrossAngle##i##j) { \
                parameter.ranges = ParameterRanges(0.f, -M_PI, M_PI); \
                parameter.name = "Biquad " #i " CrossAngle" #j; \
                parameter.symbol = "biquad_" #i "_Cross_phs" #j; \
                return; \
        } \
            if(index == kParamCrossPan##i##j) { \
                parameter.ranges = ParameterRanges(0.f, -M_PI/2, M_PI/2); \
                parameter.name = "Biquad " #i " CrossPan"  #j; \
                parameter.symbol = "biquad_" #i "_Cross_pan"  #j; \
                return; \
        } \
            if(index == kParamCrossAngleOffset##i##j) { \
                parameter.ranges = ParameterRanges(0.f, -M_PI, M_PI); \
                parameter.name = "Biquad " #i " CrossAngleOffset"  #j; \
                parameter.symbol = "biquad_" #i "_Cross_angle_off"  #j; \
                return; \
        }

#define X(i) \
        if(index == kParamInLvl##i) { \
                parameter.ranges = ParameterRanges(0.f, -1.f, 1.f); \
                parameter.name = "Biquad " #i " InLvl"; \
                parameter.symbol = "biquad_" #i "_in_lvl"; \
                return; \
        } \
            if(index == kParamInAngle##i) { \
                parameter.ranges = ParameterRanges(0.f, -M_PI, M_PI); \
                parameter.name = "Biquad " #i " InAngle"; \
                parameter.symbol = "biquad_" #i "_in_phs"; \
                return; \
        } \
        if(index == kParamInPan##i) { \
                parameter.ranges = ParameterRanges(0.f, -M_PI/2, M_PI/2); \
                parameter.name = "Biquad " #i " InPan"; \
                parameter.symbol = "biquad_" #i "_in_pan"; \
                return; \
        } \
            if(index == kParamInAngleOffset##i) { \
                parameter.ranges = ParameterRanges(0.f, -M_PI, M_PI); \
                parameter.name = "Biquad " #i " InAngleOffset"; \
                parameter.symbol = "biquad_" #i "_in_angle_off"; \
                return; \
        } \
            if(index == kParamPoleCentre##i) { \
                parameter.ranges = ParameterRanges(0.f, -1.f, 1.f); \
                parameter.name = "Biquad " #i " PoleCentre"; \
                parameter.symbol = "biquad_" #i "_pole_centre"; \
                return; \
        } \
            if(index == kParamDiscriminant##i) { \
                parameter.ranges = ParameterRanges(0.f, -24000.f, 24000.f); \
                parameter.name = "Biquad " #i " Discriminant"; \
                parameter.symbol = "biquad_" #i "_discriminant"; \
                return; \
        }\
        BIQUAD_INNER_LIST(i)\
        if(index == kParamOutPan##i) { \
        parameter.ranges = ParameterRanges(0.f, -M_PI/2, M_PI/2); \
                parameter.name = "Biquad " #i " OutPan"; \
                parameter.symbol = "biquad_" #i "_out_pan"; \
                return; \
        } \
            if(index == kParamOutAngleOffset##i) { \
        parameter.ranges = ParameterRanges(0.f, -M_PI, M_PI); \
                parameter.name = "Biquad " #i " OutAngleOffset"; \
                parameter.symbol = "biquad_" #i "_out_angle_off"; \
                return; \
        } \
            if(index == kParamOutLvl##i) { \
                parameter.ranges = ParameterRanges(1.f/25.f, -1.f, 1.f); \
                parameter.name = "Biquad " #i " OutLvl"; \
                parameter.symbol = "biquad_" #i "_out_lvl"; \
                return; \
        } \
            if(index == kParamOutAngle##i) { \
                parameter.ranges = ParameterRanges(0.f, -M_PI, M_PI); \
                parameter.name = "Biquad " #i " OutAngle"; \
                parameter.symbol = "biquad_" #i "_out_phs"; \
                return; \
        }
        BIQUAD_LIST
#undef X
#undef Y_REAL
    }

    float getParameterValue(uint32_t index) const override
    {
#define Y_REAL(i,j)\
        if(index == kParamCrossLvl##i##j) { \
                return fCrossLvl##i##j; \
        } \
            if(index==kParamCrossAngle##i##j){\
                return fCrossAngle##i##j;\
        }\
            if(index == kParamCrossPan##i##j) { \
                return fCrossPan##i##j; \
        } \
            if(index==kParamCrossAngleOffset##i##j){\
                return fCrossAngleOffset##i##j;\
        }


#define X(i) \
        if(index == kParamInLvl##i) { \
                return fInLvl##i; \
        } \
            if(index==kParamInAngle##i){\
                return fInAngle##i;\
        }\
        if(index == kParamInPan##i) { \
                return fInPan##i; \
        } \
        if(index==kParamInAngleOffset##i){\
            return fInAngleOffset##i;\
        }\
        if(index==kParamPoleCentre##i){\
            return fPoleCentre##i;\
        }\
        if(index==kParamDiscriminant##i){\
            return fDiscriminant##i;\
        }\
        BIQUAD_INNER_LIST(i)\
        if(index == kParamOutPan##i) { \
                return fOutPan##i; \
        } \
            if(index==kParamOutAngleOffset##i){\
                return fOutAngleOffset##i;\
        }\
            if(index==kParamOutLvl##i){\
                return fOutLvl##i;\
        }\
            if(index==kParamOutAngle##i){\
                return fOutAngle##i;\
        }
        BIQUAD_LIST
#undef X
#undef Y_REAL
        std::cout<<"ERRORRRRRR   "<<index<<std::endl;
    }


    void setParameterValue(uint32_t index, float value) override
    {
#define Y_REAL(i,j) SET_CROSS_PARAM(i,j)
#define X(i) SET_PARAM(i)
BIQUAD_LIST
#undef X
#undef Y_REAL

        std::cout<<"all set"<<std::endl;
        calculateMatrix();
    }

    void activate() override
    {
    }

    void initState(uint32_t index, String& key, String& defaultValue) override
    {
        if (index == 0) {
            key = "sampleData";
            defaultValue = "";
        }
    }

    String getState(const char* key) const override {
        // if (!strcmp(key,"sampleData"))
        // {

        //     size_t maxEigenSize=sizeof(float);
        //     size_t activationSize=sizeof(ActivationFunctionType);
        //     size_t matrixSize=sizeof(float)*(OUT_SIZE)*OUT_SIZE;

        //     size_t totalBytes = maxEigenSize+activationSize+matrixSize;

        //     std::vector<uint8_t> rawBinaryBuffer(totalBytes);
        //     auto *incrementalPointer=rawBinaryBuffer.data();

        //     auto *maxEigenPointer=reinterpret_cast<float*>(incrementalPointer);
        //     *maxEigenPointer=max_eigenvalue;
        //     incrementalPointer+=maxEigenSize;

        //     auto* activationPointer=reinterpret_cast<ActivationFunctionType*>(incrementalPointer);
        //     *activationPointer=activation.load(std::memory_order_relaxed);
        //     incrementalPointer+=activationSize;

        //     auto *matrixPtr=reinterpret_cast<float*>(incrementalPointer);
        //     auto *buffer=currentBufferPointer.load(std::memory_order_relaxed);
        //     for(int i=0;i<OUT_SIZE*(OUT_SIZE);i++)
        //     {
        //         matrixPtr[i]=buffer[i];
        //     }

        //     std::string encodedText = base64_encode(rawBinaryBuffer.data(), rawBinaryBuffer.size());
        //     return String(encodedText.c_str());
        // }else
            return String("");

    }

    void setState(const char *key, const char * value){
        // if(!strcmp("sampleData", key))
        // {
        //     if (strlen(value) == 0 ) {
        //         return;
        //     }


        //     std::string decodedBytes = base64_decode(std::string(value));

        //     const uint8_t* incrementalPointer = reinterpret_cast<const uint8_t*>(decodedBytes.data());

        //     size_t maxEigenSize=sizeof(float);
        //     size_t activationSize=sizeof(ActivationFunctionType);
        //     size_t matrixSize=sizeof(float)*(OUT_SIZE)*OUT_SIZE;


        //     auto *maxEigenPointer=reinterpret_cast<const float*>(incrementalPointer);
        //     max_eigenvalue=*maxEigenPointer;
        //     incrementalPointer+=maxEigenSize;

        //     auto* activationPointer=reinterpret_cast<const ActivationFunctionType*>(incrementalPointer);
        //     activation.store(*activationPointer,std::memory_order_relaxed);
        //     incrementalPointer+=activationSize;

        //     auto *matrixPtr=reinterpret_cast<const float*>(incrementalPointer);
        //     updateReady.store(false,std::memory_order_release);
        //     auto *buffer=currentBufferPointer.load(std::memory_order_acquire);
        //     float *outPointer=(buffer==weightBuffer1?weightBuffer2:weightBuffer1);
        //     for(int i=0;i<OUT_SIZE*(OUT_SIZE);i++)
        //     {
        //         outPointer[i]=matrixPtr[i];
        //     }
        //     currentBufferPointer.store(outPointer,std::memory_order_release);

        // }
    }
    /**
      Get the plugin label.@n
      This label is a short restricted name consisting of only _, a-z, A-Z and 0-9 characters.
    */
    const char* getLabel() const noexcept override
    {
        return "GRAM";
    }

    /**
      Get an extensive comment/description about the plugin.@n
      Optional, returns nothing by default.
    */
    const char* getDescription() const override
    {
        return "GRAM";
    }

    /**
      Get the plugin author/maker.
    */
    const char* getMaker() const noexcept override
    {
        return "Jean Pierre Cimalando, falkTX, Saber";
    }

    /**
      Get the plugin license (a single line of text or a URL).@n
      For commercial plugins this should return some short copyright information.
    */
    const char* getLicense() const noexcept override
    {
        return "ISC";
    }

    /**
      Get the plugin version, in hexadecimal.
      @see d_version()
    */
    uint32_t getVersion() const noexcept override
    {
        return d_version(1, 0, 0);
    }

    /**
      Get the plugin unique Id.@n
      This value is used by LADSPA, DSSI and VST plugin formats.
      @see d_cconst()
    */
    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('G', 'R', 'A', 'M');
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Init
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImGuiPluginDSP)
};

END_NAMESPACE_DISTRHO

#endif // PLUGINUI_HPP

#ifndef PLUGINDSP_HPP
#define PLUGINDSP_HPP

#include "DistrhoPlugin.hpp"
#include "WinConsoleOutput.hpp"
#include "external/base64.h"
#include "external/Eigen/Dense"
#include "Defines.hpp"
#include <atomic>
#include <ctime>
#include "external/Eigen/SVD" // Make sure to include the SVD header at the top of your file
#include <random> // <-- ADD THIS LINE HERE
#include <chrono>



START_NAMESPACE_DISTRHO

class ImGuiPluginDSP : public Plugin
{
    alignas(64) Eigen::Array<float, 30, 2> inputMatrix, outputMatrix;
    alignas(64) Eigen::Array<float, 30, 30> weightMatrix;
    float fDelay=0.f;
    float fMaxEigenvalue=1.f;
    bool consoleAttached=false;

    float note=1.f,lastNote=1.f;
    bool needsNormalise=false;
public:
    std::atomic<ActivationFunctionType> activation=activationFunctionClip;
    int lastDelay=0;


    alignas(64) float inputBuffer[MAX_DELAY+1][MATRIX_SIZE];
    int inputBufferIndex=0;
    alignas(64) float outputBuffer[MATRIX_SIZE];
    ImGuiPluginDSP()
        : Plugin(PARAM_COUNT, 0, 1) // parameters, programs, states
    {
        if (DEBUG&&!GetConsoleWindow()) {
            initConsoleOutput();
            consoleAttached=true;
        }





        for(int i=0;i<MAX_DELAY+1;i++)
        {
            for(int j=0;j<MATRIX_SIZE;j++)
            {
               inputBuffer[i][j]=0.f;

            }
        }
        for(int i=0;i<MATRIX_SIZE;i++)
        {
            for(int j=0;j<MATRIX_SIZE;j++) weightMatrix(i,j)=0.f;
            inputMatrix(i,0)=0.f;            inputMatrix(i,1)=0.f;
            outputMatrix(i,0)=0.f;            outputMatrix(i,1)=0.f;
        }


    }

    // float findMaxAmplification(float* outPointer)
    // {
    //     Eigen::Map<Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE, Eigen::RowMajor>> W(outPointer);

    //     // Isolate the square audio feedback block (e.g., 24x24)
    //     Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE> audioBlock = W.block<MATRIX_SIZE, MATRIX_SIZE>(0, 0);

    //     // Compute the Singular Value Decomposition (SVD)
    //     // We only need the singular values, so we pass 0 to skip computing U and V matrices (saves CPU)
    //     Eigen::JacobiSVD<Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE>> svd(audioBlock, 0);

    //     // The singular values are always returned sorted from highest to lowest.
    //     // Index 0 is mathematically guaranteed to be the maximum amplification factor!
    //     float maxAmplification = svd.singularValues()[0];

    //     std::cout << "Absolute Highest Single-Hop Amplification Factor: " << maxAmplification << std::endl;

    //     return maxAmplification;
    // }




    void printEigen()
    {
        Eigen::Map<Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE, Eigen::RowMajor>> W(weightMatrix.data());

        Eigen::EigenSolver<Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE>> solver(W, false);
        float maxMagnitude = 0.0f;
        for (int i = 0; i < MATRIX_SIZE; ++i) {
            maxMagnitude = std::max(maxMagnitude, std::abs(solver.eigenvalues()[i]));
        }
        std::cout<<"maxMAgnitude "<<maxMagnitude<<std::endl;
        //float mult=1/findMaxAmplification(outPointer);

    }






    void printMatrix()
    {
        for(int i=0;i<MATRIX_SIZE;i++){
            for(int j=0;j<MATRIX_SIZE;j++)        std::cout<<weightMatrix(i,j)<<" ";
            std::cout<<std::endl;
        }
        printEigen();
    }
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


        int delay=(int)std::max(0.f,std::min(fDelay*note,(float)MAX_DELAY));
        int currentLoopDelay=lastDelay;
        int lastLoopDelay=currentLoopDelay;

        time1+=getTimeInterval();

        Eigen::Map<Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE, Eigen::RowMajor>, Eigen::Aligned64>
            W(weightMatrix.data());

        // Eigen::Map<Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE+CONSTANT_KNOB_COUNT,
        //                          Eigen::RowMajor>> W(weightBufferPointer.load(std::memory_order_relaxed));

        Eigen::Map<Eigen::Vector<float, MATRIX_SIZE>> y(outputBuffer);
        time2+=getTimeInterval();
        for (uint32_t sample = 0; sample < frames; ++sample) {
            while ( curEventIndex < midiEventCount && sample == midiEvents[curEventIndex].frame )
            {
                handleMidi(&(midiEvents[curEventIndex++]));

            }
            if(note!=lastNote)
            {
                delay=(int)std::max(0.f,std::min(fDelay*note,(float)MAX_DELAY));
                lastNote=note;
            }
            int temp=delay-lastDelay;
            int temp2=(temp*((int)sample+1))/(int)frames;

            currentLoopDelay=lastDelay+temp2;
            int newIndex=(inputBufferIndex+lastLoopDelay-currentLoopDelay+MAX_DELAY+1)%(MAX_DELAY+1);
            if(newIndex<0)std::cout<<"ERRORR!!!!!"<<"ERRRORRRR!!!!!!"<<std::endl<<"current "<<
                          currentLoopDelay<<" lastD "<<lastDelay<<"   delay "<<delay<<"    sammple "<<sample<<"    frames"<<frames<<
                    "    temp"<<temp<<"    temp2 "<<temp2<<  std::endl;
            else inputBufferIndex=newIndex;
            lastLoopDelay=currentLoopDelay;

            time3+=getTimeInterval();
            Eigen::Map<Eigen::Vector<float, MATRIX_SIZE>> x(inputBuffer[inputBufferIndex]);

            time4+=getTimeInterval();



            const float* rawW = W.data();
            float* rawX = x.data();
            float*       rawY = y.data();
            const float inL=inputs[0][sample],inR=inputs[1][sample];

            rawX[0] += inL;
            rawX[1]+=inR;
            // 2. Your ultra-fast, thread-safe unrolled loop compiles perfectly now!
            time5+=getTimeInterval();
            for (int r = 0; r < MATRIX_SIZE; ++r) {
                float sum = 0.0f;

                const float* rowPtr = &rawW[r * MATRIX_SIZE];


                for (int c = 0; c < MATRIX_SIZE; ++c) {
                    sum += rowPtr[c] * rawX[c];
                }
                rawY[r]=sum;
            }

            time6+=getTimeInterval();
            if(activationFunction==activationFunctionClip)
            {
                float* rawY = y.data();
                for (int r = 0; r < MATRIX_SIZE; ++r) {
                    rawY[r] = clip(rawY[r]);
                }
            }
            if(activationFunction==activationFunctionTanh)
            {
                float* rawY = y.data();
                for (int r = 0; r < MATRIX_SIZE; ++r) {
                    rawY[r] = fastTanh(rawY[r]);
                }
            }
            time7+=getTimeInterval();

            // const float* rawOutMatL = outputMatrix.col(0).data();
            // const float* rawOutMatR = outputMatrix.col(1).data();

            // float outL=0.f,outR=0.f;
            // for (int r = 0; r < MATRIX_SIZE; ++r) {
            //     outL+=rawOutMatL[r]*rawY[r];
            //     outR+=rawOutMatR[r]*rawY[r];

            // }

            outputs[0][sample]=rawY[0];outputs[1][sample]=rawY[1];

            inputBufferIndex=(inputBufferIndex+1)%(MAX_DELAY+1);
            Eigen::Map<Eigen::Vector<float, MATRIX_SIZE>> x2(inputBuffer[(inputBufferIndex+currentLoopDelay)%(MAX_DELAY+1)]);
            x2.head<MATRIX_SIZE>() = y.eval();
            time8+=getTimeInterval();
            if(!(framecounter++%48000)) printTime();

        }
        lastDelay=lastLoopDelay;
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Information
    /**
      Initialize the parameter @a index.@n
      This function will be called once, shortly after the plugin is created.
    */
    void initParameter(uint32_t index, Parameter& parameter) override
    {
        parameter.hints=kParameterIsAutomatable;
        parameter.ranges.min = -1.f;
        parameter.ranges.max = 1.f;
        parameter.ranges.def = 0.f;


        if(index<MATRIX_SIZE*2){
            int row=index/2;
            int col=index%2;
            parameter.name=(std::string("inputMatrix")+std::to_string(row)+","+std::to_string(col)).c_str();
        }else if(index<MATRIX_SIZE*(MATRIX_SIZE+2)){
            int newIndex=index-MATRIX_SIZE*2;
            int row=newIndex/MATRIX_SIZE;
            int col=newIndex%MATRIX_SIZE;
            parameter.name=(std::string("weightMatrix")+std::to_string(row)+","+std::to_string(col)).c_str();
        }else if(index<MATRIX_SIZE*(MATRIX_SIZE+4)){
            int newIndex=index-MATRIX_SIZE*(MATRIX_SIZE+2);
            int row=newIndex/2;
            int col=newIndex%2;
            parameter.name=(std::string("outputMatrix")+std::to_string(row)+","+std::to_string(col)).c_str();
        }else if(index==MATRIX_SIZE*(MATRIX_SIZE+4)){
            parameter.ranges.min = 0.f;
            parameter.ranges.max = MAX_DELAY;
            parameter.ranges.def = 0.f;
            parameter.name = "Delay";
        }else if(index==MATRIX_SIZE*(MATRIX_SIZE+4)+1){
            parameter.ranges.min = 0.f;
            parameter.ranges.max = 1.01f;
            parameter.ranges.def = 0.999f;
            parameter.name = "Max eigenvalue";
        }



    }

    float getParameterValue(uint32_t index) const override
    {
        if(index<MATRIX_SIZE*2){
            int row=index/2;
            int col=index%2;
            return inputMatrix(row,col);
        }else if(index<MATRIX_SIZE*(MATRIX_SIZE+2)){
            int newIndex=index-MATRIX_SIZE*2;
            int row=newIndex/MATRIX_SIZE;
            int col=newIndex%MATRIX_SIZE;
            return weightMatrix(row,col);

        }else if(index<MATRIX_SIZE*(MATRIX_SIZE+4)){
            int newIndex=index-MATRIX_SIZE*(MATRIX_SIZE+2);
            int row=newIndex/2;
            int col=newIndex%2;
            return outputMatrix(row,col);
        }else if(index==MATRIX_SIZE*(MATRIX_SIZE+4)){
            return fDelay;

        }else if(index==MATRIX_SIZE*(MATRIX_SIZE+4)+1){
            return fMaxEigenvalue;
        }
    }


    void setParameterValue(uint32_t index, float value) override
    {
SET_PARAMETER_VALUE
            if(index!=kParamDelay) needsNormalise=true;
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
        if (!strcmp(key,"sampleData"))
        {

            size_t maxEigenSize=sizeof(float);
            size_t activationSize=sizeof(ActivationFunctionType);
            // size_t matrixSize=sizeof(float)*(MATRIX_SIZE+CONSTANT_KNOB_COUNT)*MATRIX_SIZE;

             size_t totalBytes = maxEigenSize+activationSize/*+matrixSize*/;

            std::vector<uint8_t> rawBinaryBuffer(totalBytes);
            auto *incrementalPointer=rawBinaryBuffer.data();

            // auto *maxEigenPointer=reinterpret_cast<float*>(incrementalPointer);
            // *maxEigenPointer=fMaxEigenvalue;
            incrementalPointer+=maxEigenSize;

            auto* activationPointer=reinterpret_cast<ActivationFunctionType*>(incrementalPointer);
            *activationPointer=activation.load(std::memory_order_relaxed);
            incrementalPointer+=activationSize;

            // auto *matrixPtr=reinterpret_cast<float*>(incrementalPointer);
            // auto *buffer=weightBufferPointer.load(std::memory_order_relaxed);
            // for(int i=0;i<MATRIX_SIZE*(MATRIX_SIZE+CONSTANT_KNOB_COUNT);i++)
            // {
            //     matrixPtr[i]=buffer[i];
            // }

            std::string encodedText = base64_encode(rawBinaryBuffer.data(), rawBinaryBuffer.size());
            return String(encodedText.c_str());
        }else return String("");

    }

    void setState(const char *key, const char * value){
        if(!strcmp("sampleData", key))
        {
            if (strlen(value) == 0 ) {
                return;
            }


            std::string decodedBytes = base64_decode(std::string(value));

            const uint8_t* incrementalPointer = reinterpret_cast<const uint8_t*>(decodedBytes.data());

            size_t maxEigenSize=sizeof(float);
            size_t activationSize=sizeof(ActivationFunctionType);
            //size_t matrixSize=sizeof(float)*(MATRIX_SIZE+CONSTANT_KNOB_COUNT)*MATRIX_SIZE;


            // auto *maxEigenPointer=reinterpret_cast<const float*>(incrementalPointer);
            // fMaxEigenvalue=*maxEigenPointer;
            incrementalPointer+=maxEigenSize;

            auto* activationPointer=reinterpret_cast<const ActivationFunctionType*>(incrementalPointer);
            activation.store(*activationPointer,std::memory_order_relaxed);
            incrementalPointer+=activationSize;

            // auto *matrixPtr=reinterpret_cast<const float*>(incrementalPointer);
            // updateReady.store(false,std::memory_order_release);
            // auto *buffer=weightBufferPointer.load(std::memory_order_acquire);
            // float *outPointer=(buffer==weightBuffer1?weightBuffer2:weightBuffer1);
            // for(int i=0;i<MATRIX_SIZE*(MATRIX_SIZE+CONSTANT_KNOB_COUNT);i++)
            // {
            //     outPointer[i]=matrixPtr[i];
            // }
            // weightBufferPointer.store(outPointer,std::memory_order_release);

        }
    }
    /**
      Get the plugin label.@n
      This label is a short restricted name consisting of only _, a-z, A-Z and 0-9 characters.
    */
    const char* getLabel() const noexcept override
    {
        return "neural";
    }

    /**
      Get an extensive comment/description about the plugin.@n
      Optional, returns nothing by default.
    */
    const char* getDescription() const override
    {
        return "neural FX";
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
        return d_cconst('n', 'e', 'u', 'r');
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Init
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImGuiPluginDSP)
};

END_NAMESPACE_DISTRHO

#endif // PLUGINUI_HPP

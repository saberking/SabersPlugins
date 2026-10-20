/*
 * ImGui plugin example
 * Copyright (C) 2021 Jean Pierre Cimalando <jp-dev@inbox.ru>
 * Copyright (C) 2021-2022 Filipe Coelho <falktx@falktx.com>
 * SPDX-License-Identifier: ISC
 */

#include "DistrhoUI.hpp"
#include "ResizeHandle.hpp"

#include <../clap/include/clap/ext/context-menu.h>
#include <../clap/include/clap/ext/state.h>
#include <../clap/include/clap/ext/params.h>
#include <windows.h>
#include <commctrl.h> // For SetWindowSubclass API
#include "PluginDSP.hpp"
#pragma comment(lib, "comctl32.lib")


START_NAMESPACE_DISTRHO


//for right click autmoaiton clip
class ImGuiPluginUI : public UI
{
    ResizeHandle fResizeHandle;
    bool needsNormalise=false;
public:
    alignas(64) Eigen::Array<float, 30, 2> inputMatrix, outputMatrix;
    alignas(64) Eigen::Array<float, 30, 30> weightMatrix;
    float fDelay;
    float fMaxEigenvalue;

    std::mt19937 gen;
    std::normal_distribution<float> d;
    ImGuiPluginUI()
        : UI(DISTRHO_UI_DEFAULT_WIDTH,DISTRHO_UI_DEFAULT_HEIGHT),
        fResizeHandle(this)
    {

        // const double scaleFactor = getScaleFactor();
        std::srand(std::time(nullptr));
        std::random_device rd;
        gen.seed(rd()); // Seed this specific instance with a hardware random

        setSize(DISTRHO_UI_DEFAULT_WIDTH,DISTRHO_UI_DEFAULT_HEIGHT);


            fResizeHandle.hide();
        // Get the global style object
        ImGuiStyle& style = ImGui::GetStyle();

        // Set the global window background color
        style.Colors[ImGuiCol_WindowBg] = ImVec4(63.0f / 255.0f, 72.0f / 255.0f, 77.0f / 255.0f, 0.12f);

    }

    void stateChanged(const char* key, const char* value){

    }

    ImGuiPluginDSP* getPluginDPSPointer(){
        auto* plugin = static_cast<ImGuiPluginDSP*>(getPluginInstancePointer());
        return plugin;
    }
    bool checkIfClapAtRuntime()
    {
        char fileBuffer[MAX_PATH] = {0};
        HMODULE hModule = NULL;

        // 🟢 Create a dummy static variable. It lives inside your plugin library's binary memory space.
        static const int dummyAnchor = 0;

        // 🟢 Pass the address of the dummy anchor variable instead of the member function pointer
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                               GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCSTR>(&dummyAnchor), &hModule);

        if (hModule) {
            GetModuleFileNameA(hModule, fileBuffer, sizeof(fileBuffer));
            std::string binaryPath(fileBuffer);

            std::transform(binaryPath.begin(), binaryPath.end(), binaryPath.begin(), ::tolower);

            std::string target = ".clap";
            if (binaryPath.length() >= target.length()) {
                return (binaryPath.compare(binaryPath.length() - target.length(), target.length(), target) == 0);
            }
        }
        return false; // 🔵 Fallback (VST3, etc.)
    }

    int getPluginFormat()
    {   if(checkIfClapAtRuntime())        return 1;
        return 0;
    }

    void setDirty(){
        //only works in bitwig
        const uint32_t activeFormat = getPluginFormat();

        if (activeFormat == 1)
        {
            auto* clapPointer=reinterpret_cast<const clap_host_t*>(getPluginDPSPointer()->host);
            if(!clapPointer)return;
            auto* hostState = reinterpret_cast<const clap_host_state_t*>(clapPointer->get_extension(clapPointer, CLAP_EXT_STATE));
            if (hostState != nullptr && hostState->mark_dirty != nullptr) {
                setParameterValue(kParamDelay,(float)std::max(0,std::min(MAX_DELAY,(int)fDelay)));


                hostState->mark_dirty(clapPointer);
            }
        }
    }

    // Window& getWindow() const override {
    //     return UI::getWindow();
    // }
    void normaliseMatrix()
    {
        Eigen::Map<Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE, Eigen::RowMajor>> W(weightMatrix.data());

        Eigen::EigenSolver<Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE>> solver(W, false);
        float maxMagnitude = 0.0f;
        for (int i = 0; i < MATRIX_SIZE-2; ++i) {
            maxMagnitude = std::max(maxMagnitude, std::abs(solver.eigenvalues()[i]));
        }
        std::cout<<"normalise"<<maxMagnitude<<std::endl;
        //maxMagnitude+=0.001f;
        float mult=fMaxEigenvalue/std::max(0.001f,maxMagnitude);

        //float mult=1/findMaxAmplification(outPointer);

        W *=mult;

        // for (int r = 0; r < MATRIX_SIZE; ++r) {
        //     for (int c = 0; c < MATRIX_SIZE; ++c) {
        //         if (std::abs(W(r, c)) > fMaxEigenvalue) {
        //             // Keep the original positive/negative sign but snap the value to 0.95f
        //             W(r, c) = std::copysign(fMaxEigenvalue, W(r, c));
        //         }
        //     }
        // }

        //         W.block<MATRIX_SIZE,2>(0,0)*=0.95f;
        // W.block<2,MATRIX_SIZE>(0,0)*=0.95f;
        for(int i=0;i<MATRIX_SIZE;i++){
            for(int j=0;j<2;j++){
                setParameterValue(i*2+j,inputMatrix(i,j));
                setParameterValue(MATRIX_SIZE*(MATRIX_SIZE+2)+i*2+j,outputMatrix(i,j));
            }
            for(int j=0;j<MATRIX_SIZE;j++){
                setParameterValue(MATRIX_SIZE*(2+i)+j,weightMatrix(i,j));
            }
        }

    }
    void randomise()
    {





        Eigen::Map<Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE, Eigen::RowMajor>> W(weightMatrix.data());

        Eigen::MatrixXf X(MATRIX_SIZE, MATRIX_SIZE);
        for (int r = 0; r < MATRIX_SIZE; ++r) {
            for (int c = 0; c < MATRIX_SIZE; ++c) {
                X(r, c) = d(gen); // Populating matrix with a true Gaussian profile
            }
        }

        // A Householder QR acting on a Gaussian matrix produces a perfectly un-biased
        // Haar-distributed random orthogonal matrix, eliminating Left/Right panning bias!
        Eigen::HouseholderQR<Eigen::MatrixXf> qr(X);
        Eigen::MatrixXf Q = qr.householderQ();

        W = (W * Q * 0.1f).eval()  + (W * 0.9f).eval();

        // Fix noise injection using the same Gaussian distribution profile
        Eigen::MatrixXf noise(MATRIX_SIZE, MATRIX_SIZE);
        const float noiseAmount = 0.05f;
        for (int r = 0; r < MATRIX_SIZE; ++r) {
            for (int c = 0; c < MATRIX_SIZE; ++c) {
                noise(r, c) = d(gen) * noiseAmount;
            }
        }
        W += noise;
        normaliseMatrix();


    }
    void delay()
    {




        // outPointer[MATRIX_SIZE-2]=1.f;
        // outPointer[(MATRIX_SIZE+CONSTANT_KNOB_COUNT)+MATRIX_SIZE-1]=1.f;
        // for(int i =2;i<MATRIX_SIZE;i++)
        // {
        //     outPointer[(MATRIX_SIZE+CONSTANT_KNOB_COUNT)*i+i-2]=1.f;

        // }
        Eigen::Map<Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE, Eigen::RowMajor>> W(weightMatrix.data());


        auto circularSequence = (Eigen::VectorXi::LinSpaced(MATRIX_SIZE, -2, MATRIX_SIZE - 3).array() + MATRIX_SIZE)
                                    .unaryExpr([](int val) { return val % MATRIX_SIZE; });

        // 2. COMPILER-SAFE REWRITE: Create a temporary copy of the old matrix state
        // This completely stops memory aliasing bugs without needing Eigen::all!
        Eigen::Matrix<float, MATRIX_SIZE, MATRIX_SIZE, Eigen::RowMajor> tempW = W;

        // 3. Remap the rows sequentially
        for (int i = 0; i < MATRIX_SIZE; ++i) {
            int targetRowIndex = circularSequence[i];
            W.row(i) += tempW.row(targetRowIndex)*0.2f; // Cleanly assigns the whole row
        }

        normaliseMatrix();


    }

protected:
    void parameterChanged(uint32_t index, float value) override {
        SET_PARAMETER_VALUE
        repaint();
    }



    void onImGuiDisplay() override {


        const float height = getHeight();
        const float width = getWidth();


        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(width , height ));

        if (ImGui::Begin("neural", nullptr, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar))
        {
            if(ImGui::Button("Randomise"))
            {
                randomise();
            }
            if(ImGui::Button("Delay##delaybutton"))
            {
                delay();
            }
            // if(ImGui::Button("Print matrix")){
            //     getPluginDPSPointer()->printMatrix();
            // }
            // if(ImGui::Button("Normalise")){
            //     getPluginDPSPointer()->normalise();
            // }
            if(ImGui::SliderFloat("Max eigenvalue", &fMaxEigenvalue, 0.9f,1.01f))
            {
                if(ImGui::IsItemActivated()){
                    editParameter(kParamMaxEigenvalue,true);
                }
                setParameterValue(kParamMaxEigenvalue,fMaxEigenvalue);
                normaliseMatrix();

            }
            if(ImGui::IsItemDeactivated()){
                editParameter(kParamMaxEigenvalue,false);
            }



            ActivationFunctionType type=getPluginDPSPointer()->activation.load(std::memory_order_relaxed);
            // 2. Render the dropdown
            if (ImGui::BeginCombo("Activation function##UniqueLabel", activationFunctionNames[type]))
            {
                for (int i = 0; i < activationFunctionCount; ++i)
                {
                    const bool is_selected = (type == i);

                    // Render each item as selectable
                    if (ImGui::Selectable(activationFunctionNames[i], is_selected))
                    {
                        type = (ActivationFunctionType)i; // Update selection state on click
                    }

                    // Set the initial keyboard/scroll focus to the currently active selection
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo(); // Always call EndCombo if BeginCombo returns true
            }
            getPluginDPSPointer()->activation.store(type,std::memory_order_relaxed);

            if(ImGui::SliderFloat("Delay##Delayslider",&fDelay,0.f,(float)MAX_DELAY))
            {
                if(ImGui::IsItemActivated())
                    editParameter(kParamDelay,true);
                setParameterValue(kParamDelay,(float)std::max(0,std::min(MAX_DELAY,(int)fDelay)));
            }

            if (ImGui::IsItemDeactivated())
            {
                editParameter(kParamDelay, false);
            }
        }

        // if (ImGui::BeginTable("Awesom table",3/*4*/,ImGuiTableFlags_SizingStretchSame)){
        //     ImGui::TableSetupColumn("Col 1", ImGuiTableColumnFlags_None, 2.f);
        //     ImGui::TableSetupColumn("Col 2", ImGuiTableColumnFlags_None, (float)MATRIX_SIZE);
        //     ImGui::TableSetupColumn("Col 3", ImGuiTableColumnFlags_None, 2.f);
        //     ImGui::TableNextColumn();
        //     ImGui::Text("Input");
        //     ImGui::Separator();
        //     ImGui::TableNextColumn();
        //     ImGui::Text("Matrix");
        //     ImGui::Separator();
        //     // ImGui::TableNextColumn();
        //     // ImGui::Text("Crossfeed");
        //     // ImGui::Separator();
        //     ImGui::TableNextColumn();
        //     ImGui::Text("Output");
        //     ImGui::Separator();
#define KNOB_PARAMS 0.f,NULL,ImGuiKnobVariant_Tick,22.f,ImGuiKnobFlags_NoInput|ImGuiKnobFlags_NoTitle
                // ImGui::TableNextColumn();
            // for(int i=0;i<MATRIX_SIZE;i++){
            //     ImGui::PushID(i);
            //     if (ImGuiKnobs::Knob("InL", &(inputMatrix(i,0)), -1.f, 1.0f,KNOB_PARAMS)) {
            //         if(ImGui::IsItemActivated())
            //             editParameter(i*2,true);
            //         setParameterValue(i*2,inputMatrix(i,0));
            //     }
            //     if (ImGui::IsItemDeactivated())
            //     {
            //         editParameter(i*2, false);
            //     }
            //     ImGui::SameLine();
            //     if (ImGuiKnobs::Knob("InR", &(inputMatrix(i,1)), -1.f, 1.0f,KNOB_PARAMS)) {
            //         if(ImGui::IsItemActivated())
            //             editParameter(i*2+1,true);
            //         setParameterValue(i*2+1,inputMatrix(i,1));
            //     }
            //     if (ImGui::IsItemDeactivated())
            //     {
            //         editParameter(i*2+1, false);
            //     }
            //     ImGui::PopID();
            // }
            // ImGui::TableNextColumn();
            for(int i=0;i<MATRIX_SIZE;i++){
                for(int j=0;j<MATRIX_SIZE;j++){
                    ImGui::PushID((i+2)*MATRIX_SIZE+j);
                    if (ImGuiKnobs::Knob("weight", &(weightMatrix(i,j)), -1.f, 1.0f,KNOB_PARAMS)) {
                        if(ImGui::IsItemActivated())
                            editParameter((i+2)*MATRIX_SIZE+j,true);
                        setParameterValue((i+2)*MATRIX_SIZE+j,weightMatrix(i,j));
                        normaliseMatrix();
                    }
                    if (ImGui::IsItemDeactivated())
                    {
                        editParameter((i+2)*MATRIX_SIZE+j, false);
                    }
                    if(j<MATRIX_SIZE-1) ImGui::SameLine();
                    ImGui::PopID();
                }

            }
        //     ImGui::TableNextColumn();
        //     for(int i=0;i<MATRIX_SIZE;i++){
        //         ImGui::PushID(i+(MATRIX_SIZE*(MATRIX_SIZE+2)));
        //         if (ImGuiKnobs::Knob("OutL", &(outputMatrix(i,0)), -1.f, 1.0f,KNOB_PARAMS)) {
        //             if(ImGui::IsItemActivated())
        //                 editParameter(MATRIX_SIZE*(MATRIX_SIZE+2)+ i*2,true);
        //             setParameterValue(MATRIX_SIZE*(MATRIX_SIZE+2)+i*2,outputMatrix(i,0));
        //         }
        //         if (ImGui::IsItemDeactivated())
        //         {
        //             editParameter(i*2, false);
        //         }
        //         ImGui::SameLine();
        //         if (ImGuiKnobs::Knob("OutR", &(outputMatrix(i,1)), -1.f, 1.0f,KNOB_PARAMS)) {
        //             if(ImGui::IsItemActivated())
        //                 editParameter(MATRIX_SIZE*(MATRIX_SIZE+2)+i*2+1,true);
        //             setParameterValue(MATRIX_SIZE*(MATRIX_SIZE+2)+i*2+1,outputMatrix(i,1));
        //         }
        //         if (ImGui::IsItemDeactivated())
        //         {
        //             editParameter(i*2+1, false);
        //         }
        //         ImGui::PopID();
        //     }

        //     ImGui::EndTable();
        // }
        ImGui::End();
    }

    ~ImGuiPluginUI(){

    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImGuiPluginUI)
};

// This must stay outside the class because it is a global framework entry point
UI* createUI()
{
    return new ImGuiPluginUI();
}

END_NAMESPACE_DISTRHO

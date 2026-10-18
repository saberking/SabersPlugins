/*
 * ImGui plugin example
 * Copyright (C) 2021 Jean Pierre Cimalando <jp-dev@inbox.ru>
 * Copyright (C) 2021-2022 Filipe Coelho <falktx@falktx.com>
 * SPDX-License-Identifier: ISC
 */

#include "DistrhoUI.hpp"
#include "ResizeHandle.hpp"
#include "Parameters.hpp"

#include <../clap/include/clap/ext/context-menu.h>
#include <../clap/include/clap/ext/state.h>
#include <../clap/include/clap/ext/params.h>
#include <windows.h>
#include <commctrl.h> // For SetWindowSubclass API
#include "PluginDSP.hpp"
#include "InfiniteKnob.hpp"
#include "DearImGuiKnobs/imgui-knobs.h"
#pragma comment(lib, "comctl32.lib")


START_NAMESPACE_DISTRHO


//for right click autmoaiton clip
class ImGuiPluginUI : public UI
{
    ResizeHandle fResizeHandle;
    bool needsNormalise=false;
public:
#define Y_REAL(i,j) CROSS_PARAM_DECL(i,j)

#define X(i) OUTER_PARAM_DECL(i)

    BIQUAD_LIST
#undef X
#undef Y_REAL
    ImGuiPluginUI()
        : UI(DISTRHO_UI_DEFAULT_WIDTH,DISTRHO_UI_DEFAULT_HEIGHT),
        fResizeHandle(this)
    {

        // const double scaleFactor = getScaleFactor();

        setSize(DISTRHO_UI_DEFAULT_WIDTH,DISTRHO_UI_DEFAULT_HEIGHT);


            fResizeHandle.hide();

        std::cout<<"finished ui cosntructor"<<std::endl;

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
        return false; // 🔵 fallback (VST3, etc.)
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


                hostState->mark_dirty(clapPointer);
            }
        }
    }

    // Window& getWindow() const override {
    //     return UI::getWindow();
    // }



protected:
    void parameterChanged(uint32_t index, float value) override {
#define Y_REAL(i,j) SET_CROSS_PARAM(i,j)
#define X(i) SET_PARAM(i)
        BIQUAD_LIST
#undef X
#undef Y_REAL
        repaint();
    }
    void displayRows()
    {
        int width=8+BIQUAD_COUNT/2;

        if (ImGui::BeginTable("Awesom table",4)){
            ImGui::TableNextColumn();
            ImGui::Text("Input");
            ImGui::Separator();
            ImGui::TableNextColumn();
            ImGui::Text("Poles");
            ImGui::Separator();
            ImGui::TableNextColumn();
            ImGui::Text("Crossfeed");
            ImGui::Separator();
            ImGui::TableNextColumn();
            ImGui::Text("Output");
            ImGui::Separator();
#define X(i)\
            ImGui::PushID(i);\
            ImGui::TableNextColumn();\
            if (ImGuiKnobs::Knob("Vol", &fInLvl##i, -4.f, 4.0f)) {\
                if(ImGui::IsItemActivated())\
                    editParameter(kParamInLvl##i,true);\
                setParameterValue(kParamInLvl##i,fInLvl##i);\
            }\
            if (ImGui::IsItemDeactivated())\
            {\
                editParameter(kParamInLvl##i, false);\
            }\
            ImGui::SameLine();\
            if (ImGuiKnobs::Knob("Pan", &fInPan##i, -M_PI/2, M_PI/2, 0.01f, "%.3f", ImGuiKnobVariant_Stepped, 0)) {\
                    if(ImGui::IsItemActivated())\
                    editParameter(kParamInPan##i,true);\
                    setParameterValue(kParamInPan##i,fInPan##i);\
            }\
                if (ImGui::IsItemDeactivated())\
            {\
                    editParameter(kParamInPan##i, false);\
            }\
            if(InfiniteKnob("Angle",&fInAngle##i,-M_PI,M_PI,0.01f,0.f,0)){\
                if(ImGui::IsItemActivated())\
                    editParameter(kParamInAngle##i,true);\
                setParameterValue(kParamInAngle##i,fInAngle##i);\
            }\
            if (ImGui::IsItemDeactivated())\
            {\
                editParameter(kParamInAngle##i, false);\
            }\
            ImGui::SameLine();\
                if(InfiniteKnob("AngleOffset",&fInAngleOffset##i,-M_PI,M_PI)){\
                    if(ImGui::IsItemActivated())\
                    editParameter(kParamInAngleOffset##i,true);\
                    setParameterValue(kParamInAngleOffset##i,fInAngleOffset##i);\
            }\
                if (ImGui::IsItemDeactivated())\
            {\
                    editParameter(kParamInAngleOffset##i, false);\
            }\
            ImGui::Separator();\
            \
            ImGui::TableNextColumn();\
                if (ImGuiKnobs::Knob("Centre", &fPoleCentre##i, -1.f, 1.0f)) {\
                    if(ImGui::IsItemActivated())\
                    editParameter(kParamPoleCentre##i,true);\
                    setParameterValue(kParamPoleCentre##i,fPoleCentre##i);\
            }\
                if (ImGui::IsItemDeactivated())\
            {\
                    editParameter(kParamPoleCentre##i, false);\
            }\
                if(ImGuiKnobs::Knob("Discriminant",&fDiscriminant##i,-4.f,4.f)){\
                    if(ImGui::IsItemActivated())\
                    editParameter(kParamDiscriminant##i,true);\
                    setParameterValue(kParamDiscriminant##i,fDiscriminant##i);\
            }\
                if (ImGui::IsItemDeactivated())\
            {\
                    editParameter(kParamDiscriminant##i, false);\
            }\
            ImGui::Separator();\
            ImGui::TableNextColumn();\
            ImGui::TableNextColumn();\
            ImGui::PopID();\
            ImGui::PushID(i+BIQUAD_COUNT);\
            if (ImGuiKnobs::Knob("Vol", &fOutLvl##i, -4.f, 4.0f)) {\
                    if(ImGui::IsItemActivated())\
                    editParameter(kParamOutLvl##i,true);\
                    setParameterValue(kParamOutLvl##i,fOutLvl##i);\
            }\
                if (ImGui::IsItemDeactivated())\
            {\
                    editParameter(kParamOutLvl##i, false);\
            }\
                ImGui::SameLine();\
                if (ImGuiKnobs::Knob("Pan", &fOutPan##i, -M_PI/2, M_PI/2, 0.01f, "%.3f", ImGuiKnobVariant_Stepped, 0)) {\
                    if(ImGui::IsItemActivated())\
                    editParameter(kParamOutPan##i,true);\
                    setParameterValue(kParamOutPan##i,fOutPan##i);\
            }\
                if (ImGui::IsItemDeactivated())\
            {\
                    editParameter(kParamOutPan##i, false);\
            }\
                if(InfiniteKnob("Angle",&fOutAngle##i,-M_PI,M_PI,0.01f,0.f,0)){\
                    if(ImGui::IsItemActivated())\
                    editParameter(kParamOutAngle##i,true);\
                    setParameterValue(kParamOutAngle##i,fOutAngle##i);\
            }\
                if (ImGui::IsItemDeactivated())\
            {\
                    editParameter(kParamOutAngle##i, false);\
            }\
                ImGui::SameLine();\
                if(InfiniteKnob("AngleOffset",&fOutAngleOffset##i,-M_PI,M_PI)){\
                    if(ImGui::IsItemActivated())\
                    editParameter(kParamOutAngleOffset##i,true);\
                    setParameterValue(kParamOutAngleOffset##i,fOutAngleOffset##i);\
            }\
                if (ImGui::IsItemDeactivated())\
            {\
                    editParameter(kParamOutAngleOffset##i, false);\
            }\
            ImGui::Separator();\
            ImGui::PopID();




            BIQUAD_LIST
#undef X

            ImGui::EndTable();
        }
    }



    void onImGuiDisplay() override {

        const float height = getHeight();
        const float width = getWidth();


        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(width , height ));

        if (ImGui::Begin("GRAM", nullptr, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar))
        {


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
            displayRows();

        }
        //if(!ImGui::IsMouseDown(ImGuiMouseButton_Left)) endDrag();

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

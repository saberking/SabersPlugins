#ifndef INFINITEKNOB_HPP
#define INFINITEKNOB_HPP
#include "imgui.h"
#include "imgui_internal.h" // Needed for ItemAdd
#include "DearImGuiKnobs/imgui-knobs.h"
#include <cmath>

bool InfiniteKnob(const char* label, float* value, float minValue, float maxValue, float speed = 0.01f, float radius = 20.0f, int flags=ImGuiKnobFlags_NoTitle) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;


    ImVec2 size = ImVec2(radius * 2.0f, radius * 2.0f);


    // Draw title
    if (!(flags & ImGuiKnobFlags_NoTitle)) {
        auto title_size = ImGui::CalcTextSize(label, NULL, false, size.x);

        // Center title
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (size.x - title_size[0]) * 0.5f);

        ImGui::Text("%s", label);
    }

    const ImGuiStyle& style = ImGui::GetStyle();
    const ImGuiID id = window->GetID(label);

    // Calculate dimensions
    ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));



    // Register the item in ImGui layout
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) return false;

    // Handle interaction
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    bool value_changed = false;
    float range=maxValue-minValue;

    ImGuiIO& io = ImGui::GetIO();
    if (held && io.MouseDelta.y != 0.0f) {
        // Drag vertically to change value endlessly
        *value -= io.MouseDelta.y * speed*range;
        while(*value>maxValue)*value-=range;
        while(*value<minValue)*value+=range;
        value_changed = true;
    }

    // Render - Infinite rotation mapping
    // We map the absolute value to a continuous visual angle (0 to 2*PI)
    float angle = M_PI*2*(*value-minValue)/range-M_PI/2;
    ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);

    // Draw the background circle
    window->DrawList->AddCircleFilled(center, radius, ImGui::GetColorU32(ImGuiCol_FrameBg), 32);

    // Draw the infinite rotating indicator dot
    ImVec2 indicator_pos = ImVec2(
        center.x + std::cos(angle) * (radius * 0.6f),
        center.y + std::sin(angle) * (radius * 0.6f)
        );
    ImU32 indicator_color = ImGui::GetColorU32(held ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab);
    window->DrawList->AddCircleFilled(indicator_pos, radius * 0.2f, indicator_color, 16);

    // Draw input
    std::string foo="##"+std::string(label)+"knob_drag";

    // if(!(rand()%100))std::cout<<foo<<std::endl;
    if (!(flags & ImGuiKnobFlags_NoInput)) {

        auto changed = ImGui::DragScalar(foo.c_str(), ImGuiDataType_Float, value, speed, &minValue, &maxValue,"%.2f",ImGuiSliderFlags_Vertical);
        if (changed) {
            value_changed = true;
        }
    }


    return value_changed;
}

#endif // INFINITEKNOB_HPP

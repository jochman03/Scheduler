#pragma once

#include <cstdint>
#include <string_view>
#include <vector>
#include <string>
#include <array>

#include "scheduler.hpp"

enum clickMode {
    click,
    select_group
};


class WindowClass{
public:
    WindowClass() : subjects({}), filenameBuffer("data.json"), groupsInDay(num_days) {};
    void Draw(std::string_view label);

    static constexpr auto popUpFlags = ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoScrollbar;
    static constexpr auto popUpSize = ImVec2(300.0f, 100.0f);
    static constexpr auto popUpButtonSize = ImVec2(120.0f, 0.0f);
    static constexpr std::array<std::string_view, 5> weekDays{"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};


private:
    const int num_days {5};

    // COLORS
    const ImVec4 group_selected_border_color = {0.3f, 0.4f, 0.5f, 1.0f};
    const ImVec4 group_constant = {0.0f, 0.4f, 0.1f, 1.0f};
    const ImVec4 group_select_button = {0.4f, 0.5f, 0.5f, 1.0f};

    const ImVec4 group_active_bg_color = {0.2f, 0.3f, 0.4f, 1.0f};
    const ImVec4 group_inactive_bg_color = {0.1f, 0.1f, 0.1f, 0.5f};
    const ImVec4 lecture_active_bg_color = {0.1f, 0.2f, 0.3f, 1.0f};
    const ImVec4 lecture_inactive_bg_color = {0.1f, 0.1f, 0.1f, 0.5f};
    const ImVec4 lecture_online_bg_color = {0.1f, 0.2f, 0.3f, 0.7f};

    const ImVec4 group_active_text_color = {1.0f, 1.0f, 1.0f, 1.0f};
    const ImVec4 group_inactive_text_color = {0.5f, 0.5f, 0.5f, 0.5f};

    std::vector<std::vector<Group*>> groupsInDay;

    std::vector<Subject> subjects;
    std::vector<uint8_t> selected_subjects;

    char filenameBuffer[256];
    bool dataLoaded{false};

    clickMode mode = click;
    Group* selectedGroup = nullptr;

    void DrawLoad();
    void DrawParameters();
    void DrawCalendar();

    void DrawSavePopup();
    void DrawLoadPopup();

    void SaveToFile(std::string filename);
    void LoadFromFile(std::string filename);
    void Clear();

    void PrintGroups();
    void SortGroupsByDay();
    void BubblesortGroups();
    void CheckCollisions();

    int GetSubjectId(Subject* subject);
};

void render(WindowClass &window_obj);

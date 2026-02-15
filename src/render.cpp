#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

#include "render.hpp"
#include "json.hpp"
#include <imgui_stdlib.h>

static bool render_text = 0;

using json = nlohmann::json;

void WindowClass::Draw(std::string_view label){
    constexpr static auto window_flags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration;
    constexpr static auto window_size = ImVec2(1920.0F, 1080.0F);
    constexpr static auto window_pos = ImVec2(0.0F, 0.0F);

    ImGuiIO& io = ImGui::GetIO();      
    ImVec2 availSize = io.DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(availSize);
    ImGui::Begin(label.data(), nullptr, window_flags);

    DrawLoad();
    ImGui::Separator();
    ImGui::Separator();
    DrawCalendar();
    ImGui::Separator();
    DrawParameters();

    DrawSavePopup();
    DrawLoadPopup();

    ImGui::End();
}

void WindowClass::DrawLoad(){
    const auto ctrl_pressed = ImGui::GetIO().KeyCtrl;
    const auto s_pressed = ImGui::IsKeyPressed(ImGuiKey_S);
    const auto l_pressed = ImGui::IsKeyPressed(ImGuiKey_L);

    if(ImGui::Button("Save") || (ctrl_pressed && s_pressed)){
        ImGui::OpenPopup("Save json");
    }
    ImGui::SameLine();
    if(ImGui::Button("Load") || (ctrl_pressed && l_pressed)){
        ImGui::OpenPopup("Load json");
    }
    ImGui::SameLine();
    if(ImGui::Button("Clear")){
        Clear();
    }
}

void WindowClass::DrawParameters(){
    if(dataLoaded){
        const auto parameters_height = 1.0f/5.0f*ImGui::GetWindowSize().y;
        const auto parameters_width = 1.0f/4.0f*ImGui::GetWindowSize().x;
        ImVec2 parameters_size(parameters_width, parameters_height);

        ImGui::BeginChild("Group Details", parameters_size, true);
        ImGui::Text("Group Details");
        ImGui::Separator();
        if(selectedGroup == nullptr){
            ImGui::Text("Select group to show details.");
        }
        else{

            ImGui::Text("Subject: ");
            ImGui::SameLine();
            ImGui::Text(selectedGroup->subject->getName().data());
            ImGui::Text("Group: ");
            ImGui::SameLine();
            std::string number = std::to_string(selectedGroup->number);
            ImGui::Text(number.data());
            std::string min = std::to_string(selectedGroup->startMin);
            if(selectedGroup->startMin < 10){
                min = "0" + std::to_string(selectedGroup->startMin);
            }
            std::string start_time = std::to_string(selectedGroup->startHour) + ":" + min;
            ImGui::Text("Start time: ");
            ImGui::SameLine();
            ImGui::Text(start_time.data());
            min = std::to_string(selectedGroup->endMin);
            if(selectedGroup->endMin < 10){
                min = "0" + std::to_string(selectedGroup->endMin);
            }
            std::string end_time = std::to_string(selectedGroup->endHour) + ":" + min;
            ImGui::Text("End time: ");
            ImGui::SameLine();
            ImGui::Text(end_time.data());
            ImGui::Text("Teacher: ");
            ImGui::SameLine();
            ImGui::Text(selectedGroup->teacher.data());
            ImGui::Text("Place: ");
            ImGui::SameLine();
            ImGui::Text(selectedGroup->place.data());
            if(selectedGroup->lecture){
                ImGui::Text("Lecture");
                if(selectedGroup->online){
                    ImGui::SameLine();
                    ImGui::Text("ONLINE");
                }
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("Subjects", parameters_size, true);
        ImGui::Text("Select Subjects");
        ImGui::Separator();
        ImGui::BeginChild("Subjects-selector");
        for(auto i = 0; i < subjects.size(); i++){
            std::string subject_name = subjects[i].getName();
            ImGui::Checkbox(subject_name.data(), reinterpret_cast<bool*>(&selected_subjects[i]));
        }

        ImGui::EndChild();
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("Selecting Mode", parameters_size, true);
        const auto button_size = ImGui::GetTextLineHeight();
        ImGui::Text("Selecting Mode");
        ImGui::Separator();
        bool change_mode = 0;
        if(mode == click){
            ImGui::PushStyleColor(ImGuiCol_Button, group_select_button);
        }
        if(ImGui::Button("###details_groups", ImVec2(button_size, button_size))){
            change_mode = 1;
        }
        if(mode == click){
            ImGui::PopStyleColor();
        }
        if(change_mode){
            mode = click;
        }

        ImGui::SameLine();
        ImGui::Text("Select group for details");

        change_mode = 0;
        if(mode == select_group){
            ImGui::PushStyleColor(ImGuiCol_Button, group_constant);
        }
        if(ImGui::Button("###constant_groups", ImVec2(button_size, button_size))){
            change_mode = 1;
        }
        if(mode == select_group){
            ImGui::PopStyleColor();
        }
        if(change_mode){
            mode = select_group;
        }

        ImGui::SameLine();
        ImGui::Text("Select constant groups");

        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("Solver", parameters_size, true);
        ImGui::Text("Solver Parameters");

        ImGui::EndChild();
    }
}

void WindowClass::DrawCalendar(){
    const auto margin = 50.0f;
    const auto window_width = ImGui::GetWindowSize().x - margin;
    const auto window_height = 4.0f/5.0f*ImGui::GetWindowSize().y;

    const auto legend_width = 100;
    const auto start_hour = 8, end_hour = 22;
    const float start_time = start_hour*60.0f, end_time = end_hour*60.0f;
    const float tile_margin = 1;
    const auto day_width = (window_width - legend_width) / num_days;
    const auto day_height = window_height - 100;
    const auto calendar_height = day_height;

    if(dataLoaded){
        if(ImGui::BeginChild("Legend", ImVec2(legend_width, day_height))){
            for(auto i = start_hour; i < end_hour; i++){
                auto index = i - start_hour;
                std::string hour_str = std::to_string(i) + ":00";
                const auto pos_y = index * day_height / (end_hour - start_hour);
                ImGui::SetCursorPosY(60);
                ImDrawList* draw = ImGui::GetWindowDrawList();

                draw->PushClipRectFullScreen();
                draw->AddLine(
                    ImVec2(0, 60 + pos_y),
                    ImVec2(window_width + margin, 60 + pos_y),
                    IM_COL32(50, 50, 50, 255),
                    1.0f
                );          
                ImGui::SetCursorPosY(20 + pos_y);
                ImGui::Text(hour_str.data());

            }
        }

        ImGui::EndChild();
        ImGui::SameLine();

        // Draw Days
        for(int i = 0; i < num_days; ++i){
            const auto day_name = weekDays[i];

            ImGui::SameLine();
            if(ImGui::BeginChild(day_name.data(), ImVec2(day_width, day_height), false)){
                ImGui::Text(day_name.data());
            }
            auto calendar_start_y = ImGui::GetCursorPosY();
            auto calendar_start_x = ImGui::GetCursorPosX();
            auto calendar_end_y = calendar_start_y + calendar_height;

            // Draw groups in calendar
            auto child_id_iter = 0;
            for(auto& group : groupsInDay[i]){
                std::string name = group->subject->getName() + " gr. " + std::to_string(group->number);
                std::string group_name = std::to_string(child_id_iter);

                const float group_start_time = group->startHour * 60.0f + group->startMin;
                const float group_end_time = group->endHour * 60.0f + group->endMin;
                const auto pos_y = calendar_start_y + (group_start_time - start_time)*
                (calendar_end_y - calendar_start_y)/(end_time - start_time);
                const auto width = day_width * group->renderWidth;
                const auto pos_end_y = calendar_start_y + (group_end_time - start_time)*
                (calendar_end_y - calendar_start_y)/(end_time - start_time);
                const auto height = pos_end_y - pos_y;
                const auto pos_x = calendar_start_x + width * group->renderIndex;
                const auto subject_id = GetSubjectId(group->subject);
                const bool selected = selected_subjects[subject_id];
                const auto is_const = group->subject->getSelectedGroup() == group;
                ImGui::SetCursorPosY(pos_y + tile_margin/2);
                ImGui::SetCursorPosX(pos_x + tile_margin/2);

                if(selectedGroup != nullptr && selectedGroup == group){
                    ImGui::PushStyleColor(ImGuiCol_Border, group_selected_border_color);
                }

                ImGui::BeginChild((name + group_name).data(), ImVec2(width - tile_margin, height - tile_margin),
                 ImGuiChildFlags_Border);

                if(selectedGroup != nullptr && selectedGroup == group){
                    ImGui::PopStyleColor();
                }

                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                ImVec4 bgColor = group_inactive_bg_color;
                if(is_const){
                    bgColor = group_constant;
                }
                else if(group->lecture){
                    if(selected){
                        bgColor = lecture_active_bg_color;
                    }
                    else{
                        bgColor = lecture_inactive_bg_color;
                    }
                }
                else{
                    if(selected){
                        bgColor = group_active_bg_color;
                    }
                    else{
                        bgColor = group_inactive_bg_color;
                    }
                }

                ImVec2 p = ImGui::GetWindowPos();
                ImVec2 s = ImGui::GetWindowSize();
                draw_list->AddRectFilled(p, ImVec2(p.x + s.x, p.y + s.y), ImGui::ColorConvertFloat4ToU32(bgColor));

                if(!selected){
                    ImGui::PushStyleColor(ImGuiCol_Text, group_inactive_text_color);
                }
                ImGui::TextWrapped(name.data());

                if(group->lecture){
                    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetTextLineHeight() - 10);
                    ImGui::Text("Lecture");
                    if(group->online){
                        ImGui::SameLine();
                        ImGui::Text("ONLINE");
                    }
                }

                if(!selected){
                    ImGui::PopStyleColor();
                }
                ImGui::EndChild();
                const auto mouse_pos = ImGui::GetMousePos();
                const auto is_mouse_hovering = ImGui::IsItemHovered();

                if(is_mouse_hovering && ImGui::IsMouseDown(ImGuiMouseButton_Left)){
                    if(mode == clickMode::click){
                        selectedGroup = group;
                    }
                    else{
                        if(!group->lecture){
                            if(is_const){
                                group->subject->selectGroup(nullptr);
                            }
                            else{
                                group->subject->selectGroup(group);
                            }
                        }
                    }
                }
                child_id_iter++;
           }
            
            ImGui::EndChild();

            if(i != num_days - 1){
                ImGui::SameLine();
            }

        }
    }

}

void render(WindowClass &window_obj){
    window_obj.Draw("Label");
}

void WindowClass::LoadFromFile(std::string filename){
    std::ifstream input(filename);
    json loaded_data;
    loaded_data << input;
    Clear();

    for (auto& [subject_name, subject_data] : loaded_data.items()) {
        Subject sub(subject_name);
        // Load groups
        if(subject_data.contains("groups")){
            auto& groups = subject_data["groups"];
            for (auto& [group_id, group_datas] : groups.items()) {
                const auto& group_data = group_datas[0];
                int id = std::stoi(group_id);
                Group group(id, group_data["start_h"], group_data["start_m"], 
                    group_data["end_h"], group_data["end_m"], group_data["day"]);
                group.teacher = group_data["teacher"];
                group.place = group_data["place"];
                sub.addGroup(group);
            }
        }
        // Load lecture
        if(subject_data.contains("lecture")){
            auto& lecture = subject_data["lecture"][0];
            Group group(1, lecture["start_h"], lecture["start_m"], 
                lecture["end_h"], lecture["end_m"], lecture["day"]);
            group.teacher = lecture["teacher"];
            group.place = lecture["place"];
            group.online = lecture["online"];
            group.lecture = true;
            sub.addLecture(group);
        }
        selected_subjects.push_back(1);
        subjects.push_back(sub);
    }
    // Background calculations

    // Sorting groups by day of the week
    SortGroupsByDay();

    // Sorting groups in week by their start time
    BubblesortGroups();

    // Indexing groups and calculating tile width
    CheckCollisions();
    dataLoaded = true;
}

void WindowClass::Clear(){
    // Clear all group pointers
    for(auto& day : groupsInDay){
        day.clear();
    }
    // Clear subjects
    subjects.clear();

    // Clear active subjects
    selected_subjects.clear();

    // Set selected group to nullptr
    selectedGroup = nullptr;
}

void WindowClass::SaveToFile(std::string filename){
    
}

void WindowClass::DrawSavePopup(){
    const auto esc_pressed = ImGui::IsKeyPressed(ImGuiKey_Escape);

    ImGui::SetNextWindowPos(ImVec2(
        ImGui::GetIO().DisplaySize.x/2.0f - popUpSize.x/2.0f,
        ImGui::GetIO().DisplaySize.y/2.0f - popUpSize.y/2.0f));
    ImGui::SetNextWindowSize(popUpSize);
    if(ImGui::BeginPopupModal("Save json", nullptr, popUpFlags)){

        ImGui::InputText("Filename", filenameBuffer,
             sizeof(filenameBuffer));
        if(ImGui::Button("Save", popUpButtonSize)){
            SaveToFile(filenameBuffer);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel", popUpButtonSize) || esc_pressed){
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
void WindowClass::DrawLoadPopup(){
    const auto esc_pressed = ImGui::IsKeyPressed(ImGuiKey_Escape);

    ImGui::SetNextWindowPos(ImVec2(
        ImGui::GetIO().DisplaySize.x/2.0f - popUpSize.x/2.0f,
        ImGui::GetIO().DisplaySize.y/2.0f - popUpSize.y/2.0f));
    ImGui::SetNextWindowSize(popUpSize);
    if(ImGui::BeginPopupModal("Load json", nullptr, popUpFlags)){

        ImGui::InputText("Filename", filenameBuffer,
             sizeof(filenameBuffer));
        if(ImGui::Button("Load", popUpButtonSize)){
            LoadFromFile(filenameBuffer);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel", popUpButtonSize) || esc_pressed){
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void WindowClass::PrintGroups(){
    for(int i = 0; i < num_days; i++){
        std::cout << "================" << std::endl;
        std::cout << weekDays[i] << std::endl;
        for(const auto* group : groupsInDay[i]){
            group->Print();
        }
        std::cout << "================" << std::endl;
    }
}

void WindowClass::SortGroupsByDay(){
    for(auto& sub : subjects){
        std::cout << sub.getName() << std::endl;
        for (int i = 0; i < sub.getGroupsNumber(); ++i){
            auto& group = sub.getGroup(i);
            group.subject = &sub;
            const auto day = group.weekDay;
            groupsInDay[day].push_back(&group);
        }
        auto& lecture = sub.getLecture();
        lecture.subject = &sub;
        const auto day = lecture.weekDay;
        groupsInDay[day].push_back(&lecture);
    }
}

void WindowClass::BubblesortGroups(){
    for(auto i = 0; i < num_days; ++i){
        auto& dayGroups = groupsInDay[i];
        const auto day_number = dayGroups.size();
        if(day_number == 0){
            return;
        }
        bool swaped = 0;
        for(auto j = 0; j < day_number - 1; j++){
            for(auto k = 0; k < day_number - j - 1; k++){
                const auto time1 = dayGroups[k]->startHour * 60 + dayGroups[k]->startMin;
                const auto time2 = dayGroups[k + 1]->startHour * 60 + dayGroups[k + 1]->startMin;
                if(time1 > time2){
                    auto temp = dayGroups[k];
                    dayGroups[k] = dayGroups[k + 1];
                    dayGroups[k + 1] = temp;
                    swaped = 1;
                }
            }
            if(!swaped){
                break;
            }
        }
    }
}

void WindowClass::CheckCollisions(){
    for(auto i = 0; i < num_days; ++i){
        auto& dayGroups = groupsInDay[i];
        const auto day_number = dayGroups.size();
        if(day_number == 0) continue;

        for(auto j = 0; j < day_number; ++j){
            dayGroups[j]->renderIndex = 0;
            dayGroups[j]->renderWidth = 1.0f;
        }

        auto block_start = 0u;
        while(block_start < day_number){
            auto block_end = block_start + 1u;

            Group* first = dayGroups[block_start];
            auto block_max_end = first->endHour * 60 + first->endMin;

            while(block_end < day_number){
                Group* g = dayGroups[block_end];
                auto start_time = g->startHour * 60 + g->startMin;
                auto end_time   = g->endHour * 60 + g->endMin;

                if(start_time >= block_max_end){
                    break;
                }

                if(end_time > block_max_end){
                    block_max_end = end_time;
                }
                block_end++;
            }

            std::vector<int> columns_end;

            for(auto j = block_start; j < block_end; ++j){
                Group* group = dayGroups[j];
                auto start_time = group->startHour * 60 + group->startMin;
                auto end_time   = group->endHour * 60 + group->endMin;

                int chosen_col = -1;

                for(auto c = 0; c < columns_end.size(); ++c){
                    if(columns_end[c] <= start_time){
                        chosen_col = (int)c;
                        break;
                    }
                }

                if(chosen_col == -1){
                    chosen_col = (int)columns_end.size();
                    columns_end.push_back(end_time);
                } else {
                    columns_end[chosen_col] = end_time;
                }

                group->renderIndex = chosen_col;
            }

            const auto col_count = (int)columns_end.size();
            const float w = 1.0f / static_cast<float>(col_count);

            for(auto j = block_start; j < block_end; ++j){
                dayGroups[j]->renderWidth = w;
            }

            block_start = block_end;
        }
    }
}

int WindowClass::GetSubjectId(Subject *subject){
    for(auto i = 0; i < subjects.size(); ++i){
        if(subjects[i] == *subject){
            return i;
        }
    }
    return 0;
}

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
        PrintGroups();
    }
}

void WindowClass::DrawParameters(){
    if(dataLoaded){
        const auto parameters_height = 1.0f/5.0f*ImGui::GetWindowSize().y;
        const auto parameters_width = 1.0f/3.0f*ImGui::GetWindowSize().x;
        ImVec2 parameters_size(parameters_width, parameters_height);
        ImGui::BeginChild("Subjects", parameters_size, true);
        ImGui::Text("Select Subjects");
            ImGui::BeginChild("Subjects-selector");
            for(auto i = 0; i < subjects.size(); i++){
                std::string subject_name = subjects[i].getName();
                ImGui::Checkbox(subject_name.data(), reinterpret_cast<bool*>(&selected_subjects[i]));
            }

            ImGui::EndChild();
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("Selecting", parameters_size, true);
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
    const auto row_height = calendar_height/(end_hour - start_hour);

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
        for(int i = 0; i < num_days; ++i){
            const auto day_name = weekDays[i];

            ImGui::SameLine();
            if(ImGui::BeginChild(day_name.data(), ImVec2(day_width, day_height), false)){
                ImGui::Text(day_name.data());
            }
            auto calendar_start_y = ImGui::GetCursorPosY();
            auto calendar_start_x = ImGui::GetCursorPosX();
            auto calendar_end_y = calendar_start_y + calendar_height;


            for(auto& group : groupsInDay[i]){
                std::string name = group->subject->getName() + " gr. " + std::to_string(group->number);
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

                ImGui::SetCursorPosY(pos_y + tile_margin/2);
                ImGui::SetCursorPosX(pos_x + tile_margin/2);
                ImGui::BeginChild(name.data(), ImVec2(width - tile_margin, height - tile_margin), ImGuiChildFlags_Border);
                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                ImVec4 bgColor = ImVec4(0.1f, 0.1f, 0.1f, 0.5f);

                if(selected){
                    bgColor = ImVec4(0.2f, 0.3f, 0.4f, 1.0f);
                }

                ImVec2 p = ImGui::GetWindowPos();
                ImVec2 s = ImGui::GetWindowSize();
                draw_list->AddRectFilled(p, ImVec2(p.x + s.x, p.y + s.y), ImGui::ColorConvertFloat4ToU32(bgColor));

                if(!selected){
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                }
                ImGui::TextWrapped(name.data());
                if(!selected){
                    ImGui::PopStyleColor();
                }
                ImGui::EndChild();

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
    for(auto& day : groupsInDay){
        day.clear();
    }
    subjects.clear();
    selected_subjects.clear();


    int iter = 0;
    for (auto& [subject_name, subject_data] : loaded_data.items()) {
        Subject sub(subject_name);
        if(subject_data.contains("groups")){
            auto& groups = subject_data["groups"];
            for (auto& [group_id, group_datas] : groups.items()) {
                const auto& group_data = group_datas[0];
                int id = std::stoi(group_id);
                Group group(id, group_data["start_h"], group_data["start_m"], 
                    group_data["end_h"], group_data["end_m"], group_data["day"]);
                group.teacher = group_data["teacher"];
                group.teacher = group_data["place"];
                sub.addGroup(group);
            }
        }
        if(subject_data.contains("lecture")){
            auto& lecture = subject_data["lecture"][0];
            Group group(1, lecture["start_h"], lecture["start_m"], 
                lecture["end_h"], lecture["end_m"], lecture["day"]);
            group.teacher = lecture["teacher"];
            group.teacher = lecture["place"];
            group.online = lecture["online"];
            sub.addLecture(group);
        }
        if((iter % 2) == 0){
            selected_subjects.push_back(0);

        }else{
            selected_subjects.push_back(1);
        }

        subjects.push_back(sub);

        iter++;
    }
    SortGroupsByDay();
    BubblesortGroups();
    CheckCollisions();
    dataLoaded = true;
}

void WindowClass::Clear(){
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

        std::vector<Group*> active;
        int max_active = 1;
        for(auto j = 0; j < day_number; ++j){
            Group* group = dayGroups[j];
            auto start_time = group->startHour * 60 + group->startMin;
            std::vector<Group*> active_temp;
            for(auto k = 0; k < active.size(); k++){
                Group* g = active[k];
                auto end_time = g->endHour * 60 + g->endMin;
                if(end_time > start_time){
                    active_temp.push_back(g);
                }
            }
            active = active_temp;

            active.push_back(group);

            auto count = active.size();
            if (count > max_active){
                max_active = count;
            }
            for(auto k = 0; k < active.size(); k++){
                if(active[k]->renderIndex < k){
                    active[k]->renderIndex = k;
                }
            }
        }
        for(auto j = 0; j < day_number; ++j){
            Group* group = dayGroups[j];
            group->renderWidth = 1.0f/static_cast<float>(max_active);
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

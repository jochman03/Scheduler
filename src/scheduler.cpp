#include "scheduler.hpp"
 
#include <vector>
#include <memory>
#include <string>
#include <iostream>

Group& Subject::getGroup(int index){
   return groups[index];
}

int Subject::getGroupsNumber() const{
    return groups.size();
}

void Subject::addGroup(Group group){
    groups.push_back(group);
}

void Subject::addLecture(Group group){
    lecture = group;
}

void Subject::setFixedGroup(Group *group){
    fixedGroup = group;
}

void Subject::setChosenGroup(Group *group){
    chosenGroup = group;
}

std::string Subject::getName()
{
    return name;
}

Group& Subject::getLecture(){
    return lecture;
}

Group* Subject::getFixedGroup(){
    return fixedGroup;
}

int Subject::getGroupIndex(Group* group){
    for(int i = 0; i < groups.size(); i++){
        if(groups[i] == *group){
            return i;
        }
    }
}

Group* Subject::getChosenGroup(){
    return chosenGroup;
}

void Subject::Print() const{
    std::cout << name << ": " << std::endl;
    for(const auto& group : groups ){
        group.Print();
    }
    std::cout << "Lecture:" <<std::endl;
    lecture.Print();
    std::cout << "---------------" << std::endl;
}

int Subject::nextId = 0;
int Group::nextId = 0;

void Group::Print() const{
    if(subject){
        std::cout << subject->getName() << " Gr. " << number << std::endl;
    }
    else{
        std::cout << "Gr. " << number << std::endl;
    }
    std::cout << weekDay << "  "<< startHour << ":" << startMin << " - " << endHour << ":" << endMin << std::endl;
    std::cout << "Width: " << renderWidth << " Index: " << renderIndex << std::endl;
}



void BubblesortGroups(std::vector<std::vector<Group*>>& groupsInDay){
    const auto num_days = 5;
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

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

std::string Subject::getName(){
    return name;
}

Group &Subject::getLecture(){
    return lecture;
}

int Subject::getChosenGroupIndex(){
    return chosenGroupIndex;
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

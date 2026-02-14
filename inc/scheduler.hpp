#pragma once

#include <vector>
#include <string>

class Subject;

struct Group{
    Group() {};
    Group(int _number, int _startHour, int _startMin, int _endHour, int _endMin, int _weekDay) :
     number(_number), startHour(_startHour), startMin(_startMin), endHour(_endHour), endMin(_endMin), weekDay(_weekDay) {};
    int number;
    int startHour;
    int startMin;
    int endHour;
    int endMin;
    int weekDay;
    std::string place;
    std::string teacher;
    bool online {false};
    void Print() const;
    float renderWidth{1};
    int renderIndex{0};

    Subject* subject;
};

class Subject {
public:
    Subject(const std::string _name) : name(_name) {};
    Group& getGroup(int index);
    int getGroupsNumber() const;
    void addGroup(Group group);
    void addLecture(Group group);
    std::string getName();
    Group& getLecture();
    int getChosenGroupIndex();
    void Print() const;
    bool operator==(const Subject& other) const {
        return other.name == name;
    }
private: 
    std::vector<Group> groups;
    std::string name;
    int chosenGroupIndex{0};
    Group lecture;
};
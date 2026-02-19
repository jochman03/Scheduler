#pragma once

#include <vector>
#include <string>

class Subject;

struct Group{
    Group() {};
    Group(int _number, int _startHour, int _startMin, int _endHour, int _endMin, int _weekDay) :
    number(_number), startHour(_startHour), startMin(_startMin), endHour(_endHour), endMin(_endMin), weekDay(_weekDay) { 
        id = nextId++;
    };
    int id;
    static int nextId;
    int number;
    int startHour;
    int startMin;
    int endHour;
    int endMin;
    int weekDay;
    std::string place;
    std::string teacher;
    bool online {false};
    bool lecture {false};
    void Print() const;
    float renderWidth{1};
    float chosenRenderWidth{1};

    int renderIndex{0};
    int chosenRenderIndex{0};

    Subject* subject;

    bool operator==(const Group& other) const {
        return other.id == id;
    }
};

class Subject {
public:
    Subject(const std::string _name) : name(_name) { 
        id = nextId++;
    };
    int id;
    static int nextId;
    Group& getGroup(int index);
    Group& getLecture();
    Group* getFixedGroup();
    int getGroupIndex(Group* group);

    Group* getChosenGroup();
    void setFixedGroup(Group* group);
    void setChosenGroup(Group* group);

    std::string getName();

    int getGroupsNumber() const;

    void addGroup(Group group);
    void addLecture(Group group);
    void Print() const;
    bool operator==(const Subject& other) const {
        return other.id == id;
    }
private: 
    std::vector<Group> groups;
    std::string name;
    Group* fixedGroup {nullptr};
    Group* chosenGroup {nullptr};
    Group lecture;
};

void BubblesortGroups(std::vector<std::vector<Group*>>& groupsInDay);

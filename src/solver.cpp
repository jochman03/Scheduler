#include <random>
#include <iostream>
#include <limits>

#include "solver.hpp"
#include <set>

static std::mt19937 gen(std::random_device{}());


static double clip(double in, double min, double max){
    if(in > max){
        in = max;
    }
    else if(in < min){
        in = min;
    }
    return in;
}

static int clip(int in, int min, int max){
    if(in > max){
        in = max;
    }
    else if(in < min){
        in = min;
    }
    return in;
}

int &Solver::getMaxIterations(){
    return _maxIterations;
}

int &Solver::getCurrentIteration(){
    return _currentIteration;
}

int &Solver::getMaxRuns(){
    return _maxRuns;
}

int &Solver::getCurrentRun(){
    return _currentRun;
}

void Solver::setMaxIterations(int iterations){
    _maxIterations = iterations;
}

void Solver::setMaxRuns(int runs){
    _maxRuns = runs;
}

void Solver::Init(){
    _running = true;
    _currentIteration = 0;
    _currentTemperature = _startTemperature;
    _currentPosition.clear();

    // generate first random solution
    for(int i = 0; i < _subjects.size(); i++){
        int groupNumber = _subjects[i]->getGroupsNumber();
        if(_subjects[i]->getFixedGroup() != nullptr){
            int index = _subjects[i]->getGroupIndex(_subjects[i]->getFixedGroup());
            _currentPosition.push_back(index);
            continue;
        }
        int random_value = static_cast<int>(std::round(_realDist(gen) * (groupNumber - 1)));
        _currentPosition.push_back(random_value);
    }
    double value = costFun(_currentPosition);
    _currentValue = value;
}

bool Solver::isRunning(){
    return _running;
}

bool Solver::hasSolution(){
    return _solution;
}

void Solver::setSubjects(std::vector<Subject *> subjects){
    _subjects = subjects;
    _dimensionNumber = subjects.size();
}

void Solver::getSolution(){
    std::vector<int> temp = _currentPosition;
    if(!_bestPosition.empty()){
        temp = _bestPosition;
    }
    for(int i = 0; i < temp.size(); i++){
        if(_subjects[i]->getGroupsNumber() == 0){
            continue;
        }
        Group& group = _subjects[i]->getGroup(temp[i]);
        _subjects[i]->setChosenGroup(&group);
    }
}

void Solver::setIncludeLectures(bool include){
    _includeLectures = include;
}

void Solver::stepIteration(){
    if(_currentIteration >= _maxIterations){
        if(_currentValue <= _bestValue){
            _bestValue = _currentValue;
            _bestPosition = _currentPosition;
        }
        if(_currentRun >= _maxRuns){
            _running = false;    
            _solution = true;
            return;
        }
        _currentRun++;
        Init();
    }
    double method = _realDist(gen) * 100.0;

    double k = 1 + (_currentTemperature / _startTemperature) * (_kMax - 1);

    std::vector<int> new_position = _currentPosition;

    int help_poor_soul = _currentValue/_overlappingPenalty * 10;

    if(method > (60-help_poor_soul)){
        // Change two values
        _indexDist = std::uniform_int_distribution<int>(0, _dimensionNumber - 1);
        int index1 = _indexDist(gen);
        int index2 = _indexDist(gen);

        double delta1 = ((_realDist(gen)*2.0) - 1) * k;
        double delta2 = ((_realDist(gen)*2.0) - 1) * k;

        if(_subjects[index1]->getFixedGroup() == nullptr){
            int value1 = static_cast<int>(std::round(new_position[index1] + delta1));
            int maxValue1 = _subjects[index1]->getGroupsNumber() - 1;
            new_position[index1] = clip(value1, 0, maxValue1);
        }
        if(_subjects[index2]->getFixedGroup() == nullptr){
            int value2 = static_cast<int>(std::round(new_position[index2] + delta2));
            int maxValue2 = _subjects[index2]->getGroupsNumber() - 1;
            new_position[index2] = clip(value2, 0, maxValue2);
        }
    }
    else{
        // Change one value
        _indexDist = std::uniform_int_distribution<int>(0, _dimensionNumber - 1);
        int index = _indexDist(gen);
        index = clip(index, 0, _dimensionNumber-1);
        if(_subjects[index]->getFixedGroup() == nullptr){
            double delta = ((_realDist(gen)*2.0) - 1) * k;
            int value = static_cast<int>(std::round(new_position[index] + delta));
            int maxValue = _subjects[index]->getGroupsNumber() - 1;
            new_position[index] = clip(value, 0, maxValue);
        }
    }
    double new_value = costFun(new_position);
    double delta = new_value - _currentValue;
    if(delta < 0){
        _currentPosition = new_position;
        _currentValue = new_value;
    }
    else{
        double random_acceptance = _realDist(gen);
        double temperature_coefficient = std::exp(-delta/_currentTemperature);
        if(random_acceptance < temperature_coefficient){
            _currentPosition = new_position;
            _currentValue = new_value;
        }
    }
    //std::cout << "TEMP: " << _currentTemperature << "\n";
    _currentTemperature = _alpha * _currentTemperature;
    _currentIteration ++;
}

void Solver::reset(){
    _subjects.clear();

    _running = false;
    _solution = false;
    _currentRun = 0;
    _currentIteration = 0;
    _currentPosition.clear();
    _currentValue = std::numeric_limits<double>::max();
    _bestPosition.clear();
    _bestValue = std::numeric_limits<double>::max();
}

double Solver::costFun(std::vector<int>& position){
    const int days_number = 5;
    std::vector<std::vector<Group*>> groupsInDay (days_number);
    
    for(int i = 0; i < _dimensionNumber; ++i){
        int groupsNumber = _subjects[i]->getGroupsNumber();

        if(groupsNumber != 0){
            Group& group = _subjects[i]->getGroup(position[i]);
            groupsInDay[group.weekDay].push_back(&group);
        }
        if(_includeLectures){
            Group& lecture = _subjects[i]->getLecture();
            if(!lecture.online){
                groupsInDay[lecture.weekDay].push_back(&lecture);
            }
        }
    }

    BubblesortGroups(groupsInDay);

    int collisions{0};
    int days{0};
    int time{0};

   for(int i = 0; i < days_number; ++i){
        if (groupsInDay[i].size() == 0){
            continue;
        }
        days++;
        Group* first_group = groupsInDay[i][0];
        Group* last_group = groupsInDay[i][groupsInDay[i].size() - 1];

        if(first_group && last_group){
            int start_time = first_group->startHour * 60 + first_group->startMin;
            int end_time = last_group->endHour * 60 + last_group->endMin;
            time += end_time - start_time;
        }

        for(int j = 0; j < groupsInDay[i].size(); ++j){
            Group* group = groupsInDay[i][j];

            for(int k = j + 1; k < groupsInDay[i].size(); ++k){

                auto g1 = groupsInDay[i][j];
                auto g2 = groupsInDay[i][k];

                int s1 = g1->startHour * 60 + g1->startMin;
                int e1 = g1->endHour * 60 + g1->endMin;
                int s2 = g2->startHour * 60 + g2->startMin;
                int e2 = g2->endHour * 60 + g2->endMin;

                if(s1 < e2 && s2 < e1){
                    collisions++;
                }
            }
        }
    }

    // for(int i = 0; i < days_number; ++i){
    //     std::cout << i << " DAY \n";
    //     for(auto& group : groupsInDay[i]){
    //         std::cout << group->subject->getName();
    //         if(group->lecture){
    //             std::cout << " Lecture";
    //         }
    //         std::cout << ",\n";
    //     }
    //     std::cout <<  "\n";
    // }


    double colissions_penalty = static_cast<double>(collisions) * _overlappingPenalty;
    double time_penalty = static_cast<double>(time) / (13.0 * 60.0 * 5.0) * _timePenalty;
    double days_penalty = static_cast<double>(days) / 5.0 * _dayPenalty;
    //std::cout << colissions_penalty << ", " << time_penalty << ", " << days_penalty << "\n";
    return colissions_penalty + time_penalty + days_penalty;
}

void Solver::setStartTemperature(double temperature){
    _startTemperature = temperature;
}

void Solver::setAlpha(double alpha){
    _alpha = alpha;
}

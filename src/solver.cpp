#include <random>
#include <iostream>
#include <limits>

#include "solver.hpp"
#include <set>

static int round_double(double num, int max, int min = 0){
    int ret = static_cast<int>(std::round(num));
    ret = std::min(ret, max);
    ret = std::max(ret, min);
    return ret;
}

int &Solver::getParticles(){
    return _particles;
}

int &Solver::getMaxIterations(){
    return _maxIterations;
}

int &Solver::getCurrentIteration(){
    return _currentIteration;
}

void Solver::setParticles(int particles){
    _particles = particles;
}

void Solver::setMaxIterations(int iterations){
    _maxIterations = iterations;
}

void Solver::Init(){
    Particle::resetBestValue();
    particles.clear();
    int dimensionsNumber = _subjects.size();
    for(int i = 0; i < _particles; ++i){
        Particle p(dimensionsNumber, _subjects);
        p._includeLectures = _includeLectures;
        particles.push_back(p);
    }
    for(int i = 0; i < _particles; ++i){
        particles[i].Init();
    }
    _currentIteration = 0;
    _running = true;
    _solution = false;
}

bool Solver::isRunning(){
    return _running;
}

bool Solver::hasSolution(){
    return _solution;
}

void Solver::setSubjects(std::vector<Subject *> subjects){
    _subjects = subjects;
}

void Solver::getSolution(){
    int dimensionsNumber = _subjects.size();
    _chosenGroups = particles[0].getSolution();
    for(auto gr : _chosenGroups){
        gr->subject->setChosenGroup(gr);
    }
}

void Solver::setIncludeLectures(bool include){
    _includeLectures = true;
}

void Solver::stepIteration(){
    if(_currentIteration >= _maxIterations){
        _running = false;    
        _solution = true;
        return;
    }
    for(int i = 0; i < _particles; i++){
        particles[i].calculatePosition();
    }
    _currentIteration ++;
}

void Solver::reset(){
    _chosenGroups.clear();
    _subjects.clear();
    particles.clear();

    _currentIteration = 0;
    _running = false;
    _solution = false;
}

Particle::Particle(const Particle &p){
    _position = p._position;
    _velocity = p._velocity;
    _subjects = p._subjects;
    _bestPosition = p._bestPosition;

    _bestValue = p._bestValue; 
    _dimensionNumber = p._dimensionNumber;
}

Particle::Particle(){

}

Particle::~Particle(){
    _subjects.clear();
    _position.clear();
    _bestPosition.clear();
    _velocity.clear();
}

std::vector<double> Particle::_globalBestPosition {};

double Particle::_globalBestValue = std::numeric_limits<double>::max();

void Particle::Init(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(0, 1);
    for(int i = 0; i < _dimensionNumber; i++){
        if(!_subjects[i]){
            // Throw some exeption
            return;
        }
        Group* fixed_group = _subjects[i]->getFixedGroup();
        if(fixed_group != nullptr){
            _position[i] = static_cast<double>(_subjects[i]->getGroupIndex(fixed_group));
            _velocity[i] = 0.0;
            continue;
        }
        int groupsNumber = _subjects[i]->getGroupsNumber();
        _position[i] = dist(gen) * groupsNumber;
        _velocity[i] = ((dist(gen)*2.0) - 1.0) * _maxVelocity;
    }
    
    _bestPosition = _position;
    _bestValue = costFun();
    if(_bestValue < _globalBestValue){
        _globalBestValue = _bestValue;
        _globalBestPosition = _bestPosition;
    }
}

double Particle::calculatePosition(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(0, 1);
    
    for(int i = 0; i < _dimensionNumber; i++){
        if(_subjects[i]->getFixedGroup() != nullptr){
            continue;
        }
        _velocity[i] = _inertion * _velocity[i] 
        + _localBestAttraction * dist(gen) * (_bestPosition[i] - _position[i])
        + _globalBestAttraction * dist(gen) * (_globalBestPosition[i] - _position[i]);

        _position[i] = _position[i] + _velocity[i];
        
        int groupsNumber = _subjects[i]->getGroupsNumber();
        if(_position[i] < 0){
            _position[i] = 0;
            _velocity[i] = -_velocity[i];
        }
        else if(_position[i] > groupsNumber - 1){
            _position[i] = static_cast<double>(groupsNumber - 1);
            _velocity[i] = -_velocity[i];
        }
    }
    double value = costFun();
    if(value < _bestValue){
        _bestValue = value;
        _bestPosition = _position;
    }
    if(value < _globalBestValue){
        _globalBestValue = value;
        _globalBestPosition = _position;
    }
    return value;
}

double Particle::costFun(){
    const int days_number = 5;
    std::vector<int> current_position (_dimensionNumber);
    std::vector<std::vector<Group*>> groupsInDay (days_number);

    // Position correction
    for(int i = 0; i < _dimensionNumber; ++i){
        int groupsNumber = _subjects[i]->getGroupsNumber();

        int pos = round_double(_position[i], groupsNumber - 1, 0);
        current_position[i] = pos;

        if(_subjects[i]->getGroupsNumber() != 0){
            Group& group = _subjects[i]->getGroup(current_position[i]);
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
        for(int j = 0; j < groupsInDay[i].size(); ++j){
            Group* group = groupsInDay[i][j];

            int startTime1 = group->startHour * 60 + group->startMin;
            int endTime1 = group->endHour * 60 + group->endMin;

            time += endTime1 - startTime1;

            if(j != groupsInDay[i].size() - 1){
                Group* next_group = groupsInDay[i][j + 1];

                int startTime2 = next_group->startHour * 60 + next_group->startMin;
                int endTime2 = next_group->endHour * 60 + next_group->endMin;

                // Overlapping
                if(startTime1 < endTime2 && startTime2 < endTime1){
                    collisions++;
                }
                else{
                    time += startTime2 - endTime1;
                }
            }
        }
    }

    // Penalty
    double colissions_penalty = static_cast<double>(collisions) * _overlappingPenalty;
    double time_penalty = static_cast<double>(time) / 1000.0 * _timePenalty;
    double days_penalty = static_cast<double>(days) / 5.0 * _dayPenalty;

    return colissions_penalty + time_penalty + days_penalty;
}

std::vector<Group *> Particle::getSolution(){
    std::vector<Group*> groups;
    for(int i = 0; i < _dimensionNumber; i++){
        const int global_best_pos = round_double(_globalBestPosition[i], _subjects[i]->getGroupsNumber() - 1, 0);
        if(_subjects[i]->getGroupsNumber() != 0){
            Group* group = &(_subjects[i]->getGroup(global_best_pos));
            groups.push_back(group);
        }
    }
    return groups;
}


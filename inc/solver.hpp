#pragma once

#include <vector>
#include "scheduler.hpp"

struct Particle{
    Particle(int dimension_number, std::vector<Subject*> subjects) : _dimensionNumber(dimension_number), _position(dimension_number), 
    _velocity(dimension_number), _subjects(subjects) {};
    Particle(const Particle& p);
    Particle();
    ~Particle();

    static std::vector<double> _globalBestPosition;
    static double _globalBestValue;
    static double getBestValue(){
        return _globalBestValue;
    }
    static void resetBestValue(){
        _globalBestValue  = std::numeric_limits<double>::max();
        _globalBestPosition = {};
    }
    std::vector<double> _position;
    std::vector<double> _velocity;
    std::vector<Subject*> _subjects;
    std::vector<double> _bestPosition;

    double _bestValue;
    int _dimensionNumber;

    double _maxVelocity {1};
    
    void Init();
    double calculatePosition();
    double costFun();
    
    std::vector<Group*> getSolution();

    // Penalty parameters
    const double _overlappingPenalty {1e7};
    const double _dayPenalty {1e5};
    const double _timePenalty {1e1};

    // Velocity calculation parameters
    const double _inertion {0.7};
    const double _localBestAttraction {1.5};
    const double _globalBestAttraction {1.5};

    bool _includeLectures{false};
};

class Solver{
private:
    int _particles {100};
    int _maxIterations {100};
    int _currentIteration {0};
    bool _running {false};
    bool _solution {false};
    bool _includeLectures{false};

    std::vector<Group*> _chosenGroups;
    std::vector<Subject*> _subjects;
    std::vector<Particle> particles;
public:
    int& getParticles();
    int& getMaxIterations();
    int& getCurrentIteration();
    
    void setParticles(int particles);
    void setMaxIterations(int iterations);
    void setSubjects(std::vector<Subject*> _subjects);
    void getSolution();
    void setIncludeLectures(bool include);
    void Init();
    bool isRunning();
    bool hasSolution();
    void stepIteration();
    void reset();
};


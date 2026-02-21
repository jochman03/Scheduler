#pragma once

#include <vector>
#include "scheduler.hpp"
#include <random>

class Solver{
private:
    int _maxIterations {100};
    int _currentIteration {0};
    int _currentRun {0};
    int _maxRuns{10};

    bool _running {false};
    bool _solution {false};
    bool _includeLectures{false};
    int _dimensionNumber {0};

    double _startTemperature {1000};
    double _currentTemperature {0};
    double _alpha {0.95};

    double _overlappingPenalty {1e2};
    double _timePenalty {2e2};
    double _dayPenalty {1e2};

    double _kMax {3};

    int _wastedTime {0};
    int _collisions {0};
    int _wastedDays {0};

    std::vector<int> _currentPosition;
    double _currentValue {0};

    std::vector<int> _bestPosition;
    double _bestValue {0};

    std::vector<Subject*> _subjects;

    std::uniform_real_distribution<double> _realDist{0.0, 1.0};
    std::uniform_int_distribution<int> _indexDist;
    std::uniform_int_distribution<int> _signDist{-1, 1};

public:
    int& getMaxIterations();
    int& getCurrentIteration();
    int& getMaxRuns();
    int& getCurrentRun();
    void setStartTemperature(double temperature);
    void setAlpha(double alpha);

    void setMaxIterations(int iterations);
    void setMaxRuns(int runs);
    void setSubjects(std::vector<Subject*> _subjects);
    void getSolution();
    void setIncludeLectures(bool include);
    void Init();
    bool isRunning();
    bool hasSolution();
    void stepIteration();
    void reset();

    double costFun(std::vector<int>& position);
}; 

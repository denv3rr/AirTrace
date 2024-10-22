#ifndef GPS_ALGORITHM_H
#define GPS_ALGORITHM_H

#include "PathCalculator.h"
#include "Object.h"
#include <utility>

class GPSAlgorithm : public PathCalculator
{
public:
    // Constructor
    GPSAlgorithm();

    // Sets GPS coordinates manually (for now, this will simulate a GPS update)
    void setGPSData(std::pair<int, int> gpsData);

    // Override calculatePath function
    std::pair<int, int> calculatePath(const Object &follower, const Object &target) override;

private:
    std::pair<int, int> gpsCoordinates;
};

#endif // GPS_ALGORITHM_H
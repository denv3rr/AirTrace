#include "core/GPSAlgorithm.h"

// Constructor
GPSAlgorithm::GPSAlgorithm() : gpsCoordinates({0, 0}) {}

// Set GPS data (can be replaced with API-based updates in the future)
void GPSAlgorithm::setGPSData(std::pair<int, int> gpsData)
{
    gpsCoordinates = gpsData;
}

// Calculate new path based on the GPS coordinates
std::pair<int, int> GPSAlgorithm::calculatePath(const Object &follower, const Object &target)
{
    auto targetPos = target.getPosition();
    auto followerPos = follower.getPosition();

    std::pair<int, int> gpsTarget = gpsCoordinates;
    if (gpsTarget == std::pair<int, int>({0, 0}))
    {
        gpsTarget = targetPos;
    }

    // Move the follower closer to the GPS-provided location
    int newX = followerPos.first + (gpsTarget.first - followerPos.first) / 2; // Moving halfway for smoother progression
    int newY = followerPos.second + (gpsTarget.second - followerPos.second) / 2;

    return {newX, newY};
}

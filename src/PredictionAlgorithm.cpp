#include "PredictionAlgorithm.h"

std::pair<int, int> PredictionAlgorithm::calculatePath(const Object &follower, const Object &target)
{
    auto predictedPosition = predictFuturePosition(target);

    // Calculate follower's new position: Move towards predicted position
    auto followerPos = follower.getPosition();
    int newX = followerPos.first + (predictedPosition.first - followerPos.first) / 2;
    int newY = followerPos.second + (predictedPosition.second - followerPos.second) / 2;

    return {newX, newY};
}

std::pair<int, int> PredictionAlgorithm::predictFuturePosition(const Object &target)
{
    auto targetPos = target.getPosition();

    // Predict based on speed or direction - simplistic example
    int speedX = 1; // Assume speed or derived from the object's behavior
    int speedY = 1;

    // Predict future position (this can be replaced with more sophisticated models like Kalman filter)
    int futureX = targetPos.first + speedX * 2; // Predict two steps ahead
    int futureY = targetPos.second + speedY * 2;

    return {futureX, futureY};
}

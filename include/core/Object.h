#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <tuple>
#include <utility>

class Object
{
public:
    Object(int id, const std::string &name, const std::pair<int, int> &position);
    Object(int id, const std::string &name, const std::tuple<int, int, int> &position);

    void moveTo(const std::pair<int, int> &newPosition);     // Move to new 2D position
    void moveTo(const std::tuple<int, int, int> &newPosition); // Move to new 3D position
    std::pair<int, int> getPosition() const;                 // Get current 2D position
    std::tuple<int, int, int> getPosition3D() const;           // Get current 3D position

    float getHeatLevel() const;
    void setHeatLevel(float heatLevel);

    int getId() const;
    std::string getName() const;

private:
    int id;
    std::string name;
    std::tuple<int, int, int> position3D; // 3D position as (x, y, z)
    float heatLevel;
};

#endif // OBJECT_H


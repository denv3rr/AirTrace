#include "core/Object.h"

// Constructor initializes ID, name, and 2D position (z defaults to 0)
Object::Object(int id, const std::string &name, const std::pair<int, int> &position)
    : id(id), name(name), position3D(position.first, position.second, 0), heatLevel(100.0f) {}

// Constructor initializes ID, name, and 3D position
Object::Object(int id, const std::string &name, const std::tuple<int, int, int> &position)
    : id(id), name(name), position3D(position), heatLevel(100.0f) {}

// Moves object to a new 3D position
void Object::moveTo(const std::tuple<int, int, int> &newPosition)
{
    position3D = newPosition;
}

// Moves object to a new 2D position (z preserved)
void Object::moveTo(const std::pair<int, int> &newPosition)
{
    position3D = std::make_tuple(newPosition.first, newPosition.second, std::get<2>(position3D));
}

// Returns the current 2D position of the object
std::pair<int, int> Object::getPosition() const
{
    return {std::get<0>(position3D), std::get<1>(position3D)};
}

// Returns the current 3D position of the object
std::tuple<int, int, int> Object::getPosition3D() const
{
    return position3D;
}

float Object::getHeatLevel() const
{
    return heatLevel;
}

void Object::setHeatLevel(float heatLevel)
{
    this->heatLevel = heatLevel;
}

// Returns the object's ID
int Object::getId() const
{
    return id;
}

// Returns the object's name
std::string Object::getName() const
{
    return name;
}


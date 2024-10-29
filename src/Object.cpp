#include "Object.h"

// Constructor initializes ID, name, and 3D position
Object::Object(int id, const std::string &name, const std::tuple<int, int, int> &position)
    : id(id), name(name), position3D(position) {}

// Moves object to a new 3D position
void Object::moveTo(const std::tuple<int, int, int> &newPosition)
{
    position3D = newPosition;
}

// Returns the current 3D position of the object
std::tuple<int, int, int> Object::getPosition3D() const
{
    return position3D;
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

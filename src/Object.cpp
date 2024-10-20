#include "Object.h"

Object::Object(int id, const std::string &name, std::pair<int, int> position)
    : id(id), name(name), position(position) {}

void Object::moveTo(const std::pair<int, int> &newPosition)
{
    position = newPosition;
}

std::pair<int, int> Object::getPosition() const
{
    return position;
}

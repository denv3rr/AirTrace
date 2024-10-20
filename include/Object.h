#ifndef OBJECT_H
#define OBJECT_H

#include <utility>
#include <string>

class Object
{
public:
    Object(int id, const std::string &name, std::pair<int, int> position);
    void moveTo(const std::pair<int, int> &newPosition);
    std::pair<int, int> getPosition() const;

private:
    int id;
    std::string name;
    std::pair<int, int> position;
};

#endif // OBJECT_H

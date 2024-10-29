#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <tuple>
#include <utility>

class Object
{
public:
    Object(int id, const std::string &name, const std::tuple<int, int, int> &position);

    void moveTo(const std::tuple<int, int, int> &newPosition); // Move to new 3D position
    std::tuple<int, int, int> getPosition3D() const;           // Get current 3D position

    int getId() const;
    std::string getName() const;

private:
    int id;
    std::string name;
    std::tuple<int, int, int> position3D; // 3D position as (x, y, z)
};

#endif // OBJECT_H

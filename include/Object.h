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

    // TEST getHeatLevel function
    float getHeatLevel() const
    {
        return 25.0f; // Arbitrary value; replace with other simulation logic as needed for testing
    }

private:
    int id;
    std::string name;
    std::pair<int, int> position;
};

#endif // OBJECT_H

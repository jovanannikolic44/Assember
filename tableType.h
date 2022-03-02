#include <iostream>
using namespace std;
class tableType {
public:
    int id;
    string name;
    string section;
    int value;
    char scope;
    bool isDefined;
    bool isSection;

public:
    tableType(int id, string name, string section, int value, char scope, bool isSection) {
        this->id = id;
        this->name = name;
        this->section = section;
        this->value = value;
        this->scope = scope;
        this->isDefined = false;
        this->isSection = isSection;
    }

    bool operator <(const tableType &tableObj) const
    {
        return id < tableObj.id;
    }
};

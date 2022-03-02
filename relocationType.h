#include <iostream>
using namespace std;

class RelocationTableType {
private:
    int offset;
    int symb_id;
    string rel_type;
    string sectionName;
public:
    RelocationTableType(int offset, string rel_type, int symb_id, string sectionName){
        this->offset = offset;
        this->rel_type = rel_type;
        this->symb_id = symb_id;
        this->sectionName = sectionName;
    }

    int getOffset() {
        return offset;
    }

    void setOffset(int offset) {
        this->offset = offset;
    }

    int getSymbId() {
        return symb_id;
    }

    void setSymbId(int symb_id) {
        this->symb_id = symb_id;
    }

    string getRelType() {
        return rel_type;
    }

    void setRelType(string rel_type) {
        this->rel_type = rel_type;
    }

    void setSectionName(string sectionName) {
        this->sectionName = sectionName;
    }

    string getSectionName() {
        return sectionName;
    }
};

class RelocationCodeType {
private:
    string sectionName;
public:
    list<string> data;

public:
    RelocationCodeType(string sectionName) {
        this->sectionName = sectionName;
    }

    string getSectionName() {
        return sectionName;
    }

    void setSectionName(string name) {
        sectionName = name;
    }

};
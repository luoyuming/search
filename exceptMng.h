#pragma once
#include "common.h"

class CExceptMng {

public:
    CExceptMng();
    ~CExceptMng();
    bool checkExcept(string & domain, const string & url);
private:
    std::set<string> setFlag;
};
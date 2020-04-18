#include "exceptMng.h"


CExceptMng::CExceptMng()
{
   
    setFlag.insert("/blog");
    setFlag.insert("/magazines");
    setFlag.insert("/topic");
    setFlag.insert("/forums");
}
CExceptMng::~CExceptMng() {

}

bool CExceptMng::checkExcept(string & domain, const string & url)
{
    bool bResult = false;
    if ("www.iteye.com" == domain) {
        bResult = true;
        
        for (auto & item : setFlag) {
           auto pos = url.find(item);
           if (string::npos != pos) {
               bResult = false;
               break;
           }
        }
    }
    return bResult;
}
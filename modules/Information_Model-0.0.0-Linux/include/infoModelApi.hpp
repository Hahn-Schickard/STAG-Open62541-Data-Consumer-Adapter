#ifndef INFOMODELAPI
#define INFOMODELAPI

#include <string>
#include "valueDataType.hpp"


class InfoModelApi
{    
    int Update(std::string refId, ValueWrapper valueWraper);
};

#endif
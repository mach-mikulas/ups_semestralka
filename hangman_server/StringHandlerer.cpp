//
// Created by machm on 21.12.2023.
//

#include "StringHandlerer.h"

std::string StringHandlerer::removeEndl(std::string str){
    if (!str.empty() && str[str.length()-1] == '\n') {
        str.erase(str.length()-1);
    }

    return str;
}
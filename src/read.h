//
// Created by danylo.kolinko on 5/11/19.
//

#ifndef LAB4_READ_H
#define LAB4_READ_H

#include <sstream>
#include <vector>
#include <map>
#include "read.h"

struct Reader{
    std::stringstream read_txt(std::string & address);
    std::stringstream read_zip(std::string & address);
    bool is_txt(const std::string& f);
    bool is_archive(const std::string& f);

};


#endif //LAB4_READ_H

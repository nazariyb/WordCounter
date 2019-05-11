#ifndef LAB4_READ_H
#define LAB4_READ_H

#include <sstream>
#include <vector>
#include <map>
#include "read.h"

struct Reader{
    static void read_txt (std::string &address, std::stringstream &ss);

    static void read_archive (std::string &address, std::stringstream &ss);

    static bool is_txt (const std::string &f);

    static bool is_archive (const std::string &f);

};


#endif //LAB4_READ_H

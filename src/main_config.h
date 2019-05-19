#ifndef LAB4_MAIN_CONFIG_H
#define LAB4_MAIN_CONFIG_H

#include <map>
#include <string>
#include <vector>

enum Error
    {
    OPEN_FILE_ERROR = 2, READ_FILE_ERROR = 3, WRITE_FILE_ERROR = 4, READ_ARCHIVE_ERROR = 5
    };

using StringVector = std::vector<std::string>;
using Pair = std::pair<std::string, int>;
using WordMap = std::map<std::string, size_t>;
using Maps = std::vector<WordMap>;

#endif //LAB4_MAIN_CONFIG_H

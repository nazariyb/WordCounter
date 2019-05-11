//
// Created by danylo.kolinko on 5/11/19.
//

#ifndef LAB4_READ_H
#define LAB4_READ_H

#include <sstream>
#include <vector>
#include <map>
enum Error
{
    OPEN_FILE_ERROR = 2, READ_FILE_ERROR = 3, WRITE_FILE_ERROR = 4, READ_ARCHIVE_ERROR = 5
};

using StringVector = std::vector<std::string>;
using Pair = std::pair<std::string, int>;
std::vector<Pair> wordsVector;
using WordMap = std::map<std::string, size_t>;
using Maps = std::vector<WordMap>;

class read {
    std::stringstream read_txt(std::string & address);
    std::stringstream read_zip(std::string & address);

};


#endif //LAB4_READ_H

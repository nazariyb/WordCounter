#ifndef LAB4_FILE_READER_H
#define LAB4_FILE_READER_H

#include <sstream>
#include <vector>
#include <map>
#include "file_reader.h"

struct Reader
    {
    static void read_txt (std::string &address, std::stringstream &ss);
    static void read_txt (std::string &address, std::string &s);

    static void read_archive (std::string &address, std::stringstream &ss);

    static bool is_txt (const std::string &f);

    static bool is_archive (const std::string &f);

    static void extract (std::string &from, std::string &to);

    static int copy_data (struct archive *ar, struct archive *aw);

    };


#endif //LAB4_FILE_READER_H

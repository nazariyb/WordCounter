#include <fstream>
#include <iostream>
#include "file_reader.h"
#include <algorithm>
#include <archive.h>
#include <archive_entry.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "../src/main_config.h"


bool Reader::is_archive (const std::string &filename)
{
    StringVector archive_exts{".zip", ".tar", ".gz", ".tar.gz", ".7z"};
    for (auto &ext: archive_exts) {
        if (boost::iequals(boost::filesystem::extension(filename), ext))
            return true;
    }
    return false;
}

bool Reader::is_txt (const std::string &filename)
{
    return boost::iequals(boost::filesystem::extension(filename), ".txt");
}

void Reader::read_txt (std::string &address, std::stringstream &ss)
{
    std::ifstream f(address);
    ss << f.rdbuf();
}

void Reader::read_archive (std::string &address, std::stringstream &ss)
{

    //if file is archive open it and read content of all its files
    if (Reader::is_archive(address)) {
        struct archive *a;
        struct archive_entry *entry;
        int r;

        a = archive_read_new();
        archive_read_support_filter_all(a);
        archive_read_support_format_all(a);
        r = archive_read_open_filename(a, address.c_str(), 10240);
        if (r != ARCHIVE_OK) {
            std::cerr << "Error while opening archive.\n"
                         "Check filename / path to file / whether it is not corrupted: "
                      << address
                      << std::endl;
            throw std::invalid_argument("Error while opening archive.");
        }

        // get entries of archive
        while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            size_t size;
            auto *buff = new char[sizeof(entry)];
            for (;;) {
                size = archive_read_data(a, buff, sizeof(entry));
                if (size == 0) { break; }

                for (size_t i = 0; i < size; ++i) { ss << buff[i]; }
            }
            delete[] buff;
            archive_read_data_skip(a);
        }
        r = archive_read_free(a);
        if (r != ARCHIVE_OK) {
            std::cerr << "Error while closing archive.\n"
                         "Try to finish work..."
                      << std::endl;
            throw std::invalid_argument("Error while closing archive");
        }
    }

}
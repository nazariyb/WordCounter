//
// Created by danylo.kolinko on 5/11/19.
//

#include <fstream>
#include <iostream>
#include "read.h"
#include <algorithm>
#include <archive.h>
#include <archive_entry.h>
#include "main_config.h"


bool Reader::is_archive (const std::string& f)
{
    StringVector v{".zip", ".tar", ".gz", ".tar.gz", ".7z"};
    return (std::find(v.begin(), v.end(), f.substr(f.find_last_of('.'))) != v.end());
}

bool Reader::is_txt (const std::string& f)
{
    StringVector s {".txt"};
    return (std::find(s.begin(), s.end(), f.substr(f.find_last_of('.'))) != s.end());
}

std::stringstream &Reader::read_txt (std::string &address)
{
    std::ifstream f(address);
    std::stringstream chunk;
    chunk << f.rdbuf();
    return chunk;
}

std::stringstream &Reader::read_archive (std::string &address)
{
    std::stringstream words_stream;

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
            auto *buff = new char [sizeof(entry)];
            for (;;) {
                size = archive_read_data(a, buff, sizeof(entry));
                if (size == 0) { break; }

                for (size_t i = 0; i < size; ++i) { words_stream << buff[i]; }
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
        return words_stream;
    }

}
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
    f.close();
}

void Reader::read_txt (std::string &address, std::string &s)
{
//    std::ifstream file(address);
//    auto const start_pos = file.tellg();
//    file.ignore(std::numeric_limits<std::streamsize>::max());
//    auto const char_count = file.gcount();
//    file.seekg(start_pos);
//    auto text = std::vector<char>((unsigned long) char_count);
//    (file).read(&text[0], text.size());
//    file.close();
//    s = std::string(text.begin(), text.end());

     std::ifstream f(address);
     std::stringstream ss;
     ss << f.rdbuf();
     s = std::move(ss.str());
     f.close();
}

int Reader::copy_data (struct archive *ar, struct archive *aw)
{
    int r;
    const void *buff;
    size_t size;
    la_int64_t offset;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r < ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(aw));
            return (r);
        }
    }
}


void Reader::extract (std::string &from, std::string &to)
{
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    int r;

    /* Select which attributes we want to restore. */
    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_compression_all(a);
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);
    if ((r = archive_read_open_filename(a, from.c_str(), 10240)))
        throw std::runtime_error{"can't open archive"};
    for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r < ARCHIVE_OK)
            std::cerr << archive_error_string(a) << std::endl;
        if (r < ARCHIVE_WARN)
            throw std::runtime_error{"can't open archive"};

        const char *currentFile = archive_entry_pathname(entry);
        if (!is_txt(currentFile)) continue;
        const std::string fullOutputPath = to + currentFile;
        archive_entry_set_pathname(entry, fullOutputPath.c_str());
        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK)
            std::cerr << archive_error_string(ext) << std::endl;
        else if (archive_entry_size(entry) > 0) {
            r = copy_data(a, ext);
            if (r < ARCHIVE_OK)
                std::cerr << archive_error_string(ext) << std::endl;
            if (r < ARCHIVE_WARN)
                throw std::runtime_error{"can't open archive"};
        }
        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK)
            std::cerr << archive_error_string(ext) << std::endl;
        if (r < ARCHIVE_WARN)
            throw std::runtime_error{"can't open archive"};
    }
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
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
            const char *currentFile = archive_entry_pathname(entry);
            //std::cout << currentFile << std::endl;
            const std::string fullOutputPath =
                    std::string{"/mnt/c/Users/3naza/OneDrive/Documents/acs/labs/lab4/temp/"} + currentFile;
            std::cout << fullOutputPath << std::endl;
            archive_entry_set_pathname(entry, fullOutputPath.c_str());
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
        //        std::string new_address;
        //        std::copy(address.begin(), address.rfind('/'), new_address.begin());
        //        std::ofstream temp_file{address};
    }

}
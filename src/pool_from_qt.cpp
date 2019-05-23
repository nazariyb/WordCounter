#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <map>
#include <atomic>
#include <algorithm>
#include <archive.h>
#include <archive_entry.h>
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <thread>
#include <cmath>
#include <mutex>
#include <string>
#include <boost/asio.hpp>
#include <QtConcurrentMap>
#include "main_config.h"
#include "conf_reader.h"
#include "../dependencies/file_reader.h"
#include "../dependencies/thread_safe_queue.h"
#include "map_manipulation.h"

inline std::chrono::steady_clock::time_point get_current_time_fenced() {
    static_assert(std::chrono::steady_clock::is_steady, "Timer should be steady (monotonic).");
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = std::chrono::steady_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}

template<class D>
inline long long to_us(const D &d) {
    return std::chrono::duration_cast<std::chrono::microseconds>(d).count();
}

StringVector find_files_to_index(std::string &directory_name) {
    WordMap wMap;
    // iterate through text
    StringVector txt;
    std::string extract_to{"../temp/"};

    if (!boost::filesystem::exists(extract_to)) {
        boost::filesystem::path dstFolder = extract_to;
        boost::filesystem::create_directory(dstFolder);
    }

    for (boost::filesystem::recursive_directory_iterator end, dir(directory_name);
         dir != end; ++dir) {
        std::string pathname{(*dir).path().string()};

        if (Reader::is_txt(pathname)) {
            txt.push_back(pathname);
        }
        if (Reader::is_archive(pathname)) {
            try {
                Reader::extract(pathname, extract_to);
            } catch (...) {}
        }
    }

    for (boost::filesystem::recursive_directory_iterator end, dir("../temp");
         dir != end; ++dir) {
        std::string pathname{(*dir).path().string()};
        if (Reader::is_txt(pathname))
            txt.push_back(pathname);
    }
    return txt;
}

WordMap index_text(std::string file_stream) {
    std::string temp, text;
    using namespace boost::locale::boundary;

    auto wMap = new WordMap;

    // iterate through text

        // normalize encoding
        text = boost::locale::normalize(file_stream);

        // bound text by words
        ssegment_index map(word, text.begin(), text.end());
        map.rule(word_any);

        // convert them to fold case and add to the vector
        for (ssegment_index::iterator it = map.begin(), e = map.end(); it != e; ++it) {
            ++(*wMap)[boost::locale::fold_case(it->str())];



    }
    return *wMap;
}


int main(int argc, char **argv) {

    // Create system default locale
    boost::locale::generator gen;
    std::locale loc = gen("");
    std::locale::global(loc);
    std::wcout.imbue(loc);
    std::ios_base::sync_with_stdio(false);

    // set name of configuration file
    std::string config_file("../config.dat");
    if (argc >= 2) { config_file = argv[1]; }

    // try to open that file
    std::ifstream cf(config_file);
    if (!cf.is_open()) {
        std::cerr << "Error while opening configuration file.\n"
                     "Check filename or path to file: "
                  << config_file
                  << std::endl;
        return OPEN_FILE_ERROR;
    }

    // try to read configurations and save them to map conf
    std::map<std::string, std::string> conf;
    try {
        conf = read_conf(cf, '=');
        cf.close();
    }
    catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return READ_FILE_ERROR;
    }

    StringVector stream_vector{};

    if (!boost::filesystem::exists("../temp")) {
        boost::filesystem::path dstFolder = "../temp";
        boost::filesystem::create_directory(dstFolder);
    }


    std::cout << "Exploring " << conf["infile"] << "..." << std::endl;
    auto files_to_index = find_files_to_index(conf["infile"]);

    std::string s;

    std::cout << "Start processing data..." << std::endl;

    auto start_working = get_current_time_fenced();

    for (auto &filepath: files_to_index) {
        try {
            Reader::read_txt(filepath, s);
            stream_vector.push_back(std::move(s));
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            continue;
        }
    }
    std::cout << "Data read" << std::endl;

    auto result = QtConcurrent::blockingMappedReduced(stream_vector, index_text, merge_2_maps);
    std::vector<Pair> wordsVector;

    auto finish_working = get_current_time_fenced();

    for (auto &word: result) {
        wordsVector.emplace_back(std::move(word));
    }

    //    /mnt/c/Users/3naza/Desktop/gutenberg
    std::vector<Pair> sorted_numbers;
    std::cout << "Sorting by numbers..." << std::endl;
    std::copy(wordsVector.begin(), wordsVector.end(), std::back_inserter(sorted_numbers));
    std::sort(sorted_numbers.begin(), sorted_numbers.end(), [](Pair a, Pair b) { return a.second > b.second; });

    write_results(conf["out_by_n"], sorted_numbers);

    std::vector<Pair> sorted_words;
    std::cout << "Sorting by words..." << std::endl;
    std::copy(wordsVector.begin(), wordsVector.end(), std::back_inserter(sorted_words));
    std::sort(sorted_words.begin(), sorted_words.end(), [](Pair a, Pair b) {
        return boost::locale::comparator<char, boost::locale::collator_base::secondary>().operator()(a.first,
                                                                                                     b.first);
    });

    write_results(conf["out_by_a"], wordsVector);

    std::cout << "Total time: " << to_us(finish_working - start_working) << std::endl;
    return 0;

}

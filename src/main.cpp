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
#include <functional>

#include <cmath>
#include <mutex>
#include "read.h"
#include "main_config.h"
#include "conf_reader.h"
#include "../dependencies/thread_safe_queue.h"
#include "../dependencies/thread_pool.h"

#include "map_manipulation.h"

inline std::chrono::steady_clock::time_point get_current_time_fenced ()
{
    static_assert(std::chrono::steady_clock::is_steady, "Timer should be steady (monotonic).");
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = std::chrono::steady_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}

template<class D>
inline long long to_us (const D &d)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(d).count();
}


std::map<std::string, StringVector> find_files_to_index (std::string &directory_name)
{
    WordMap wMap;
    // iterate through text
    StringVector txt, zip;
    for (boost::filesystem::recursive_directory_iterator end, dir(directory_name);
         dir != end; ++dir) {
        std::string pathname{(*dir).path().string()};

        if (Reader::is_txt(pathname))
            txt.push_back(pathname);
        if (boost::iequals(boost::filesystem::extension(*dir), ".zip"))
            zip.push_back(pathname);
    }
    std::map<std::string, StringVector> resultMap;
    resultMap["txt"] = txt;
    resultMap["zip"] = zip;
    return resultMap;
}

void index_text (thread_safe_queue<std::stringstream> &stream_queue,
                 thread_safe_queue<WordMap> &maps_queue)
{
    while (true) {

        auto file_stream = stream_queue.try_pop();

        WordMap wMap;
        // iterate through text
        std::string temp;
        while (getline(*file_stream, temp)) {
            // normalize encoding
            std::string text = boost::locale::normalize(temp);

            // bound text by words
            using namespace boost::locale::boundary;
            ssegment_index map(word, text.begin(), text.end());
            map.rule(word_any);

            // convert them to fold case and add to the vector
            for (ssegment_index::iterator it = map.begin(), e = map.end(); it != e; ++it) {
                ++wMap[boost::locale::fold_case(it->str())];
            }
        }
//        maps_queue.push(wMap);
        if (wMap.empty())
            return;
    }
}


int main (int argc, char **argv)
{
    // Create system default locale
    boost::locale::generator gen;
    std::locale loc = gen("");
    std::locale::global(loc);
    std::wcout.imbue(loc);
    std::ios_base::sync_with_stdio(false);

    std::vector<Pair> wordsVector;

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

    thread_safe_queue<std::stringstream> stream_queue{};
    thread_safe_queue<WordMap> maps_queue{};


    std::cout << "Exploring " << conf["infile"] << "..." << std::endl;
    auto files_to_index = find_files_to_index(conf["infile"]);
    std::stringstream ss;

    std::vector<std::thread> index_threads, merge_threads;

    for (int i = 0; i < 6; ++i)
        index_threads.emplace_back(index_text, std::ref(stream_queue), std::ref(maps_queue));
    for (int i = 0; i < 2; ++i)
        merge_threads.emplace_back(merge_two_maps, std::ref(maps_queue));


    for (auto &filepath: files_to_index["txt"]) {
        try {
            Reader::read_txt(filepath, ss);
            stream_queue.push(std::move(ss));
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            continue;
        }
    }

    for (auto &filepath: files_to_index["zip"]) {
        try {
            Reader::read_archive(filepath, ss);
            stream_queue.push(std::move(ss));
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            continue;
        }
    }

    auto result = maps_queue.get_result();
    for (auto& word: result) {
        std::cout << word.first << " : " << word.second << std::endl;
    }
    return 0;
//    /mnt/c/Users/3naza/Desktop/gutenberg
    std::cout << "Sorting by numbers..." << std::endl;
    std::vector<Pair> sorted_words;
    std::copy(wordsVector.begin(), wordsVector.end(), std::back_inserter(sorted_words));
    std::sort(wordsVector.begin(), wordsVector.end(), [] (Pair a, Pair b) { return a.second > b.second; });

    write_results(conf["out_by_n"], sorted_words);

    std::cout << "Sorting by words..." << std::endl;
    std::copy(wordsVector.begin(), wordsVector.end(), std::back_inserter(sorted_words));
    std::sort(wordsVector.begin(), wordsVector.end(), [] (Pair a, Pair b) {
        return boost::locale::comparator<char, boost::locale::collator_base::secondary>().operator()(a.first,
                                                                                                     b.first);
    });


    write_results(conf["out_by_a"], wordsVector);


    return 0;

}

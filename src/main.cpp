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
#include "read.h"
#include "main_config.h"
#include "conf_reader.h"
#include "../dependencies/thread_safe_queue.h"

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

void write_file (const std::string &filename, const std::vector<Pair> &words)
{
    std::ofstream outfile(filename);
    for (auto &word : words) {
        outfile << word.first;
        outfile.width(20 - word.first.length());
        outfile << " :    " << word.second << std::endl;
    }
    outfile.close();
}

std::map<std::string, StringVector> find_files_to_index (std::string &directory_name)
{
    WordMap wMap;
    // iterate through text
    StringVector txt, zip;
    for (boost::filesystem::recursive_directory_iterator end, dir(directory_name);
         dir != end; ++dir) {
        std::string pathname{(*dir).path().string()};

        if (boost::iequals(boost::filesystem::extension(*dir), ".txt"))
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
        maps_queue.push(wMap);
        if (wMap.empty())
            return;
    }
}

void merge_two_maps (thread_safe_queue<WordMap> maps_queue)
{
    while (true) {
        auto map_pair = maps_queue.double_pop();
        if (map_pair.first.empty() || map_pair.second.empty()) {
            if (maps_queue.size() == 0) {
                maps_queue.double_push(map_pair.first, map_pair.second);
                return;
            } else {
                maps_queue.double_push(map_pair.first, map_pair.second);
                continue;
            }
        }

        WordMap merged_map{map_pair.first};
        for (auto &word: map_pair.second) {
            merged_map[word.first] += word.second;
        }
        maps_queue.push(merged_map);
    }
}

void process_data (const StringVector &stream_vector, size_t start_pos, size_t end_pos,
                   WordMap &wordsMap, std::mutex &mt)
{
    WordMap wMap;
    // iterate through text
    for (; start_pos < end_pos; ++start_pos) {
        // normalize encoding
        std::string text = boost::locale::normalize(stream_vector[start_pos]);

        // bound text by words
        using namespace boost::locale::boundary;
        ssegment_index map(word, text.begin(), text.end());
        map.rule(word_any);

        // convert them to fold case and add to the vector
        for (ssegment_index::iterator it = map.begin(), e = map.end(); it != e; ++it) {
            ++wMap[boost::locale::fold_case(it->str())];
        }
    }
    std::lock_guard<std::mutex> lock(mt);
    wordsMap = wMap;
}



int main (int argc, char **argv)
{
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

    thread_safe_queue<std::stringstream> read_queue{};
    Reader read{};


    auto files_to_index = find_files_to_index(conf["infile"]);
    for (auto & filepath: files_to_index["txt"]){
        auto ss = read.read_txt(filepath);
        read_queue.push(ss);

    }
    for (auto & filepath: files_to_index["zip"]){
        auto ss = read.read_txt(filepath);
        read_queue.push(ss);
    }



    // Create system default locale
    boost::locale::generator gen;
    std::locale loc = gen("");
    std::locale::global(loc);
    std::wcout.imbue(loc);
    std::ios_base::sync_with_stdio(false);

    StringVector stream_vector;


    //    auto start_sorting = get_current_time_fenced();
    // sort by numbers
    std::cout << "Sorting by numbers..." << std::endl;
    std::vector<Pair> sorted_numbers;
    std::copy(wordsVector.begin(), wordsVector.end(), std::back_inserter(sorted_numbers));
    std::sort(sorted_numbers.begin(), sorted_numbers.end(), [] (Pair a, Pair b) { return a.second > b.second; });

    // sort by words
    std::cout << "Sorting by words..." << std::endl;
    std::vector<Pair> sorted_words;
    std::copy(wordsVector.begin(), wordsVector.end(), std::back_inserter(sorted_words));
    std::sort(sorted_words.begin(), sorted_words.end(),
              [] (Pair a, Pair b) {
                  return boost::locale::comparator<char, boost::locale::collator_base::secondary>().operator()(a.first,
                                                                                                               b.first);
              });
    //    auto end_sorting = get_current_time_fenced();
    //    auto time_sorting = to_us(end_sorting - start_sorting);

    // write results to files
    std::cout << "Save results..." << std::endl;

    try {
        write_file(conf["out_by_n"], sorted_numbers);
    } catch (std::exception &ex) {
        std::cerr << "Error while saving results to "
                  << conf["out_by_n"]
                  << ". Try to complete work..."
                  << std::endl;
        return WRITE_FILE_ERROR;
    }

    try {
        write_file(conf["out_by_a"], sorted_words);
    } catch (std::exception &ex) {
        std::cerr << "Error while saving results to "
                  << conf["out_by_a"]
                  << ". Try to complete work..."
                  << std::endl;
        return WRITE_FILE_ERROR;
    }

    return 0;
}
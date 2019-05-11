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

void index_text (thread_safe_queue<std::stringstream> stream_queue,
                 thread_safe_queue<WordMap> maps_queue)
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


StringVector find_files_to_index (std::string &directory_name)
{
    StringVector sv;
    for (boost::filesystem::recursive_directory_iterator end, dir(directory_name);
         dir != end; ++dir) {
        std::string pathname{(*dir).path().string()};
        //        std::cout << "|" << pathname << "|" << std::endl;
        //          if (is_txt(pathname)) {
        //              sv.push_back(pathname);
        std::cout << pathname << std::endl;
        //          }
        //         std::cout << *dir << "\n";  // full path
        //        std::cout << dir->path().filename() << "\n"; // just last bit
    }
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

    find_files_to_index(conf["infile"]);

    std::stringstream words_stream;

    auto start_reading = get_current_time_fenced();
    // if file is archive open it and read content of all its files
    if (is_archive(conf["infile"])) {
        struct archive *a;
        struct archive_entry *entry;
        int r;

        // open archive
        std::cout << "Opening " << conf["infile"] << std::endl;
        a = archive_read_new();
        archive_read_support_filter_all(a);
        archive_read_support_format_all(a);
        r = archive_read_open_filename(a, conf["infile"].c_str(), 10240);
        if (r != ARCHIVE_OK) {
            std::cerr << "Error while opening archive.\n"
                         "Check filename / path to file / whether it is not corrupted: "
                      << conf["infile"]
                      << std::endl;
            return READ_ARCHIVE_ERROR;
        }

        // get entries of archive
        while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            std::cout << "Reading " << archive_entry_pathname(entry) << std::endl;
            size_t size;
            auto *buff = (char *) malloc(sizeof(entry));
            for (;;) {
                size = archive_read_data(a, buff, sizeof(entry));
                if (size == 0) { break; }
                for (size_t i = 0; i < size; ++i) { words_stream << buff[i]; }
            }
            free(buff);
            archive_read_data_skip(a);
        }
        r = archive_read_free(a);
        if (r != ARCHIVE_OK) {
            std::cerr << "Error while closing archive.\n"
                         "Try to finish work..."
                      << std::endl;
        }
    } else {
        // otherwise file's type is text document, just read it
        std::cout << "Reading " << conf["infile"] << std::endl;
        std::ifstream infile(conf["infile"]);
        if (!infile.is_open()) {
            std::cerr << "Error while opening data file.\n"
                         "Check filename / path to file / whether it is not corrupted: "
                      << conf["infile"]
                      << std::endl;
            return OPEN_FILE_ERROR;
        }
        words_stream << infile.rdbuf();
        infile.close();
    }
    auto end_reading = get_current_time_fenced();
    auto time_reading = to_us(end_reading - start_reading);

    // Create system default locale
    boost::locale::generator gen;
    std::locale loc = gen("");
    std::locale::global(loc);
    std::wcout.imbue(loc);
    std::ios_base::sync_with_stdio(false);

    StringVector stream_vector;

    std::string tmp;
    while (getline(words_stream, tmp)) { stream_vector.push_back(tmp); }

    double threads_number{std::stod(conf["threads"])};
    if (argc == 3) { threads_number = std::stod(argv[2]); }
    std::vector<std::thread> threads;

    std::cout << "Processing data..." << std::endl;
    auto start_processing = get_current_time_fenced();
    Maps maps;
    size_t step{(size_t) (std::ceil(stream_vector.size() / threads_number))};
    size_t start_pos{0};
    size_t end_pos{0};
    std::mutex mt;

    for (int i = 0; i < threads_number; ++i) {
        auto *wordMap = new WordMap();
        maps.push_back(*wordMap);
    }

    // create and run threads
    for (int i = 0; i < threads_number - 1; ++i) {
        start_pos = end_pos;
        end_pos = start_pos + step;
        threads.emplace_back(process_data, std::ref(stream_vector), start_pos, end_pos, std::ref(maps[i]),
                             std::ref(mt));
    }
    // when number of threads is 1, program works like sequential one
    start_pos = end_pos;
    end_pos = stream_vector.size();
    threads.emplace_back(process_data, std::ref(stream_vector), start_pos, end_pos, std::ref(maps[maps.size() - 1]),
                         std::ref(mt));

    for (auto &thread : threads) { thread.join(); }

    // merge results
    WordMap wordsMap;
    for (const auto &wMap: maps) {
        for (auto &word : wMap) {
            wordsMap[word.first] += word.second;
        }
    }
    auto end_processing = get_current_time_fenced();
    auto time_processing = to_us(end_processing - start_processing);

    // save data from map to vector
    for (auto &words_number : wordsMap) {
        wordsVector.emplace_back(words_number.first, words_number.second);
    }

    auto start_sorting = get_current_time_fenced();
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
    auto end_sorting = get_current_time_fenced();
    auto time_sorting = to_us(end_sorting - start_sorting);

    // write results to files
    std::cout << "Save results..." << std::endl;

    auto start_saving = get_current_time_fenced();

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
    auto end_saving = get_current_time_fenced();
    auto time_saving = to_us(end_saving - start_saving);

    std::cout << "Loading: " << time_reading << std::endl;
    std::cout << "Processing: " << time_processing << std::endl;
    std::cout << "Sorting: " << time_sorting << std::endl;
    std::cout << "Saving: " << time_saving << std::endl;
    std::cout << "Total time: " << time_reading + time_processing + time_sorting + time_saving << std::endl;

    return 0;
}
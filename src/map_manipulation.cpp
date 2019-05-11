//
// Created by danylo.kolinko on 5/11/19.
//

#include <vector>
#include <algorithm>
#include "map_manipulation.h"
#include <string>
#include <iostream>
#include <fstream>
#include "main_config.h"
#include "../dependencies/thread_safe_queue.h"

void merge_two_maps(thread_safe_queue<WordMap> maps_queue) {
    while (true) {
        auto map_pair = maps_queue.double_pop();
        if (map_pair.first.empty() || map_pair.second.empty()) {
            if (maps_queue.empty()) {
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

void write_file(const std::string &filename, const std::vector<Pair> &words) {
    std::ofstream outfile(filename);
    for (auto &word : words) {
        outfile << word.first;
        outfile.width(20 - word.first.length());
        outfile << " :    " << word.second << std::endl;
    }
    outfile.close();
}


void write_results(const std::string & outfile, std::vector<Pair> & sorted_numbers){
    try {
        write_file(outfile, sorted_numbers);
    } catch (std::exception &ex) {
        std::cerr << "Error while saving results to "
                  << outfile
                  << ". Try to complete work..."
                  << std::endl;
        throw(std::invalid_argument("Error while saving results"));
    }

}
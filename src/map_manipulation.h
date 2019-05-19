#ifndef LAB4_MAP_MANIPULATION_H
#define LAB4_MAP_MANIPULATION_H

#include <string>
#include <atomic>
#include "main_config.h"
#include "../dependencies/thread_safe_queue.h"

void write_results (const std::string &outfile, std::vector<Pair> &sorted_numbers);

void merge_two_maps (thread_safe_queue<WordMap> &maps_queue);


#endif //LAB4_MAP_MANIPULATION_H

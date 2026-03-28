// fetchDanbooruTags, fetches tags from danbooru and saves them in csv format into a .txt file
// Copyright (C) 2026  spectre-bit
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <sstream>
#include <string_view>
#include <filesystem>
#include <charconv>
#include <fstream>


#include "cpr/cpr.h"
#include "nlohmann/json.hpp"

#define MAX_RETRIES 10

const std::string URL= "https://danbooru.donmai.us/tags.json";
const std::string PARAM_LIMIT = "limit";
const std::string PARAM_PAGE = "page";
const std::string PARAM_HIDE_EMPTY = "search[hide_empty]";
const std::string PARAM_MIN_POSTS = "search[post_count]";
const std::string LIMIT = "1000";
const std::string HIDE_EMPTY = "yes";

std::mutex names_mutex;
std::vector<std::string> all_names;
std::atomic<int> current_page{1};
std::atomic<bool> done{false};

int min_posts = 0;
std::string filename = "danbooru_tags.txt";

cpr::Response getResponse(int page) {
    return cpr::Get(cpr::Url{URL}, cpr::Parameters{
        {PARAM_LIMIT, LIMIT},
        {PARAM_PAGE, std::to_string(page)},
        {PARAM_MIN_POSTS, ">=" + std::to_string(min_posts)},
        {PARAM_HIDE_EMPTY, HIDE_EMPTY}
    });
}

std::string join(const std::vector<std::string> &strings, const char *const delimiter) {
    std::ostringstream ss{};
    std::copy(strings.begin(), strings.end() - 1, std::ostream_iterator<std::string>(ss, delimiter));
    ss << strings.back();
    return ss.str();
}

void worker() {
    int retries = 0;

    while (!done) {
        int page = current_page.fetch_add(1, std::memory_order_relaxed);

    retry:
        cpr::Response response = getResponse(page);

        if (response.status_code != 200) {
            if (retries == MAX_RETRIES) {
                std::cout << "Thread is stuck, got no response after 10 retries, aborting program" << std::endl;
                done = true;
                return;
            }
            std::cerr << "Error, got status code " << response.status_code << "\nRetry in 1 second" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            ++retries;
            goto retry;
        }

        nlohmann::json json;
        try {
            json = nlohmann::json::parse(response.text);
        } catch (...) {
            std::cerr << "JSON parse error on page " << page << std::endl;
            continue;
        }

        if (json.empty()) {
            done = true;
            return;
        }

        std::vector<std::string> local_names;
        local_names.reserve(json.size());

        for (const nlohmann::json &item : json) {
            local_names.push_back(item["name"].get<std::string>() + "," + std::to_string(item["post_count"].get<int>()));
        }

        {
            std::lock_guard<std::mutex> lock(names_mutex);
            all_names.insert(all_names.end(), std::make_move_iterator(local_names.begin()), std::make_move_iterator(local_names.end()));
        }

        std::cout << "got " << json.size() << " tags, meeting the requirements, from page: " << page << std::endl;
        retries = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        std::cout << "too many arguments" << std::endl;
        std::cerr << "Usage: fetchAllDanbooruTags <optional: minimum post count>" << std::endl;
        return 1;
    }
    if (argc == 2) {
        std::string_view arg = argv[1];
        auto [ptr, ec] = std::from_chars(arg.data(), arg.data() + arg.size(), min_posts);

        if (ec == std::errc::invalid_argument) {
            std::cerr << "Invalid input, this is no integer: " << ptr << std::endl;
            return 1;
        }
        if (ec == std::errc::result_out_of_range) {
            std::cerr << "too big, the maximum value is: " << (std::numeric_limits<int>::max)() << std::endl;
            return 1;
        }
        if (min_posts < 0) {
            std::cerr << "Invalid input the minimum amount of posts has to be at least 0" << std::endl;
            return 1;
        }
        if (*ptr != '\0') {
            std::cerr << min_posts << " read " << ptr << " is rejected\n"
                      << std::endl;
        }

        filename = "danbooru_tags_at_least_" + std::to_string(min_posts) + ".txt";

        std::cout << "start fetching danbooru tags with at least " << min_posts << " posts" << std::endl;
    } else {
        std::cout << "start fetching all danbooru tags" << std::endl;
    }

    constexpr int NUM_THREADS = 8;

    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(worker);
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
    for (std::thread &t : threads) {
        t.join();
    }

    std::cout << "Total tags: " << all_names.size() << std::endl;

    std::ofstream out{filename};
    out << join(all_names, "\n") << std::endl;
    out.close();

    std::cout << "created \"" + filename + "\" at " << std::filesystem::current_path() << std::endl;

    return 0;
}

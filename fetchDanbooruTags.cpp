#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <queue>
#include <chrono>

#include "cpr/cpr.h"
#include "nlohmann/json.hpp"

#define MAX_RETRIES 10

const std::string URL_PART_1 = "https://danbooru.donmai.us/tags.json?limit=";
const std::string URL_PART_2 = "&page=";
const std::string URL_PART_3 = "&search[hide_empty]=yes";
const std::string LIMIT = "1000";

std::mutex names_mutex;
std::vector<std::string> all_names;
std::atomic<int> current_page{1};
std::atomic<bool> done{false};

int min_posts = -1;
std::string filename = "danbooru_tags.txt";


cpr::Response getResponse(int page) {
    return cpr::Get(cpr::Url{URL_PART_1 + LIMIT + URL_PART_2 + std::to_string(page) + URL_PART_3});
}

std::string join(const std::vector<std::string>& strings, const char* const delimiter) {
    std::ostringstream ss{};
    std::copy(strings.begin(), strings.end() - 1, std::ostream_iterator<std::string>(ss, delimiter));
    ss << strings.back();
    return ss.str();
}

void worker() {
    int retries = 0;
    int count = 0;

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
        }
        catch (...) {
            std::cerr << "JSON parse error on page " << page << "\n";
            continue;
        }

        if (json.empty()) {
            done = true;
            return;
        }

        std::vector<std::string> local_names;
        local_names.reserve(json.size());

        for (const nlohmann::json &item : json) {
            if (item["post_count"].get<int>() >= min_posts) {
                local_names.push_back(item["name"].get<std::string>() + "," + std::to_string(item["post_count"].get<int>()));
                ++count;
            }
        }

        {
            std::lock_guard<std::mutex> lock(names_mutex);
            all_names.insert(all_names.end(), std::make_move_iterator(local_names.begin()), std::make_move_iterator(local_names.end()));
        }

        std::cout << "got " << count << " tags, meeting the requirements, from page: " << page << std::endl;
        retries = 0;
        count = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
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
            std::cerr << min_posts << " read " << ptr << " is rejected\n" << std::endl;
        }

        filename = "danbooru_tags_at_least_" + std::to_string(min_posts) + ".txt";

        std::cout << "start fetching danbooru tags with at least " << min_posts << " posts" << std::endl;
    }
    else {
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



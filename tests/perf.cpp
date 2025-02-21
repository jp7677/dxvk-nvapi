#include "nvapi_tests_private.h"
#include "../src/util/util_string.h"

using namespace Catch::Matchers;

TEST_CASE("Caching and looking simulations", "[.benchmarks]") {
    constexpr auto element_count = 25;

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, element_count - 1);

    auto values = std::vector<std::string>();
    for (auto i = 0U; i < element_count; i++)
        values.emplace_back(dxvk::str::format("Some string representing an instance to something else - ", i));

    std::unordered_map<uint64_t*, std::string> umap;
    std::map<uint64_t*, std::string> map;
    std::vector<std::pair<uint64_t*, std::string>> vec;
    static std::mutex m_mutex;
    static std::shared_mutex m_smutex;

    for (auto& s : values) {
        umap.emplace(reinterpret_cast<uint64_t*>(&s), s);
        map.emplace(reinterpret_cast<uint64_t*>(&s), s);
        vec.emplace_back(reinterpret_cast<uint64_t*>(&s), s);
    }

    REQUIRE(values.size() == element_count);
    REQUIRE(umap.size() == element_count);
    REQUIRE(map.size() == element_count);
    REQUIRE(vec.size() == element_count);

    BENCHMARK("unordered_map") {
        auto& k = values[dist(rng)];
        if (const auto it = umap.find(reinterpret_cast<uint64_t*>(&k)); it != umap.end())
            return it->second;

        throw std::runtime_error("not found");
    };

    BENCHMARK("scoped_lock") {
        std::scoped_lock lock{m_mutex};
        auto& k = values[dist(rng)];
        if (const auto it = umap.find(reinterpret_cast<uint64_t*>(&k)); it != umap.end())
            return it->second;

        throw std::runtime_error("not found");
    };

    BENCHMARK("shared_lock") {
        std::shared_lock lock{m_smutex};
        auto& k = values[dist(rng)];
        if (const auto it = umap.find(reinterpret_cast<uint64_t*>(&k)); it != umap.end())
            return it->second;

        throw std::runtime_error("not found");
    };

    BENCHMARK("map") {
        auto& k = values[dist(rng)];
        if (const auto it = map.find(reinterpret_cast<uint64_t*>(&k)); it != map.end())
            return it->second;

        throw std::runtime_error("not found");
    };

    BENCHMARK("vector") {
        auto& k = values[dist(rng)];
        if (const auto it = std::find_if(vec.begin(), vec.end(), [&k](auto& item) { return item.first == reinterpret_cast<uint64_t*>(&k); }); it != vec.end())
            return it->second;

        throw std::runtime_error("not found");
    };
}

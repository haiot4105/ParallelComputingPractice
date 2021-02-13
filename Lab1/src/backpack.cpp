
#include <iostream>
#include <cmath>
#include <set>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <chrono>
#include "csv_document/csvdocument.h"

constexpr size_t N_THREADS = 6;
constexpr size_t N_CYCLES = 1;
const std::vector<std::string> tests =
        {
                "_5",
                "_8",
                "_10",
                "_14",
                "_15",
                "_16",
                "_17",
                "_18",
                "_19",
                "_20",
                "_21",
                "_22",
                "_23",
                "_24",
                "_25",
                "_26"
        };


double find_price(std::vector<std::pair<double, double>> values, size_t  n, double capacity)
{
    size_t current = 1;
    double w = 0;
    double price = 0;
    for(const auto& el : values)
    {
        if((n & current) != 0)
        {
            w += el.first;
            price += el.second;
            if(w > capacity) return -1;
        }
        current <<= 1;
    }
    return price;
}

void task(size_t pid, const std::vector<std::pair<double, double>>& values, double capacity, size_t& result, double& max_price )
{
    size_t N = values.size();
    size_t S = 1 << N;
    size_t s = S / N_THREADS;
    size_t start = s * pid;
    size_t count = (pid != (N_THREADS - 1)) ? s : s + S % N_THREADS;
    result = 0;
    max_price = 0;
    for(size_t i = start; i < start + count; i++)
    {
        double p = find_price(values, i, capacity);
        if (p > max_price)
        {
            result = i;
            max_price = p;
        }
    }
}

std::vector<size_t> findOptimalUsingBruteForce (const std::vector<std::pair<double, double>>& values, double capacity)
{
    std::vector<size_t> result = {};
    std::vector<std::thread> threads;
    std::vector<size_t> local_result(N_THREADS);
    std::vector<double> local_max_price(N_THREADS);

    for (size_t pid = 0; pid < N_THREADS; pid++)
    {
        threads.emplace_back(task, pid, std::cref(values), capacity, std::ref(local_result[pid]), std::ref(local_max_price[pid]));
    }
    for (auto& t : threads)
    {
        t.join();
    }

    size_t max_index = std::distance(local_max_price.begin(), std::max_element(local_max_price.begin(), local_max_price.end()));
    size_t bin_result = local_result[max_index];

    for(size_t i = 0; i < values.size(); i++)
    {
        if(bin_result & (1 << i)) result.emplace_back(1);
        else result.emplace_back(0);
    }

    return result;
}

int main()
{

    std::vector<std::vector<std::pair<double, double>>> all_values;
    std::vector<double> all_capacities;
    for (const auto& test : tests)
    {

        CsvDocument testCsv(';');
        if(!testCsv.load(std::string("../tests/test") + test + ".csv"))
        {
            std::cout << "File openning error: " << std::string("../tests/test") + test + ".csv" << "\n";
        }

        std::vector<std::pair<double, double>> values;
        double capacity = 0;

        for (size_t i = 0; i < testCsv.size(); ++i)
        {
            values.emplace_back(testCsv[i][0].toDouble(), testCsv[i][1].toDouble());
            capacity += testCsv[i][0].toDouble();
        }
        capacity *= 0.5;

        all_values.push_back(values);
        all_capacities.push_back(capacity);

        auto backpack = findOptimalUsingBruteForce(values, capacity);

        double res_cap = 0;
        for(size_t el_i = 0; auto& el : backpack)
        {
            if(el) res_cap += values[el_i].first;
            el_i ++;
        }


        std::cout << "test" << test << ": ";
        if(res_cap > capacity)
        {
            std::cout << "FAIL\n";
            exit(-1);
        }

        CsvDocument resultCsv(';');
        if(!resultCsv.load(std::string("../tests/bpresult") + test + ".csv"))
        {
            std::cout << "File openning error: " << std::string("../tests/test") + test + ".csv" << "\n";
        }

        for(size_t i = 0; i < resultCsv[0].size(); i++)
        {
            if(backpack[i] != resultCsv[0][i].toUnsignedLongLong())
            {
                std::cout << "FAIL\n";
                exit(-1);
            }
        }
        std::cout << "OK\n";
    }

    std::cout << "Check time\n";

    auto start = std::chrono::system_clock::now();

    for(size_t cycle = 0; cycle < N_CYCLES; cycle++)
    {
        for(size_t t = 0; t < all_values.size(); t++)
        {
            findOptimalUsingBruteForce(all_values[t], all_capacities[t]);
        }
    }
    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << std::setprecision(10) << static_cast<double>(elapsed.count()) / (1000 * N_CYCLES)   << "\n";
    return 0;

}


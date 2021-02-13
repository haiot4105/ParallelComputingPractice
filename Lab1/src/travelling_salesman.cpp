#include <iostream>
#include <cmath>
#include <set>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <chrono>
#include "csv_document/csvdocument.h"


constexpr size_t N_THREADS = 6;
constexpr size_t N_CYCLES = 2000;

double computePathCost(const std::vector<size_t>& path, const std::vector<std::vector<double>>& matrix, size_t start)
{
    double ret_val = matrix[0][start];
    size_t prev = start;

    for(auto &node : path)
    {
        ret_val += matrix[prev][node];
        prev = node;
    }
    ret_val += matrix[path.back()][0];

    return ret_val;

}


std::vector<std::string> tests =
        {
            "_5",
            "_8",
            "_10",
//            "_14",
        };

void findOptimalInIntervalUsingBruteForce(const std::vector<std::vector<double>>& matrix, size_t interval_begin,
                                        size_t interval_size, double& result_length, std::vector<size_t>& result_path)
{

    result_length = std::numeric_limits<double>::infinity();
    size_t start_node = (interval_begin < matrix.size()) ? interval_begin : matrix.size() ;
    size_t interval_end = (interval_begin + interval_size < matrix.size()) ? (interval_begin + interval_size) : matrix.size();

    for(; start_node < interval_end; start_node++)
    {
        auto path = std::vector<size_t>();
        for(int i = 1; i < matrix.size(); i++)
        {
            if(i != start_node)
            {
                path.push_back(i);
            }
        }

        do
        {
            double r = computePathCost(path, matrix, start_node);

            if(r < result_length)
            {
                result_length = r;
                result_path = {0, start_node};
                result_path.insert(result_path.end(), path.begin(), path.end());
            }
        }
        while(std::next_permutation(path.begin(), path.end()));
    }
}


std::vector<size_t> findOptimalUsingBruteForce (const std::vector<std::vector<double>>& matrix)
{
    std::vector<std::thread> threads;
    std::vector<double> local_best_lengths(N_THREADS);
    std::vector<std::vector<size_t>> local_best_paths(N_THREADS);
    size_t intreval_begin = 1, interval_size = 0;

    double best_length;

    for (size_t i = 0; i < N_THREADS; i++)
    {
        intreval_begin = intreval_begin + interval_size;
        interval_size = (matrix.size() - 1) / N_THREADS + (i < (matrix.size() - 1) % N_THREADS);
        threads.emplace_back(findOptimalInIntervalUsingBruteForce, std::cref(matrix), intreval_begin, interval_size, std::ref(local_best_lengths[i]), std::ref(local_best_paths[i]));
    }
    for (auto& t : threads)
    {
        t.join();
    }

    size_t min_index = std::distance(local_best_lengths.begin(), std::min_element(local_best_lengths.begin(), local_best_lengths.end()));

    return local_best_paths[min_index];
}

int main(int /*argc*/, char* /*argv*/[])
{
    std::cout << "Check solutions: \n";
    std::vector<std::vector<std::vector<double>>> tasks;
    for (const auto& test : tests)
    {
        CsvDocument testCsv(';');
        if(!testCsv.load(std::string("../tests/test") + test + ".csv"))
        {
            std::cout << "File openning error: " << std::string("../tests/test") + test + ".csv" << "\n";
        }
        std::vector<std::vector<double> > matrix;
        for (size_t i = 0; i < testCsv.size(); ++i)
        {
            matrix.emplace_back();
            for (size_t j = 0; j < testCsv.size(); ++j)
            {
                matrix[i].push_back(std::sqrt(std::pow(testCsv[j][0].toDouble()
                                                       - testCsv[i][0].toDouble(), 2)
                                              + std::pow(testCsv[j][1].toDouble()
                                                         - testCsv[i][1].toDouble(), 2)));
            }
        }
        tasks.push_back(matrix);

        auto path = findOptimalUsingBruteForce(matrix);
        for(auto& el : path)
        {
            std::cout << el << " ";
        }

        CsvDocument resultCsv(';');
        if(!resultCsv.load(std::string("../tests/result") + test + ".csv"))
        {
            std::cout << "File openning error: " << std::string("../tests/test") + test + ".csv" << "\n";
        }
        if (path.empty() || (resultCsv.size() > path.size()))
        {
            std::cout << "test" << test << ": FAIL\n";
            continue;
        }
        std::set<std::pair<size_t, size_t> > edges;
        auto prev = path.back();
        for (size_t i = 0; i < resultCsv.size(); ++i)
        {
            edges.emplace(prev, path[i]);
            prev = path[i];
        }
        auto ok = true;
        prev = resultCsv[resultCsv.size() - 1][0].toUnsignedLongLong();
        for (size_t i = 0; i < resultCsv.size(); ++i)
        {
            if (edges.find(std::pair<size_t, size_t>(prev, resultCsv[i][0].toUnsignedLongLong())) == edges.end() &&
                edges.find(std::pair<size_t, size_t>(resultCsv[i][0].toUnsignedLongLong(), prev)) == edges.end())
            {
                std::cout << "test" << test << ": FAIL\n";
                ok = false;
                break;
            }
            prev = resultCsv[i][0].toUnsignedLongLong();
        }
        if (ok)
        {
            std::cout << "test" << test << ": OK\n";
        }
    }

    std::cout << "Check time\n";

    auto start = std::chrono::system_clock::now();

    for(size_t cycle = 0; cycle < N_CYCLES; cycle++)
    {
        for(auto& task : tasks)
        {
            findOptimalUsingBruteForce(task);
        }
    }
    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << std::setprecision(10) << static_cast<double>(elapsed.count()) / (1000 * N_CYCLES)   << "\n";
    return 0;
}


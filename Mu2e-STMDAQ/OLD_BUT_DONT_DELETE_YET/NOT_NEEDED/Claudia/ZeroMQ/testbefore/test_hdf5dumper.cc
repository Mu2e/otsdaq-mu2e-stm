
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <highfive/H5File.hpp>
#include <highfive/H5DataType.hpp>

int main()
{
    auto log = spdlog::stdout_color_mt("HDF5");

    int nevts = 100;

    for (int j = 0; j < nevts; j++)
    {

        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }

    return 0;
}

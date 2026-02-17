

//#include "boost/histogram.hpp"
//#include "boost/histogram/axis/regular.hpp"
//#include "boost/histogram/make_histogram.hpp"

#include <chrono>
#include <iostream>
#include <random>
#include <thread>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "MIDASAcquirer.hh"
#include "STMOnlineMonitor.hh"

//using namespace boost::histogram;

int main()
{
    auto mon_logger = spdlog::stdout_color_mt("Monitoring");

    STMOnlineMonitor monitor("tcp://*:9929", "tcp://*:9930");

    // make histogram

    //auto h = make_histogram(axis::regular<>(100, 0, 100, "gaussian"));

    //auto h2 = make_histogram(axis::regular<>(100, 0, 80, "gaus2"));
    // sample from a random distribution

    std::default_random_engine generator;
    std::normal_distribution<double> distribution(50.0, 5.0);
    std::normal_distribution<double> dist2(20, 2);

    int nmessages = 100;

    for (int j = 0; j < nmessages; j++)
    {
      //for (int i = 0; i < 5; i++)
      //    h(distribution(generator));
      //for (int i = 0; i < 50; i++)
      //    h2(dist2(generator));

        // publish this histogram on the socket
        //monitor.queueHistogram(h);
        //monitor.queueHistogram(h2);
        monitor.sendMessage();

        spdlog::info("Sent two histograms");

        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }
    return 0;
}

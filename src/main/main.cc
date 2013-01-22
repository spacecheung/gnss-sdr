/*!
 * \file main.cc
 * \brief Main file of the GNSS-SDR program.
 * \author Carlos Aviles, 2010. carlos.avilesr(at)googlemail.com
 *
 * It sets up the logging system, creates a ControlThread object,
 * makes it run, and releases memory back when the main thread has ended.
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */
#ifndef GNSS_SDR_VERSION
#define GNSS_SDR_VERSION "0.0.1"
#endif

#include <boost/filesystem.hpp>
#include <gflags/gflags.h>
#include <glog/log_severity.h>
#include <glog/logging.h>
#include <gnuradio/gr_msg_queue.h>
#include "control_thread.h"
#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include "concurrent_queue.h"
#include "gps_navigation_message.h"
#include <sys/time.h>
#include <ctime>
#include <memory>

using google::LogMessage;

DECLARE_string(log_dir);

/*!
 * \todo  make this queue generic for all the GNSS systems (javi)
 */

/*
 * Concurrent queue that communicates the Telemetry Decoder
 * to the Observables modules
 */
concurrent_queue<Gps_Navigation_Message> global_gps_nav_msg_queue;

int main(int argc, char** argv)
{
    const std::string intro_help(
            std::string("\nGNSS-SDR is an Open Source GNSS Software Defined Receiver\n")
    +
    "Copyright (C) 2010-2012 (see AUTHORS file for a list of contributors)\n"
    +
    "This program comes with ABSOLUTELY NO WARRANTY;\n"
    +
    "See COPYING file to see a copy of the General Public License\n \n");


    google::SetUsageMessage(intro_help);
    google::SetVersionString(GNSS_SDR_VERSION);
    google::ParseCommandLineFlags(&argc, &argv, true);

    std::cout << "Initializing GNSS-SDR... Please wait." << std::endl;

    google::InitGoogleLogging(argv[0]);
    if (FLAGS_log_dir.empty())
        {
             // temp_directory_path() is only available from
             // Boost 1.45. Ubuntu 10.10 and Debian 6.0.6 ships with 1.42
             std::cout << "Logging will be done at "
                 //<< boost::filesystem::temp_directory_path()
                 << "/tmp"
                 << std::endl
                 << "Use gnss-sdr --log_dir=/path/to/log to change that."
                 << std::endl;
        }
    else
        {
            const boost::filesystem::path p (FLAGS_log_dir);
            if (!boost::filesystem::exists(p))
                {
                    std::cout << "The path "
                        << FLAGS_log_dir
                        << " does not exist, attempting to create it"
                        << std::endl;
                    boost::filesystem::create_directory(p);
                }
            std::cout << "Logging with be done at "
                << FLAGS_log_dir << std::endl;
        }

    std::unique_ptr<ControlThread> control_thread(new ControlThread());

    // record startup time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long int begin = tv.tv_sec * 1000000 + tv.tv_usec;

    control_thread->run();

    // report the elapsed time
    gettimeofday(&tv, NULL);
    long long int end = tv.tv_sec * 1000000 + tv.tv_usec;
    std::cout << "Total GNSS-SDR run time "
        << ((double)(end - begin))/1000000.0
        << " [seconds]" << std::endl;

    google::ShutDownCommandLineFlags();
    std::cout << "GNSS-SDR program ended." << std::endl;
}

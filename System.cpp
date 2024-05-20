#include "System.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

std::vector<size_t> get_cpu_times()
{
    std::ifstream proc_stat("/proc/stat");
    proc_stat.ignore(5, ' '); // Skip the 'cpu' prefix.
    std::vector<size_t> times;

    for (size_t time; proc_stat >> time;)
		times.push_back(time);

    return times;
}

bool get_cpu_times(size_t& idle_time, size_t& total_time)
{
    const std::vector<size_t> cpu_times = get_cpu_times();

    if (cpu_times.size() < 4)
		return false;

    idle_time = cpu_times[3]; // Idle time is the 4th value in /proc/stat
    total_time = std::accumulate(cpu_times.begin(), cpu_times.end(), 0);

    return true;
}

std::map<std::string, NetworkInterface> getNetworkUsage()
{
    std::map<std::string, NetworkInterface> interfaces;
    std::ifstream file("/proc/net/dev");
    std::string line;

    // Skip the first two header lines
    std::getline(file, line);
    std::getline(file, line);

    while (std::getline(file, line))
	{
        std::istringstream iss(line);
        std::string ifname;
        unsigned long int rx_bytes = 0;
        unsigned long int rx_packets = 0;
        unsigned long int error = 0;
        unsigned long int dummy = 0;
        unsigned long int tx_bytes = 0;
        unsigned long int tx_packets = 0;

        iss >> ifname
		// Receive
			>> rx_bytes >> rx_packets >> error
			// drop 	fifo 	 frame compressed  multicast
			>> dummy >> dummy >> dummy >> dummy >> dummy
		// Transmit
			>> tx_bytes >> tx_packets
			// errs drop fifo colls carrier compressed
			>> dummy >> dummy >> dummy >> dummy >> dummy >> dummy;

        if (ifname.find(":") != std::string::npos)
		{
			std::string name = ifname.substr(0, ifname.length() - 1);

			if(name == "lo")
				continue;

			if(name.find("docker") != std::string::npos)
				continue;

            interfaces[name] = {
				rx_bytes,
				tx_bytes
			};
		}
    }

    return interfaces;
}
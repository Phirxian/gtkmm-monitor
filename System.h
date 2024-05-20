#include <numeric>
#include <vector>
#include <string>
#include <map>

struct NetworkInterface
{
    unsigned long long rx_bytes;
    unsigned long long tx_bytes;
};

std::map<std::string, NetworkInterface> getNetworkUsage();

std::vector<size_t> get_cpu_times();
bool get_cpu_times(size_t& idle_time, size_t& total_time);
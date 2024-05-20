#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <glibmm/main.h>

#include "DynamicGraphDisplay.h"
#include "System.h"

constexpr float colors[][3] = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0},
	{1.0, 1.0, 0.0},
	{1.0, 0.0, 1.0},
	{0.0, 1.0, 1.0},
};

constexpr float step_size = 4.0;
constexpr unsigned int refresh_data_time = 500;
constexpr unsigned int refresh_time = 30;
constexpr float speed = 0.5;

class SystemMonitor : public Gtk::Window
{
	public:
		SystemMonitor();
		~SystemMonitor() override;

	private:
		bool on_cpu_timeout();
		bool on_net_timeout();
		bool on_redraw();

		std::map<std::string, NetworkInterface> previous_network;
		size_t previous_idle_time = 0;
		size_t previous_total_time = 0;

		DynamicGraphDisplay *cpu_monitor;
		DynamicGraphDisplay *recv_monitor;
		DynamicGraphDisplay *send_monitor;
		Gtk::Box *layout;
};

SystemMonitor::SystemMonitor()
{
    set_title("GTKmm 4.0 - System Monitor Demo");
    set_default_size(600, 200);

    layout = new Gtk::Box(Gtk::Orientation::VERTICAL, 2);
	layout->set_homogeneous(true);

	previous_network = getNetworkUsage();

	Glib::signal_timeout().connect(sigc::mem_fun(*this, &SystemMonitor::on_cpu_timeout), refresh_data_time);
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &SystemMonitor::on_net_timeout), refresh_data_time);

	Glib::signal_timeout().connect(sigc::mem_fun(*this, &SystemMonitor::on_redraw), refresh_time);

	cpu_monitor = new DynamicGraphDisplay();
    cpu_monitor->step_size = step_size;
	cpu_monitor->add_graph();
    layout->append(*cpu_monitor);

	recv_monitor = new DynamicGraphDisplay();
    recv_monitor->step_size = step_size;
    layout->append(*recv_monitor);

	send_monitor = new DynamicGraphDisplay();
    send_monitor->step_size = step_size;
    layout->append(*send_monitor);

    set_child(*layout);
    show();
}

SystemMonitor::~SystemMonitor()
{
    delete cpu_monitor;
    delete recv_monitor;
    delete send_monitor;
}

bool SystemMonitor::on_redraw()
{
	cpu_monitor->base_translation += speed;
	recv_monitor->base_translation += speed;
	send_monitor->base_translation += speed;
	
	cpu_monitor->queue_draw();
	recv_monitor->queue_draw();
	send_monitor->queue_draw();

	return true;
}

bool SystemMonitor::on_cpu_timeout()
{
    size_t idle_time, total_time;

    if (!get_cpu_times(idle_time, total_time))
		return false;

    const float idle_time_delta = idle_time - previous_idle_time;
    const float total_time_delta = total_time - previous_total_time;
    const float utilization = 1.0 - idle_time_delta / total_time_delta;

    previous_idle_time = idle_time;
    previous_total_time = total_time;

	cpu_monitor->add_peak(utilization*100, 0);
	cpu_monitor->base_translation = std::fmod(
		cpu_monitor->base_translation,
		cpu_monitor->step_size
	);

	return true;
}

bool SystemMonitor::on_net_timeout()
{
	auto data = getNetworkUsage();

	for(size_t i = recv_monitor->count(); i<data.size(); ++i)
		recv_monitor->add_graph(colors[i][0], colors[i][1], colors[i][2]);

	for(size_t i = send_monitor->count(); i<data.size(); ++i)
		send_monitor->add_graph(colors[i][0], colors[i][1], colors[i][2]);

	size_t i = 0;

	for (auto const& x : data)
	{
		long int rx_bytes_delta = (x.second.rx_bytes - previous_network[x.first].rx_bytes);
		long int tx_bytes_delta = (x.second.tx_bytes - previous_network[x.first].tx_bytes);
		recv_monitor->add_peak(rx_bytes_delta * 1e-5, i);
		send_monitor->add_peak(tx_bytes_delta * 1e-5, i);
		i++;
	}

	
	recv_monitor->base_translation = std::fmod(recv_monitor->base_translation, recv_monitor->step_size);
	send_monitor->base_translation = std::fmod(send_monitor->base_translation, send_monitor->step_size);

	previous_network = data;

	return true;
}

int main(int argc, char* argv[])
{
	auto app = Gtk::Application::create("org.gtkmm.examples.base");
	return app->make_window_and_run<SystemMonitor>(argc, argv);
}

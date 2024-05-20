#include <gtkmm/drawingarea.h>
#include <cairomm/context.h>

enum class PropertyType {
    PROP_STEP_SIZE = 1,
    PROP_TYPE,
};

struct MonitorData
{
	MonitorData()
	{
		r = g = b = 0.0f;
		max = 1.0f;
	}

	// curve color
	float r, g, b;
	// peak history
	std::vector<float> history;
	// distance between peak
	std::vector<float> step;
	// normalization factor
	float max;
};

class DynamicGraphDisplay : public Gtk::DrawingArea
{
	enum UPDATE_TYPE
	{
		GRU_STEP_BASED,
		GRU_CONTINIOUS,
	};

	enum RENDER_TYPE
	{
		GRR_BACKGROUND_ONLY,
		GRR_LINE,
		GRR_CURVE,
	};

	public:
		DynamicGraphDisplay();
		~DynamicGraphDisplay() override;

		Glib::RefPtr<MonitorData> add_graph(float r = 0.0, float g = 0.0, float b = 0.0);

		void add_peak(float peak, int index);
		void clear();

		inline void set_update_mode(UPDATE_TYPE type)
		{
			update = type;
    		queue_draw();
		}

		inline void set_render_type(RENDER_TYPE type)
		{
			render = type;
			switch(type)
			{
				case GRR_LINE:
    				set_draw_func(sigc::mem_fun(*this, &DynamicGraphDisplay::on_draw_line));
				break;
				case GRR_CURVE:
    				set_draw_func(sigc::mem_fun(*this, &DynamicGraphDisplay::on_draw_curve));
				break;
				default:
    				set_draw_func(sigc::mem_fun(*this, &DynamicGraphDisplay::on_draw_background));
			}
    		queue_draw();
		}

		inline Glib::RefPtr<MonitorData> get_graph(size_t i)
		{
			return graph[i];
		}

		inline size_t count()
		{
			return graph.size();
		}
 
		// only draw the background
		void on_draw_background(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
 
		// draw the background and render each graph as multiple line
		void on_draw_line(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);

		// draw the background and render each graph as secession of bezier curve
		void on_draw_curve(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);

		float base_translation = 0.f;
		float step_size = 2.f;
	private:
		void on_notify_theme_name();

		void cairo_set_color(const Cairo::RefPtr<Cairo::Context>& cr, float r, float g, float b, float alpha);

		bool dark_mode;
		UPDATE_TYPE update = GRU_CONTINIOUS;
		RENDER_TYPE render = GRR_CURVE;

		std::vector<Glib::RefPtr<MonitorData>> graph;
};


Gtk::Widget* xtm_process_monitor_new();
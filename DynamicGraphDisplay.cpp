#include <cairomm/surface.h>
#include <gtkmm/settings.h>
#include <iostream>

#include "DynamicGraphDisplay.h"

DynamicGraphDisplay::DynamicGraphDisplay()
{
    auto settings = Gtk::Settings::get_default();

	//settings->signal_notify_property("gtk-theme-name");
    //settings->signal_notify_property("gtk-theme-name").connect(
    //    sigc::mem_fun(*this, &DynamicGraphDisplay::on_notify_theme_name), false);

    signal_realize().connect(sigc::mem_fun(*this, &DynamicGraphDisplay::on_notify_theme_name), false);

	on_notify_theme_name();
	set_render_type(GRR_CURVE);
}

Glib::RefPtr<MonitorData> DynamicGraphDisplay::add_graph(float r, float g, float b)
{
	auto G = Glib::RefPtr<MonitorData>(new MonitorData);
	G->r = r;
	G->g = g;
	G->b = b;
	graph.push_back(G);
    return G;
}

DynamicGraphDisplay::~DynamicGraphDisplay()
{
    for (unsigned int i = 0; i < graph.size(); i++)
	{
        graph[i]->history.clear();
        graph[i]->step.clear();
	}
}

void DynamicGraphDisplay::on_notify_theme_name()
{
    auto settings = Gtk::Settings::get_default();
    std::string themename = settings->property_gtk_theme_name().get_value();
    bool prefdark = settings->property_gtk_application_prefer_dark_theme().get_value();

	// old theme without prefer dark property
	dark_mode = prefdark || themename.find("dark") != std::string::npos;

    queue_draw();
}

void DynamicGraphDisplay::cairo_set_color(const Cairo::RefPtr<Cairo::Context>& cr, float r, float g, float b, float alpha)
{
    if (dark_mode)
        cr->set_source_rgba(1.0 - r, 1.0 - g, 1.0 - b, alpha);
	else
        cr->set_source_rgba(r, g, b, alpha);
}


void DynamicGraphDisplay::on_draw_background(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height)
{
    if (height < 3)
        return;

    cr->rectangle(0.0, 0.0, width, height);
    cairo_set_color(cr, 0.96, 0.96, 0.96, 1.0);
    cr->fill_preserve();
    cairo_set_color(cr, 0.0, 0.0, 0.0, 1.0);
    cr->set_line_width(0.75);
    cr->stroke();

    cairo_set_color(cr, 0.0, 0.0, 0.0, 0.3);
    cr->set_line_width(1.0);
    std::vector<double> dashed = {1.5};
    cr->set_dash(dashed, 0);

    for (int i = 25; i <= 75; i += 25)
	{
        cr->move_to(1.5, i * height / 100 + 0.5);
        cr->line_to(width - 0.5, i * height / 100 + 0.5);
        cr->stroke();
    }

    cr->unset_dash();

    cr->set_line_width(0.85);
    cr->set_line_cap(Cairo::Context::LineCap::ROUND);
    cr->set_line_join(Cairo::Context::LineJoin::ROUND);
    cr->set_antialias(Cairo::Antialias::ANTIALIAS_DEFAULT);
}

void DynamicGraphDisplay::on_draw_line(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height)
{
	on_draw_background(cr, width, height);

    for (unsigned int g = 0; g < graph.size(); ++g)
	{
		auto G = graph[g];
        cairo_set_color(cr, G->r, G->g, G->b, 0.3f);
		float total_step = 0.0;

    	cr->move_to(width, height);

		if(G->history.size() == 0)
			continue;

        float peak = G->history[0] / G->max;
		float py = (1.0 - peak) * height;
    	cr->line_to(width, py);
		total_step += G->step[0];
        cr->translate(-G->step[0], 0);

		if(update == GRU_CONTINIOUS)
		{
			total_step += base_translation;
		    cr->translate(-base_translation, 0);
		}
		
        for (unsigned int i = 1; i<G->history.size(); ++i)
		{
            peak = G->history[i] / G->max;
			py = (1.0 - peak) * height;
			total_step += G->step[i];
            cr->line_to(width, py);
            cr->translate(-G->step[i], 0);
		}

        cr->line_to(width, py);
        cr->line_to(width, height);
        cr->fill_preserve();
        cairo_set_color(cr, G->r, G->g, G->b, 1.0f);
        cr->stroke();

        cr->translate(total_step, 0);
    }
}


void DynamicGraphDisplay::on_draw_curve(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height)
{
	on_draw_background(cr, width, height);

    for (unsigned int g = 0; g < graph.size(); ++g)
	{
		float total_step = 0.0;
		auto G = graph[g];

	    cr->move_to(width, height);
        cairo_set_color(cr, G->r, G->g, G->b, 0.3f);

		if(G->history.size() < 1)
			continue;

		float old_py = (1.0 - G->history[0] / G->max) * height;
		float py = old_py;
		float next_py = old_py;
		unsigned int i = 0;

		//total_step -= 2*G->step[0];
	    //cr->translate(2*G->step[0], 0);

		cr->line_to(width, height);
		cr->line_to(width, old_py);

		total_step += G->step[0];
	    cr->translate(-G->step[0], 0);

		if(update == GRU_CONTINIOUS)
		{
			total_step += base_translation;
		    cr->translate(-base_translation, 0);
		}
		
        for (i = 1; i<G->history.size()-1; ++i)
		{
			py = (1.0 - G->history[i] / G->max) * height;
			old_py = (1.0 - G->history[i-1] / G->max) * height;
			next_py = (1.0 - G->history[i+1] / G->max) * height;

			if (total_step > width)
				break;

			cr->curve_to(
				width-G->step[i-1]/4, (old_py+py)/2,
				width+G->step[i+1]/4, (next_py+py)/2,
				width, py
			);

			total_step += G->step[i];
		    cr->translate(-G->step[i], 0);

			old_py = py;
		}

		total_step += G->step[i];
		cr->translate(-G->step[i], 0);

		cr->curve_to(
			width+G->step[i-1]/4, (old_py+py)/2,
			width, next_py,
			width-G->step[i]/4, height
		);

		//cr->line_to(width, next_py);
		//cr->line_to(width, height);

        cr->fill_preserve();
        cairo_set_color(cr, G->r, G->g, G->b, 1.0f);
        cr->stroke();

        cr->translate(total_step, 0);
    }
}

void DynamicGraphDisplay::add_peak(float peak, int index)
{
    if (peak > graph[index]->max)
        graph[index]->max = peak;

    graph[index]->history.insert(graph[index]->history.begin(), peak);

	if(update == GRU_CONTINIOUS)
	{
    	graph[index]->step.insert(graph[index]->step.begin(), base_translation);
		base_translation = 0;
	}
	else
    	graph[index]->step.insert(graph[index]->step.begin(), step_size);

	// maximum history length
	if(graph[index]->history.size() > 10000)
	{
		graph[index]->history.pop_back();
		graph[index]->step.pop_back();
	}

    queue_draw();
}

void DynamicGraphDisplay::clear()
{
    for (unsigned int i = 0; i < graph.size(); ++i)
	{
        graph[i]->history.clear();
        graph[i]->step.clear();
        graph[i]->max = 1.0f;
	}
    queue_draw();
}

Gtk::Widget* xtm_process_monitor_new()
{
    return Gtk::manage(new DynamicGraphDisplay());
}
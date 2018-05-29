#ifndef GATE_UI_HPP
#define GATE_UI_HPP

#include <gtkmm/adjustment.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include "plugin_ui_base.hpp"

class GateUi : public Gtk::Grid, public PluginUiBase {
   public:
    GateUi(BaseObjectType* cobject,
           const Glib::RefPtr<Gtk::Builder>& refBuilder,
           const std::string& settings_name);
    ~GateUi();

    static std::shared_ptr<GateUi> create(std::string settings_name);

    void reset();

    void on_new_gating(double value);

   private:
    Gtk::Adjustment *attack, *release, *threshold, *knee, *ratio, *range,
        *makeup;
    Gtk::LevelBar* gating;
    Gtk::Label* gating_label;
    Gtk::ComboBoxText *detection, *stereo_link;
};

#endif

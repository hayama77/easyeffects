/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "effects_base_ui.hpp"

EffectsBaseUi::EffectsBaseUi(const Glib::RefPtr<Gtk::Builder>& builder,
                             Glib::RefPtr<Gtk::IconTheme> icon_ptr,
                             EffectsBase* effects_base,
                             const std::string& schema)
    : effects_base(effects_base),
      schema(schema),
      settings(Gio::Settings::create(schema)),
      app_settings(Gio::Settings::create("com.github.wwmm.easyeffects")),
      pm(effects_base->pm),
      selected_plugins(Gtk::StringList::create({"initial_value"})) {
  // loading builder widgets

  stack_top = builder->get_widget<Gtk::Stack>("stack_top");

  stack_plugins = builder->get_widget<Gtk::Stack>("stack_plugins");

  add_plugins_to_stack_plugins();

  settings->signal_changed("plugins").connect([&, this](const auto& key) { add_plugins_to_stack_plugins(); });

  // enabling notifications

  effects_base->bass_loudness->post_messages = true;
  effects_base->compressor->post_messages = true;
  effects_base->convolver->post_messages = true;
  effects_base->crossfeed->post_messages = true;
  effects_base->crystalizer->post_messages = true;
  effects_base->deesser->post_messages = true;
  effects_base->delay->post_messages = true;
  effects_base->echo_canceller->post_messages = true;
  effects_base->equalizer->post_messages = true;
  effects_base->exciter->post_messages = true;
  effects_base->filter->post_messages = true;
  effects_base->gate->post_messages = true;
  effects_base->limiter->post_messages = true;
  effects_base->loudness->post_messages = true;
  effects_base->maximizer->post_messages = true;
  effects_base->multiband_compressor->post_messages = true;
  effects_base->multiband_gate->post_messages = true;
  effects_base->output_level->post_messages = true;
  effects_base->pitch->post_messages = true;
  effects_base->reverb->post_messages = true;
  effects_base->rnnoise->post_messages = true;
  effects_base->stereo_tools->post_messages = true;
}

EffectsBaseUi::~EffectsBaseUi() {
  util::debug("effects_base_ui: destroyed");

  for (auto& c : connections) {
    c.disconnect();
  }

  // do not send notifications when the window is closed

  effects_base->deesser->post_messages = false;
  effects_base->delay->post_messages = false;
  effects_base->echo_canceller->post_messages = false;
  effects_base->filter->post_messages = false;
  effects_base->gate->post_messages = false;
  effects_base->loudness->post_messages = false;
  effects_base->multiband_compressor->post_messages = false;
  effects_base->multiband_gate->post_messages = false;
  effects_base->output_level->post_messages = false;
  effects_base->pitch->post_messages = false;
  effects_base->reverb->post_messages = false;
  effects_base->rnnoise->post_messages = false;
  effects_base->spectrum->post_messages = false;
  effects_base->stereo_tools->post_messages = false;

  // disabling bypass when closing the window

  effects_base->deesser->bypass = false;
  effects_base->delay->bypass = false;
  effects_base->echo_canceller->bypass = false;
  effects_base->filter->bypass = false;
  effects_base->gate->bypass = false;
  effects_base->loudness->bypass = false;
  effects_base->multiband_compressor->bypass = false;
  effects_base->multiband_gate->bypass = false;
  effects_base->output_level->bypass = false;
  effects_base->pitch->bypass = false;
  effects_base->reverb->bypass = false;
  effects_base->rnnoise->bypass = false;
  effects_base->spectrum->bypass = false;
  effects_base->stereo_tools->bypass = false;
}

void EffectsBaseUi::add_plugins_to_stack_plugins() {
  std::string path = "/" + schema + "/";

  std::replace(path.begin(), path.end(), '.', '/');

  // Adding to the stack the plugins in the list that are not there yet

  for (const auto& name : settings->get_string_array("plugins")) {
    if (name == plugin_name::deesser) {
      auto* const deesser_ui = DeesserUi::add_to_stack(stack_plugins, path);

      deesser_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->deesser->bypass = deesser_ui->bypass->get_active(); });

      effects_base->deesser->input_level.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_input_level));
      effects_base->deesser->output_level.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_output_level));
      effects_base->deesser->compression.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_compression));
      effects_base->deesser->detected.connect(sigc::mem_fun(*deesser_ui, &DeesserUi::on_new_detected));

      effects_base->deesser->bypass = false;
    } else if (name == plugin_name::echo_canceller) {
      auto* const echo_canceller_ui = EchoCancellerUi::add_to_stack(stack_plugins, path);

      echo_canceller_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->echo_canceller->bypass = echo_canceller_ui->bypass->get_active(); });

      effects_base->echo_canceller->input_level.connect(
          sigc::mem_fun(*echo_canceller_ui, &EchoCancellerUi::on_new_input_level));
      effects_base->echo_canceller->output_level.connect(
          sigc::mem_fun(*echo_canceller_ui, &EchoCancellerUi::on_new_output_level));

      effects_base->echo_canceller->bypass = false;
    } else if (name == plugin_name::gate) {
      auto* const gate_ui = GateUi::add_to_stack(stack_plugins, path);

      gate_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->gate->bypass = gate_ui->bypass->get_active(); });

      effects_base->gate->input_level.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_input_level));
      effects_base->gate->output_level.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_output_level));
      effects_base->gate->gating.connect(sigc::mem_fun(*gate_ui, &GateUi::on_new_gating));

      effects_base->gate->bypass = false;
    } else if (name == plugin_name::multiband_compressor) {
      auto* const multiband_compressor_ui = MultibandCompressorUi::add_to_stack(stack_plugins, path);

      multiband_compressor_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->multiband_compressor->bypass = multiband_compressor_ui->bypass->get_active(); });

      multiband_compressor_ui->set_pipe_manager_ptr(pm);

      effects_base->multiband_compressor->input_level.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_input_level));
      effects_base->multiband_compressor->output_level.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_output_level));

      effects_base->multiband_compressor->frequency_range.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_frequency_range));
      effects_base->multiband_compressor->envelope.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_envelope));
      effects_base->multiband_compressor->curve.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_curve));
      effects_base->multiband_compressor->reduction.connect(
          sigc::mem_fun(*multiband_compressor_ui, &MultibandCompressorUi::on_new_reduction));

      effects_base->multiband_compressor->bypass = false;
    } else if (name == plugin_name::multiband_gate) {
      auto* const multiband_gate_ui = MultibandGateUi::add_to_stack(stack_plugins, path);

      multiband_gate_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->multiband_gate->bypass = multiband_gate_ui->bypass->get_active(); });

      effects_base->multiband_gate->input_level.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_input_level));
      effects_base->multiband_gate->output_level.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output_level));

      effects_base->multiband_gate->output0.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output0));
      effects_base->multiband_gate->output1.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output1));
      effects_base->multiband_gate->output2.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output2));
      effects_base->multiband_gate->output3.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_output3));

      effects_base->multiband_gate->gating0.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating0));
      effects_base->multiband_gate->gating1.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating1));
      effects_base->multiband_gate->gating2.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating2));
      effects_base->multiband_gate->gating3.connect(
          sigc::mem_fun(*multiband_gate_ui, &MultibandGateUi::on_new_gating3));

      effects_base->multiband_gate->bypass = false;
    } else if (name == plugin_name::pitch) {
      auto* const pitch_ui = PitchUi::add_to_stack(stack_plugins, path);

      pitch_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->pitch->bypass = pitch_ui->bypass->get_active(); });

      effects_base->pitch->input_level.connect(sigc::mem_fun(*pitch_ui, &PitchUi::on_new_input_level));
      effects_base->pitch->output_level.connect(sigc::mem_fun(*pitch_ui, &PitchUi::on_new_output_level));

      effects_base->pitch->bypass = false;
    } else if (name == plugin_name::rnnoise) {
      auto* const rnnoise_ui = RNNoiseUi::add_to_stack(stack_plugins, path);

      rnnoise_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->rnnoise->bypass = rnnoise_ui->bypass->get_active(); });

      rnnoise_ui->set_transient_window(transient_window);

      effects_base->rnnoise->input_level.connect(sigc::mem_fun(*rnnoise_ui, &RNNoiseUi::on_new_input_level));
      effects_base->rnnoise->output_level.connect(sigc::mem_fun(*rnnoise_ui, &RNNoiseUi::on_new_output_level));

      effects_base->rnnoise->bypass = false;
    } else if (name == plugin_name::stereo_tools) {
      auto* const stereo_tools_ui = StereoToolsUi::add_to_stack(stack_plugins, path);

      stereo_tools_ui->bypass->signal_toggled().connect(
          [=, this]() { effects_base->stereo_tools->bypass = stereo_tools_ui->bypass->get_active(); });

      effects_base->stereo_tools->input_level.connect(
          sigc::mem_fun(*stereo_tools_ui, &StereoToolsUi::on_new_input_level));
      effects_base->stereo_tools->output_level.connect(
          sigc::mem_fun(*stereo_tools_ui, &StereoToolsUi::on_new_output_level));

      effects_base->stereo_tools->bypass = false;
    }
  }
}

void EffectsBaseUi::set_transient_window(Gtk::Window* transient_window) {
  this->transient_window = transient_window;

  for (auto* child = stack_plugins->get_first_child(); child != nullptr; child = child->get_next_sibling()) {
    const auto page = stack_plugins->get_page(*child);

    if (page->get_name() == plugin_name::rnnoise) {
      dynamic_cast<RNNoiseUi*>(child)->set_transient_window(transient_window);
    }
  }
}

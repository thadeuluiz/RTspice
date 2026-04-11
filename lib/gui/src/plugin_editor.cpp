#include "plugin_editor.hpp"

#include <string>

using namespace rtspice::gui;

plugin_editor::plugin_editor(plugin_processor& processor)
    : juce::AudioProcessorEditor(&processor), processor_(processor) {
  setSize(500, 320);

  open_button_.onClick = [this] { open_netlist_(); };
  addAndMakeVisible(open_button_);

  status_label_.setJustificationType(juce::Justification::topLeft);
  status_label_.setBorderSize({8, 8, 8, 8});
  addAndMakeVisible(status_label_);

  params_view_.setViewedComponent(&params_container_, false);
  params_view_.setScrollBarsShown(true, false);
  addAndMakeVisible(params_view_);

  refresh_status_();
  refresh_controls_();
}

void plugin_editor::resized() {
  auto bounds = getLocalBounds().reduced(10);
  open_button_.setBounds(bounds.removeFromTop(32));
  bounds.removeFromTop(8);
  status_label_.setBounds(bounds.removeFromTop(78));
  bounds.removeFromTop(8);
  params_view_.setBounds(bounds);
  layout_controls_();
}

void plugin_editor::open_netlist_() {
  file_chooser_ = std::make_unique<juce::FileChooser>("Choose an RTspice netlist", juce::File{},
                                                       "*.net");
  file_chooser_->launchAsync(
      juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
      [this](const juce::FileChooser& chooser) {
        const auto selected = chooser.getResult();
        if (selected == juce::File{}) {
          file_chooser_.reset();
          return;
        }

        std::string error_message;
        if (!processor_.load_netlist_file(selected, error_message)) {
          juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "RTspice",
                                                 error_message);
        }
        refresh_status_();
        refresh_controls_();
        file_chooser_.reset();
      });
}

void plugin_editor::refresh_status_() {
  status_label_.setText(processor_.status_text(), juce::dontSendNotification);
}

void plugin_editor::refresh_controls_() {
  param_controls_.clear();

  constexpr int row_height = 30;
  constexpr int label_width = 180;
  constexpr int row_margin = 6;

  const auto params = processor_.runtime_params();
  for (std::size_t i = 0; i < params.size(); ++i) {
    const auto& param = params[i];
    auto control = std::make_unique<param_control_>();
    control->name = param.name;

    control->label.setText(param.name, juce::dontSendNotification);
    control->label.setJustificationType(juce::Justification::centredLeft);
    params_container_.addAndMakeVisible(control->label);

    control->slider.setRange(0.0, 1.0, 0.001);
    control->slider.setValue(param.value, juce::dontSendNotification);
    control->slider.setSliderStyle(juce::Slider::LinearHorizontal);
    control->slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    control->slider.onValueChange = [this, name = control->name, slider = &control->slider] {
      processor_.set_runtime_param(name, static_cast<float>(slider->getValue()));
    };
    params_container_.addAndMakeVisible(control->slider);

    const int y = static_cast<int>(i) * (row_height + row_margin);
    control->label.setBounds(0, y, label_width, row_height);
    control->slider.setBounds(label_width + 8, y, 250, row_height);

    param_controls_.push_back(std::move(control));
  }

  const int total_rows = static_cast<int>(param_controls_.size());
  const int content_height =
      total_rows == 0 ? row_height : total_rows * (row_height + row_margin);
  params_container_.setSize(params_view_.getWidth() - 14, content_height);
  layout_controls_();
}

void plugin_editor::layout_controls_() {
  const auto width = juce::jmax(params_view_.getWidth() - 14, 350);
  constexpr int row_height = 30;
  constexpr int row_margin = 6;
  constexpr int label_width = 180;

  for (std::size_t i = 0; i < param_controls_.size(); ++i) {
    const int y = static_cast<int>(i) * (row_height + row_margin);
    param_controls_[i]->label.setBounds(0, y, label_width, row_height);
    param_controls_[i]->slider.setBounds(label_width + 8, y, width - label_width - 12,
                                         row_height);
  }
}

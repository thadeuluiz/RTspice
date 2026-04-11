#ifndef RTSPICE_PLUGIN_EDITOR_INC
#define RTSPICE_PLUGIN_EDITOR_INC

#include "plugin_processor.hpp"

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace rtspice::gui {

class plugin_editor : public juce::AudioProcessorEditor {
public:
  explicit plugin_editor(plugin_processor&);
  ~plugin_editor() override = default;

  void resized() override;

private:
  struct param_control_ {
    std::string name;
    juce::Label label;
    juce::Slider slider;
  };

  void open_netlist_();
  void refresh_status_();
  void refresh_controls_();
  void layout_controls_();

  plugin_processor& processor_;
  std::unique_ptr<juce::FileChooser> file_chooser_;
  juce::Viewport params_view_;
  juce::Component params_container_;
  std::vector<std::unique_ptr<param_control_>> param_controls_;
  juce::TextButton open_button_{"Open netlist (.net)"};
  juce::Label status_label_;
};

} // namespace rtspice::gui

#endif // RTSPICE_PLUGIN_EDITOR_INC

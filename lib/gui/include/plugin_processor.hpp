#ifndef RTSPICE_PLUGIN_PROCESSOR_INC
#define RTSPICE_PLUGIN_PROCESSOR_INC

#include "circuit.hpp"

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <memory>
#include <string>
#include <vector>

namespace rtspice::gui {

class plugin_editor;

class plugin_processor : public juce::AudioProcessor {
public:
  struct runtime_param {
    std::string name;
    float value;
  };

  plugin_processor();
  ~plugin_processor() override = default;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  bool isBusesLayoutSupported(const juce::AudioProcessor::BusesLayout& layouts) const override;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return "RTspice"; }
  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 0.0; }

  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int, const juce::String&) override {}

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  bool load_netlist_file(const juce::File& file, std::string& error_message);
  juce::String status_text() const;
  std::vector<runtime_param> runtime_params() const;
  void set_runtime_param(const std::string& name, float value);

private:
  bool parse_components_(const std::string& netlist,
                         std::vector<components::component::ptr>& components,
                         std::string& title,
                         std::string& error_message) const;
  void rebuild_io_bindings_();

  mutable juce::SpinLock state_lock_;
  std::unique_ptr<circuit::circuit> circuit_;

  std::vector<float*> input_bindings_;
  std::vector<circuit::entry_reference<const float>> output_bindings_;
  std::vector<std::pair<std::string, std::atomic<float>*>> param_bindings_;
  std::vector<std::string> input_names_;
  std::vector<std::string> output_names_;

  juce::File loaded_netlist_file_;
  std::string loaded_title_{"No netlist loaded"};
  double sample_rate_{44100.0};
};

} // namespace rtspice::gui

#endif // RTSPICE_PLUGIN_PROCESSOR_INC

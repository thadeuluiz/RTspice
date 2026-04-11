#include "plugin_processor.hpp"

#include "netlist_parser.hpp"
#include "plugin_editor.hpp"

#include <algorithm>
#include <exception>
#include <string>
#include <utility>
#include <vector>

using namespace rtspice::gui;

plugin_processor::plugin_processor()
    : juce::AudioProcessor(BusesProperties()
                               .withInput("Input", juce::AudioChannelSet::stereo(), true)
                               .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {}

bool plugin_processor::isBusesLayoutSupported(
    const juce::AudioProcessor::BusesLayout& layouts) const {
  const auto& input = layouts.getMainInputChannelSet();
  const auto& output = layouts.getMainOutputChannelSet();

  if (input.isDisabled() || output.isDisabled()) {
    return false;
  }

  return input == output && (input == juce::AudioChannelSet::mono() ||
                             input == juce::AudioChannelSet::stereo());
}

void plugin_processor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
  sample_rate_ = sampleRate;
}

void plugin_processor::releaseResources() {}

void plugin_processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
  juce::ScopedNoDenormals no_denormals;

  const juce::SpinLock::ScopedTryLockType lock(state_lock_);
  if (!lock.isLocked() || circuit_ == nullptr || sample_rate_ <= 0.0) {
    buffer.clear();
    return;
  }

  const auto num_samples = buffer.getNumSamples();
  const auto num_inputs = juce::jmin(getTotalNumInputChannels(), buffer.getNumChannels());
  const auto num_outputs = juce::jmin(getTotalNumOutputChannels(), buffer.getNumChannels());
  const auto delta_t = static_cast<float>(1.0 / sample_rate_);

  for (int sample = 0; sample < num_samples; ++sample) {
    for (std::size_t i = 0; i < input_bindings_.size(); ++i) {
      const auto channel = static_cast<int>(i);
      const auto value =
          channel < num_inputs ? buffer.getReadPointer(channel)[sample] : 0.0f;
      *input_bindings_[i] = value;
    }

    const auto step = circuit_->advance_(delta_t);

    for (std::size_t i = 0; i < output_bindings_.size(); ++i) {
      const auto channel = static_cast<int>(i);
      if (channel < num_outputs) {
        buffer.getWritePointer(channel)[sample] = step > 0 ? *output_bindings_[i] : 0.0f;
      }
    }

    for (int channel = static_cast<int>(output_bindings_.size()); channel < num_outputs;
         ++channel) {
      buffer.getWritePointer(channel)[sample] = 0.0f;
    }
  }
}

juce::AudioProcessorEditor* plugin_processor::createEditor() {
  return new plugin_editor(*this);
}

void plugin_processor::getStateInformation(juce::MemoryBlock& destData) {
  juce::ValueTree state("rtspice_state");
  state.setProperty("netlist_path", loaded_netlist_file_.getFullPathName(), nullptr);
  juce::MemoryOutputStream stream(destData, true);
  state.writeToStream(stream);
}

void plugin_processor::setStateInformation(const void* data, int sizeInBytes) {
  const auto state = juce::ValueTree::readFromData(data, static_cast<std::size_t>(sizeInBytes));
  if (!state.isValid()) {
    return;
  }

  const auto path = state.getProperty("netlist_path").toString();
  if (path.isNotEmpty()) {
    std::string error_message;
    load_netlist_file(juce::File(path), error_message);
  }
}

bool plugin_processor::parse_components_(const std::string& netlist,
                                         std::vector<components::component::ptr>& components,
                                         std::string& title,
                                         std::string& error_message) const {
  using namespace rtspice::parser;

  std::vector<std::string> statements;
  auto netlist_it = netlist.cbegin();
  const auto parsed_lines =
      qi::phrase_parse(netlist_it, netlist.cend(),
                       line_parser<std::string::const_iterator, qi::ascii::space_type>{},
                       qi::ascii::space, statements);
  if (!parsed_lines || netlist_it != netlist.cend()) {
    error_message = "Invalid netlist syntax.";
    return false;
  }

  if (statements.empty()) {
    error_message = "Netlist has no statements.";
    return false;
  }

  title = statements.front();
  for (auto statement_it = std::next(statements.cbegin()); statement_it != statements.cend();
       ++statement_it) {
    components::component::ptr component_;
    auto parse_it = statement_it->cbegin();
    const auto parsed_statement = qi::phrase_parse(
        parse_it, statement_it->cend(),
        statement_parser<std::string::const_iterator, qi::ascii::space_type>{},
        qi::ascii::space, component_);

    if (!parsed_statement || parse_it != statement_it->cend() || component_ == nullptr) {
      error_message = "Invalid statement: " + *statement_it;
      return false;
    }

    components.emplace_back(std::move(component_));
  }

  return true;
}

void plugin_processor::rebuild_io_bindings_() {
  input_bindings_.clear();
  output_bindings_.clear();
  param_bindings_.clear();
  input_names_.clear();
  output_names_.clear();

  if (circuit_ == nullptr) {
    return;
  }

  std::vector<std::pair<std::string, float*>> sorted_inputs;
  sorted_inputs.reserve(circuit_->inputs().size());
  for (auto& [name, value] : circuit_->inputs()) {
    sorted_inputs.emplace_back(name, &value);
  }
  std::sort(sorted_inputs.begin(), sorted_inputs.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

  std::vector<std::pair<std::string, circuit::entry_reference<const float>>> sorted_outputs;
  sorted_outputs.reserve(circuit_->outputs().size());
  for (auto& [name, value] : circuit_->outputs()) {
    sorted_outputs.emplace_back(name, value);
  }
  std::sort(sorted_outputs.begin(), sorted_outputs.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

  std::vector<std::pair<std::string, std::atomic<float>*>> sorted_params;
  sorted_params.reserve(circuit_->params().size());
  for (auto& [name, value] : circuit_->params()) {
    sorted_params.emplace_back(name, &value);
  }
  std::sort(sorted_params.begin(), sorted_params.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

  for (const auto& [name, ptr] : sorted_inputs) {
    input_names_.push_back(name);
    input_bindings_.push_back(ptr);
  }

  for (const auto& [name, ref] : sorted_outputs) {
    output_names_.push_back(name);
    output_bindings_.push_back(ref);
  }

  param_bindings_ = std::move(sorted_params);
}

bool plugin_processor::load_netlist_file(const juce::File& file, std::string& error_message) {
  if (!file.existsAsFile()) {
    error_message = "Netlist file does not exist.";
    return false;
  }

  std::vector<components::component::ptr> components;
  std::string title;
  if (!parse_components_(file.loadFileAsString().toStdString(), components, title, error_message)) {
    return false;
  }

  std::unique_ptr<circuit::circuit> new_circuit;
  try {
    new_circuit = std::make_unique<circuit::circuit>(std::move(components));
  } catch (const std::exception& ex) {
    error_message = ex.what();
    return false;
  }

  const juce::SpinLock::ScopedLockType lock(state_lock_);
  circuit_ = std::move(new_circuit);
  loaded_netlist_file_ = file;
  loaded_title_ = std::move(title);
  rebuild_io_bindings_();
  return true;
}

juce::String plugin_processor::status_text() const {
  const juce::SpinLock::ScopedLockType lock(state_lock_);
  juce::String status;
  status << "Title: " << loaded_title_ << "\n";
  status << "Inputs: " << static_cast<int>(input_names_.size()) << "\n";
  status << "Outputs: " << static_cast<int>(output_names_.size()) << "\n";
  status << "Params: " << static_cast<int>(param_bindings_.size()) << "\n";
  if (loaded_netlist_file_.existsAsFile()) {
    status << "Netlist: " << loaded_netlist_file_.getFileName();
  } else {
    status << "Netlist: (none)";
  }
  return status;
}

std::vector<plugin_processor::runtime_param> plugin_processor::runtime_params() const {
  const juce::SpinLock::ScopedLockType lock(state_lock_);
  std::vector<runtime_param> params;
  params.reserve(param_bindings_.size());

  for (const auto& [name, value] : param_bindings_) {
    params.push_back({name, value->load(std::memory_order_relaxed)});
  }

  return params;
}

void plugin_processor::set_runtime_param(const std::string& name, float value) {
  const juce::SpinLock::ScopedLockType lock(state_lock_);
  for (auto& [param_name, param_value] : param_bindings_) {
    if (param_name == name) {
      param_value->store(std::clamp(value, 0.0f, 1.0f), std::memory_order_relaxed);
      return;
    }
  }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new plugin_processor();
}

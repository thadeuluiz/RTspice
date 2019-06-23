#headers needed as compilation files for Qt
set(sources include/main_window.hpp
            include/circuit_widget.hpp
            include/jack_widget.hpp
            include/knob.hpp

            src/main_window.cpp
            src/circuit_widget.cpp
            src/jack_widget.cpp
            src/knob.cpp)

add_library(gui ${sources})

target_include_directories(gui
  PUBLIC
  include/)

target_link_libraries(gui
  PUBLIC
  circuit
  parser
  Qt5::Widgets)

add_library(gui include/main_window.hpp include/knob.hpp src/main_window.cpp src/knob.cpp)
target_include_directories(gui PUBLIC include/)
target_link_libraries(gui PUBLIC Qt5::Widgets)

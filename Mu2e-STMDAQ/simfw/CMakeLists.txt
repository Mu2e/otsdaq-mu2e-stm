# simfw/CMakeLists.txt

add_library(Sim SHARED
  gen_data.cc
)
set_target_properties(Sim PROPERTIES OUTPUT_NAME "Sim")
target_link_libraries(Sim PRIVATE stmdaq_all_deps)

add_executable(run_sim.exe
  run_sim.cc
)

target_link_libraries(run_sim.exe PRIVATE
  stmdaq_all_deps
  stmdaq_python_deps
  Sim

  Utils
  Buffers
  Processing
  Config    
)

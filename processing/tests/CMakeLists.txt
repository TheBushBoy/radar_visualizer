add_executable(test_load test_load.cpp)
target_compile_definitions(test_load PRIVATE RADAR_DATA_DIR="${RADAR_DATA_DIR}")
target_link_libraries(test_load PRIVATE radar_processing)
add_test(NAME test_load COMMAND test_load)

add_executable(test_metrics test_metrics.cpp)
target_compile_definitions(test_metrics PRIVATE RADAR_DATA_DIR="${RADAR_DATA_DIR}")
target_link_libraries(test_metrics PRIVATE radar_processing)
add_test(NAME test_metrics COMMAND test_metrics)

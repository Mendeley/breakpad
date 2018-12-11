include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/)

set(BREAKPAD_LIBS breakpad)
if (APPLE)
  find_library(APPLICATIONSERVICES_LIB ApplicationServices)
  find_library(FOUNDATION_LIB Foundation)
  set(BREAKPAD_LIBS ${BREAKPAD_LIBS} ${FOUNDATION_LIB} ${APPLICATIONSERVICES_LIB})
endif()

add_executable(buggy_app buggy_app.cc)
target_link_libraries(buggy_app ${BREAKPAD_LIBS})

# the test app must be built with at least basic
# debug info in order to successfully extract function
# names from it
if (UNIX)
  target_link_libraries(buggy_app pthread)
  set_target_properties(buggy_app PROPERTIES COMPILE_FLAGS -g)

  # the test currently relies on minidump_stackwalk which is currently only built
  # under Linux/Mac
  add_test(stacktrace_test python ${CMAKE_CURRENT_SOURCE_DIR}/stacktrace_test.py
           WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

set_target_properties(
	buggy_app
	PROPERTIES 
		FOLDER "Externals/breakpad"
)


if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
	include(CodeCoverage)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_COVERAGE})
  SETUP_TARGET_FOR_COVERAGE(
                  coverage            # Name for custom target.
                  mfast_test          # Name of the test driver executable that runs the tests.
                                      # NOTE! This should always have a ZERO as exit code
                                      # otherwise the coverage generation will not complete.
                  coverage            # Name of output directory.
                  )
endif()

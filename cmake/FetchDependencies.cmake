

include(FetchContent)

find_package(Catch2 CONFIG)
find_package(tinyxml2 CONFIG)

if(NOT Catch2_FOUND OR NOT tinyxml2_FOUND)
   include(FetchContent)
endif()

if(NOT Catch2_FOUND)
   FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG v2.13.7
   )
   set(CATCH_BUILD_STATIC_LIBRARY ON)
   FetchContent_MakeAvailable(Catch2)
endif()

if(tinyxml2_FOUND)
   set(TINYXML2_LIB tinyxml2::tinyxml2)
else()
   FetchContent_Declare(
      tinyxml2
      GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
      GIT_TAG 9.0.0
   )

   if(NOT tinyxml2_POPULATED)
      # Fetch the content using previously declared details
      FetchContent_Populate(tinyxml2)
      set(TINYXML2_SRC ${tinyxml2_SOURCE_DIR}/tinyxml2.cpp)
   endif()
endif()
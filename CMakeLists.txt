cmake_minimum_required(VERSION 3.22)
project(FileStructureComparison)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        libfmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG        b6f4ceaed0a0a24ccf575fab6c56dd50ccf6f1a9 # release-8.1.1
)
FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG        master
)
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.x
)
FetchContent_Declare(
        date
        GIT_REPOSITORY https://github.com/HowardHinnant/date.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(date libfmt spdlog cxxopts)

add_executable(FileStructureComparison main.cpp)

target_include_directories(FileStructureComparison SYSTEM PUBLIC ${libfmt_SOURCE_DIR}/include)
target_include_directories(FileStructureComparison SYSTEM PUBLIC ${cxxopts_SOURCE_DIR}/include)
target_include_directories(FileStructureComparison SYSTEM PUBLIC ${spdlog_SOURCE_DIR}/include)
target_include_directories(FileStructureComparison SYSTEM PUBLIC ${date_SOURCE_DIR}/include)

target_compile_definitions(FileStructureComparison PUBLIC FMT_HEADER_ONLY)


# The above includes are marked as SYSTEM to prevent warnings, but CMAKE can't
# enforce that on MSVC since the compiler flags are new, so manually add them
# if MSVC is the compiler.
if (MSVC)
    target_compile_options(FileStructureComparison PRIVATE /external:anglebrackets /external:W0)
endif ()

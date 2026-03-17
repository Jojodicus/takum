include(FetchContent)
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1)
FetchContent_MakeAvailable(Catch2)

add_executable(test_takum test_takum.cpp)
target_link_libraries(test_takum PRIVATE takum Catch2::Catch2WithMain)
add_test(NAME takum COMMAND test_takum)

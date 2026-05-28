# 项目名称
# project(MemoryPoolProject)

# 设置 C++ 标准
# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED True)

# 头文件目录
# include_directories(include)

# 源文件
# file(GLOB SRC_FILES "src/*.cpp")
# file(GLOB TEST_FILES "tests/*.cpp")

# 为项目添加可执行文件
add_executable(${PROJECT_NAME} ${SRC_FILES} ${TEST_FILES})

# 添加编译选项
target_compile_options(${PROJECT_NAME} PRIVATE -g -pthread)

# Link libraries
# target_link_libraries(${PROJECT_NAME} pthread)

# Link libraries (if any)
# target_link_libraries(${PROJECT_NAME} <library_name>)
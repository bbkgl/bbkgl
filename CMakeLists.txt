cmake_minimum_required(VERSION 3.10)
project(bbkgl)

set(CMAKE_CXX_STANDARD 11)

# 设置库的输出路径，PROJECT_BINARY_DIR在此处是执行cmake的目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
# 设置可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
# 设置安装库的路径
set(INSTALL_PATH_LIB lib)
# 设置安装头文件的路径
set(INSTALL_PATH_INCLUDE include)


set(SRC_LIST "base" "net" "http")

foreach(SUB_DIR ${SRC_LIST})
    include_directories(${PROJECT_SOURCE_DIR}/src/${SUB_DIR})
    aux_source_directory(src/${SUB_DIR} SUB_LIST)

    install(DIRECTORY src/${SUB_DIR} DESTINATION ${INSTALL_PATH_INCLUDE} FILES_MATCHING PATTERN "*.h")
endforeach()

# 将所有源文件打包成动态库Sinetlib_shared
add_library(${CMAKE_PROJECT_NAME}_shared SHARED ${SUB_LIST})
# 链接到需要的库pthread
target_link_libraries(${CMAKE_PROJECT_NAME}_shared pthread)
# 将生成的库文件名改为Sinetlib（系统会自动扩为libSinetlib.so）
set_target_properties(${CMAKE_PROJECT_NAME}_shared PROPERTIES OUTPUT_NAME "${CMAKE_PROJECT_NAME}")
# 安装到/usr/local/lib，同样的，在执行make install时才会执行
install(TARGETS ${CMAKE_PROJECT_NAME}_shared ARCHIVE DESTINATION ${INSTALL_PATH_LIB} LIBRARY DESTINATION ${INSTALL_PATH_LIB})

# 将所有源文件打包成静态库Sinetlib_static
add_library(${CMAKE_PROJECT_NAME}_static STATIC ${SUB_LIST})
# 链接到需要的库pthread
target_link_libraries(${CMAKE_PROJECT_NAME}_shared pthread)
# 将生成的库文件名改为Sinetlib（系统会自动扩为libSinetlib.a）
set_target_properties(${CMAKE_PROJECT_NAME}_static PROPERTIES OUTPUT_NAME "${CMAKE_PROJECT_NAME}")
# 安装到/usr/local/lib，同样的，在执行make install时才会执行
install(TARGETS ${CMAKE_PROJECT_NAME}_static ARCHIVE DESTINATION ${INSTALL_PATH_LIB})

# 添加子目录，这样example目录下的CMakeLists.txt也会被执行
add_subdirectory(example)
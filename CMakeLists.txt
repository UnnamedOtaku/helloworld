cmake_minimum_required(VERSION 3.15)

project(TEST)

find_package(OpenGL REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Stb REQUIRED)

set(CMAKE_CXX_STANDARD 20)

file(GLOB SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES}
	src/stb_image.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/build/debug"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build/release"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_SOURCE_DIR}/assets"
		"$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
	COMMENT "Copying assets directory to build output"
	VERBATIM
)

target_include_directories(${PROJECT_NAME} PRIVATE ${Stb_INCLUDE_DIR} include)

target_link_libraries(${PROJECT_NAME} PRIVATE glad::glad glfw glm::glm ${OPENGL_LIBRARIES} gdi32 winmm)


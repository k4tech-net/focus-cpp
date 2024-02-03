cmake_minimum_required(VERSION 3.15)
project(nanovg)
set(CMAKE_CXX_STANDARD 17)

set(nanovg_dir ${CMAKE_CURRENT_LIST_DIR})
file(GLOB nanovg_sources ${nanovg_dir}/src/*.c ${nanovg_dir}/src/*.h)
add_library(nanovg STATIC ${nanovg_sources})
target_include_directories(nanovg PUBLIC ${nanovg_dir}/src)
# target_include_directories(nanovg PUBLIC ${nanovg_dir}/..)


option(NANOVG_BUILD_EXAMPLES "Build nanovg examples" OFF)

if(NANOVG_BUILD_EXAMPLES)
    # nanovg example
    find_package(glfw3 REQUIRED)

    if (APPLE)
        set(nanovg_example_link_libraries "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo" "-framework Carbon")
    endif()

    add_executable(nanovg_example_gl3
        ${nanovg_dir}/example/example_gl3_refac.cpp
        ${nanovg_dir}/example/demo.c
        ${nanovg_dir}/example/perf.c
        ${nanovg_dir}/example/example_stb_impl_image.c
        ${nanovg_dir}/example/example_stb_impl_image_write.c
    )
    target_link_libraries(nanovg_example_gl3
        PRIVATE
        nanovg
        glfw
        ${nanovg_example_link_libraries}
    )

    add_executable(nanovg_example_fbo
        ${nanovg_dir}/example/example_fbo.c
        ${nanovg_dir}/example/perf.c
        ${nanovg_dir}/example/example_stb_impl_image.c
        ${nanovg_dir}/example/example_stb_impl_image_write.c
    )
    target_link_libraries(nanovg_example_fbo
        PRIVATE
        nanovg
        glfw
        ${nanovg_example_link_libraries}
    )

endif()

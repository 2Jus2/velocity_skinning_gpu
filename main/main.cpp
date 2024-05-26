#include <mach/mach.h>
#include <iostream>
#include <chrono>
#include <map>

// Include VCL library
#include "vcl/vcl.hpp"

// Include common part for exercises
#include "main/helper_scene/helper_scene.hpp"

// Include exercises
#include "scenes/scenes.hpp"

// ************************************** //
// Global data declaration
// ************************************** //

// Storage for shaders indexed by their names
std::map<std::string, GLuint> shaders;

// General shared elements of the scene such as camera and its controller, visual elements, etc.
scene_structure scene;

// The graphical interface. Contains Window object and GUI related variables.
gui_structure gui;

// Part specific data - you will specify this object in the corresponding exercise part.
scene_model scene_current;

// Timer query for GPU time measurement
GLuint timerQuery;

// Global counter for GPU memory usage
size_t totalGpuMemoryUsed = 0;

// CPU memory usage measurement 
void reportMemoryUsage() {
    mach_port_t host_port = mach_host_self();
    mach_msg_type_number_t host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
    vm_size_t page_size;
    vm_statistics_data_t vm_stat;

    host_page_size(host_port, &page_size);
    if (host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) == KERN_SUCCESS) {
        long long free_memory = vm_stat.free_count * page_size;
        long long used_memory = (vm_stat.active_count + vm_stat.inactive_count + vm_stat.wire_count) * page_size;
        std::cout << "Free memory: " << free_memory / 1024 / 1024 << " MB, ";
        std::cout << "Used memory: " << used_memory / 1024 / 1024 << " MB" << std::endl;
    }
}

// Enhanced buffer creation with logging for GPU memory usage
GLuint createBuffer(GLsizeiptr size, GLenum usage) {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, usage);
    totalGpuMemoryUsed += size; // Update a global counter
    std::cout << "Allocated GPU buffer of size: " << size << " bytes" << std::endl;
    return buffer;
}

// ************************************** //
// GLFW event listeners
// ************************************** //

void window_size_callback(GLFWwindow* /*window*/, int width, int height);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_click_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// ************************************** //
// Start program
// ************************************** //

int main() {
    vcl::unit_test();

    // ************************************** //
    // Initialization and data setup
    // ************************************** //

    // Initialize external libraries and window
    initialize_interface(gui);

    // Set GLFW events listener
    glfwSetCursorPosCallback(gui.window, cursor_position_callback);
    glfwSetMouseButtonCallback(gui.window, mouse_click_callback);
    glfwSetScrollCallback(gui.window, mouse_scroll_callback);
    glfwSetKeyCallback(gui.window, keyboard_input_callback);
    glfwSetWindowSizeCallback(gui.window, window_size_callback);

    load_shaders(shaders);
    setup_scene(scene, gui, shaders);

    // Initialize GPU timer query
    glGenQueries(1, &timerQuery);

    opengl_debug();
    std::cout << "*** Setup Data ***" << std::endl;
    scene_current.setup_data(shaders, scene, gui);
    std::cout << "\t [OK] Data setup" << std::endl;
    opengl_debug();

    // ************************************** //
    // Animation loop
    // ************************************** //

    std::cout << "*** Start GLFW animation loop ***" << std::endl;
    vcl::glfw_fps_counter fps_counter;

    while (!glfwWindowShouldClose(gui.window)) {
        opengl_debug();

        // Start GPU timer query
        glBeginQuery(GL_TIME_ELAPSED, timerQuery);

        auto start_time = std::chrono::high_resolution_clock::now(); // Start timing

        // Clear all color and zbuffer information before drawing on the screen
        clear_screen();
        opengl_debug();
        // Set a white image texture by default

        // Create the basic GUI structure with ImGui
        gui_start_basic_structure(gui, scene);

        // Perform computation and draw calls for each iteration loop
        scene_current.frame_draw(shaders, scene, gui);
        opengl_debug();

        auto end_time = std::chrono::high_resolution_clock::now(); // End timing
        std::chrono::duration<double, std::milli> elapsed_time = end_time - start_time;
        std::cout << "Computation Time (Per frame): " << elapsed_time.count() << " ms" << std::endl; // Output elapsed time

        // End GPU timer query
        glEndQuery(GL_TIME_ELAPSED);

        GLuint64 elapsedTime;
        glGetQueryObjectui64v(timerQuery, GL_QUERY_RESULT, &elapsedTime);
        double timeInMilliseconds = elapsedTime / 1000000.0;
        std::cout << "GPU Time (Per frame): " << timeInMilliseconds << " ms" << std::endl;

        // Report CPU memory usage for the current frame
        reportMemoryUsage();

        // Report total GPU memory usage
        std::cout << "Total GPU Memory Used: " << totalGpuMemoryUsed / 1024 / 1024 << " MB" << std::endl;

        // Render GUI and update window
        ImGui::End();

        scene.camera_control.update = !(ImGui::IsAnyWindowFocused());
        vcl::imgui_render_frame(gui.window);

        update_fps_title(gui.window, gui.window_title, fps_counter);

        glfwSwapBuffers(gui.window);
        glfwPollEvents();
        opengl_debug();
    }
    std::cout << "*** Stop GLFW loop ***" << std::endl;

    // Cleanup ImGui and GLFW
    vcl::imgui_cleanup();

    glfwDestroyWindow(gui.window);
    glfwTerminate();

    return 0;
}

void window_size_callback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
    scene.camera.perspective.image_aspect = width / static_cast<float>(height);;
    scene.window_width = width;
    scene.window_height = height;
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    scene.camera_control.update_mouse_move(scene.camera, window, float(xpos), float(ypos));
    scene_current.mouse_move(scene, window);
}

void mouse_click_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGui::SetWindowFocus(nullptr);
    scene.camera_control.update_mouse_click(scene.camera, window, button, action, mods);
    scene_current.mouse_click(scene, window, button, action, mods);
}

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    scene_current.mouse_scroll(scene, window, float(xoffset), float(yoffset));
}

void keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    scene_current.keyboard_input(scene, window, key, scancode, action, mods);
}

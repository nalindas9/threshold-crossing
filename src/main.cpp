#include <direct.h>
#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <stdlib.h>     /* srand, rand */

#include <vector>
#include <utility>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stuff.hpp"

#include "simulated_data.hpp"
#include "iostream"
#include<windows.h>
#include <thread>
/************************** DESCRIPTION **************************/
// We're going to produce some data on a separate thread and give you access to a queue where you can retrieve
// the data for processing.

// This data will be of type simdata::simulated_data, which you can see in "simulated_data.hpp".
// Each data will contain an array of floats with have length 1500. These data will be produced
// at a rate of 10,000 per second.

// GOAL:
// The goal of this project is to do the following

// * Display the newest data
// *** This is being done in the given implementation, but will require some rethinking after modification to perform the below tasks as well.

// * Detect and display "threshhold crossings." For this, the first point at which the amplitude crosses the threshhold is reported.
//   No further reporting is done while the amplitude remains above the threshhold. If the amplitude falls below the threshhold and
//   then rises above it again, then a new report is generated. 
//   More specifically:
// *** Define the threshhold as a value of 0.9f
// *** For a given data point, with a particular sequence_id and data_index, a threshhold crossing has occurred when:
// ***** The amplitude is greater than or equal to 0.9f
// ***** The data of the previous 'sequence' did not have a maximum data point greater than or equal to 0.9f
// ***** The amplitude of the preceeding data point did not have an amplitude greater than or equal to 0.9f
// *** The display can be done like so --- ImGui::Text("Threshhold crossed in sequence %zu at index %zu", sequence_id, data_index);
// *** A video of the expected output will be provided. The data is generated deterministically, so the outputs you generate should
//     be the same as in the video.
/************************** /DESCRIPTION *************************/
void foo(size_t id, int max_idx) {
    std::cout << "hello\n";
    ImGui::Text("Threshhold crossed in sequence %zu at index %zu \n", id, max_idx);
}

std::deque<simdata::ptr<simdata::simulated_data>> data_q = {};
int main()
{
    /************** GUI initialization stuff, don't worry about this **************/
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar;
    window_flags = window_flags |= ImGuiWindowFlags_MenuBar;

    auto window = imgui_stuff::ImguiStartup();
    if (!window) return 1;

    imgui_stuff::init(
        "fonts/cambria.ttf",
        "fonts/cambriab.ttf",
        "fonts/cambriai.ttf",
        "fonts/cambriaz.ttf",
        18.0f, 16.0f);
    /******************************************************************************/

    // This is the queue that holds the simulated data as it is produced.
    // Try taking a look at it's member functions and understand what they do.
    simdata::worker_queue<simdata::simulated_data> q;

    // The thread to produce simulated data is set up here

    // This is the function that the thread uses to produce the data
    auto producer_fn = []()
    {
        static simdata::simulation_state sim_state;
        return sim_state.produce_data();
    };


    // The thread is instantiated here.
    simdata::producer_thread<simdata::simulated_data> producer(producer_fn, &q);

    // If you want to create your own thread to process the data, feel free to use an object of type
    // simdata::worker_thread<simdata::simulated_data>

    // Main loop
    // This should be running at the refresh rate of your monitor, so probably 60 Hz
    //simdata::ptr<simdata::simulated_data> data;
    //data->values = {};
    float previous_max = 0;
    int max_idx = 0;
    simdata::ptr<simdata::simulated_data> data;
    
    
    auto previous_data = data;

    std::cout << "q size: " << q.size_hint() << std::endl;
    std::unordered_map<size_t, std::pair<bool, int>> threshold_crossings = {};
    int frame_cnt = 0;
    
    std::string exit = "";
    while (!glfwWindowShouldClose(window))
    {
        imgui_stuff::MainLoopBegin(window_flags);
        //std::cout << "Enter esc to exit\n";
        //std::cin >> exit;
        frame_cnt++;
        auto newest_data = q.try_pop_newest();
        
        //std::cout << "q size: " << q.size_hint() << std::endl;
        if (newest_data && (previous_data != NULL))
        {
            
            //std::cout << "newest_data: " << newest_data->sequence_id << " previous_data:" << previous_data->sequence_id << std::endl;
            ImGui::Text("Sequence: %zu", newest_data->sequence_id);
            data_q.push_back(newest_data);
            // ***** The amplitude is greater than or equal to 0.9f
            float max = *std::max_element(newest_data->values.begin(),
                                          newest_data->values.end());
            // ***** The data of the previous 'sequence' did not have a maximum data point greater than or equal to 0.9f
            previous_max = *std::max_element(previous_data->values.begin(),
                                             previous_data->values.end());
            max_idx = std::max_element(previous_data->values.begin(),
                                       previous_data->values.end()) - previous_data->values.begin();

            //std::cout << "max: " << max << " previous_max:" << previous_max << std::endl;

            ImGui::PlotLines("##data", newest_data->values.data(), 
                             newest_data->values.size(), 
                             0, 
                             0,
                             0.0f, 
                             1.0f, 
                             { 800, 400 });
            if ((max >= 0.9) && (previous_max < 0.9)) {
                ImGui::Text("Threshhold crossed in sequence %zu at index %zu \n", newest_data->sequence_id, max_idx);
                std::pair<bool, int> value;
                value.first = true;
                value.second = max_idx;
                threshold_crossings[newest_data->sequence_id] = value;
            }
        }
        
        previous_data = newest_data;
        imgui_stuff::MainLoopEnd(window);
    }

    std::cout << "dataq size: " << data_q.size() << std::endl;
    std::vector<size_t> sequence_ids = {};
    for (int i = 0; i < data_q.size(); i++) {
        imgui_stuff::MainLoopBegin(window_flags);
        std::cout << "Enter\n";
        auto data = data_q[i];
        if (data) {
            ImGui::Text("Sequence: %zu", data->sequence_id);
            ImGui::PlotLines("##data", data->values.data(),
                data->values.size(),
                0,
                0,
                0.0f,
                1.0f,
                { 800, 400 });

            if (threshold_crossings.find(data->sequence_id) != threshold_crossings.end()) {
                threshold_crossings[data->sequence_id].first = true;
                sequence_ids.push_back(data->sequence_id);
            }
            
            for (int i = 0; i < sequence_ids.size(); i++)
                ImGui::Text("Threshhold crossed in sequence %zu at index %zu \n", 
                            sequence_ids[i],
                            threshold_crossings[sequence_ids[i]].second);
        }
        Sleep(1);
        imgui_stuff::MainLoopEnd(window);
    }
    
    imgui_stuff::ImguiShutdown(window);

    return 0;

}
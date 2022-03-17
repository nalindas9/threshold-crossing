# threshold-crossing

[![License:MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://github.com/nalindas9/threshold-crossing/blob/master/LICENSE)

## About
 We're going to produce some data on a separate thread and give you access to a queue where you can retrieve the data for processing. 
 This data will be of type simdata::simulated_data, which you can see in "simulated_data.hpp".
 Each data will contain an array of floats with have length 1500. These data will be produced at a rate of 10,000 per second.
 
 GOAL: The goal of this project is to do the following

 - Display the newest data
 
   This is being done in the given implementation, but will require some rethinking after modification to perform the below tasks as well.

 - Detect and display "Threshhold crossed in sequence %zu at index %zu, sequence_id, data_index" 
 
   For this, the first point at which the amplitude crosses the threshhold is reported. 
   No further reporting is done while the amplitude remains above the threshhold. If the amplitude falls below the threshhold and 
   then rises above it again, then a new report is generated. 
   
   - More specifically:
   
      Define the threshhold as a value of 0.9f
      For a given data point, with a particular sequence_id and data_index, a threshhold crossing has occurred when: 
         - The amplitude is greater than or equal to 0.9f
         - The data of the previous 'sequence' did not have a maximum data point greater than or equal to 0.9f
         - The amplitude of the preceeding data point did not have an amplitude greater than or equal to 0.9f
         - The data is generated deterministically. So output should be same everytime the program runs.

## Output

https://user-images.githubusercontent.com/44141068/158737827-4fd1d087-8400-4e7c-a6d9-4b7bdd725558.mp4

## System and library requirements
- C++ 20
- CMake version 3.22.22022201-MSVC_2
- Windows 10

## How to Run
- Install Visual Studio 2019+. The free Community Edition is perfect.
You will need the usual c++ features installed.

- Open the folder in Visual Studio.
In Visual Studio, click File -> Open -> Folder
OR
Right click in File Explorer -> Open with Visual Studio

- Be patient while CMake cache is generated.
You may open the CMakeLists.txt file in Visual Studio and save it. Even if it hasn't changed, this will trigger cache generation.

- Once the cache starts to generate, at the top you will see a pulldown with "Select Startup Item."
Wait until the cache has generated. You will see "Cmake generation finished." in the output window.
Once that is done, the "Select Startup Item" pulldown will have, as the last option, "trial_project.exe". Select it.

- You can now select compile and run. You can change between Debug and Release mode with the pulldown to the left of the "Select Startup Item" pulldown.

## Reference
- [ImGUI](https://github.com/ocornut/imgui)

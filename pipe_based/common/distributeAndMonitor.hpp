// #pragma once
// #include "hyperion.hpp"

// typedef struct
// {
//     const char *host;
//     const unsigned short port;
//     const int size; // number of lights
// } Slave;

// typedef std::vector<Slave> Distribution;

// template <class T_INPUT_COLOR = RGBA>
// std::vector<InputSlicer::Slice> createSlices(
//     ControlHubInput<T_INPUT_COLOR> *input,
//     Distribution slaves
//     )
// {
//     std::vector<InputSlicer::Slice> slices;
//     int start = 0;
//     for(auto slave : slaves){
//         slices.push_back({ 
//             int( start * sizeof(T_INPUT_COLOR)), 
//             int( slave.size * sizeof(T_INPUT_COLOR)), 
//             true
//         });
//         start += slave.size;
//     }

//     slices.push_back({0, int( input->length() * sizeof(T_INPUT_COLOR)), false});
 
//     if (start != input->length()){
//         Log::error("COMMON","createSliceAndMonitorPipes: Total length of slaves (%d) does not equal number of lights in the input (%d).",start,input->length());
//     }

//     return slices;
// }

// template <class T_OUTPUT_COLOR = GRB, class T_INPUT_COLOR = RGBA>
// void distribute(
//     Hyperion *hyp,
//     Distribution slaves,
//     InputSlicer *splitInput,
//     LUT *lut = nullptr)
// {
//     for (int i = 0; i < splitInput->size() - 1; i++)
//     {
//         auto pipe = new ConvertPipe<T_INPUT_COLOR, T_OUTPUT_COLOR>(
//             splitInput->getInput(i),
//             new UDPOutput(slaves[i].host, slaves[i].port, 60),
//             lut);
//         hyp->addPipe(pipe);
//     }
// }

// template <class T_OUTPUT_COLOR = GRB, class T_INPUT_COLOR = RGBA>
// void distributeAndMonitor(
//     Hyperion *hyp,
//     ControlHubInput<T_INPUT_COLOR> *input,
//     PixelMap *pixelMap,
//     Distribution slaves,
//     LUT *lut = nullptr,
//     float monitorDotSize=0.01)
// {
//     auto slices = createSlices<T_INPUT_COLOR>(input, slaves);
//     auto splitInput = new InputSlicer(input,slices);
//     distribute<T_OUTPUT_COLOR, T_INPUT_COLOR>(hyp, slaves, splitInput, lut);

//     hyp->addPipe(
//         new ConvertPipe<T_INPUT_COLOR, RGB>(
//             splitInput->getInput(slices.size()-1),
//             new MonitorOutput(&hyp->webServer, pixelMap, 60, monitorDotSize)));
// }

// template <class T_OUTPUT_COLOR = GRB, class T_INPUT_COLOR = RGBA>
// void distributeAndMonitor3d(
//     Hyperion *hyp,
//     ControlHubInput<T_INPUT_COLOR> *input,
//     PixelMap3d *pixelMap,
//     Distribution slaves,
//     LUT *lut = nullptr,
//     float monitorDotSize=0.01)
// {
//     auto slices = createSlices<T_INPUT_COLOR>(input, slaves);
//     auto splitInput = new InputSlicer(input,slices);
//     distribute<T_OUTPUT_COLOR, T_INPUT_COLOR>(hyp, slaves, splitInput, lut);

//     hyp->addPipe(
//         new ConvertPipe<T_INPUT_COLOR, RGB>(
//             splitInput->getInput(slices.size()-1),
//             new MonitorOutput3d(&hyp->webServer, pixelMap, 60, monitorDotSize)));
// }


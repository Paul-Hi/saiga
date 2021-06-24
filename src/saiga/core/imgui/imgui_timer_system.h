/**
 * Copyright (c) 2021 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include "saiga/config.h"
#include "saiga/core/time/timer.h"

#include <map>
#include <vector>

namespace Saiga
{
// The timer system is able to render nice graphs and tables
// from a list of times. This class has various specializations for
// CPU, CUDA, and OpenGL timers.
//
// If you want to create your own timer system you have to derive from this class and implement the method
//    virtual std::unique_ptr<TimestampTimer> CreateTimer() = 0;
//
// The interface TimestampTimer defines the necessary functions for the TimerSystem to work.
// For asynchronous domains such as OpenGL or CUDA, I recommend Double- or Triple-Buffered
// timer primitives. As an example you can look at the OpenGL implementation at saiga/opengl/imgui/imgui_opengl.h
//
// Usage in the application (with the recommended Scoped Timers):
//
//    simulation_timer->BeginFrame();
//    {
//        auto timer = simulation_timer->Measure("Sorting");
//        linked_cell->SortParticlesIntoGrid();
//    }
//
//    {
//        auto timer = simulation_timer->Measure("Collision Detection");
//        CreateContactConstraints();
//    }
//    simulation_timer->EndFrame();
//
//    // ... (in the render call)
//    simulation_timer->Imgui();
//
class SAIGA_CORE_API TimerSystem
{
   public:
    // One measurement is given by the start and end tick (in ns)
    using Measurement = std::pair<uint64_t, uint64_t>;


    struct SAIGA_CORE_API TimeData
    {
        explicit TimeData(std::unique_ptr<TimestampTimer> timer, int& current_depth)
            : timer(std::move(timer)), current_depth(current_depth)
        {
        }

        // Users are only allowed to call start and stop
        void Start();
        void Stop();

       private:
        friend class TimerSystem;
        void ResizeSamples(int num_samples);
        void AddTime(Measurement t);
        void ComputeStatistics();
        std::vector<float> ComputeTimes();


        std::unique_ptr<TimestampTimer> timer;

        Measurement last_measurement = {0, 0};
        std::vector<Measurement> measurements_ms;
        int count   = 0;
        bool active = false;
        int& current_depth;

        // stats
        int depth;
        std::string name;
        // statistics (all in ms)
        float stat_last   = 0;
        float stat_min    = 0;
        float stat_max    = 0;
        float stat_median = 0;
        float stat_mean   = 0;
        float stat_sdev   = 0;
    };

    struct ScopedTimingSection
    {
        ScopedTimingSection(TimeData& sec) : sec(sec) { sec.Start(); }
        ~ScopedTimingSection() { sec.Stop(); }
        TimeData& sec;
    };

    // ==== Public Interface ====

    TimerSystem(const std::string& name) : system_name(name) {}
    virtual ~TimerSystem() {}

    ScopedTimingSection Measure(const std::string& name) { return ScopedTimingSection(GetTimer(name)); }
    TimeData& GetTimer(const std::string& name);

    void BeginFrame();
    void EndFrame();
    void Imgui();

    void Enable(bool v = true) { render_window = v; }

   protected:
    // Called by the sub classes. This abstracts the various timer implementations from the measurements
    void Imgui(const std::string& name, ArrayView<TimeData*> timers, TimeData* total_time);

    void ImguiTable(ArrayView<TimeData*> timers, TimeData* total_time);
    void ImguiTimeline(ArrayView<TimeData*> timers, TimeData* total_time);

    void ImguiTooltip(TimeData* td, TimeData* total_time);


    bool render_window = true;
    int num_samples    = 100;

    // we count the number of frames so that the expensive statistic recomputation is only done
    // once every #num_samples frames.
    int current_frame = 0;


    int current_depth = 0;

    bool capturing   = true;
    int current_view = 1;

    bool normalize_time      = true;
    float absolute_scale_fps = 60;

    std::string system_name;
    std::map<std::string, std::shared_ptr<TimeData>> data;

    // These methods are only for specific derived implementations that require setup code.
    // For example, in CUDA we setup the relative frame timers, because default CUDA events are only able to measure
    // time differences.
    virtual void BeginFrameImpl() {}
    virtual void EndFrameImpl() {}
    virtual std::unique_ptr<TimestampTimer> CreateTimer() = 0;
};


}  // namespace Saiga
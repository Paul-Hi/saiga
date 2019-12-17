/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include "saiga/core/time/timer.h"
#include "saiga/vision/imu/Imu.h"

#include "CameraData.h"

#include <fstream>
#include <iomanip>
#include <thread>

namespace Saiga
{
class CameraBase2
{
   public:
    virtual ~CameraBase2() {}

    virtual void close() {}
    virtual bool isOpened() { return true; }
    // transforms the pose to the ground truth reference frame.
    virtual SE3 CameraToGroundTruth() { return SE3(); }
    SE3 GroundTruthToCamera() { return CameraToGroundTruth().inverse(); }


    // Optional IMU data if the camera provides it.
    // The returned vector contains all data from frame-1 to frame.
    virtual std::vector<Imu::Data> ImuDataForFrame(int frame) { return {}; }
    virtual std::optional<Imu::Sensor> getIMU() { return {}; }
};

/**
 * Interface class for different datset inputs.
 */
template <typename _FrameType>
class SAIGA_TEMPLATE CameraBase : public CameraBase2
{
   public:
    using FrameType = _FrameType;
    virtual ~CameraBase() {}

    // Blocks until the next image is available
    // Returns true if success.
    virtual bool getImageSync(FrameType& data) = 0;

    // Returns false if no image is currently available
    virtual bool getImage(FrameType& data) { return getImageSync(data); }


   protected:
    int currentId = 0;
};


struct SAIGA_VISION_API DatasetParameters
{
    // The playback fps. Doesn't have to match the actual camera fps.
    double fps = 30;

    // Root directory of the dataset. The exact value depends on the dataset type.
    std::string dir;

    // Ground truth file. Only used for the kitti dataset. The other datasets have them included in the main directory.
    std::string groundTruth;

    // Throw away all frames before 'startFrame'
    int startFrame = 0;

    // Only load 'maxFrames' after the startFrame.
    int maxFrames = -1;

    // Load images in parallel with omp
    bool multiThreadedLoad = true;

    void fromConfigFile(const std::string& file);
};

/**
 * Interface for cameras that read datasets.
 */
template <typename FrameType>
class SAIGA_TEMPLATE DatasetCameraBase : public CameraBase<FrameType>
{
   public:
    DatasetCameraBase(const DatasetParameters& params) : params(params)
    {
        timeStep =
            std::chrono::duration_cast<tick_t>(std::chrono::duration<double, std::micro>(1000000.0 / params.fps));
        timer.start();
        lastFrameTime = timer.stop();
        nextFrameTime = lastFrameTime + timeStep;
    }

    bool getImageSync(FrameType& data) override
    {
        if (!this->isOpened())
        {
            return false;
        }


        auto t = timer.stop();

        if (t < nextFrameTime)
        {
            std::this_thread::sleep_for(nextFrameTime - t);
            nextFrameTime += timeStep;
        }
        else if (t < nextFrameTime + timeStep)
        {
            nextFrameTime += timeStep;
        }
        else
        {
            nextFrameTime = t + timeStep;
        }


        auto&& img = frames[this->currentId];
        this->currentId++;
        data = std::move(img);
        return true;
    }

    virtual bool isOpened() override { return this->currentId < (int)frames.size(); }
    size_t getFrameCount() { return frames.size(); }

    // Saves the groundtruth in TUM-Trajectory format:
    // <timestamp> <translation x y z> <rotation x y z w>
    void saveGroundTruthTrajectory(const std::string& file)
    {
        std::ofstream strm(file);
        strm << std::setprecision(20);
        for (auto& f : frames)
        {
            double time = f.timeStamp;
            SAIGA_ASSERT(f.groundTruth);

            SE3 gt = f.groundTruth.value();
            Vec3 t = gt.translation();
            Quat q = gt.unit_quaternion();
            strm << time << " " << t(0) << " " << t(1) << " " << t(2) << " " << q.x() << " " << q.y() << " " << q.z()
                 << " " << q.w() << std::endl;
        }
    }

    // Completely removes the frames between from and to
    void eraseFrames(int from, int to)
    {
        frames.erase(frames.begin() + from, frames.begin() + to);
        imuDataForFrame.erase(imuDataForFrame.begin() + from, imuDataForFrame.begin() + to);
    }

    void computeImuDataPerFrame()
    {
        // Create IMU per frame vector by adding all imu datas from frame_i to frame_i+1 to frame_i+1.
        imuDataForFrame.resize(frames.size());
        int currentImuid = 0;

        for (int i = 0; i < frames.size(); ++i)
        {
            auto& a = frames[i];
            for (; currentImuid < imuData.size(); ++currentImuid)
            {
                auto id = imuData[currentImuid];
                if (id.timestamp < a.timeStamp)
                {
                    imuDataForFrame[i].push_back(id);
                }
                else
                {
                    break;
                }
            }
        }
    }

    std::vector<Imu::Data> ImuDataForFrame(int frame) override { return imuDataForFrame[frame]; }
    virtual std::optional<Imu::Sensor> getIMU() override
    {
        return imuData.empty() ? std::optional<Imu::Sensor>() : imu;
    }

   protected:
    AlignedVector<FrameType> frames;
    DatasetParameters params;

    Imu::Sensor imu;
    std::vector<Imu::Data> imuData;
    std::vector<std::vector<Imu::Data>> imuDataForFrame;

   private:
    Timer timer;
    tick_t timeStep;
    tick_t lastFrameTime;
    tick_t nextFrameTime;
};



}  // namespace Saiga
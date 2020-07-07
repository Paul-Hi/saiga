/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include "saiga/core/util/Range.h"
#include "saiga/core/util/Thread/omp.h"
#include "saiga/vision/VisionTypes.h"
#include "saiga/vision/kernels/BAPose.h"
#include "saiga/vision/kernels/Robust.h"

#include "PoseOptimizationScene.h"

#include <vector>

namespace Saiga
{
inline Vec6 SmoothPose(const SE3& pose, const SE3& expected, double scale,
                       Matrix<double, 6, 6>* jacobian_pose = nullptr)
{
    Sophus::SE3d T_j_i   = expected.inverse() * pose;
    Sophus::Vector6d res = Sophus::se3_logd(T_j_i);
    Vec6 residual        = res * scale;

    if (jacobian_pose)
    {
        Sophus::Matrix6d J;
        Sophus::rightJacobianInvSE3Decoupled(res, J);

        Eigen::Matrix3d R = scale * pose.so3().inverse().matrix();

        Sophus::Matrix6d Adj;
        Adj.setZero();
        Adj.topLeftCorner<3, 3>()     = R;
        Adj.bottomRightCorner<3, 3>() = R;
        Adj.topRightCorner<3, 3>()    = Sophus::SO3d::hat(pose.inverse().translation()) * R;

        *jacobian_pose = J * Adj;
    }

    return residual;
}


inline Vec6 SmoothPoseRotation(const SE3& pose, const SE3& expected, double scale,
                               Matrix<double, 6, 6>* jacobian_pose = nullptr)
{
    Sophus::SE3d T_j_i   = expected.inverse() * pose;
    Sophus::Vector6d res = Sophus::se3_logd(T_j_i);
    res.head<3>().setZero();
    Vec6 residual = res * scale;

    if (jacobian_pose)
    {
        Sophus::Matrix6d J;
        Sophus::rightJacobianInvSE3Decoupled(res, J);

        Eigen::Matrix3d R = scale * pose.so3().inverse().matrix();

        Sophus::Matrix6d Adj;
        Adj.setZero();
        //        Adj.topLeftCorner<3, 3>()     = R;
        Adj.bottomRightCorner<3, 3>() = R;
        //        Adj.topRightCorner<3, 3>()    = Sophus::SO3d::hat(pose.inverse().translation()) * R;

        *jacobian_pose = J * Adj;
    }

    return residual;
}


template <typename T, bool Normalized = false, Kernel::LossFunction loss_function = Kernel::LossFunction::Huber>
struct SAIGA_TEMPLATE SAIGA_ALIGN_CACHE RobustSmoothPoseOptimization
{
   public:
    using CameraType = StereoCamera4Base<T>;
    using SE3Type    = Sophus::SE3<T>;
    using Vec2       = Eigen::Matrix<T, 2, 1>;
    using Vec3       = Eigen::Matrix<T, 3, 1>;
    using Vec4       = Eigen::Matrix<T, 4, 1>;
    using Obs        = ObsBase<T>;

    static constexpr int JParams = 6;
    using StereoKernel           = typename Saiga::Kernel::BAPoseStereo<T, false>;
    using MonoKernel             = typename Saiga::Kernel::BAPoseMono<T, false, Normalized>;
    using StereoJ                = typename StereoKernel::JacobiType;
    using MonoJ                  = typename MonoKernel::JacobiType;
    using JType                  = Eigen::Matrix<T, JParams, JParams>;
    using BType                  = Eigen::Matrix<T, JParams, 1>;
    using CompactJ               = Eigen::Matrix<T, 6, 6>;
    using XType                  = Eigen::Matrix<T, 6, 1>;

    RobustSmoothPoseOptimization(T thMono = 2.45, T thStereo = 2.8, T chi1Epsilon = 0.01, int maxOuterIts = 4,
                                 int maxInnerIts = 10)
        : maxOuterIts(maxOuterIts), maxInnerIts(maxInnerIts)
    {
        chi1Mono         = thMono;
        chi1Stereo       = thStereo;
        deltaChi1Epsilon = chi1Epsilon;

        deltaChi2Epsilon = deltaChi1Epsilon * deltaChi1Epsilon;
        chi2Mono         = chi1Mono * chi1Mono;
        chi2Stereo       = chi1Stereo * chi1Stereo;
    }

    /**
     * Scale all thresholds by factor.
     * Usefull for example when operating in normalized image space.
     * Then you can scale the thresholds by 2/(fx+fy)
     */
    void scaleThresholds(T factor)
    {
        chi1Mono *= factor;
        chi1Stereo *= factor;
        deltaChi1Epsilon *= factor;

        deltaChi2Epsilon = deltaChi1Epsilon * deltaChi1Epsilon;
        chi2Mono         = chi1Mono * chi1Mono;
        chi2Stereo       = chi1Stereo * chi1Stereo;
    }

    int optimizePoseRobust(PoseOptimizationScene<T>& scene)
    {
        return optimizePoseRobust(scene.wps, scene.obs, scene.outlier, scene.pose, scene.K, scene.prediction,
                                  scene.weight_rotation, scene.weight_translation);
    }

    int optimizePoseRobust(const AlignedVector<Vec3>& wps, const AlignedVector<Obs>& obs, AlignedVector<int>& outlier,
                           SE3Type& guess, const CameraType& camera, const SE3Type& prediction, T weight_rotation,
                           T weight_translation)
    {
        StereoJ JrowS;
        MonoJ JrowM;


        int N       = wps.size();
        int inliers = 0;

        for (auto outerIt : Range(0, maxOuterIts))
        {
            bool robust = outerIt < (maxOuterIts - 1);
            JType JtJ;
            BType Jtb;
            T lastChi2sum     = std::numeric_limits<T>::infinity();
            SE3Type lastGuess = guess;

            // compute current outlier threshold
            // we start a bit higher than the given
            // threshold and reduce it in each iteration
            // note: the huber threshold does not change!
            auto chi2s = chi2Stereo;
            auto chi2m = chi2Mono;
            int k      = maxOuterIts - 1 - outerIt;
            chi2s      = chi1Stereo * pow(1.2, k);
            chi2s      = chi2s * chi2s;
            chi2m      = chi1Mono * pow(1.2, k);
            chi2m      = chi2m * chi2m;

            for (auto innerIt : Range(0, maxInnerIts))
            {
                JtJ.setZero();
                Jtb.setZero();
                T chi2sum = 0;
                inliers   = 0;


                for (auto i : Range(0, N))
                {
                    if (outlier[i]) continue;

                    auto& o  = obs[i];
                    auto& wp = wps[i];

                    if (o.stereo())
                    {
                        Vec3 res;
                        bool correct_depth = StereoKernel::evaluateResidualAndJacobian(camera, guess, wp, o.ip, o.depth,
                                                                                       res, JrowS, o.weight);
                        auto res_2         = res.squaredNorm();


                        // Remove outliers
                        if (outerIt > 0 && innerIt == 0)
                        {
                            if (res_2 > chi2s || !correct_depth)
                            {
                                outlier[i] = true;
                                continue;
                            }
                        }
                        T loss_weight = 1.0;
                        if (robust)
                        {
                            auto rw     = Kernel::Loss(loss_function, chi1Stereo, res_2);
                            res_2       = rw(0);
                            loss_weight = rw(1);
                        }
                        chi2sum += res_2;
                        JtJ += loss_weight * (JrowS.transpose() * JrowS);
                        Jtb += loss_weight * JrowS.transpose() * res;
                        inliers++;
                    }
                    else
                    {
                        Vec2 res;
                        bool correct_depth =
                            MonoKernel::evaluateResidualAndJacobian(camera, guess, wp, o.ip, res, JrowM, o.weight);
                        auto res_2 = res.squaredNorm();
#if 1
                        // Remove outliers
                        if (outerIt > 0 && innerIt == 0)
                        {
                            if (res_2 > chi2m || !correct_depth)
                            {
                                outlier[i] = true;
                                continue;
                            }
                        }
#endif

                        T loss_weight = 1.0;
                        if (robust)
                        {
                            auto rw     = Kernel::Loss(loss_function, chi1Mono, res_2);
                            res_2       = rw(0);
                            loss_weight = rw(1);
                        }

                        chi2sum += res_2;
                        JtJ += loss_weight * (JrowM.transpose() * JrowM);
                        Jtb += loss_weight * JrowM.transpose() * res;
                        inliers++;
                    }
                }

                if (weight_rotation > 0)
                {
                    JType J_smooth;
                    //                    Vec6 res = SmoothPose(guess, prediction, prediction_weight, &J_smooth);
                    Vec6 res = SmoothPoseRotation(guess, prediction, weight_rotation, &J_smooth);



                    chi2sum += res.squaredNorm();
                    JtJ += J_smooth.transpose() * J_smooth;
                    Jtb -= J_smooth.transpose() * res;
                }


                T deltaChi  = lastChi2sum - chi2sum;
                lastChi2sum = chi2sum;


                if (deltaChi < 0)
                {
                    // the error got worse :(
                    // -> discard step
                    guess = lastGuess;
                    break;
                }

                lastGuess = guess;

                XType x;
                x     = JtJ.ldlt().solve(Jtb);
                guess = Sophus::se3_expd(x) * guess;


                // early termination if the error doesn't change
                // normalize by number of inliers
                if (deltaChi < deltaChi2Epsilon * inliers)
                {
                    //                    std::cout << "inner " << innerIt << std::endl;
                    break;
                }
            }
        }

        return inliers;
    }

   private:
    T chi2Mono;
    T chi2Stereo;
    T chi1Mono;
    T chi1Stereo;
    T deltaChi1Epsilon;
    T deltaChi2Epsilon;
    int maxOuterIts;
    int maxInnerIts;


};  // namespace Saiga



}  // namespace Saiga

﻿/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#include "CeresBA.h"

#include "saiga/time/timer.h"

#include "Eigen/Sparse"
#include "Eigen/SparseCholesky"
#include "ceres/ceres.h"
#include "ceres/problem.h"
#include "ceres/rotation.h"
#include "ceres/solver.h"

#include "CeresKernel_BA_Intr4.h"
#include "local_parameterization_se3.h"

namespace Saiga
{
void CeresBA::solve(Scene& scene, const BAOptions& options)
{
    SAIGA_OPTIONAL_BLOCK_TIMER(options.debugOutput);
    ceres::Problem problem;


    Sophus::test::LocalParameterizationSE3* camera_parameterization = new Sophus::test::LocalParameterizationSE3;
    for (size_t i = 0; i < scene.extrinsics.size(); ++i)
    {
        problem.AddParameterBlock(scene.extrinsics[i].se3.data(), 7, camera_parameterization);
    }

    for (auto& img : scene.images)
    {
        auto& extr   = scene.extrinsics[img.extr].se3;
        auto& camera = scene.intrinsics[img.intr];

        for (auto& ip : img.stereoPoints)
        {
            if (!ip) continue;
            auto& wp = scene.worldPoints[ip.wp].p;
            double w = ip.weight * scene.scale();


            if (ip.depth > 0)
            {
                auto stereoPoint   = ip.point(0) - scene.bf / ip.depth;
                auto cost_function = CostBAStereo<>::create(camera, ip.point, stereoPoint, scene.bf, Vec2(w, w));
                ceres::LossFunction* lossFunction =
                    options.huberStereo > 0 ? new ceres::HuberLoss(options.huberStereo) : nullptr;
                problem.AddResidualBlock(cost_function, lossFunction, extr.data(), wp.data());
            }
            else
            {
                auto cost_function = CostBAMono::create(camera, ip.point, w);
                ceres::LossFunction* lossFunction =
                    options.huberMono > 0 ? new ceres::HuberLoss(options.huberMono) : nullptr;
                problem.AddResidualBlock(cost_function, lossFunction, extr.data(), wp.data());
            }
        }
    }


    //    double costInit = 0;
    //    ceres::Problem::EvaluateOptions defaultEvalOptions;
    //    problem.Evaluate(defaultEvalOptions, &costInit, nullptr, nullptr, nullptr);

    ceres::Solver::Options ceres_options;
    ceres_options.minimizer_progress_to_stdout = options.debugOutput;
    ceres_options.max_num_iterations           = options.maxIterations;
    ceres_options.max_linear_solver_iterations = options.maxIterativeIterations;
    ceres_options.min_linear_solver_iterations = options.maxIterativeIterations;
    ceres_options.min_relative_decrease        = 1e-50;
    ceres_options.function_tolerance           = 1e-50;


    switch (options.solverType)
    {
        case BAOptions::SolverType::Direct:
            ceres_options.linear_solver_type = ceres::LinearSolverType::SPARSE_SCHUR;
            break;
        case BAOptions::SolverType::Iterative:
            ceres_options.linear_solver_type = ceres::LinearSolverType::ITERATIVE_SCHUR;
            break;
    }

    ceres::Solver::Summary summaryTest;

    {
        SAIGA_OPTIONAL_BLOCK_TIMER(options.debugOutput);
        ceres::Solve(ceres_options, &problem, &summaryTest);
    }

    //    double costFinal = 0;
    //    problem.Evaluate(defaultEvalOptions, &costFinal, nullptr, nullptr, nullptr);

    //    std::cout << "optimizePoints residual: " << costInit << " -> " << costFinal << endl;
}

}  // namespace Saiga
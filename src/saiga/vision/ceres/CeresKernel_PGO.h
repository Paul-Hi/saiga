﻿/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include "saiga/vision/VisionTypes.h"
#include "saiga/vision/kernels/PGO.h"

#include "ceres/autodiff_cost_function.h"

namespace Saiga
{
struct CostPGO
{
    using PGOTransformation = SE3;
    CostPGO(const PGOTransformation& invMeassurement, double weight = 1)
        : _inverseMeasurement(invMeassurement), weight(weight)
    {
    }

    using CostType         = CostPGO;
    using CostFunctionType = ceres::AutoDiffCostFunction<CostType, PGOTransformation::DoF, 7, 7>;
    template <typename... Types>
    static CostFunctionType* create(const Types&... args)
    {
        return new CostFunctionType(new CostType(args...));
    }

    template <typename T>
    bool operator()(const T* const _from, const T* const _to, T* _residual) const
    {
        Eigen::Map<Sophus::SE3<T> const> const from(_from);
        Eigen::Map<Sophus::SE3<T> const> const to(_to);
        Eigen::Map<Eigen::Matrix<T, PGOTransformation::DoF, 1>> residual(_residual);

        Sophus::SE3<T> inverseMeasurement = _inverseMeasurement.cast<T>();
#ifdef LSD_REL
        //        auto est_j_i = to * from.inverse();
        auto est_j_i = from.inverse() * to;
        auto error_  = inverseMeasurement * est_j_i;
//        auto error_ = from.inverse() * to * inverseMeasurement;
#else

        // = dlog(measurement_T_i_j * inv(estimate_T_w_j) * estimate_T_w_i)
        // C * v1->estimate() * v2->estimate().inverse();
        auto error_ = inverseMeasurement.inverse() * from * to.inverse();



        //        auto error_ = inverseMeasurement.inverse() * from * to.inverse();
        //        auto error_ = from * to.inverse() * inverseMeasurement.inverse(); // working
        //        auto error_ = inverseMeasurement * to * from.inverse();

//        auto est_j_i = from.inverse() * to;
//        auto error_  = est_j_i * inverseMeasurement.inverse();
#endif

        residual = error_.log() * weight;
        //        residual[6] = T(0);


        return true;
    }

   private:
    PGOTransformation _inverseMeasurement;

    //    Quat meas_q;
    //    Vec3 meas_t;
    //    double meas_scale;

    double weight;
};


class CostPGOAnalytic : public ceres::SizedCostFunction<7, 7, 7>
{
   public:
    using PGOTransformation = SE3;

    static constexpr int DOF = PGOTransformation::DoF;
    using T                  = double;

    using Kernel = Saiga::Kernel::PGO<PGOTransformation>;

    CostPGOAnalytic(const PGOTransformation& invMeassurement, double weight = 1)
        : _inverseMeasurement(invMeassurement), weight(weight)
    {
    }

    virtual ~CostPGOAnalytic() {}

    virtual bool Evaluate(double const* const* _parameters, double* _residuals, double** _jacobians) const
    {
        Eigen::Map<Sophus::SE3<T> const> const from(_parameters[0]);
        Eigen::Map<Sophus::SE3<T> const> const to(_parameters[1]);
        Eigen::Map<Eigen::Matrix<T, DOF, 1>> residual(_residuals);



        if (!_jacobians)
        {
            // only compute residuals
            Kernel::ResidualType res;
            Kernel::evaluateResidual(from, to, _inverseMeasurement, res, weight);
            residual = res;
        }
        else
        {
            // compute both
            Kernel::PoseJacobiType JrowFrom, JrowTo;
            Kernel::ResidualType res;
            Kernel::evaluateResidualAndJacobian(from, to, _inverseMeasurement, res, JrowFrom, JrowTo, weight);

            residual = res;



            if (_jacobians[0])
            {
                Eigen::Map<Eigen::Matrix<T, DOF, 7, Eigen::RowMajor>> jpose2(_jacobians[0]);
                jpose2.setZero();
                jpose2.block<DOF, DOF>(0, 0) = JrowFrom;
            }
            if (_jacobians[1])
            {
                Eigen::Map<Eigen::Matrix<T, DOF, 7, Eigen::RowMajor>> jpose2(_jacobians[1]);
                jpose2.setZero();
                jpose2.block<DOF, DOF>(0, 0) = JrowTo;
            }
        }


        return true;
    }

   private:
    PGOTransformation _inverseMeasurement;
    double weight;
};

}  // namespace Saiga

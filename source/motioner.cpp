#include "motioner.h"
#include "utils.h"
#include <rl/math/Transform.h>
#include <rl/mdl/JacobianInverseKinematics.h>
#include <rl/mdl/Kinematic.h>
#include <rl/mdl/Model.h>
#include <rl/mdl/NloptInverseKinematics.h>

using namespace roblib;

struct roblib::MotionerPrivate {
    rl::mdl::Model* model = nullptr;
};

Motioner::Motioner(const std::string& model_file_path)
    : d_(new MotionerPrivate)
{
    d_->model = ModelUtils::isUrdfFile(model_file_path) ? ModelUtils::getModelFromUrdf(model_file_path) : ModelUtils::getModelFromXml(model_file_path);
}

Motioner::Motioner(const Motioner& other)
{
    d_ = new MotionerPrivate;
    d_->model = new rl::mdl::Model();
    *d_->model = *other.d_->model;
}

Motioner::~Motioner()
{
    delete d_->model;
    delete d_;
}

int Motioner::getDof() const
{
    return d_->model->getDof();
}

int Motioner::getOperationDof() const
{
    return d_->model->getOperationalDof();
}

xyzWithQuaternion Motioner::getEndEffectorQuatPos() const
{
    auto kin_model = dynamic_cast<rl::mdl::Kinematic*>(d_->model);
    rl::math::Transform end_effector_pos = kin_model->getOperationalPosition(0);
    return ModelUtils::rlTransform2XyzQuat(end_effector_pos);
}

xyzWithEuler Motioner::getEndEffectorEulerPos() const
{
    auto kin_model = dynamic_cast<rl::mdl::Kinematic*>(d_->model);
    rl::math::Transform end_effector_pos = kin_model->getOperationalPosition(0);
    return ModelUtils::rlTransform2XyzEuler(end_effector_pos);
}

std::vector<double> Motioner::getJointDegree() const
{
    std::vector<double> res;
    auto kin_model = dynamic_cast<rl::mdl::Kinematic*>(d_->model);

    for (int i = 0; i < kin_model->getPosition().size(); ++i) {
        res.push_back(kin_model->getPosition()[i]);
    }
    return res;
}

std::optional<xyzWithQuaternion> Motioner::getEndEffectorPosByDegree(const std::vector<double>& joint_degrees) const
{
    if (joint_degrees.size() != getDof()) {
        return std::nullopt;
    }

    rl::math::Vector position { getDof() };
    for (int i = 0; i < position.size(); ++i) {
        position[i] = joint_degrees[i] * rl::math::DEG2RAD;
    }
    auto kin_model = dynamic_cast<rl::mdl::Kinematic*>(d_->model);
    kin_model->setPosition(position);
    kin_model->forwardPosition();

    rl::math::Transform end_effector_pos = kin_model->getOperationalPosition(0);

    return ModelUtils::rlTransform2XyzQuat(end_effector_pos);
}

std::optional<xyzWithQuaternion> Motioner::getEndEffectorPosByRadian(const std::vector<double>& joint_radians) const
{
    if (joint_radians.size() != getDof()) {
        return std::nullopt;
    }

    rl::math::Vector position { getDof() };
    for (int i = 0; i < position.size(); ++i) {
        position[i] = joint_radians[i];
    }
    auto kin_model = dynamic_cast<rl::mdl::Kinematic*>(d_->model);
    kin_model->setPosition(position);
    kin_model->forwardPosition();

    rl::math::Transform end_effector_pos = kin_model->getOperationalPosition(0);

    return ModelUtils::rlTransform2XyzQuat(end_effector_pos);
}

std::vector<double> Motioner::getDegreesByTXyzQuat(const xyzWithQuaternion& target) const
{
    auto trans = ModelUtils::xyzQuat2RlTransfrom(target);
    auto kin_model = dynamic_cast<rl::mdl::Kinematic*>(d_->model);

    rl::mdl::JacobianInverseKinematics ik(kin_model);
    ik.goals.push_back({ trans, 0 });

    std::vector<double> res;

    if (ik.solve()) {
        for (int i = 0; i < kin_model->getPosition().size(); ++i) {
            res.push_back(kin_model->getPosition()[i] * rl::math::RAD2DEG);
        }
    }
    return res;
}

std::vector<double> Motioner::getRadiansByTXyzQuat(const xyzWithQuaternion& target) const
{
    auto trans = ModelUtils::xyzQuat2RlTransfrom(target);
    auto kin_model = dynamic_cast<rl::mdl::Kinematic*>(d_->model);

    rl::mdl::JacobianInverseKinematics ik(kin_model);
    ik.goals.push_back({ trans, 0 });

    std::vector<double> res;

    if (ik.solve()) {
        for (int i = 0; i < kin_model->getPosition().size(); ++i) {
            res.push_back(kin_model->getPosition()[i]);
        }
    }
    return res;
}

std::vector<double> Motioner::getDegreesByXyzEuler(const xyzWithEuler& target) const
{
    auto trans = ModelUtils::xyzEuler2RlTransfrom(target);
    auto kin_model = dynamic_cast<rl::mdl::Kinematic*>(d_->model);

    rl::mdl::JacobianInverseKinematics ik(kin_model);
    ik.goals.push_back({ trans, 0 });

    std::vector<double> res;

    if (ik.solve()) {
        for (int i = 0; i < kin_model->getPosition().size(); ++i) {
            res.push_back(kin_model->getPosition()[i] * rl::math::RAD2DEG);
        }
    }
    return res;
}

std::vector<double> Motioner::getRadiansByXyzEuler(const xyzWithEuler& target) const
{
    auto trans = ModelUtils::xyzEuler2RlTransfrom(target);
    auto kin_model = dynamic_cast<rl::mdl::Kinematic*>(d_->model);

    rl::mdl::JacobianInverseKinematics ik(kin_model);
    ik.goals.push_back({ trans, 0 });

    std::vector<double> res;

    if (ik.solve()) {
        for (int i = 0; i < kin_model->getPosition().size(); ++i) {
            res.push_back(kin_model->getPosition()[i]);
        }
    }
    return res;
}
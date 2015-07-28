#include "Conversion.hpp"

namespace Conversion {
    Eigen::Quaternionf tfToEigen(const tf::Quaternion tfQuat) {
        return Eigen::Quaternionf(
                tfQuat.w(), tfQuat.x(), tfQuat.y(), tfQuat.z());
    }

    tf::Quaternion eigenToTf(const Eigen::Quaternionf eigenQuat) {
        return tf::Quaternion(
                eigenQuat.x(), eigenQuat.y(), eigenQuat.z(), eigenQuat.w());
    }

    Eigen::Translation3f tfToEigen(const tf::Vector3 tfTranslation) {
        return Eigen::Translation3f(
                tfTranslation.x(), tfTranslation.y(), tfTranslation.z());
    }

    tf::Vector3 eigenToTf(const Eigen::Translation3f eigenTranslation) {
        return tf::Vector3(eigenTranslation.x(), eigenTranslation.y(),
                eigenTranslation.z());
    }

    Eigen::Matrix4f tfToEigen(const tf::Transform tfTransfo) {
        Eigen::Quaternionf eigenQuat = tfToEigen(tfTransfo.getRotation());
        Eigen::Translation3f translation = tfToEigen(tfTransfo.getOrigin());
        Eigen::Transform<float,3,Eigen::Affine> output = translation*eigenQuat;

        return output.matrix();
    }

    Eigen::Quaternionf getQuat(const Eigen::Matrix4f transfo) {
        return Eigen::Quaternionf(Eigen::Matrix3f(transfo.block(0,0,3,3)));
    }

    Eigen::Vector3f getRPY(const Eigen::Matrix4f transfo) {
        Eigen::Quaternionf eigenQuat = Conversion::getQuat(transfo);

        double roll, pitch, yaw;
        tf::Matrix3x3(eigenToTf(eigenQuat)).getRPY(roll, pitch, yaw);

        return Eigen::Vector3f(roll, pitch, yaw);
    }

    Eigen::Translation3f getTranslation(const Eigen::Matrix4f transfo) {
        return Eigen::Translation3f(transfo.block(0,3,3,1));
    }

    Eigen::Matrix4f getPoseDiff(const tf::Pose& start, const tf::Pose& end) {
        Eigen::Vector3f startPosition =
            Conversion::tfToEigen(start.getOrigin()).vector();
        Eigen::Vector3f endPosition =
            Conversion::tfToEigen(end.getOrigin()).vector();
        Eigen::Vector3f diffPosition = endPosition - startPosition;

        Eigen::Quaternionf startRot =
            Conversion::tfToEigen(start.getRotation());
        Eigen::Quaternionf endRot =
            Conversion::tfToEigen(end.getRotation());
        Eigen::Quaternionf diffRot = endRot * startRot.inverse();

        Eigen::Transform<float,3,Eigen::Affine> composedTransfo =
            Eigen::Translation3f(diffPosition)*diffRot;

        return composedTransfo.matrix();
    }

    Eigen::Matrix4f getPoseComposition(const Eigen::Matrix4f& start,
            const Eigen::Matrix4f& increment) {
        Eigen::Vector3f startTrans =
            Conversion::getTranslation(start).vector();
        Eigen::Vector3f incrementTrans =
            Conversion::getTranslation(increment).vector();
        Eigen::Vector3f finalTranslation = startTrans + incrementTrans;

        Eigen::Quaternionf startQuat = Conversion::getQuat(start);
        Eigen::Quaternionf incrementQuat = Conversion::getQuat(increment);
        Eigen::Quaternionf finalQuat = incrementQuat * startQuat;

        Eigen::Matrix4f finalPose = Eigen::Matrix4f::Identity();
        finalPose.block<3,3>(0,0) = finalQuat.matrix();
        finalPose.block<3,1>(0,3) = finalTranslation;

        return finalPose;
    }
}

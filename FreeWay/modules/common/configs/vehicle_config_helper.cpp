#include "modules/common/configs/vehicle_config_helper.h"

#include <algorithm>
#include <cmath>

#include "spider/common/file.h"
#include "modules/common/configs/config_gflags.h"

namespace dharma {
namespace common {

VehicleConfig VehicleConfigHelper::vehicle_config_;
bool VehicleConfigHelper::is_init_ = false;

VehicleConfigHelper::VehicleConfigHelper() {}

void VehicleConfigHelper::Init() { Init(FLAGS_vehicle_config_path); }

void VehicleConfigHelper::Init(const std::string &config_file) {
    VehicleConfig params;
    CHECK(spider::common::GetProtoFromFile(config_file, &params))
            << "Unable to parse vehicle config file " << config_file;
    Init(params);
}

void VehicleConfigHelper::Init(const VehicleConfig &vehicle_params) {
    vehicle_config_ = vehicle_params;
    is_init_ = true;
}

const VehicleConfig &VehicleConfigHelper::GetConfig() {
    if (!is_init_) {
        Init();
    }
    return vehicle_config_;
}

double VehicleConfigHelper::MinSafeTurnRadius() {
    const auto &param = vehicle_config_.vehicle_param();
    double lat_edge_to_center =
            std::max(param.left_edge_to_center(), param.right_edge_to_center());
    double lon_edge_to_center =
            std::max(param.front_edge_to_center(), param.back_edge_to_center());
    return std::sqrt((lat_edge_to_center + param.min_turn_radius()) *
                     (lat_edge_to_center + param.min_turn_radius()) +
                     lon_edge_to_center * lon_edge_to_center);
}

common::math::Box2d VehicleConfigHelper::GetBoundingBox(
        const common::PathPoint &path_point) {
    const auto &vehicle_param = vehicle_config_.vehicle_param();
    double diff_truecenter_and_pointX = (vehicle_param.front_edge_to_center() -
                                         vehicle_param.back_edge_to_center()) /
            2.0;
    common::math::Vec2d true_center(
                path_point.x() +
                diff_truecenter_and_pointX * std::cos(path_point.theta()),
                path_point.y() +
                diff_truecenter_and_pointX * std::sin(path_point.theta()));

    return common::math::Box2d(true_center, path_point.theta(),
                               vehicle_param.length(), vehicle_param.width());
}

}  // namespace common
}  // namespace dharma

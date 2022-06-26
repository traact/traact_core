/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_PATTERN_INSTANCE_PORTINSTANCE_H_
#define TRAACT_CORE_SRC_TRAACT_PATTERN_INSTANCE_PORTINSTANCE_H_

#include <traact/pattern/Port.h>
#include <traact/traact_core_export.h>
#include <optional>
#include <spdlog/spdlog.h>
namespace traact::pattern::instance {

class TRAACT_CORE_EXPORT PatternInstance;
class TRAACT_CORE_EXPORT PortGroupInstance;

//
using ComponentID_PortName = std::pair<std::string, std::string>;

struct TRAACT_CORE_EXPORT PortInstance {
    using Ptr = PortInstance *;
    using ConstPtr = const PortInstance *;

    PortInstance() = default;
    PortInstance(Port t_port, PortGroupInstance *t_port_group_instance);

    [[nodiscard]] const std::string getName() const;

    [[nodiscard]] const std::string &getDataType() const;

    [[nodiscard]] int getPortIndex() const;

    [[nodiscard]] std::set<traact::pattern::instance::PortInstance::ConstPtr> connectedToPtr() const;

    [[nodiscard]] ComponentID_PortName getId() const;

    [[nodiscard]] bool isConnected() const;

    [[nodiscard]] PortType getPortType() const;

    Port port{};
    PortGroupInstance* port_group_instance{nullptr};
    ComponentID_PortName connected_to{};
    int getTimeDomain() const;
};

struct TRAACT_CORE_EXPORT PortGroupInstance {

    PortGroupInstance() = default;
    PortGroupInstance(PortGroup t_port_group, PatternInstance *t_pattern_instance, int t_port_group_instance_id);

    template<typename T>
    void setParameter(const std::string& name, T value){
//        if(!port_group.parameter.contains("name")){
//            SPDLOG_WARN("unknown parameter name {0}", name);
//        }
        nlohmann::json new_value = value;
        switch(port_group.parameter[name]["default"].type()){
            case nlohmann::detail::value_t::string:{
                port_group.parameter[name]["value"] = new_value.template get<std::string>();
                break;
            }
            case nlohmann::detail::value_t::number_float:{
                port_group.parameter[name]["value"] = new_value.template get<double>();
                break;
            }
            case nlohmann::detail::value_t::number_unsigned:{
                port_group.parameter[name]["value"] = new_value.template get<uint64_t>();
                break;
            }
            case nlohmann::detail::value_t::number_integer:{
                port_group.parameter[name]["value"] = new_value.template get<int64_t>();
                break;
            }
            case nlohmann::detail::value_t::boolean :{
                port_group.parameter[name]["value"] = new_value.template get<bool>();
                break;
            }
            default:{
                port_group.parameter[name]["value"] = new_value;
                break;
            }
        }

    }
    [[nodiscard]] std::string getProducerPortName(const std::string &internal_port_name) const;
    [[nodiscard]] std::string getConsumerPortName(const std::string &internal_port_name) const;



    int getPortGroupStartIndex(int local_time_domain, PortType port_type) const;
    int getPortCount(int local_time_domain, PortType port_type) const;

    const std::vector<PortInstance> &getPortList(PortType port_type) const;

    template<typename ParaType>
    bool setValueFromParameter(std::string parameter_name, ParaType &parameter_out) {
        auto& parameter = port_group.parameter;

        if (!parameter.contains(parameter_name)) {
            parameter_out = ParaType();
            SPDLOG_ERROR("Missing parameter section: {0}, using default value: {1}", parameter_name, parameter_out);
            return false;
        } else {
            if(!parameter[parameter_name].contains("value")){
                parameter_out = parameter[parameter_name]["default"];
                SPDLOG_WARN("Missing parameter value: {0}, using default value: {1}", parameter_name, parameter_out);
            } else {
                parameter_out = parameter[parameter_name]["value"];
            }

        }
        return true;
    }

    PortGroup port_group{};
    PatternInstance* pattern_instance{nullptr};
    int port_group_instance_index{-1};
    std::vector<PortInstance> producer_ports;
    std::vector<PortInstance> consumer_ports;
};


}

#endif //TRAACT_CORE_SRC_TRAACT_PATTERN_INSTANCE_PORTINSTANCE_H_

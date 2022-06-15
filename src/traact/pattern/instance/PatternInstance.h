/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_PATTERN_INSTANCE_PATTERNINSTANCE_H_
#define TRAACT_CORE_SRC_TRAACT_PATTERN_INSTANCE_PATTERNINSTANCE_H_
#include <traact/pattern/instance/PortInstance.h>
#include <traact/pattern/Pattern.h>
#include <traact/datatypes.h>
#include <traact/traact_core_export.h>

namespace traact::pattern::instance {

struct TRAACT_CORE_EXPORT GraphInstance;

using LocalConnectedOutputPorts = std::vector<bool>;


struct TRAACT_CORE_EXPORT PatternInstance {
 public:
    PatternInstance(std::string t_instance_id, Pattern pattern,
                    GraphInstance *graph);

    using Ptr = std::shared_ptr<PatternInstance>;

    PatternInstance() = default;
    ~PatternInstance() = default;


    [[nodiscard]] std::string getPatternName() const;

    [[nodiscard]] int getTimeDomainCount() const;
    [[nodiscard]] component::ComponentType getComponentType(int time_domain) const;
    [[nodiscard]] traact::Concurrency getConcurrency(int time_domain) const;

    [[nodiscard]] PortInstance::ConstPtr getProducerPort(const std::string &name) const;
    [[nodiscard]] PortInstance::ConstPtr getConsumerPort(const std::string &name) const;
    [[nodiscard]] PortInstance::Ptr getConsumerPort(const std::string &name);
    [[nodiscard]] PortInstance::ConstPtr getPort(const std::string &name) const;
    [[nodiscard]] std::vector<traact::pattern::instance::PortInstance::ConstPtr> getProducerPorts(int global_time_domain) const;
    [[nodiscard]] std::vector<PortInstance::ConstPtr> getConsumerPorts(int time_domain) const;

    [[nodiscard]] LocalConnectedOutputPorts getOutputPortsConnected(int time_domain) const;

    template<typename T>
        void setParameter(const std::string& name, T value){
        port_groups_.at(kDefaultPortGroupIndex)[0]->port_group.parameter[name]["value"] = value;
        }

    [[nodiscard]] int getPortGroupCount(const std::string &port_group_name) const;
    PortGroupInfo getPortGroupInfo(const std::string &port_group_name) const;
    PortGroupInstance& instantiatePortGroup(const std::string& port_group_name);
    bool isInTimeDomain(int global_time_domain) const;

    std::string instance_id{"invalid"};
    std::string display_name{"invalid"};
    GraphInstance *parent_graph{nullptr};
    Pattern local_pattern{};

    std::vector< std::vector<std::shared_ptr<PortGroupInstance>> > port_groups_{};
    std::map<std::string, int> port_group_name_to_index_;
    std::vector<int> local_to_global_time_domain;
    int getPortGroupIndex(const std::string &group_name) const;
    std::tuple<int, int, int> getPortGroupOffset(int port_group_index, PortType port_type, int local_time_domain) const;
};
}

#endif //TRAACT_CORE_SRC_TRAACT_PATTERN_INSTANCE_PATTERNINSTANCE_H_

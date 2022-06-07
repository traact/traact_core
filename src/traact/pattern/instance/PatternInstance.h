/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_CORE_SRC_TRAACT_PATTERN_INSTANCE_PATTERNINSTANCE_H_
#define TRAACT_CORE_SRC_TRAACT_PATTERN_INSTANCE_PATTERNINSTANCE_H_
#include <traact/pattern/instance/PortInstance.h>
#include <traact/pattern/Pattern.h>
#include <traact/datatypes.h>
#include <traact/traact_core_export.h>

namespace traact::pattern::instance {

class TRAACT_CORE_EXPORT GraphInstance;

class TRAACT_CORE_EXPORT PatternInstance {
 public:
    PatternInstance(std::string id, Pattern pattern_pointer,
                    GraphInstance *graph);

    typedef typename std::shared_ptr<PatternInstance> Ptr;
    PatternInstance();
    virtual ~PatternInstance();

    std::string getPatternName() const;
    traact::Concurrency getConcurrency() const;

    PortInstance::ConstPtr getProducerPort(const std::string &name) const;
    PortInstance::ConstPtr getConsumerPort(const std::string &name) const;
    PortInstance::ConstPtr getPort(const std::string &name) const;
    std::set<PortInstance::ConstPtr> getProducerPorts() const;
    std::set<PortInstance::ConstPtr> getConsumerPorts() const;

    std::string instance_id;
    GraphInstance *parent_graph;
    Pattern local_pattern;
    std::vector<PortInstance> producer_ports;
    std::vector<PortInstance> consumer_ports;
    int time_domain{0};

};
}

#endif //TRAACT_CORE_SRC_TRAACT_PATTERN_INSTANCE_PATTERNINSTANCE_H_

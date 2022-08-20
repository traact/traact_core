/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_INCLUDE_TRAACT_FACADE_FACADE_H_
#define TRAACT_INCLUDE_TRAACT_FACADE_FACADE_H_

#include <traact/pattern/instance/GraphInstance.h>
#include <traact/dataflow/Network.h>
#include <traact/component/ComponentGraph.h>
#include <traact/facade/PluginFactory.h>
#include <future>

namespace traact::facade {

class TRAACT_CORE_EXPORT Facade {
 public:

    Facade(PluginFactory::Ptr plugin_factory, dataflow::Network::Ptr dataflow_network);
    ~Facade();
    void loadDataflow(pattern::instance::GraphInstance::Ptr graph_instance);
    void loadDataflow(std::string filename);

    bool start();
    bool blockingStart();

    std::shared_future<void> getFinishedFuture();
    dataflow::DataflowState::SharedPtr getDataflowState();

    bool stop();
    void stopAsync();

    void parameterChanged(const std::string &instance_id);

    pattern::Pattern::Ptr instantiatePattern(const std::string &pattern_name);

    component::Component::Ptr getComponent(std::string id);

    template<typename T> auto getComponentAs(std::string instance_id) const {
        return std::dynamic_pointer_cast<T>(component_graph_->getComponent(instance_id));
    }
    template<typename T> auto findComponents() const {
        std::vector<std::shared_ptr<T>> components;
        for(auto& instance_component : component_graph_->getPatterns()) {
            auto current_component = std::dynamic_pointer_cast<T>(instance_component.second);
            if(current_component){
                components.push_back(current_component);
            }
        }
        return components;
    }

    std::vector<pattern::Pattern::Ptr> GetAllAvailablePatterns();

    bool isRunning() const;

 private:
    std::shared_ptr<PluginFactory> factory_;
    pattern::instance::GraphInstance::Ptr graph_instance_;
    component::ComponentGraph::Ptr component_graph_;
    dataflow::Network::Ptr network_;
    bool should_stop_{false};
    bool running_{false};
    std::promise<void> finished_promise_;
    std::shared_future<void> finished_future_;
    std::mutex stop_lock_;
    std::shared_ptr<std::thread> stop_thread_{nullptr};


};
}

#endif //TRAACT_INCLUDE_TRAACT_FACADE_FACADE_H_

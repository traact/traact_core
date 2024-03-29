/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <traact/facade/Facade.h>
#include <traact/serialization/JsonGraphInstance.h>

#include <traact/util/Logging.h>
#include <stdlib.h>
#include <fstream>

traact::facade::Facade::Facade(PluginFactory::Ptr plugin_factory, dataflow::Network::Ptr dataflow_network)
    : factory_(plugin_factory),
      network_(dataflow_network),
      finished_promise_(),
      finished_future_(finished_promise_.get_future()) {

    network_->setGenericFactoryObjects(factory_);
    network_->setMasterSourceFinishedCallback(std::bind(&Facade::stopAsync, this));

}
traact::facade::Facade::~Facade() {

    if (network_)
        network_->stop();
    network_.reset();
    component_graph_.reset();
    graph_instance_.reset();
    if(stop_thread_){
        stop_thread_->join();
        stop_thread_.reset();
    }
}
void traact::facade::Facade::loadDataflow(traact::pattern::instance::GraphInstance::Ptr graph_instance) {
    SPDLOG_DEBUG("loading dataflow from graph instance");

    auto run_error = graph_instance->checkRunnable();
    if(run_error){
        SPDLOG_ERROR(run_error.value());
        throw std::invalid_argument(run_error.value());
    }

    graph_instance_ = graph_instance;

    component_graph_ = std::make_shared<component::ComponentGraph>(graph_instance_);

    for (auto &dataflow_component : graph_instance_->getAll()) {

        try {
            SPDLOG_DEBUG("Create component: {0}", dataflow_component->getPatternName());
            auto new_component =
                factory_->instantiateComponent(dataflow_component->getPatternName(), dataflow_component->instance_id);

            new_component->configureInstance(*dataflow_component);

            component_graph_->addPattern(dataflow_component->instance_id, new_component);

        } catch (std::out_of_range e) {
            throw std::out_of_range("trying to instantiate unknown pattern: " + dataflow_component->getPatternName());
        } catch (...) {
            throw std::invalid_argument(
                "exception while trying to instantiate pattern: " + dataflow_component->getPatternName());
        }

    }

}
void traact::facade::Facade::loadDataflow(std::string filename) {
    auto loaded_pattern_graph_ptr = std::make_shared<pattern::instance::GraphInstance>();
    nlohmann::json jsongraph;
    std::ifstream graphfile;
    graphfile.open(filename);
    graphfile >> jsongraph;
    graphfile.close();
    ns::from_json(jsongraph, *loaded_pattern_graph_ptr);
    loadDataflow(loaded_pattern_graph_ptr);
}

bool traact::facade::Facade::start() {


    network_->setComponentGraph(component_graph_);

    running_ = network_->start();
    return running_;
}
bool traact::facade::Facade::stop() {
    bool call_stop{false};
    {
        std::unique_lock guard(stop_lock_);
        if(!should_stop_) {
            should_stop_ = true;
            call_stop = true;
        }
    }

    if(call_stop){
        call_stop = network_->stop();
        finished_promise_.set_value();
        running_ = false;
    } else {
        SPDLOG_WARN("stop already called");
    }

    return call_stop;
}
traact::component::Component::Ptr traact::facade::Facade::getComponent(std::string id) {
    return component_graph_->getComponent(id);
}
traact::pattern::Pattern::Ptr traact::facade::Facade::instantiatePattern(const std::string &pattern_name) {
    try {
        return factory_->instantiatePattern(pattern_name);
    } catch (...) {
        throw std::invalid_argument("exception trying to instantiate pattern: " + pattern_name);
    }

}

std::shared_future<void> traact::facade::Facade::getFinishedFuture() {
    return finished_future_;
}

bool traact::facade::Facade::blockingStart() {
    if (!start())
        return false;

    auto finished = getFinishedFuture();
    while (finished.wait_for(std::chrono::seconds(1)) != std::future_status::ready) {
        if (should_stop_) {
            SPDLOG_INFO("waiting for dataflow to stop");
        }
    }
    return true;
}

void traact::facade::Facade::stopAsync() {
    SPDLOG_TRACE("Facade source finished callback");
    std::unique_lock guard(stop_lock_);

    if(!stop_thread_){
        stop_thread_ = std::make_shared<std::thread>([local_facade = this](){
            SPDLOG_TRACE("Facade source finished in thread");
            local_facade->stop();
        });
    }
    SPDLOG_TRACE("Facade source finished callback done");
}

std::vector<traact::pattern::Pattern::Ptr> traact::facade::Facade::GetAllAvailablePatterns() {
    std::vector<pattern::Pattern::Ptr> result;
    for (const auto &pattern_name : factory_->getPatternNames()) {
        result.push_back(factory_->instantiatePattern(pattern_name));
    }
    return result;
}
void traact::facade::Facade::parameterChanged(const std::string &instance_id) {
    network_->parameterChanged(instance_id);

}
bool traact::facade::Facade::isRunning() const{
    return running_;
}
traact::dataflow::DataflowState::SharedPtr traact::facade::Facade::getDataflowState() {
    return network_->getDataflowState();
}


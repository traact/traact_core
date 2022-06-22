/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/


#include <gtest/gtest.h>
#include <traact/pattern/Pattern.h>
#include <traact/pattern/instance/GraphInstance.h>
#include "TestDatatypes.h"
using InPort0 = traact::buffer::PortConfig<TestStringHeader, 0>;
using InPort1 = traact::buffer::PortConfig<TestStringHeader, 1>;
using InPort2 = traact::buffer::PortConfig<TestStringHeader, 2>;

using OutPort0 = traact::buffer::PortConfig<TestStringHeader, 0>;
using OutPort1 = traact::buffer::PortConfig<TestStringHeader, 1>;
using OutPort2 = traact::buffer::PortConfig<TestStringHeader, 2>;

void testPort(const traact::pattern::Port &test, const traact::pattern::Port &loaded) {
    EXPECT_EQ(test.name, loaded.name);
    EXPECT_EQ(test.port_index, loaded.port_index);
    EXPECT_EQ(test.port_type, loaded.port_type);
    EXPECT_EQ(test.datatype, loaded.datatype);
    EXPECT_EQ(test.time_domain, loaded.time_domain);
}

void testPortGroup(const traact::pattern::PortGroup &test, const traact::pattern::PortGroup &loaded) {
    EXPECT_EQ(test.name, loaded.name);
    EXPECT_EQ(test.group_index, loaded.group_index);
    EXPECT_EQ(test.max, loaded.max);
    EXPECT_EQ(test.min, loaded.min);
    EXPECT_EQ(test.parameter.dump(), loaded.parameter.dump());
    EXPECT_EQ(test.consumer_ports.size(), loaded.consumer_ports.size());
    for (int i = 0; i < test.consumer_ports.size(); ++i) {
        testPort(test.consumer_ports[i], loaded.consumer_ports[i]);
    }
    EXPECT_EQ(test.producer_ports.size(), loaded.producer_ports.size());
    for (int i = 0; i < test.producer_ports.size(); ++i) {
        testPort(test.producer_ports[i], loaded.producer_ports[i]);
    }
}

void testPortInstance(const traact::pattern::instance::PortInstance &test, const traact::pattern::instance::PortInstance &loaded) {
    testPort(test.port, loaded.port);
    EXPECT_EQ(test.connected_to, loaded.connected_to);
}

void testPortGroupInstance(const traact::pattern::instance::PortGroupInstance &test, const traact::pattern::instance::PortGroupInstance &loaded) {
    EXPECT_EQ(test.port_group_instance_index, loaded.port_group_instance_index);
    testPortGroup(test.port_group, loaded.port_group);
    EXPECT_EQ(test.consumer_ports.size(), loaded.consumer_ports.size());
    for (int i = 0; i < test.consumer_ports.size(); ++i) {
        testPortInstance(test.consumer_ports[i], loaded.consumer_ports[i]);
    }
    EXPECT_EQ(test.producer_ports.size(), loaded.producer_ports.size());
    for (int i = 0; i < test.producer_ports.size(); ++i) {
        testPortInstance(test.producer_ports[i], loaded.producer_ports[i]);
    }

}
TEST(PatternGraphSerialization, TestPatternGraph) {
    using namespace traact;
    using namespace traact::pattern;
    using namespace traact::pattern::instance;

    GraphInstance test_graph("test_graph");

    Pattern test_pattern("test_pattern", Concurrency::UNLIMITED, component::ComponentType::SYNC_FUNCTIONAL);
    test_pattern.addParameter("int_parameter", 2, 0, 10)
        .addParameter("float_parameter", 5.0, 0.0, 10.0)
        .addStringParameter("string_parameter", "foo")
        .addConsumerPort<InPort0>("input_0")
        .addConsumerPort<InPort1>("input_1")
        .addConsumerPort<InPort2>("input_2")
        .addProducerPort<OutPort0>("output_0")
        .addProducerPort<OutPort1>("output_1")
        .addProducerPort<OutPort2>("output_2")
        .beginPortGroup("group_0")
        .addConsumerPort<InPort0>("group_input_0")
        .addConsumerPort<InPort1>("group_input_1")
        .addConsumerPort<InPort2>("group_input_2")
        .addProducerPort<OutPort0>("group_output_0")
        .addProducerPort<OutPort1>("group_output_1")
        .addProducerPort<OutPort2>("group_output_2")
        .addParameter("int_parameter", 2, 0, 10)
        .addParameter("float_parameter", 5.0, 0.0, 10.0)
        .addStringParameter("string_parameter", "foo")
        .endPortGroup();

    Pattern::Ptr test_pattern_ptr = std::make_shared<Pattern>(test_pattern);

    auto pattern_instance_0 = test_graph.addPattern("instance_0", test_pattern_ptr, 0);
    auto pattern_instance_1 = test_graph.addPattern("instance_1", test_pattern_ptr, 0);

    auto &pattern_0_group_0_instance = pattern_instance_0->instantiatePortGroup("group_0");
    auto &pattern_0_group_1_instance = pattern_instance_0->instantiatePortGroup("group_0");

    auto &pattern_1_group_0_instance = pattern_instance_1->instantiatePortGroup("group_0");

    test_graph.connect("instance_0", "output_0", "instance_1", "input_1");
    test_graph.connect("instance_0", "output_1", "instance_1", "input_2");
    test_graph.connect("instance_0", "output_2", "instance_1", "input_0");

    test_graph.connect("instance_0",
                       pattern_0_group_0_instance.getProducerPortName("group_output_0"),
                       "instance_1",
                       pattern_1_group_0_instance.getConsumerPortName("group_input_1"));
    test_graph.connect("instance_0",
                       pattern_0_group_0_instance.getProducerPortName("group_output_1"),
                       "instance_1",
                       pattern_1_group_0_instance.getConsumerPortName("group_input_2"));
    test_graph.connect("instance_0",
                       pattern_0_group_1_instance.getProducerPortName("group_output_0"),
                       "instance_1",
                       pattern_1_group_0_instance.getConsumerPortName("group_input_0"));

    buffer::TimeDomainManagerConfig time_domain_manager_config;
    time_domain_manager_config.time_domain = 0;
    time_domain_manager_config.cpu_count = 5;
    time_domain_manager_config.max_offset = TimeDuration(14578554);
    time_domain_manager_config.sensor_frequency = 45.0;
    time_domain_manager_config.source_mode = SourceMode::IMMEDIATE_RETURN;
    time_domain_manager_config.missing_source_event_mode = MissingSourceEventMode::CANCEL_OLDEST;
    time_domain_manager_config.ringbuffer_size = 16;
    time_domain_manager_config.max_delay = TimeDuration(900000);
    test_graph.timedomain_configs.emplace(0, time_domain_manager_config);

    nlohmann::json json_graph;
    ns::to_json(json_graph, test_graph);
    std::string graph_string = json_graph.dump(4);

    nlohmann::json loaded_json_graph = nlohmann::json::parse(graph_string);

    GraphInstance loaded_graph;
    ns::from_json(loaded_json_graph, loaded_graph);

    for (const auto &[time_domain, test_config] : test_graph.timedomain_configs) {
        auto &loaded_config = loaded_graph.timedomain_configs.at(time_domain);

        EXPECT_EQ(test_config.time_domain, loaded_config.time_domain);
        EXPECT_EQ(test_config.cpu_count, loaded_config.cpu_count);
        EXPECT_EQ(test_config.sensor_frequency, loaded_config.sensor_frequency);
        EXPECT_EQ(test_config.source_mode, loaded_config.source_mode);
        EXPECT_EQ(test_config.missing_source_event_mode, loaded_config.missing_source_event_mode);
        EXPECT_EQ(test_config.ringbuffer_size, loaded_config.ringbuffer_size);
        EXPECT_EQ(test_config.max_delay, loaded_config.max_delay);
        EXPECT_EQ(test_config.max_offset, loaded_config.max_offset);
    }

    for (const auto &[instance_id, test_pattern] : test_graph.pattern_instances) {
        auto &loaded_pattern = loaded_graph.pattern_instances.at(instance_id);
        EXPECT_EQ(test_pattern->display_name, loaded_pattern->display_name);
        EXPECT_EQ(test_pattern->instance_id, loaded_pattern->instance_id);
        EXPECT_EQ(test_pattern->local_to_global_time_domain.size(), loaded_pattern->local_to_global_time_domain.size());
        for (int i = 0; i < test_pattern->local_to_global_time_domain.size(); ++i) {
            EXPECT_EQ(test_pattern->local_to_global_time_domain[i], loaded_pattern->local_to_global_time_domain[i]);

        }
        EXPECT_EQ(test_pattern->port_group_name_to_index.size(), loaded_pattern->port_group_name_to_index.size());
        for (const auto &[group_name, group_index] : test_pattern->port_group_name_to_index) {
            EXPECT_EQ(group_index, loaded_pattern->port_group_name_to_index.at(group_name));
        }

        EXPECT_EQ(test_pattern->local_pattern.name, loaded_pattern->local_pattern.name);
        EXPECT_EQ(test_pattern->local_pattern.time_domain_component_type.size(),
                  loaded_pattern->local_pattern.time_domain_component_type.size());
        EXPECT_EQ(test_pattern->local_pattern.concurrency.size(), loaded_pattern->local_pattern.concurrency.size());
        for (int i = 0; i < test_pattern->local_pattern.time_domain_component_type.size(); ++i) {
            EXPECT_EQ(test_pattern->local_pattern.time_domain_component_type[i],
                      loaded_pattern->local_pattern.time_domain_component_type[i]);
            EXPECT_EQ(test_pattern->local_pattern.concurrency[i], loaded_pattern->local_pattern.concurrency[i]);
        }
        EXPECT_EQ(test_pattern->local_pattern.port_groups.size(), loaded_pattern->local_pattern.port_groups.size());
        for (int i = 0; i < test_pattern->local_pattern.port_groups.size(); ++i) {
            testPortGroup(test_pattern->local_pattern.port_groups[i], loaded_pattern->local_pattern.port_groups[i]);
        }

        EXPECT_EQ(test_pattern->port_groups.size(), loaded_pattern->port_groups.size());
        for (int port_group_index = 0; port_group_index < test_pattern->port_groups.size(); ++port_group_index) {
            EXPECT_EQ(test_pattern->port_groups[port_group_index].size(), loaded_pattern->port_groups[port_group_index].size());
            for (int group_instance_index = 0;
                 group_instance_index < test_pattern->port_groups[port_group_index].size(); ++group_instance_index) {
                testPortGroupInstance(*test_pattern->port_groups[port_group_index][group_instance_index],
                                      *loaded_pattern->port_groups[port_group_index][group_instance_index]);
            }
        }


    }
    nlohmann::json json_loaded_graph;
    ns::to_json(json_loaded_graph, loaded_graph);
    EXPECT_EQ(json_graph.dump(), json_loaded_graph.dump());


}
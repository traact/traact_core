/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACTTEST_SRC_TRAACT_NETWORK_DYNAMICJOINNODE_H_
#define TRAACTTEST_SRC_TRAACT_NETWORK_DYNAMICJOINNODE_H_

#include <tbb/flow_graph.h>
#include <traact/datatypes.h>
#include <tuple>
#include "DataflowMessage.h"

namespace traact::dataflow {

class DynamicJoinNode {
 public:

    DynamicJoinNode(tbb::flow::graph &graph, int num_inputs) : num_inputs_(num_inputs), join_node_(nullptr) {
        using namespace tbb::flow;
        switch (num_inputs_) {
            case 0:throw std::out_of_range("unsupported number of senders used");
            case 1: {
                auto func_p = new function_node<TraactMessage, TraactMessage>(graph,
                                                                              unlimited,
                                                                              std::bind(&DynamicJoinNode::join1,
                                                                                        this,
                                                                                        std::placeholders::_1));
                function_node_ = func_p;
                break;
            }
            case 2: {
                auto join_p = new join_node<std::tuple<TraactMessage, TraactMessage>, key_matching<uint64_t> >(graph);
                join_node_ = join_p;
                auto func_p = new function_node<tuple<TraactMessage, TraactMessage>, TraactMessage>(graph,
                                                                                                    unlimited,
                                                                                                    std::bind(&DynamicJoinNode::join2,
                                                                                                              this,
                                                                                                              std::placeholders::_1));
                function_node_ = func_p;
                make_edge(*join_p, *func_p);
                break;
            }
            case 3: {
                auto join_p =
                    new join_node<std::tuple<TraactMessage, TraactMessage, TraactMessage>, key_matching<uint64_t> >(
                        graph);
                join_node_ = join_p;
                auto
                    func_p = new function_node<tuple<TraactMessage, TraactMessage, TraactMessage>, TraactMessage>(graph,
                                                                                                                  unlimited,
                                                                                                                  std::bind(
                                                                                                                      &DynamicJoinNode::join3,
                                                                                                                      this,
                                                                                                                      std::placeholders::_1));
                function_node_ = func_p;
                make_edge(*join_p, *func_p);
                break;
            }
            case 4: {
                auto join_p =
                    new join_node<std::tuple<TraactMessage, TraactMessage, TraactMessage, TraactMessage>,
                                  key_matching<uint64_t> >(graph);
                join_node_ = join_p;
                auto func_p =
                    new function_node<tuple<TraactMessage, TraactMessage, TraactMessage, TraactMessage>, TraactMessage>(
                        graph,
                        unlimited,
                        std::bind(&DynamicJoinNode::join4,
                                  this,
                                  std::placeholders::_1));
                function_node_ = func_p;
                make_edge(*join_p, *func_p);
                break;
            }
            case 5: {
                auto join_p =
                    new join_node<std::tuple<TraactMessage, TraactMessage, TraactMessage, TraactMessage, TraactMessage>,
                                  key_matching<uint64_t> >(graph);
                join_node_ = join_p;
                auto func_p =
                    new function_node<tuple<TraactMessage, TraactMessage, TraactMessage, TraactMessage, TraactMessage>,
                                      TraactMessage>(graph,
                                                     unlimited,
                                                     std::bind(&DynamicJoinNode::join5,
                                                               this,
                                                               std::placeholders::_1));
                function_node_ = func_p;
                make_edge(*join_p, *func_p);
                break;
            }
            case 6: {
                auto join_p =
                    new join_node<std::tuple<TraactMessage,
                                             TraactMessage,
                                             TraactMessage,
                                             TraactMessage,
                                             TraactMessage,
                                             TraactMessage>, key_matching<uint64_t> >(graph);
                join_node_ = join_p;
                auto func_p = new function_node<tuple<TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage>, TraactMessage>(graph,
                                                                                     unlimited,
                                                                                     std::bind(&DynamicJoinNode::join6,
                                                                                               this,
                                                                                               std::placeholders::_1));
                function_node_ = func_p;
                make_edge(*join_p, *func_p);
                break;
            }
            case 7: {
                auto join_p =
                    new join_node<std::tuple<TraactMessage,
                                             TraactMessage,
                                             TraactMessage,
                                             TraactMessage,
                                             TraactMessage,
                                             TraactMessage,
                                             TraactMessage>, key_matching<uint64_t> >(graph);
                join_node_ = join_p;
                auto func_p = new function_node<tuple<TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage>, TraactMessage>(graph,
                                                                                     unlimited,
                                                                                     std::bind(&DynamicJoinNode::join7,
                                                                                               this,
                                                                                               std::placeholders::_1));
                function_node_ = func_p;
                make_edge(*join_p, *func_p);
                break;
            }
            default:
            case 8: {
                auto join_p =
                    new join_node<std::tuple<TraactMessage,
                                             TraactMessage,
                                             TraactMessage,
                                             TraactMessage,
                                             TraactMessage,
                                             TraactMessage,
                                             TraactMessage,
                                             TraactMessage>, key_matching<uint64_t> >(graph);
                join_node_ = join_p;
                auto func_p = new function_node<tuple<TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage,
                                                      TraactMessage>, TraactMessage>(graph,
                                                                                     unlimited,
                                                                                     std::bind(&DynamicJoinNode::join8,
                                                                                               this,
                                                                                               std::placeholders::_1));
                nested_node_ = new DynamicJoinNode(graph, num_inputs - 7);
                function_node_ = func_p;
                make_edge(*join_p, *func_p);

                make_edge(nested_node_->getSender(), input_port<7>(*join_p));

                break;
            }
//        default:
//            throw std::out_of_range("unsupported number of senders used");
        }
    }
    virtual ~DynamicJoinNode() {
        using namespace tbb::flow;

        switch (num_inputs_) {
            case 1: {
                auto *tmp2 =
                    static_cast<function_node<TraactMessage, TraactMessage> * >(function_node_);
                delete tmp2;
                break;
            }
            case 2: {
                auto *tmp =
                    static_cast<join_node<std::tuple<TraactMessage, TraactMessage>,
                                          key_matching<uint64_t> > * >(join_node_);
                delete tmp;
                auto *tmp2 =
                    static_cast<function_node<tuple<TraactMessage, TraactMessage>, TraactMessage> * >(function_node_);
                delete tmp2;
                break;
            }
            case 3: {
                auto *tmp =
                    static_cast<join_node<std::tuple<TraactMessage, TraactMessage, TraactMessage>,
                                          key_matching<uint64_t> > * >(join_node_);
                delete tmp;
                auto *tmp2 =
                    static_cast<function_node<tuple<TraactMessage, TraactMessage, TraactMessage>,
                                              TraactMessage> * >(function_node_);
                delete tmp2;
                break;
            }
            case 4: {
                auto *tmp =
                    static_cast<join_node<std::tuple<TraactMessage, TraactMessage, TraactMessage, TraactMessage>,
                                          key_matching<uint64_t> > * >(join_node_);
                delete tmp;
                auto *tmp2 =
                    static_cast<function_node<tuple<TraactMessage, TraactMessage, TraactMessage, TraactMessage>,
                                              TraactMessage> * >(function_node_);
                delete tmp2;
                break;
            }
            case 5: {
                auto *tmp =
                    static_cast<join_node<std::tuple<TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage>,
                                          key_matching<uint64_t> > * >(join_node_);
                delete tmp;
                auto *tmp2 =
                    static_cast<function_node<tuple<TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage>,
                                              TraactMessage> * >(function_node_);
                delete tmp2;
                break;
            }
            case 6: {
                auto *tmp =
                    static_cast<join_node<std::tuple<TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage>,
                                          key_matching<uint64_t> > * >(join_node_);
                delete tmp;
                auto *tmp2 =
                    static_cast<function_node<tuple<TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage>,
                                              TraactMessage> * >(function_node_);
                delete tmp2;
                break;
            }
            case 7: {
                auto *tmp =
                    static_cast<join_node<std::tuple<TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage>,
                                          key_matching<uint64_t> > * >(join_node_);
                delete tmp;
                auto *tmp2 =
                    static_cast<function_node<tuple<TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage>,
                                              TraactMessage> * >(function_node_);
                delete tmp2;
                break;
            }
            default:
            case 8: {
                auto *tmp =
                    static_cast<join_node<std::tuple<TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage,
                                                     TraactMessage>,
                                          key_matching<uint64_t> > * >(join_node_);
                delete tmp;
                auto *tmp2 =
                    static_cast<function_node<tuple<TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage,
                                                    TraactMessage>,
                                              TraactMessage> * >(function_node_);
                delete tmp2;
                delete nested_node_;
                break;
            }
                //default:break;

        }

    }

    tbb::flow::sender<TraactMessage> &getSender() {
        using namespace tbb::flow;
        switch (num_inputs_) {
            case 1: {
                auto *tmp =
                    static_cast<function_node<TraactMessage, TraactMessage> * >(function_node_);
                return *tmp;
            }
            case 2: {
                auto *tmp =
                    static_cast<function_node<std::tuple<TraactMessage, TraactMessage>,
                                              TraactMessage> * >(function_node_);
                return *tmp;
            }
            case 3: {
                auto *tmp =
                    static_cast<function_node<std::tuple<TraactMessage, TraactMessage, TraactMessage>,
                                              TraactMessage> * >(function_node_);
                return *tmp;
            }
            case 4: {
                auto *tmp =
                    static_cast<function_node<std::tuple<TraactMessage, TraactMessage, TraactMessage, TraactMessage>,
                                              TraactMessage> * >(function_node_);
                return *tmp;
            }
            case 5: {
                auto *tmp =
                    static_cast<function_node<std::tuple<TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage>,
                                              TraactMessage> * >(function_node_);
                return *tmp;
            }
            case 6: {
                auto *tmp =
                    static_cast<function_node<std::tuple<TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage>,
                                              TraactMessage> * >(function_node_);
                return *tmp;
            }
            case 7: {
                auto *tmp =
                    static_cast<function_node<std::tuple<TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage>,
                                              TraactMessage> * >(function_node_);
                return *tmp;
            }
            default:
            case 8: {
                auto *tmp =
                    static_cast<function_node<std::tuple<TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage,
                                                         TraactMessage>,
                                              TraactMessage> * >(function_node_);
                return *tmp;
            }
//	  default:
//	      throw std::out_of_range("unsupported number of senders used");
        }

    }

    tbb::flow::receiver<TraactMessage> &getReceiver(int index) {
        using namespace tbb::flow;

        switch (num_inputs_) {
            case 1: {
                if (index != 0)
                    throw std::out_of_range("unsupported index for receiver of dynamic join node");

                auto *tmp =
                    static_cast<function_node<TraactMessage, TraactMessage> * >(function_node_);
                return *tmp;
            }
            case 2: {
                switch (index) {
                    case 0: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage, TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<0>(*tmp);
                    }
                    case 1: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage, TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<1>(*tmp);
                    }
                    default:throw std::out_of_range("unsupported index for receiver of dynamic join node");
                }
            }
            case 3: {
                switch (index) {
                    case 0: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage, TraactMessage, TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<0>(*tmp);
                    }
                    case 1: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage, TraactMessage, TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<1>(*tmp);
                    }
                    case 2: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage, TraactMessage, TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<2>(*tmp);
                    }
                    default:throw std::out_of_range("unsupported index for receiver of dynamic join node");
                }
            }
            case 4: {
                switch (index) {
                    case 0: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<0>(*tmp);
                    }
                    case 1: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<1>(*tmp);
                    }
                    case 2: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<2>(*tmp);
                    }
                    case 3: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<3>(*tmp);
                    }
                    default:throw std::out_of_range("unsupported index for receiver of dynamic join node");
                }
            }
            case 5: {
                switch (index) {
                    case 0: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<0>(*tmp);
                    }
                    case 1: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<1>(*tmp);
                    }
                    case 2: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<2>(*tmp);
                    }
                    case 3: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<3>(*tmp);
                    }
                    case 4: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<4>(*tmp);
                    }
                    default:throw std::out_of_range("unsupported index for receiver of dynamic join node");
                }
            }
            case 6: {
                switch (index) {
                    case 0: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<0>(*tmp);
                    }
                    case 1: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<1>(*tmp);
                    }
                    case 2: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<2>(*tmp);
                    }
                    case 3: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<3>(*tmp);
                    }
                    case 4: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<4>(*tmp);
                    }
                    case 5: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<5>(*tmp);
                    }
                    default:throw std::out_of_range("unsupported index for receiver of dynamic join node");
                }
            }
            case 7: {
                switch (index) {
                    case 0: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<0>(*tmp);
                    }
                    case 1: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<1>(*tmp);
                    }
                    case 2: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<2>(*tmp);
                    }
                    case 3: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<3>(*tmp);
                    }
                    case 4: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<4>(*tmp);
                    }
                    case 5: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<5>(*tmp);
                    }
                    case 6: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<6>(*tmp);
                    }

                    default:throw std::out_of_range("unsupported index for receiver of dynamic join node");
                }
            }
            default:
            case 8: {
                switch (index) {
                    case 0: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<0>(*tmp);
                    }
                    case 1: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<1>(*tmp);
                    }
                    case 2: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<2>(*tmp);
                    }
                    case 3: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<3>(*tmp);
                    }
                    case 4: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<4>(*tmp);
                    }
                    case 5: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<5>(*tmp);
                    }
                    case 6: {
                        auto *tmp =
                            static_cast<join_node<std::tuple<TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage,
                                                             TraactMessage>,
                                                  key_matching<uint64_t> > * >(join_node_);
                        return input_port<6>(*tmp);
                    }
                    default:
                    case 7: {
                        return nested_node_->getReceiver(index - 7);
                    }

                        //default:throw std::out_of_range("unsupported index for receiver of dynamic join node");
                }
            }
                //default:throw std::out_of_range("unsupported numver of inputs for dynamic join node");
        }

    }

    TraactMessage join1(const TraactMessage &in) {
        return in;
    }

    TraactMessage join2(const std::tuple<TraactMessage, TraactMessage> &in) {

        TraactMessage result = std::get<0>(in);

        result.merge(std::get<1>(in));

        return result;
    }
    TraactMessage join3(const std::tuple<TraactMessage, TraactMessage, TraactMessage> &in) {
        TraactMessage result = std::get<0>(in);
        result.merge(std::get<1>(in));
        result.merge(std::get<2>(in));

        return result;
    }
    TraactMessage join4(const std::tuple<TraactMessage, TraactMessage, TraactMessage, TraactMessage> &in) {
        TraactMessage result = std::get<0>(in);
        result.merge(std::get<1>(in));
        result.merge(std::get<2>(in));
        result.merge(std::get<3>(in));
        return result;
    }
    TraactMessage join5(const std::tuple<TraactMessage,
                                         TraactMessage,
                                         TraactMessage,
                                         TraactMessage,
                                         TraactMessage> &in) {
        TraactMessage result = std::get<0>(in);
        result.merge(std::get<1>(in));
        result.merge(std::get<2>(in));
        result.merge(std::get<3>(in));
        result.merge(std::get<4>(in));
        return result;
    }
    TraactMessage join6(const std::tuple<TraactMessage,
                                         TraactMessage,
                                         TraactMessage,
                                         TraactMessage,
                                         TraactMessage,
                                         TraactMessage> &in) {
        TraactMessage result = std::get<0>(in);
        result.merge(std::get<1>(in));
        result.merge(std::get<2>(in));
        result.merge(std::get<3>(in));
        result.merge(std::get<4>(in));
        result.merge(std::get<5>(in));
        return result;
    }
    TraactMessage join7(const std::tuple<TraactMessage,
                                         TraactMessage,
                                         TraactMessage,
                                         TraactMessage,
                                         TraactMessage,
                                         TraactMessage,
                                         TraactMessage> &in) {
        TraactMessage result = std::get<0>(in);
        result.merge(std::get<1>(in));
        result.merge(std::get<2>(in));
        result.merge(std::get<3>(in));
        result.merge(std::get<4>(in));
        result.merge(std::get<5>(in));
        result.merge(std::get<6>(in));
        return result;
    }
    TraactMessage join8(const std::tuple<TraactMessage,
                                         TraactMessage,
                                         TraactMessage,
                                         TraactMessage,
                                         TraactMessage,
                                         TraactMessage,
                                         TraactMessage,
                                         TraactMessage> &in) {
        TraactMessage result = std::get<0>(in);
        result.merge(std::get<1>(in));
        result.merge(std::get<2>(in));
        result.merge(std::get<3>(in));
        result.merge(std::get<4>(in));
        result.merge(std::get<5>(in));
        result.merge(std::get<6>(in));
        result.merge(std::get<7>(in));
        return result;
    }
 private:
    void *join_node_;
    void *function_node_;

    const int num_inputs_;
    DynamicJoinNode *nested_node_;

};

}

#endif //TRAACTTEST_SRC_TRAACT_NETWORK_DYNAMICJOINNODE_H_

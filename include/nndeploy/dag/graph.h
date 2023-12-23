#ifndef _NNDEPLOY_DAG_GRAPH_H_
#define _NNDEPLOY_DAG_GRAPH_H_

#include "nndeploy/base/common.h"
#include "nndeploy/base/glic_stl_include.h"
#include "nndeploy/base/log.h"
#include "nndeploy/base/macro.h"
#include "nndeploy/base/object.h"
#include "nndeploy/base/status.h"
#include "nndeploy/base/string.h"
#include "nndeploy/base/value.h"
#include "nndeploy/dag/edge.h"
#include "nndeploy/dag/graph/executor.h"
#include "nndeploy/dag/node.h"
#include "nndeploy/device/buffer.h"
#include "nndeploy/device/buffer_pool.h"
#include "nndeploy/device/device.h"
#include "nndeploy/device/tensor.h"

namespace nndeploy {
namespace dag {

class NNDEPLOY_CC_API GraphParam : public base::Param {
 public:
  ParallelType parallel_type_ = kParallelTypeNone;
};

class NNDEPLOY_CC_API Graph : public Node {
 public:
  Graph(const std::string &name, Edge *input, Edge *output);
  Graph(const std::string &name, std::initializer_list<Edge *> inputs,
        std::initializer_list<Edge *> outputs);
  ~Graph();

  Edge *createEdge(const std::string &name);
  EdgeWrapper *addEdge(Edge *edge);

  template <typename T,
            typename std::enable_if<std::is_base_of<Node, T>{}, int>::type = 0>
  Node *createNode(const std::string &name, Edge *input, Edge *output) {
    NNDEPLOY_CHECK_PARAM_NULL_RET_NULL(input, "input is null!");
    NNDEPLOY_CHECK_PARAM_NULL_RET_NULL(output, "output is null!");
    Node *node = dynamic_cast<Node *>(new T(name, input, output));
    NodeWrapper *node_wrapper = new NodeWrapper();
    node_wrapper->is_external_ = false;
    node_wrapper->node_ = node;
    node_wrapper->name_ = name;
    EdgeWrapper *input_wrapper = findEdgeWrapper(edge_repository_, input);
    if (input_wrapper == nullptr) {
      input_wrapper = this->addEdge(input);
    }
    input_wrapper->consumers_.emplace_back(node_wrapper);
    EdgeWrapper *output_wrapper = findEdgeWrapper(edge_repository_, output);
    if (output_wrapper == nullptr) {
      output_wrapper = this->addEdge(output);
    }
    output_wrapper->producers_.emplace_back(node_wrapper);

    node_repository_.emplace_back(node_wrapper);
    return node;
  }
  template <typename T,
            typename std::enable_if<std::is_base_of<Node, T>{}, int>::type = 0>
  Node *createNode(const std::string &name,
                   std::initializer_list<Edge *> inputs,
                   std::initializer_list<Edge *> outputs) {
    Node *node = dynamic_cast<Node *>(new T(name, inputs, outputs));
    NodeWrapper *node_wrapper = new NodeWrapper();
    node_wrapper->is_external_ = false;
    node_wrapper->node_ = node;
    node_wrapper->name_ = name;
    for (auto input : inputs) {
      EdgeWrapper *input_wrapper = findEdgeWrapper(edge_repository_, input);
      if (input_wrapper == nullptr) {
        input_wrapper = this->addEdge(input);
      }
      input_wrapper->consumers_.emplace_back(node_wrapper);
    }
    for (auto output : outputs) {
      EdgeWrapper *output_wrapper = findEdgeWrapper(edge_repository_, output);
      if (output_wrapper == nullptr) {
        output_wrapper = this->addEdge(output);
      }
      output_wrapper->producers_.emplace_back(node_wrapper);
    }

    node_repository_.emplace_back(node_wrapper);
    return node;
  }
  template <typename T,
            typename std::enable_if<std::is_base_of<Node, T>{}, int>::type = 0>
  Node *createInfer(const std::string &name, base::InferenceType type,
                    Edge *input, Edge *output) {
    NNDEPLOY_CHECK_PARAM_NULL_RET_NULL(input, "input is null!");
    NNDEPLOY_CHECK_PARAM_NULL_RET_NULL(output, "output is null!");
    Node *node = dynamic_cast<Node *>(new T(name, type, input, output));
    NodeWrapper *node_wrapper = new NodeWrapper();
    node_wrapper->is_external_ = false;
    node_wrapper->node_ = node;
    node_wrapper->name_ = name;
    EdgeWrapper *input_wrapper = findEdgeWrapper(edge_repository_, input);
    if (input_wrapper == nullptr) {
      input_wrapper = this->addEdge(input);
    }
    input_wrapper->consumers_.emplace_back(node_wrapper);
    EdgeWrapper *output_wrapper = findEdgeWrapper(edge_repository_, output);
    if (output_wrapper == nullptr) {
      output_wrapper = this->addEdge(output);
    }
    output_wrapper->producers_.emplace_back(node_wrapper);

    node_repository_.emplace_back(node_wrapper);
    return node;
  }
  template <typename T,
            typename std::enable_if<std::is_base_of<Node, T>{}, int>::type = 0>
  Node *createInfer(const std::string &name, base::InferenceType type,
                    std::initializer_list<Edge *> inputs,
                    std::initializer_list<Edge *> outputs) {
    Node *node = dynamic_cast<Node *>(new T(name, type, inputs, outputs));
    NodeWrapper *node_wrapper = new NodeWrapper();
    node_wrapper->is_external_ = false;
    node_wrapper->node_ = node;
    node_wrapper->name_ = name;
    for (auto input : inputs) {
      EdgeWrapper *input_wrapper = findEdgeWrapper(edge_repository_, input);
      if (input_wrapper == nullptr) {
        input_wrapper = this->addEdge(input);
      }
      input_wrapper->consumers_.emplace_back(node_wrapper);
    }
    for (auto output : outputs) {
      EdgeWrapper *output_wrapper = findEdgeWrapper(edge_repository_, output);
      if (output_wrapper == nullptr) {
        output_wrapper = this->addEdge(output);
      }
      output_wrapper->producers_.emplace_back(node_wrapper);
    }

    node_repository_.emplace_back(node_wrapper);
    return node;
  }
  base::Status addNode(Node *node);

  base::Status setNodeParam(const std::string &node_name, base::Param *param);
  base::Param *getNodeParam(const std::string &node_name);

  base::Status init();
  base::Status deinit();

  base::Status run();

  base::Status dump(std::ostream &oss = std::cout);

  base::Status setParallelType(const ParallelType &type);

 protected:
  std::vector<EdgeWrapper *> edge_repository_;
  std::vector<NodeWrapper *> node_repository_;

  std::shared_ptr<Executor> executor_;
};

using createGraphFunc = std::function<Graph *(
    const std::string &name, base::InferenceType inference_type,
    base::DeviceType device_type, Edge *input, Edge *output,
    base::ModelType model_type, bool is_path,
    std::vector<std::string> model_value)>;

std::map<std::string, createGraphFunc> &getGlobalGraphCreatorMap();

class TypeGraphRegister {
 public:
  explicit TypeGraphRegister(const std::string &name, createGraphFunc func) {
    getGlobalGraphCreatorMap()[name] = func;
  }
};

extern NNDEPLOY_CC_API Graph *createGraph(const std::string &name,
                                          base::InferenceType inference_type,
                                          base::DeviceType device_type,
                                          Edge *input, Edge *output,
                                          base::ModelType model_type,
                                          bool is_path,
                                          std::vector<std::string> model_value);

}  // namespace dag
}  // namespace nndeploy

#endif  // _NNDEPLOY_DAG_GRAPH_H_

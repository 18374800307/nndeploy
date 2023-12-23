#include "flag.h"
#include "nndeploy/base/glic_stl_include.h"
#include "nndeploy/base/time_profiler.h"
#include "nndeploy/dag/node.h"
#include "nndeploy/device/device.h"
#include "nndeploy/model/detect/yolo/yolo.h"
#include "nndeploy/thread_pool/thread_pool.h"

using namespace nndeploy;

cv::Mat drawBox(cv::Mat &cv_mat, model::DetectResult &result) {
  // float w_ratio = float(cv_mat.cols) / float(640);
  // float h_ratio = float(cv_mat.rows) / float(640);
  float w_ratio = float(cv_mat.cols);
  float h_ratio = float(cv_mat.rows);
  const int CNUM = 80;
  cv::RNG rng(0xFFFFFFFF);
  cv::Scalar_<int> randColor[CNUM];
  for (int i = 0; i < CNUM; i++)
    rng.fill(randColor[i], cv::RNG::UNIFORM, 0, 256);
  int i = -1;
  for (auto bbox : result.bboxs_) {
    std::array<float, 4> box;
    box[0] = bbox.bbox_[0];  // 640.0;
    box[2] = bbox.bbox_[2];  // 640.0;
    box[1] = bbox.bbox_[1];  // 640.0;
    box[3] = bbox.bbox_[3];  // 640.0;
    box[0] *= w_ratio;
    box[2] *= w_ratio;
    box[1] *= h_ratio;
    box[3] *= h_ratio;
    int width = box[2] - box[0];
    int height = box[3] - box[1];
    int id = bbox.label_id_;
    NNDEPLOY_LOGE("box[0]:%f, box[1]:%f, width :%d, height :%d\n", box[0],
                  box[1], width, height);
    cv::Point p = cv::Point(box[0], box[1]);
    cv::Rect rect = cv::Rect(box[0], box[1], width, height);
    cv::rectangle(cv_mat, rect, randColor[id]);
    std::string text = " ID:" + std::to_string(id);
    cv::putText(cv_mat, text, p, cv::FONT_HERSHEY_PLAIN, 1, randColor[id]);
  }
  return cv_mat;
}

//
int main(int argc, char *argv[]) {
  gflags::ParseCommandLineNonHelpFlags(&argc, &argv, true);
  if (demo::FLAGS_usage) {
    demo::showUsage();
    return -1;
  }

  thread_pool::ThreadPool *tp = new thread_pool::ThreadPool(8);

   // 检测模型的有向无环图graph名称，例如:
   // NNDEPLOY_YOLOV5/NNDEPLOY_YOLOV6/NNDEPLOY_YOLOV8
   std::string name = demo::getName();
   // 推理后端类型，例如:
   // kInferenceTypeOpenVino / kInferenceTypeTensorRt / kInferenceTypeOnnxRuntime
   base::InferenceType inference_type = demo::getInferenceType();
   // 推理设备类型，例如:
   // kDeviceTypeCodeX86:0/kDeviceTypeCodeCuda:0/...
   base::DeviceType device_type = demo::getDeviceType();
   // 模型类型，例如:
   // kModelTypeOnnx/kModelTypeMnn/...
   base::ModelType model_type = demo::getModelType();
   // 模型是否是路径
   bool is_path = demo::isPath();
   // 模型路径或者模型字符串
   std::vector<std::string> model_value = demo::getModelValue();
//  // 检测模型的有向无环图graph名称，例如:
//  // NNDEPLOY_YOLOV5/NNDEPLOY_YOLOV6/NNDEPLOY_YOLOV8
//  std::string name = "NNDEPLOY_YOLOV6";
//  // 推理后端类型，例如:
//  // kInferenceTypeOpenVino / kInferenceTypeTensorRt / kInferenceTypeOnnxRuntime
//  base::InferenceType inference_type = base::kInferenceTypeOnnxRuntime;
//  // 推理设备类型，例如:
//  // kDeviceTypeCodeX86:0/kDeviceTypeCodeCuda:0/...
//  base::DeviceType device_type = base::kDeviceTypeCodeX86;
//  // 模型类型，例如:
//  // kModelTypeOnnx/kModelTypeMnn/...
//  base::ModelType model_type = base::kModelTypeOnnx;
//  // 模型是否是路径
//  bool is_path = true;
//  // 模型路径或者模型字符串
//  std::string model_value_path =
//      "/home/always/huggingface/nndeploy/model_zoo/detect/yolo/yolov6m.onnx";
//  std::vector<std::string> model_value;
//  model_value.push_back(model_value_path);
  // 有向无环图graph的输入边packert
  dag::Edge input("detect_in");
  // 有向无环图graph的输出边packert
  dag::Edge output("detect_out");
  // 创建检测模型有向无环图graph
  dag::Graph *graph =
      dag::createGraph(name, inference_type, device_type, &input, &output,
                       model_type, is_path, model_value);
  if (graph == nullptr) {
    NNDEPLOY_LOGE("graph is nullptr");
    return -1;
  }

  // 初始化有向无环图graph
  NNDEPLOY_TIME_POINT_START("graph->init()");
  base::Status status = graph->init();
  if (status != base::kStatusCodeOk) {
    NNDEPLOY_LOGE("graph init failed");
    return -1;
  }
  NNDEPLOY_TIME_POINT_END("graph->init()");

  // 有向无环图graph的输入图片路径
   std::string input_path = demo::getInputPath();
//  std::string input_path =
//      "/home/always/huggingface/nndeploy/test_data/detect/sample.jpg";
  // opencv读图
  cv::Mat input_mat = cv::imread(input_path);
  // 将图片写入有向无环图graph输入边
  input.set(input_mat, 0);

  // 有向无环图Graphz运行
  NNDEPLOY_TIME_POINT_START("graph->run()");
  status = graph->run();
  if (status != base::kStatusCodeOk) {
    NNDEPLOY_LOGE("graph run failed");
    return -1;
  }
  NNDEPLOY_TIME_POINT_END("graph->run()");

  // 得到有向无环图graph的输出结果
  model::DetectResult *result = (model::DetectResult *)output.getParam(nullptr);
  if (result == nullptr) {
    NNDEPLOY_LOGE("result is nullptr");
    return -1;
  }

  drawBox(input_mat, *result);
   std::string ouput_path = demo::getOutputPath();
//  std::string ouput_path =
//      "/home/always/huggingface/nndeploy/temp/sample_output.jpg";
  cv::imwrite(ouput_path, input_mat);

  // 有向无环图graphz反初始化
  NNDEPLOY_TIME_POINT_START("graph->deinit()");
  status = graph->deinit();
  if (status != base::kStatusCodeOk) {
    NNDEPLOY_LOGE("graph deinit failed");
    return -1;
  }
  NNDEPLOY_TIME_POINT_END("graph->deinit()");

  NNDEPLOY_TIME_PROFILER_PRINT("detetct time profiler");

  // 有向无环图graphz销毁
  delete graph;

  NNDEPLOY_LOGE("hello world!\n");

  return 0;
}

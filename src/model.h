#ifndef NNT_MODEL_H
#define NNT_MODEL_H

#include <string>
#include <vector>
#include <memory>
#include <cuchar>
#include <boost/variant.hpp>

#include "schemas/schema_generated.h"

namespace nnt {

class FlatBufferModel {
 public:
  FlatBufferModel(const std::string& file);
  ~FlatBufferModel();
  char* data();
  int Length() const;

 private:
  char *data_;
  int len_;
};

class Buffer {
 public:
  Buffer(std::vector<u_char>&& buf): buf_(std::move(buf)) {}

  const std::vector<u_char>& Data() const {
    return buf_;
  }

  const u_char* RawData() const {
    return buf_.data();
  }

 private:
  std::vector<u_char> buf_;
};

enum class ActivationFunctionType: int8_t {
  NONE,
  RELU,
  RELU1,
  RELU6,
  TANH,
  SIGN_BIT
};

enum class TensorType: int8_t {
  FLOAT32,
  FLOAT16,
  INT32,
  UINT8,
  INT8,
  INT64,
  STRING,
#ifdef NEWER_TENSORFLOW
  BOOL
#endif
};

enum class Padding: int8_t { UNKNOWN = 0, SAME, VALID };

enum class BuiltinOperator {
  NONE,
  ADD,
  AVERAGE_POOL_2D,
  CONCATENATION,
  CONV_2D,
  DEPTHWISE_CONV_2D,
  DEQUANTIZE,
  EMBEDDING_LOOKUP,
#ifdef NEWER_TENSORFLOW
  FLOOR,
#endif
  FULLY_CONNECTED,
  HASHTABLE_LOOKUP,
  L2_NORMALIZATION,
  L2_POOL_2D,
  LOCAL_RESPONSE_NORMALIZATION,
  LOGISTIC,
  LSH_PROJECTION,
  LSTM,
  MAX_POOL_2D,
  MUL,
  RELU,
  RELU1,
  RELU6,
  RESHAPE,
  RESIZE_BILINEAR,
  RNN,
  SOFTMAX,
  SPACE_TO_DEPTH,
  SVDF,
  TANH,
  CONCAT_EMBEDDINGS,
  SKIP_GRAM,
  CALL,
  CUSTOM,
  EMBEDDING_LOOKUP_SPARSE,
  PAD,
  UNIDIRECTIONAL_SEQUENCE_RNN,
  GATHER,
  BATCH_TO_SPACE_ND,
  SPACE_TO_BATCH_ND,
  TRANSPOSE,
  MEAN,
  SUB,
  DIV,
  SQUEEZE,
  UNIDIRECTIONAL_SEQUENCE_LSTM,
  STRIDED_SLICE,
  BIDIRECTIONAL_SEQUENCE_RNN,
  EXP,
  TOPK_V2,
  SPLIT,
  LOG_SOFTMAX,
  DELEGATE,
  BIDIRECTIONAL_SEQUENCE_LSTM,
  CAST,
  PRELU,
  MAXIMUM,
  ARG_MAX,
  MINIMUM,
  LESS,
  NEG
};

enum class BuiltinOptionsType {
  None,
  Conv2DOptions,
  DepthwiseConv2DOptions,
  ConcatEmbeddingsOptions,
  LSHProjectionOptions,
  Pool2DOptions,
  SVDFOptions,
  RNNOptions,
  FullyConnectedOptions,
  SoftmaxOptions,
  ConcatenationOptions,
  AddOptions,
  L2NormOptions,
  LocalResponseNormalizationOptions,
  LSTMOptions,
  ResizeBilinearOptions,
  CallOptions,
  ReshapeOptions,
  SkipGramOptions,
  SpaceToDepthOptions,
  EmbeddingLookupSparseOptions,
  MulOptions,
  PadOptions,
  GatherOptions,
  BatchToSpaceNDOptions,
  SpaceToBatchNDOptions,
  TransposeOptions,
  ReducerOptions,
  SubOptions,
  DivOptions,
  SqueezeOptions,
  SequenceRNNOptions,
  StridedSliceOptions,
  ExpOptions,
  TopKV2Options,
  SplitOptions,
  LogSoftmaxOptions,
  CastOptions,
  DequantizeOptions,
  #ifdef NEWER_TENSORFLOW
    MaximumMinimumOptions,
    ArgMaxOptions,
    LessOptions,
    NegOptions,
  #else
    MaximumOptions
  #endif
};

struct OperatorCode {
  BuiltinOperator builtin_code;
  std::string custom_code;
};

struct BuiltinOptions {
  BuiltinOptions(BuiltinOptionsType type_op): type(type_op) {}

  BuiltinOptionsType type;
};

struct NoneOptions: public BuiltinOptions {
  NoneOptions(): BuiltinOptions(BuiltinOptionsType::None) {}
};

struct Conv2DOptions: public BuiltinOptions {
  Conv2DOptions(): BuiltinOptions(BuiltinOptionsType::Conv2DOptions) {}

  Padding padding;
  int stride_w;
  int stride_h;
#ifdef NEWER_TENSORFLOW
  int dilation_w_factor;
  int dilation_h_factor;
#endif
  ActivationFunctionType fused_activation_function;
};

struct Pool2DOptions: public BuiltinOptions {
  Pool2DOptions(): BuiltinOptions(BuiltinOptionsType::Pool2DOptions) {}

  Padding padding;
  int stride_w;
  int stride_h;
  int filter_width;
  int filter_height;
  ActivationFunctionType fused_activation_function;
};

struct DepthwiseConv2DOptions: public BuiltinOptions {
  DepthwiseConv2DOptions()
    : BuiltinOptions(BuiltinOptionsType::DepthwiseConv2DOptions) {}

  Padding padding;
  int stride_w;
  int stride_h;
  int depth_multiplier;
  ActivationFunctionType fused_activation_function;
};

struct ConcatEmbeddingsOptions: public BuiltinOptions {
  ConcatEmbeddingsOptions()
    : BuiltinOptions(BuiltinOptionsType::ConcatEmbeddingsOptions) {}

  int num_channels;
  std::vector<int> num_columns_per_channel;
  std::vector<int> embedding_dim_per_channel;
};

enum class LSHProjectionType: int8_t {
  UNKNOWN = 0,
  SPARSE = 1,
  DENSE = 2,
};

struct LSHProjectionOptions: public BuiltinOptions {
  LSHProjectionOptions()
    : BuiltinOptions(BuiltinOptionsType::LSHProjectionOptions) {}

  LSHProjectionType type;
};

struct SVDFOptions: public BuiltinOptions {
  SVDFOptions(): BuiltinOptions(BuiltinOptionsType::SVDFOptions) {}

  int rank;
  ActivationFunctionType fused_activation_function;
};

struct RNNOptions: public BuiltinOptions {
  RNNOptions(): BuiltinOptions(BuiltinOptionsType::RNNOptions) {}

  ActivationFunctionType fused_activation_function;
};

struct SequenceRNNOptions: public BuiltinOptions {
  SequenceRNNOptions()
    : BuiltinOptions(BuiltinOptionsType::SequenceRNNOptions) {}

  bool time_major;
  ActivationFunctionType fused_activation_function;
};

struct BidirectionalSequenceRNNOptions: public BuiltinOptions {
  bool time_major;
  ActivationFunctionType fused_activation_function;
};

struct FullyConnectedOptions: public BuiltinOptions {
  FullyConnectedOptions()
    : BuiltinOptions(BuiltinOptionsType::FullyConnectedOptions) {}

  ActivationFunctionType fused_activation_function;
};

struct SoftmaxOptions: public BuiltinOptions {
  SoftmaxOptions()
    : BuiltinOptions(BuiltinOptionsType::SoftmaxOptions) {}

  float beta;
};

struct ConcatenationOptions: public BuiltinOptions {
  ConcatenationOptions()
    : BuiltinOptions(BuiltinOptionsType::ConcatenationOptions) {}

  int axis;
  ActivationFunctionType fused_activation_function;
};

struct AddOptions: public BuiltinOptions {
  AddOptions(): BuiltinOptions(BuiltinOptionsType::AddOptions) {}

  ActivationFunctionType fused_activation_function;
};

struct MulOptions: public BuiltinOptions {
  MulOptions(): BuiltinOptions(BuiltinOptionsType::MulOptions) {}

  ActivationFunctionType fused_activation_function;
};

struct L2NormOptions: public BuiltinOptions {
  L2NormOptions(): BuiltinOptions(BuiltinOptionsType::L2NormOptions) {}

  ActivationFunctionType fused_activation_function;
};

struct LocalResponseNormalizationOptions: public BuiltinOptions {
  LocalResponseNormalizationOptions()
    : BuiltinOptions(BuiltinOptionsType::LocalResponseNormalizationOptions) {}

  int radius;
  float bias;
  float alpha;
  float beta;
};

struct LSTMOptions: public BuiltinOptions {
  LSTMOptions(): BuiltinOptions(BuiltinOptionsType::LSTMOptions) {}

  float cell_clip;
  float proj_clip;
  ActivationFunctionType fused_activation_function;
};

struct ResizeBilinearOptions: public BuiltinOptions {
  ResizeBilinearOptions()
    : BuiltinOptions(BuiltinOptionsType::ResizeBilinearOptions) {}

  bool align_corners;
};

struct CallOptions: public BuiltinOptions {
  CallOptions(): BuiltinOptions(BuiltinOptionsType::CallOptions) {}

  uint subgraph;
};

struct PadOptions: public BuiltinOptions {
  PadOptions(): BuiltinOptions(BuiltinOptionsType::PadOptions) {}
};

struct ReshapeOptions: public BuiltinOptions {
  ReshapeOptions(): BuiltinOptions(BuiltinOptionsType::ReshapeOptions) {}

  std::vector<int> new_shape;
};

struct SpaceToBatchNDOptions: public BuiltinOptions {
  SpaceToBatchNDOptions()
    : BuiltinOptions(BuiltinOptionsType::SpaceToBatchNDOptions) {}
};

struct BatchToSpaceNDOptions: public BuiltinOptions {
  BatchToSpaceNDOptions()
    : BuiltinOptions(BuiltinOptionsType::BatchToSpaceNDOptions) {}
};

struct SkipGramOptions: public BuiltinOptions {
  SkipGramOptions(): BuiltinOptions(BuiltinOptionsType::SkipGramOptions) {}

  int ngram_size;
  int max_skip_size;
  bool include_all_ngrams;
};

struct SpaceToDepthOptions: public BuiltinOptions {
  SpaceToDepthOptions()
    : BuiltinOptions(BuiltinOptionsType::SpaceToDepthOptions) {}

  int block_size;
};

struct SubOptions: public BuiltinOptions {
  SubOptions(): BuiltinOptions(BuiltinOptionsType::SubOptions) {}

  ActivationFunctionType fused_activation_function;
};

struct DivOptions: public BuiltinOptions {
  DivOptions(): BuiltinOptions(BuiltinOptionsType::DivOptions) {}

  ActivationFunctionType fused_activation_function;
};

struct TopKV2Options: public BuiltinOptions {
  TopKV2Options(): BuiltinOptions(BuiltinOptionsType::TopKV2Options) {}
};

enum class CombinerType : int8_t {
  SUM = 0,
  MEAN = 1,
  SQRTN = 2,
};

struct EmbeddingLookupSparseOptions: public BuiltinOptions {
  EmbeddingLookupSparseOptions()
    : BuiltinOptions(BuiltinOptionsType::EmbeddingLookupSparseOptions) {}

  CombinerType combiner;
};

struct GatherOptions: public BuiltinOptions {
  GatherOptions(): BuiltinOptions(BuiltinOptionsType::GatherOptions) {}

  int axis;
};

struct TransposeOptions: public BuiltinOptions {
  TransposeOptions(): BuiltinOptions(BuiltinOptionsType::TransposeOptions) {}
};

struct ExpOptions: public BuiltinOptions {
  ExpOptions(): BuiltinOptions(BuiltinOptionsType::ExpOptions) {}
};

struct ReducerOptions: public BuiltinOptions {
  ReducerOptions(): BuiltinOptions(BuiltinOptionsType::ReducerOptions) {}

  bool keep_dims;
};

struct SqueezeOptions: public BuiltinOptions {
  SqueezeOptions(): BuiltinOptions(BuiltinOptionsType::SqueezeOptions) {}

  std::vector<int> squeeze_dims;
};

struct SplitOptions: public BuiltinOptions {
  SplitOptions(): BuiltinOptions(BuiltinOptionsType::SplitOptions) {}

  int num_splits;
};

struct StridedSliceOptions: public BuiltinOptions {
  StridedSliceOptions()
      : BuiltinOptions(BuiltinOptionsType::StridedSliceOptions) {}

  int begin_mask;
  int end_mask;
  int ellipsis_mask;
  int new_axis_mask;
  int shrink_axis_mask;
};

struct LogSoftmaxOptions: public BuiltinOptions {
  LogSoftmaxOptions(): BuiltinOptions(BuiltinOptionsType::LogSoftmaxOptions) {}
};

struct CastOptions: public BuiltinOptions {
  CastOptions(): BuiltinOptions(BuiltinOptionsType::CastOptions) {}

  TensorType in_data_type;
  TensorType out_data_type;
};

struct DequantizeOptions: public BuiltinOptions {
  DequantizeOptions(): BuiltinOptions(BuiltinOptionsType::DequantizeOptions) {}
};

#ifdef NEWER_TENSORFLOW
struct MaximumMinimumOptions: public BuiltinOptions {
  MaximumMinimumOptions()
      : BuiltinOptions(BuiltinOptionsType::MaximumMinimumOptions) {}
};

struct ArgMaxOptions: public BuiltinOptions {
  ArgMaxOptions(): BuiltinOptions(BuiltinOptionsType::ArgMaxOptions) {}

  TensorType output_type;
};

struct LessOptions: public BuiltinOptions {
  LessOptions(): BuiltinOptions(BuiltinOptionsType::LessOptions) {}
};

struct NegOptions: public BuiltinOptions {
  NegOptions(): BuiltinOptions(BuiltinOptionsType::NegOptions) {}
};
#else
struct MaximumOptions: public BuiltinOptions {
  MaximumOptions(): BuiltinOptions(BuiltinOptionsType::MaximumOptions) {}
};
#endif

class Operator {
 public:
  Operator(int index, const OperatorCode& op_code,
      std::unique_ptr<BuiltinOptions> builtin_op,
      const std::string& builtin_op_str, std::vector<int>&& inputs,
      std::vector<int>&& outputs)
    : index_(index)
    , op_code_(op_code)
    , builtin_op_str_(builtin_op_str)
    , inputs_(std::move(inputs))
    , outputs_(std::move(outputs))
    , builtin_op_(std::move(builtin_op)) {}

  Operator(Operator&& op)
    : index_(op.index_)
    , op_code_(op.op_code_)
    , builtin_op_str_(std::move(op.builtin_op_str_))
    , inputs_(std::move(op.inputs_))
    , outputs_(std::move(op.outputs_))
    , builtin_op_(std::move(op.builtin_op_)) {}

  Operator(const Operator&) = delete;

  int index() const {
    return index_;
  }

  const std::string& builtin_op_str() const {
    return builtin_op_str_;
  }

  const std::vector<int>& inputs() const {
    return inputs_;
  }

  const std::vector<int>& outputs() const {
    return outputs_;
  }

  const BuiltinOptions& builtin_op() const {
    return *builtin_op_;
  }

  const OperatorCode& op_code() const {
    return op_code_;
  }

 private:
  int index_;
  const OperatorCode& op_code_;
  std::string builtin_op_str_;
  std::vector<int> inputs_;
  std::vector<int> outputs_;
  std::unique_ptr<BuiltinOptions> builtin_op_;
};

struct QuantizationParameters {
  std::vector<float> min;
  std::vector<float> max;
  std::vector<float> scale;
  std::vector<long> zero_point;
};

class Tensor {
 public:
  Tensor(std::vector<int>&& shape, TensorType tensor_type,
      const std::string& name, const Buffer& buffer, uint buffer_index,
      std::unique_ptr<QuantizationParameters> quantization)
    : shape_(std::move(shape))
    , tensor_type_(tensor_type)
    , name_(name)
    , buffer_(buffer)
    , buffer_index_(buffer_index)
    , quantization_(std::move(quantization)) {}

  Tensor(const Tensor& tensor) = delete;

  Tensor(Tensor&& tensor)
    : shape_(std::move(tensor.shape_))
    , tensor_type_(tensor.tensor_type_)
    , name_(std::move(tensor.name_))
    , buffer_(tensor.buffer_)
    , buffer_index_(tensor.buffer_index_)
    , quantization_(std::move(tensor.quantization_)) {}

  const std::string& name() const {
    return name_;
  }

  const std::vector<int>& shape() const {
    return shape_;
  }

  TensorType tensor_type() const {
    return tensor_type_;
  }

  const Buffer& buffer() const {
    return buffer_;
  }

  uint buffer_index() const {
    return buffer_index_;
  }

  bool HasQuantization() const {
    return bool(quantization_);
  }

  const QuantizationParameters& quantization() const {
    return *quantization_;
  }

 private:
  std::vector<int> shape_;
  TensorType tensor_type_;
  std::string name_;
  const Buffer& buffer_;
  uint buffer_index_;
  std::unique_ptr<QuantizationParameters> quantization_;
};

class Graph {
 public:
  Graph() = default;

  void SetInputs(std::vector<int>&& inputs) {
    inputs_ = std::move(inputs);
  }

  void SetOutputs(std::vector<int>&& outputs) {
    outputs_ = std::move(outputs);
  }

  void AddTensor(Tensor&& tensor) {
    tensors_.push_back(std::move(tensor));
  }

  void AddOperator(Operator&& op) {
    operators_.push_back(std::move(op));
  }

  const std::vector<Tensor>& Tensors() const {
    return tensors_;
  }

  std::vector<Tensor>& Tensors() {
    return tensors_;
  }

  const std::vector<Operator>& Operators() const {
    return operators_;
  }

  const std::vector<int>& Inputs() const {
    return inputs_;
  }

  const std::vector<int>& Outputs() const {
    return outputs_;
  }

 private:
  std::vector<Tensor> tensors_;
  std::vector<Operator> operators_;
  std::vector<int> inputs_;
  std::vector<int> outputs_;
};

class Model {
 public:
  Model(const std::string& file);

  const char* description();

  Graph& graph() {
    return graph_;
  }

  const std::vector<Buffer>& Buffers() const {
    return buffers_;
  }

 private:
  void PopulateGraph();

  void PopulateGraphInputs(const tflite::SubGraph* graph);

  void PopulateGraphOutputs(const tflite::SubGraph* graph);

  TensorType ConvertTensorType(tflite::TensorType type);

  void PopulateGraphTensors(const tflite::SubGraph* graph);

  void PopulateGraphOperators(const tflite::SubGraph* graph);

  void PopulateBuffers();

  BuiltinOperator ConvertOperatorCode(tflite::BuiltinOperator type);

  void PopulateOperatorsCode();

  std::unique_ptr<NoneOptions> MakeNoneOptions(const tflite::Operator* op);

  std::unique_ptr<Conv2DOptions> MakeConv2DOptions(
      const tflite::Operator* op);

  std::unique_ptr<Pool2DOptions> MakePool2DOptions(
      const tflite::Operator* op);

  std::unique_ptr<DepthwiseConv2DOptions> MakeDepthwiseConv2DOptions(
      const tflite::Operator* op);

  std::unique_ptr<ConcatEmbeddingsOptions> MakeConcatEmbeddingsOptions(
      const tflite::Operator* op);

  std::unique_ptr<LSHProjectionOptions> MakeLSHProjectionOptions(
      const tflite::Operator* op);

  std::unique_ptr<SVDFOptions> MakeSVDFOptions(const tflite::Operator* op);

  std::unique_ptr<RNNOptions> MakeRNNOptions(const tflite::Operator* op);

  std::unique_ptr<SequenceRNNOptions> MakeSequenceRNNOptions(
      const tflite::Operator* op);

  std::unique_ptr<FullyConnectedOptions> MakeFullyConnectedOptions(
      const tflite::Operator* op);

  std::unique_ptr<SoftmaxOptions> MakeSoftmaxOptions(
      const tflite::Operator* op);

  std::unique_ptr<ConcatenationOptions> MakeConcatenationOptions(
      const tflite::Operator* op);

  std::unique_ptr<BuiltinOptions> HandleBuiltinOptions(
      const tflite::Operator* op);

  std::unique_ptr<AddOptions> MakeAddOptions(const tflite::Operator* op);

  std::unique_ptr<MulOptions> MakeMulOptions(const tflite::Operator* op);

  std::unique_ptr<L2NormOptions> MakeL2NormOptions(
      const tflite::Operator* op);

  std::unique_ptr<LocalResponseNormalizationOptions>
  MakeLocalResponseNormalizationOptions(const tflite::Operator* op);

  std::unique_ptr<LSTMOptions> MakeLSTMOptions(const tflite::Operator* op);

  std::unique_ptr<ResizeBilinearOptions> MakeResizeBilinearOptions(
      const tflite::Operator* op);

  std::unique_ptr<CallOptions> MakeCallOptions(const tflite::Operator* op);

  std::unique_ptr<PadOptions> MakePadOptions(const tflite::Operator* op);

  std::unique_ptr<ReshapeOptions> MakeReshapeOptions(
      const tflite::Operator* op);

  std::unique_ptr<SpaceToBatchNDOptions> MakeSpaceToBatchNDOptions(
      const tflite::Operator* op);

  std::unique_ptr<SkipGramOptions> MakeSkipGramOptions(
      const tflite::Operator* op);

  std::unique_ptr<SpaceToDepthOptions> MakeSpaceToDepthOptions(
      const tflite::Operator* op);

  std::unique_ptr<BatchToSpaceNDOptions> MakeBatchToSpaceNDOptions(
      const tflite::Operator* op);

  std::unique_ptr<SubOptions> MakeSubOptions(const tflite::Operator* op);

  std::unique_ptr<DivOptions> MakeDivOptions(const tflite::Operator* op);

  std::unique_ptr<EmbeddingLookupSparseOptions>
  MakeEmbeddingLookupSparseOptions(const tflite::Operator* op);

  std::unique_ptr<GatherOptions> MakeGatherOptions(
      const tflite::Operator* op);

  std::unique_ptr<TransposeOptions> MakeTransposeOptions(
      const tflite::Operator* op);

  std::unique_ptr<ReducerOptions> MakeReducerOptions(const tflite::Operator* op);

  std::unique_ptr<SqueezeOptions> MakeSqueezeOptions(
      const tflite::Operator* op);

  std::unique_ptr<ExpOptions> MakeExpOptions(const tflite::Operator* op);

  std::unique_ptr<TopKV2Options> MakeTopKV2Options(
      const tflite::Operator* op);

  std::unique_ptr<SplitOptions> MakeSplitOptions(const tflite::Operator* op);

  std::unique_ptr<LogSoftmaxOptions> MakeLogSoftmaxOptions(
      const tflite::Operator* op);

  std::unique_ptr<CastOptions> MakeCastOptions(const tflite::Operator* op);

  std::unique_ptr<DequantizeOptions> MakeDequantizeOptions(const tflite::Operator* op);

#ifdef NEWER_TENSORFLOW
  std::unique_ptr<MaximumMinimumOptions> MakeMaximumMinimumOptions(
      const tflite::Operator* op);

  std::unique_ptr<ArgMaxOptions> MakeArgMaxOptions(const tflite::Operator* op);

  std::unique_ptr<LessOptions> MakeLessOptions(const tflite::Operator* op);

  std::unique_ptr<NegOptions> MakeNegOptions(const tflite::Operator* op);
#else
  std::unique_ptr<MaximumOptions> MakeMaximumOptions(
      const tflite::Operator* op);
#endif
  Padding ConvertPadding(tflite::Padding padding);

  ActivationFunctionType ConvertActivationFunction(
      tflite::ActivationFunctionType fn_activation_type);

  FlatBufferModel flat_buffers_;
  const tflite::Model *fb_model_;
  std::vector<Buffer> buffers_;
  std::vector<OperatorCode> operators_code_;
  Graph graph_;
};

template<class T, class Ptr>
std::vector<T> AssignVector(Ptr ptr) {
  std::vector<T> vec;

  if (!ptr) {
    return vec;
  }

  for (auto it = ptr->begin(); it != ptr->end(); ++it) {
    vec.push_back(*it);
  }

  return vec;
}

}

#endif

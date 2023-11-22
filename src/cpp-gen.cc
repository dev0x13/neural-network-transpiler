#include "cpp-gen.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <functional>
#include <boost/algorithm/string.hpp>

#include "exception.h"

namespace nnt {

std::string TensorsHeader::Generate() {
  const std::vector<Tensor>& tensors = model_.graph().Tensors();
  std::string str_buf;

  for (const auto& t : tensors) {
    if (!t.buffer().Data().empty()) {
      std::string str_vec;
      for (const auto& c : t.buffer().Data()) {
        str_buf += c;
      }
    }
  }

  return str_buf;
}

std::string TensorsHeader::Assembler() {
  return Generate();
}

std::string ModelGen::Generate() {
  std::string str_init = "Init";
  return str_init;
}

std::string ModelGen::TensorTypeStr(TensorType type) {
  switch (type) {
    case TensorType::FLOAT32:
      return "ANEURALNETWORKS_TENSOR_FLOAT32";
      break;

    case TensorType::INT32:
      return "ANEURALNETWORKS_TENSOR_INT32";
      break;

    case TensorType::UINT8:
      return "ANEURALNETWORKS_TENSOR_QUANT8_ASYMM";
      break;

    default:
      FATAL("Tensor type not valid for Android NNAPI")
  }
}

std::string ModelGen::TensorCppTypeStr(TensorType type) {
  switch (type) {
    case TensorType::FLOAT32:
      return "FLOAT16";
      break;

    case TensorType::INT32:
      return "int32_t";
      break;

    case TensorType::UINT8:
      return "char";
      break;

    default:
      FATAL("Tensor type not valid for Android NNAPI")
  }
}

std::string ModelGen::TensorDim(const std::vector<int>& dim) {
  std::string str_out = "{";

  for (const auto& e : dim) {
    str_out += std::to_string(e) + ",";
  }

  str_out = str_out.substr(0, str_out.length() - 1);
  str_out += "}";

  return str_out;
}

float ModelGen::TensorQuantizationScale(const QuantizationParameters& q) {
  if (q.scale.size() > 0) {
    return q.scale[0];
  } else {
    return 0.0f;
  }
}

int ModelGen::TensorQuantizationZeroPoint(
    const QuantizationParameters& q) {
  if (q.zero_point.size() > 0) {
    return q.zero_point[0];
  } else {
    return 0;
  }
}

std::string ModelGen::GenerateTensorType(const Tensor& tensor, int count) {
  std::stringstream ss;
  std::string dimensions = TensorDim(tensor.shape());

  ss << "uint32_t dimensions_" << count << "[] = " << dimensions << ";\n";
  ss << "ANeuralNetworksOperandType operand_type_" << count << " {\n";

  std::string str_tensor_type = TensorTypeStr(tensor.tensor_type());
  int dimension_count = tensor.shape().size();

  float scale;
  int zero_point;

  if (tensor.HasQuantization()) {
    scale = TensorQuantizationScale(tensor.quantization());
    zero_point = TensorQuantizationZeroPoint(tensor.quantization());
  } else {
    scale = 0.0f;
    zero_point = 0;
  }

  ss << "  .type = " << str_tensor_type << ",\n";
  ss << "  .dimensionCount = " << dimension_count << ",\n";
  ss << "  .dimensions = " << "dimensions_" << count << ",\n";

  ss.precision(7);
  if (scale == 0) {
    ss << "  .scale = 1.0f,\n";
  } else{
    ss << "  .scale = " << scale << "f,\n";
  }

  ss << "  .zeroPoint = " << zero_point << "\n";
  ss << "};\n\n";

  return ss.str();
}

std::string ModelGen::CheckStatus(const boost::format& msg) {
  std::stringstream ss;

  // nnapi always check the result of operation
  ss << "if (status != ANEURALNETWORKS_NO_ERROR) {\n";
  ss << "  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,\n";
  ss << "      \"" << boost::str(msg) << "\");\n";
  ss << "  return false;\n";
  ss << "}\n\n";

  return ss.str();
}

std::string ModelGen::GenerateTensorsCode() {
  Graph& graph = model_.graph();
  std::stringstream ss;

  int count = 0;
  for (const auto& tensor: graph.Tensors()) {
    // insert operand type
    ss << GenerateTensorType(tensor, count);

    // insert nnapi operand
    ss << "status = ANeuralNetworksModel_addOperand(model, ";
    ss << "&operand_type_" << count << ");\n";
    ss << CheckStatus(boost::format("ANeuralNetworksModel_addOperand failed"
        "for operand %1%")%count);

    size_t buf_size = tensor.buffer().Data().size();

    if (buf_size > 0) {
      // get tensor size
      ss << "tensor_size = " << buf_size << ";\n";

      // insert operand value
      ss << "status = ANeuralNetworksModel_setOperandValueFromMemory(model, ";
      ss << count << ", mem, offset, tensor_size);\n\n";
      ss << CheckStatus(boost::format(
          "ANeuralNetworksModel_setOperandValueFromMemory "
          "failed for operand %1%")%count);

      // calculates the offset
      ss << "offset += tensor_size;\n";
    }

    ++count;
  }

  count_operands_ = count;
  tensor_pos_ = graph.Tensors().size();

  return ss.str();
}

std::string ModelGen::GenerateOpInputs(const std::vector<int>& inputs,
    size_t num_params) {
  // inputs loop
  std::string str_in = "";

  // insert data params like conv filters params
  for (const auto& in_value : inputs) {
    str_in += " " + std::to_string(in_value) + ",";
  }

  // insert hiperparams like conv stride
  size_t tensor_start_pos = tensor_pos_;
  for (; tensor_pos_ < (tensor_start_pos + num_params); tensor_pos_++) {
    str_in += " " + std::to_string(tensor_pos_) + ",";
  }

  str_in = str_in.substr(0, str_in.length() - 1);
  return str_in;
}

std::string ModelGen::GenerateOpOutputs(const std::vector<int>& outputs) {
  // outputs loop
  std::string str_out = "";

  for (const auto& out_value : outputs) {
    str_out += " " + std::to_string(out_value) + ",";
  }

  str_out = str_out.substr(0, str_out.length() - 1);
  return str_out;
}

std::string ModelGen::OpTypeStr(BuiltinOperator op_type) {
  switch (op_type) {
    case BuiltinOperator::ADD:
      return "ANEURALNETWORKS_ADD";
      break;

    case BuiltinOperator::AVERAGE_POOL_2D:
      return "ANEURALNETWORKS_AVERAGE_POOL_2D";
      break;

    case BuiltinOperator::MAX_POOL_2D:
      return "ANEURALNETWORKS_MAX_POOL_2D";
      break;

    case BuiltinOperator::L2_POOL_2D:
      return "ANEURALNETWORKS_L2_POOL_2D";
      break;

    case BuiltinOperator::CONV_2D:
      return "ANEURALNETWORKS_CONV_2D";
      break;

    case BuiltinOperator::RELU:
      return "ANEURALNETWORKS_RELU";
      break;

    case BuiltinOperator::RELU6:
      return "ANEURALNETWORKS_RELU6";
      break;

    case BuiltinOperator::TANH:
      return "ANEURALNETWORKS_TANH";
      break;

    case BuiltinOperator::LOGISTIC:
      return "ANEURALNETWORKS_LOGISTIC";
      break;

    case BuiltinOperator::DEPTHWISE_CONV_2D:
      return "ANEURALNETWORKS_DEPTHWISE_CONV_2D";
      break;

    case BuiltinOperator::CONCATENATION:
      return "ANEURALNETWORKS_CONCATENATION";
      break;

    case BuiltinOperator::SOFTMAX:
      return "ANEURALNETWORKS_SOFTMAX";
      break;

    case BuiltinOperator::FULLY_CONNECTED:
      return "ANEURALNETWORKS_FULLY_CONNECTED";
      break;

    case BuiltinOperator::RESHAPE:
      return "ANEURALNETWORKS_RESHAPE";
      break;

    case BuiltinOperator::SPACE_TO_DEPTH:
      return "ANEURALNETWORKS_SPACE_TO_DEPTH";
      break;

    case BuiltinOperator::LSTM:
      return "ANEURALNETWORKS_LSTM";
      break;

    default:
      FATAL(boost::format("Not supported type on NNAPI"))
  }
}

std::string ModelGen::AddScalarInt32(int value) {
  std::stringstream ss;

  ss << "CHECK_ADD_SCALAR(AddScalarInt32(" << count_operands_ << ", "
     << value << "))\n";

  ++count_operands_;
  return ss.str();
}

std::string ModelGen::AddScalarFloat32(float value) {
  std::stringstream ss;

  ss << "CHECK_ADD_SCALAR(AddScalarFloat32(" << count_operands_ << ", "
     << value << "))\n";

  ++count_operands_;
  return ss.str();
}

std::tuple<size_t, std::string> ModelGen::OpParams(const Operator& op) {
  std::stringstream ss;
  size_t num_params = 0;

  auto check = [&op](BuiltinOptionsType type) {
    if (op.builtin_op().type != type) {
      FATAL(boost::format("Operator node type wrong"));
    }
  };

  switch (op.op_code().builtin_code) {
    case BuiltinOperator::ADD:
      ss << AddScalarInt32(0);
      num_params = 1;
      break;

    case BuiltinOperator::L2_POOL_2D:
    case BuiltinOperator::MAX_POOL_2D:
    case BuiltinOperator::AVERAGE_POOL_2D: {
      check(BuiltinOptionsType::Pool2DOptions);
      const Pool2DOptions& pool_options = static_cast<const Pool2DOptions&>(
          op.builtin_op());

      ss << AddScalarInt32(static_cast<int>(pool_options.padding));
      ss << AddScalarInt32(pool_options.stride_w);
      ss << AddScalarInt32(pool_options.stride_h);
      ss << AddScalarInt32(pool_options.filter_width);
      ss << AddScalarInt32(pool_options.filter_height);
      ss << AddScalarInt32(static_cast<int>(
          pool_options.fused_activation_function));
      num_params = 6;
      break;
    }

    case BuiltinOperator::CONV_2D: {
      check(BuiltinOptionsType::Conv2DOptions);
      const Conv2DOptions& conv_options = static_cast<const Conv2DOptions&>(
          op.builtin_op());

      ss << AddScalarInt32(static_cast<int>(conv_options.padding));
      ss << AddScalarInt32(conv_options.stride_w);
      ss << AddScalarInt32(conv_options.stride_h);
      ss << AddScalarInt32(static_cast<int>(
          conv_options.fused_activation_function));
      num_params = 4;
      break;
    }

    case BuiltinOperator::DEPTHWISE_CONV_2D: {
      check(BuiltinOptionsType::DepthwiseConv2DOptions);
      const DepthwiseConv2DOptions& dept_conv_options =
          static_cast<const DepthwiseConv2DOptions&>(op.builtin_op());

      ss << AddScalarInt32(static_cast<int>(dept_conv_options.padding));
      ss << AddScalarInt32(dept_conv_options.stride_w);
      ss << AddScalarInt32(dept_conv_options.stride_h);
      ss << AddScalarInt32(dept_conv_options.depth_multiplier);
      ss << AddScalarInt32(static_cast<int>(
          dept_conv_options.fused_activation_function));
      num_params = 5;
      break;
    }

    case BuiltinOperator::FULLY_CONNECTED: {
      check(BuiltinOptionsType::FullyConnectedOptions);
      const FullyConnectedOptions& fully_con_options =
          static_cast<const FullyConnectedOptions&>(op.builtin_op());

      ss << AddScalarInt32(static_cast<int>(
          fully_con_options.fused_activation_function));
      num_params = 1;
      break;
    }

    case BuiltinOperator::CONCATENATION: {
      check(BuiltinOptionsType::ConcatenationOptions);
      const ConcatenationOptions& concat_options =
          static_cast<const ConcatenationOptions&>(op.builtin_op());

      ss << AddScalarInt32(concat_options.axis);
      ss << AddScalarInt32(static_cast<int>(
          concat_options.fused_activation_function));
      num_params = 2;
      break;
    }

    case BuiltinOperator::SOFTMAX: {
      check(BuiltinOptionsType::SoftmaxOptions);
      const SoftmaxOptions& softmax_options =
          static_cast<const SoftmaxOptions&>(op.builtin_op());

      ss << AddScalarFloat32(softmax_options.beta);
      num_params = 1;
      break;
    }

    case BuiltinOperator::SPACE_TO_DEPTH: {
      check(BuiltinOptionsType::SpaceToDepthOptions);
      const SpaceToDepthOptions& space2depth_options =
          static_cast<const SpaceToDepthOptions&>(op.builtin_op());

      ss << AddScalarInt32(space2depth_options.block_size);
      num_params = 1;
      break;
    }

    case BuiltinOperator::LSTM: {
      check(BuiltinOptionsType::LSTMOptions);
      // TODO: Check better on TfLite how lstm parametes is filled
      const LSTMOptions& lstm_options = static_cast<const LSTMOptions&>(
          op.builtin_op());

      ss << AddScalarInt32(static_cast<int>(
          lstm_options.fused_activation_function));
      ss << AddScalarInt32(lstm_options.cell_clip);
      ss << AddScalarInt32(lstm_options.proj_clip);
      num_params = 3;
      break;
    }

    default:
      num_params = 0;
  }

  return std::tuple<size_t, std::string>(num_params, ss.str());
}

std::string ModelGen::GenerateOpCode() {
  Graph& graph = model_.graph();
  std::stringstream ss;

  int count = 0;
  for (const auto& op: graph.Operators()) {
    size_t num_params;
    std::string str_params;
    std::tie(num_params, str_params) = OpParams(op);
    ss << str_params << "\n";
    ss << "uint32_t input_operands_" << count << "[] = { ";
    ss << GenerateOpInputs(op.inputs(), num_params) << " };\n";

    ss << "uint32_t output_operands_" << count << "[] = {";
    ss << GenerateOpOutputs(op.outputs()) << " };\n\n";

    ss << "status = ANeuralNetworksModel_addOperation(model, ";
    ss << OpTypeStr(op.op_code().builtin_code) << ", sizeof(input_operands_" ;
    ss << count <<") / sizeof(uint32_t), input_operands_" << count << ", ";
    ss << "sizeof(output_operands_" << count << ") / sizeof(uint32_t), ";
    ss << "output_operands_" << count << ");\n";

    ss << CheckStatus(boost::format(
        "ANeuralNetworksModel_addOperation failed for operation %1%")%count);

    ++count;
  }

  return ss.str();
}

std::string ModelGen::GenerateInputsAndOutputs() {
  Graph& graph = model_.graph();
  std::stringstream ss;

  size_t num_inputs = graph.Inputs().size();;
  ss << "uint32_t input_indexes[" << num_inputs << "] = {";

  std::string str_input;
  for (int i : graph.Inputs()) {
    str_input += " " + std::to_string(i) + ",";
  }

  str_input = str_input.substr(0, str_input.length() - 1);
  ss << str_input << " };\n";

  size_t num_outputs = graph.Outputs().size();
  ss << "uint32_t output_indexes[" << num_outputs << "] = {";

  std::string str_output;
  for (int i : graph.Outputs()) {
    str_output += " " + std::to_string(i) + ",";
  }

  str_output = str_output.substr(0, str_output.length() - 1);
  ss << str_output << " };\n";

  ss << "ANeuralNetworksModel_identifyInputsAndOutputs(model, "
     << num_inputs << ", input_indexes, " << num_outputs
     << ", output_indexes);\n";

  return ss.str();
}

std::string TensorType(const Tensor& tensor) {
  switch (tensor.tensor_type()) {
    case TensorType::FLOAT32:
      return "float32_t";
      break;

    case TensorType::INT32:
      return "int32_t";
      break;

    case TensorType::UINT8:
      return "int8_t";
      break;

    default:
      FATAL("Tensor type not valid for Android NNAPI")
  }
}

int ModelGen::TensorSize(const Tensor& tensor) {
  int size = 1;
  for (int shape_i : tensor.shape()) {
    size *= shape_i;

    if (tensor.tensor_type() == TensorType::FLOAT32 ||
        tensor.tensor_type() == TensorType::INT32) {
      size *= 4;
    }
  }

  return size;
}

std::string ModelGen::GenerateInputFunctions() {
  Graph& graph = model_.graph();
  std::string str_input;

  str_input += "bool SetInput(const int8_t *buffer) {\n";

  int start = 0;
  int cnt = 0;
  for (int i : graph.Inputs()) {
    const Tensor& tensor = graph.Tensors()[i];
    int size = TensorSize(tensor);

    str_input += "  int status = ANeuralNetworksExecution_setInput(run, " +
        std::to_string(cnt++) + ", NULL, &buffer[" + std::to_string(start) +
        "], " + std::to_string(size) + ");\n";

    str_input += CheckStatus(boost::format(
        "ANeuralNetworksExecution_setInput failed"));

    start += size;
  }

  str_input += "  return true;\n}\n\n";

  return str_input;
}

std::string ModelGen::GenerateOutputFunctions() {
  Graph& graph = model_.graph();
  std::string str_output;

  str_output += "bool SetOutput(int8_t *buffer) {\n";

  int start = 0;
  int cnt = 0;
  for (int i : graph.Outputs()) {
    const Tensor& tensor = graph.Tensors()[i];
    int size = TensorSize(tensor);

    str_output += "  int status = ANeuralNetworksExecution_setOutput(run, " +
        std::to_string(cnt++) + ", NULL, &buffer[" + std::to_string(start) +
        "], " + std::to_string(size) + ");\n";

    str_output += CheckStatus(boost::format(
        "ANeuralNetworksExecution_setOutput failed"));

    start += size;
  }

  str_output += "  return true;\n}\n\n";

  return str_output;
}

std::string ModelGen::GenerateHeader() {
  std::string str =
#include "templates/top_nn_cc.tpl"
  ;
  return str;
}

std::string ModelGen::Assembler() {
  std::string code;
  code = GenerateHeader();
  code += GenerateTensorsCode();
  code += GenerateOpCode();
  code += GenerateInputsAndOutputs();

  code += R"(
    status = ANeuralNetworksModel_finish(model);
    if (status != ANEURALNETWORKS_NO_ERROR) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
                            "ANeuralNetworksModel_finish failed");
        return false;
    }
  )";

  // close model function
  code += "return true;\n}\n\n";

  code += GenerateInputFunctions();
  code += GenerateOutputFunctions();

  // close namespace
  code += "\n}\n\n";

  return code;
}

std::string ModelGenHeader::GenerateHeader() {
  std::string str =
  #include "templates/top_nn_h.tpl"
  ;
  return str;
}

std::string ModelGenHeader::Assembler() {
  std::string str = GenerateHeader();
  str += "}";

  return str;
}

template<class Fn>
int ModelGenJni::TotalSize(Fn&& fn) {
  Graph& graph = model_.graph();
  int total_size = 0;

  for (int i : fn()) {
    int size = 1;
    const Tensor& tensor = graph.Tensors()[i];

    for (int shape_i : tensor.shape()) {
      size *= shape_i;

      if (tensor.tensor_type() == TensorType::FLOAT32 ||
          tensor.tensor_type() == TensorType::INT32) {
        size *= 4;
      }
    }
    total_size += size;
  }

  return total_size;
}

std::string ModelGenJni::GenerateJni() {
  std::string str =
#include "templates/jni.tpl"
  ;

  Graph& graph = model_.graph();

  auto fn_in = std::bind(&Graph::Inputs, &graph);
  int total_input_size = TotalSize(fn_in);

  auto fn_out = std::bind(&Graph::Outputs, &graph);
  int total_output_size = TotalSize(fn_out);

  boost::replace_all(str, "@TOTAL_INPUT_SIZE",
      std::to_string(total_input_size));
  boost::replace_all(str, "@TOTAL_OUTPUT_SIZE",
      std::to_string(total_output_size));
  boost::replace_all(str, "@JAVA_PACKAGE", java_package_);

  return str;
}

std::string ModelGenJni::Assembler() {
  boost::replace_all(java_package_, ".", "_");
  std::string str = GenerateJni();
  return str;
}

void CppGen::GenFiles(const boost::filesystem::path& path,
    const std::string& java_path) {
  GenTensorsDataFile(path);
  GenCppFile(path);
  GenHFile(path);
  GenJniFile(path, java_path);
}

void CppGen::GenTensorsDataFile(const boost::filesystem::path& path) {
  const boost::filesystem::path& fname("weights_biases.bin");
  std::string str_path = (path / fname).string();
  std::ofstream tensors_file(str_path,
      std::ofstream::out | std::ofstream::binary);

  if (!tensors_file.is_open()) {
    FATAL(boost::format("Fail on create weights_biases.bin file on: %1%")
        %str_path)
  }

  TensorsHeader tensor_header(model_);
  std::string buf = tensor_header.Assembler();
  tensors_file.write(buf.c_str(), buf.length());
  tensors_file.close();

  std::cout << "File: " << str_path << " generated\n";
}

void CppGen::GenCppFile(const boost::filesystem::path& path) {
  const boost::filesystem::path& fname("nn.cc");
  std::string str_path = (path / fname).string();
  std::ofstream cc_file(str_path, std::ofstream::out | std::ofstream::binary);

  if (!cc_file.is_open()) {
    FATAL("Fail on create nn.cc file")
  }

  ModelGen model(model_);
  std::string code = model.Assembler();
  cc_file.write(code.c_str(), code.length());
  cc_file.close();

  std::cout << "File: " << str_path << " generated\n";
}

void CppGen::GenHFile(const boost::filesystem::path& path) {
  const boost::filesystem::path& fname("nn.h");
  std::string str_path = (path / fname).string();

  std::ofstream cc_file(str_path, std::ofstream::out | std::ofstream::binary);

  if (!cc_file.is_open()) {
    FATAL("Fail on create nn.h file")
  }

  ModelGenHeader model(model_);
  std::string code = model.Assembler();
  cc_file.write(code.c_str(), code.length());
  cc_file.close();

  std::cout << "File: " << str_path << " generated\n";
}

void CppGen::GenJniFile(const boost::filesystem::path& path,
    const std::string& java_package) {
  const boost::filesystem::path& fname("jni.cc");
  std::string str_path = (path / fname).string();
  std::ofstream jni_file(str_path, std::ofstream::out | std::ofstream::binary);

  if (!jni_file.is_open()) {
    FATAL("Fail on create nn.h file")
  }

  ModelGenJni model(model_, java_package);
  std::string code = model.Assembler();
  jni_file.write(code.c_str(), code.length());
  jni_file.close();

  std::cout << "File: " << str_path << " generated\n";
}

}

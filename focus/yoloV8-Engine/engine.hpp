#include <opencv2/opencv.hpp>
#include "random"
#include <iostream>
#include <memory>

#include "onnxruntime_cxx_api.h"

class Engine {
	public:
		std::tuple<int, int, cv::Mat> runInference(cv::Mat img);
};

class ONNXInferencer {
public:
    ONNXInferencer(const std::wstring& model_path) {
        // Initialize ONNX Runtime session with logging
        env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "ONNXInferencer");
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);

        session = std::make_unique<Ort::Session>(*env, model_path.c_str(), session_options);

        // Get input and output names
        size_t num_input_nodes = session->GetInputCount();
        input_node_names.reserve(num_input_nodes);
        input_node_dims.reserve(num_input_nodes);

        Ort::AllocatorWithDefaultOptions allocator;

        for (size_t i = 0; i < num_input_nodes; ++i) {
            auto input_name_ptr = session->GetInputNameAllocated(i, allocator);
            input_node_names.push_back(input_name_ptr.get());

            Ort::TypeInfo type_info = session->GetInputTypeInfo(i);
            auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

            input_node_dims.push_back(tensor_info.GetShape());
        }

        size_t num_output_nodes = session->GetOutputCount();
        output_node_names.reserve(num_output_nodes);

        for (size_t i = 0; i < num_output_nodes; ++i) {
            auto output_name_ptr = session->GetOutputNameAllocated(i, allocator);
            output_node_names.push_back(output_name_ptr.get());
        }
    }

    std::vector<float> runInference(cv::Mat& input_image) {
        try {
            // Convert image to 3 channels if it has 4 channels
            if (input_image.channels() == 4) {
                cv::cvtColor(input_image, input_image, cv::COLOR_BGRA2BGR);
            }

            // Resize the image to the model's expected input size
            int model_input_height = input_node_dims[0][2];
            int model_input_width = input_node_dims[0][3];
            cv::resize(input_image, input_image, cv::Size(model_input_width, model_input_height));

            // Convert image to float and normalize (assuming the model expects [0, 1] range)
            input_image.convertTo(input_image, CV_32F, 1.0 / 255.0);

            // Prepare input tensor
            std::vector<int64_t> input_tensor_shape{ 1, input_image.channels(), input_image.rows, input_image.cols };
            size_t input_tensor_size = input_image.total() * input_image.channels();
            std::vector<float> input_tensor_values(input_tensor_size);

            std::memcpy(input_tensor_values.data(), input_image.data, input_tensor_size * sizeof(float));

            std::vector<const char*> input_names(input_node_names.size());
            std::vector<const char*> output_names(output_node_names.size());

            for (size_t i = 0; i < input_node_names.size(); ++i) {
                input_names[i] = input_node_names[i].c_str();
            }

            for (size_t i = 0; i < output_node_names.size(); ++i) {
                output_names[i] = output_node_names[i].c_str();
            }

            Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
                Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault),
                input_tensor_values.data(), input_tensor_size, input_tensor_shape.data(), input_tensor_shape.size()
            );

            auto output_tensors = session->Run(Ort::RunOptions{ nullptr },
                input_names.data(), &input_tensor, 1,
                output_names.data(), output_names.size());

            std::vector<float> output_tensor_values;
            for (auto& tensor : output_tensors) {
                float* float_array = tensor.GetTensorMutableData<float>();
                output_tensor_values.insert(output_tensor_values.end(), float_array, float_array + tensor.GetTensorTypeAndShapeInfo().GetElementCount());
            }

            return output_tensor_values;
        }
        catch (const Ort::Exception& e) {
            std::cerr << "ONNX Runtime exception: " << e.what() << std::endl;
            return {};
        }
        catch (const std::exception& e) {
            std::cerr << "Standard exception: " << e.what() << std::endl;
            return {};
        }
        catch (...) {
            std::cerr << "Unknown exception occurred!" << std::endl;
            return {};
        }
    }

private:
    std::unique_ptr<Ort::Env> env;
    std::unique_ptr<Ort::Session> session;
    std::vector<std::string> input_node_names;
    std::vector<std::vector<int64_t>> input_node_dims;
    std::vector<std::string> output_node_names;
};

class ONNXInferencer2 {
public:
    ONNXInferencer2(const std::wstring& model_path, bool use_cuda = false) {
        // Initialize ONNX Runtime environment
        env_ = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "ONNXInferencer");

        // Initialize session options
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);

        //if (use_cuda) {
        //    // Use CUDA for GPU inference
        //    Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_CUDA(session_options, 0));
        //}
        //else {
        //    // Use CPU for inference
        //    Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_CPU(session_options, 1));
        //}

        // Create the ONNX Runtime session
        session_ = std::make_unique<Ort::Session>(*env_, model_path.c_str(), session_options);

        // Get input and output node names
        Ort::AllocatorWithDefaultOptions allocator;
        input_name_ = session_->GetInputNameAllocated(0, allocator).release();
        output_name_ = session_->GetOutputNameAllocated(0, allocator).release();

        // Get the input tensor shape
        auto input_type_info = session_->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo();
        input_dims_ = input_type_info.GetShape();
    }

    ~ONNXInferencer2() {
        // Clean up allocated memory for input and output names
        Ort::AllocatorWithDefaultOptions allocator;
        allocator.Free(input_name_);
        allocator.Free(output_name_);
    }

    std::vector<float> infer(const cv::Mat& input_frame) {
        cv::Mat resized_frame;

        // Check if the input frame has 4 channels and convert it to 3 channels if necessary
        if (input_frame.channels() == 4) {
            cv::cvtColor(input_frame, resized_frame, cv::COLOR_BGRA2BGR);
        }
        else if (input_frame.channels() == 1) {
            cv::cvtColor(input_frame, resized_frame, cv::COLOR_GRAY2BGR);
        }
        else {
            resized_frame = input_frame;
        }

        // Ensure the input frame is resized to the model's expected input dimensions
        cv::resize(resized_frame, resized_frame, cv::Size(input_dims_[3], input_dims_[2]));

        // Convert the input frame to float and normalize
        cv::Mat input_blob;
        resized_frame.convertTo(input_blob, CV_32F, 1.0 / 255.0);

        // Prepare input tensor values
        std::vector<float> input_tensor_values(input_blob.total() * input_blob.channels());
        std::vector<cv::Mat> input_channels(input_blob.channels());
        cv::split(input_blob, input_channels);

        // Flatten the input blob
        for (int i = 0; i < input_blob.channels(); ++i) {
            std::memcpy(input_tensor_values.data() + i * input_blob.total(), input_channels[i].data, input_blob.total() * sizeof(float));
        }

        // Create input tensor from the input frame
        std::vector<int64_t> input_shape{ 1, input_blob.channels(), input_blob.rows, input_blob.cols };
        size_t input_tensor_size = input_tensor_values.size();
        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, input_tensor_values.data(), input_tensor_size, input_shape.data(), input_shape.size());

        // Run the inference
        auto output_tensors = session_->Run(Ort::RunOptions{ nullptr }, &input_name_, &input_tensor, 1, &output_name_, 1);

        // Extract the output
        float* output_data = output_tensors.front().GetTensorMutableData<float>();
        std::vector<int64_t> output_shape = output_tensors.front().GetTensorTypeAndShapeInfo().GetShape();
        size_t output_tensor_size = 1;
        for (auto dim : output_shape) output_tensor_size *= dim;

        return std::vector<float>(output_data, output_data + output_tensor_size);
    }

private:
    std::unique_ptr<Ort::Env> env_;
    std::unique_ptr<Ort::Session> session_;
    char* input_name_;
    char* output_name_;
    std::vector<int64_t> input_dims_;
};
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>


/**
 * \brief padding value when letterbox changes image size ratio
 */
const int& DEFAULT_LETTERBOX_PAD_VALUE = 114;


/**
 * @brief Resizes an image to fit a new shape with padding (letterboxing) while maintaining aspect ratio.
 *
 * @param image Input image to be resized and padded.
 * @param outImage Output image after resizing and padding.
 * @param newShape New shape for the output image.
 * @param color Padding color, default is gray if not specified.
 * @param auto_ If true, adjusts padding to be a multiple of the stride.
 * @param scaleFill If true, scales the image to fill the new shape completely without padding.
 * @param scaleUp If false, prevents the image from being scaled up beyond its original size.
 * @param stride Stride value used when auto_ is true.
 */
void letterbox(const cv::Mat& image,
    cv::Mat& outImage,
    const cv::Size& newShape,
    cv::Scalar_<double> color,
    bool auto_,
    bool scaleFill,
    bool scaleUp, int stride
) {
    // Get original image size
    cv::Size shape = image.size();

    // Calculate the scaling ratio to fit the new shape
    float r = std::min(static_cast<float>(newShape.height) / static_cast<float>(shape.height),
        static_cast<float>(newShape.width) / static_cast<float>(shape.width));

    // If scaling up is not allowed, cap the ratio at 1.0
    if (!scaleUp)
        r = std::min(r, 1.0f);

    // Calculate new size after scaling
    float ratio[2]{ r, r };
    int newUnpad[2]{ static_cast<int>(std::round(static_cast<float>(shape.width) * r)),
                     static_cast<int>(std::round(static_cast<float>(shape.height) * r)) };

    // Calculate padding to be added to width and height
    auto dw = static_cast<float>(newShape.width - newUnpad[0]);
    auto dh = static_cast<float>(newShape.height - newUnpad[1]);

    // Adjust padding for stride if auto_ is true
    if (auto_)
    {
        dw = static_cast<float>((static_cast<int>(dw) % stride));
        dh = static_cast<float>((static_cast<int>(dh) % stride));
    }
    // If scaleFill is true, scale the image to fill the new shape without padding
    else if (scaleFill)
    {
        dw = 0.0f;
        dh = 0.0f;
        newUnpad[0] = newShape.width;
        newUnpad[1] = newShape.height;
        ratio[0] = static_cast<float>(newShape.width) / static_cast<float>(shape.width);
        ratio[1] = static_cast<float>(newShape.height) / static_cast<float>(shape.height);
    }

    // Divide padding equally for both sides
    dw /= 2.0f;
    dh /= 2.0f;

    // Resize the image if necessary
    if (shape.width != newUnpad[0] || shape.height != newUnpad[1])
    {
        cv::resize(image, outImage, cv::Size(newUnpad[0], newUnpad[1]));
    }
    else
    {
        outImage = image.clone();
    }

    // Calculate top, bottom, left, and right padding
    int top = static_cast<int>(std::round(dh - 0.1f));
    int bottom = static_cast<int>(std::round(dh + 0.1f));
    int left = static_cast<int>(std::round(dw - 0.1f));
    int right = static_cast<int>(std::round(dw + 0.1f));

    // Set default padding color if none is specified
    if (color == cv::Scalar()) {
        color = cv::Scalar(DEFAULT_LETTERBOX_PAD_VALUE, DEFAULT_LETTERBOX_PAD_VALUE, DEFAULT_LETTERBOX_PAD_VALUE);
    }

    // Add padding to the resized image
    cv::copyMakeBorder(outImage, outImage, top, bottom, left, right, cv::BORDER_CONSTANT, color);
}

cv::Mat scale_image(const cv::Mat& resized_mask, const cv::Size& im0_shape, const std::pair<float, cv::Point2f>& ratio_pad) {
    cv::Size im1_shape = resized_mask.size();

    // Check if resizing is needed
    if (im1_shape == im0_shape) {
        return resized_mask.clone();
    }

    float gain, pad_x, pad_y;

    if (ratio_pad.first < 0.0f) {
        gain = std::min(static_cast<float>(im1_shape.height) / static_cast<float>(im0_shape.height),
            static_cast<float>(im1_shape.width) / static_cast<float>(im0_shape.width));
        pad_x = (im1_shape.width - im0_shape.width * gain) / 2.0f;
        pad_y = (im1_shape.height - im0_shape.height * gain) / 2.0f;
    }
    else {
        gain = ratio_pad.first;
        pad_x = ratio_pad.second.x;
        pad_y = ratio_pad.second.y;
    }

    int top = static_cast<int>(pad_y);
    int left = static_cast<int>(pad_x);
    int bottom = static_cast<int>(im1_shape.height - pad_y);
    int right = static_cast<int>(im1_shape.width - pad_x);

    // Clip and resize the mask
    cv::Rect clipped_rect(left, top, right - left, bottom - top);
    cv::Mat clipped_mask = resized_mask(clipped_rect);
    cv::Mat scaled_mask;
    cv::resize(clipped_mask, scaled_mask, im0_shape);

    return scaled_mask;
}


void scale_image2(cv::Mat& scaled_mask, const cv::Mat& resized_mask, const cv::Size& im0_shape, const std::pair<float, cv::Point2f>& ratio_pad) {
    cv::Size im1_shape = resized_mask.size();

    // Check if resizing is needed
    if (im1_shape == im0_shape) {
        scaled_mask = resized_mask.clone();
        return;
    }

    float gain, pad_x, pad_y;

    if (ratio_pad.first < 0.0f) {
        gain = std::min(static_cast<float>(im1_shape.height) / static_cast<float>(im0_shape.height),
                        static_cast<float>(im1_shape.width) / static_cast<float>(im0_shape.width));
        pad_x = (im1_shape.width - im0_shape.width * gain) / 2.0f;
        pad_y = (im1_shape.height - im0_shape.height * gain) / 2.0f;
    }
    else {
        gain = ratio_pad.first;
        pad_x = ratio_pad.second.x;
        pad_y = ratio_pad.second.y;
    }

    int top = static_cast<int>(pad_y);
    int left = static_cast<int>(pad_x);
    int bottom = static_cast<int>(im1_shape.height - pad_y);
    int right = static_cast<int>(im1_shape.width - pad_x);

    // Clip and resize the mask
    cv::Rect clipped_rect(left, top, right - left, bottom - top);
    cv::Mat clipped_mask = resized_mask(clipped_rect);
    cv::resize(clipped_mask, scaled_mask, im0_shape);
}

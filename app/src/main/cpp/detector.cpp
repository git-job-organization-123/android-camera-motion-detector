#include <opencv2/core.hpp> // OpenCV core
#include <opencv2/imgproc.hpp> // OpenCV COLOR_
#include <opencv2/features2d.hpp> // OpenCV fast feature detector

using namespace cv;

// NV21 image from Android device
cv::Mat grayImage;

// Motion or edge image
cv::Mat processedImage;

// Fast feature detector
cv::Ptr<cv::Feature2D> featureDetector;

// Detected points or motion points (red squares)
std::vector<cv::KeyPoint> keypoints;

void dilate(cv::Mat* image, int x, int y) {
  cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
  cv::dilate(*image, *image, kernel);
}

void detectMotion(cv::Mat* motionImage = nullptr) {
  static cv::Mat prevGrayImage;

  // Check if the previous frame is invalid
  if (prevGrayImage.empty()) {
    // Initialize the previous frame and points
    prevGrayImage = grayImage;
    return;
  }

  // Declare two frames
  Mat frame1, frame2;

  // Convert frames to grayscale
  cv::cvtColor(grayImage, frame1, COLOR_BGR2GRAY);
  cv::cvtColor(prevGrayImage, frame2, COLOR_BGR2GRAY);

  // Declare the difference image
  cv::Mat diff;

  // Compute the absolute difference between frames
  cv::absdiff(frame2, frame1, diff);
                             
  if (motionImage != nullptr && (previewMode == PreviewMode::DETECT_PREVIEW_MOTION_RED
                             ||  previewMode == PreviewMode::DETECT_PREVIEW_MOTION_GREEN
                             ||  previewMode == PreviewMode::DETECT_PREVIEW_MOTION_BLUE)) {
    cv::Mat coloredDiff;
    cv::cvtColor(diff, coloredDiff, COLOR_GRAY2BGR);
    std::vector<cv::Mat> channels;
    cv::split(coloredDiff, channels);

    if (previewMode == PreviewMode::DETECT_PREVIEW_MOTION_RED) {
      channels[2] = channels[2] * 3.5;
    }
    else if (previewMode == PreviewMode::DETECT_PREVIEW_MOTION_GREEN) {
      channels[1] = channels[1] * 3.5;
    }
    else if (previewMode == PreviewMode::DETECT_PREVIEW_MOTION_BLUE) {
      channels[0] = channels[0] * 3.5;
    }

    cv::Mat coloredImage;
    cv::merge(channels, coloredImage);
    diff = coloredImage;
  }

  if (previewMode != PreviewMode::DETECT_PREVIEW_MOTION_WHITE) {
    cv::threshold(diff, diff, 50, 255, THRESH_BINARY);
  }

  if (motionImage != nullptr) { // Get motion image as Mat
    if (previewMode == PreviewMode::DETECT_PREVIEW_MOTION_GRAYSCALE) {
      // Make edges thicker
      dilate(&diff, 50, 50);

      grayImage.copyTo(diff, diff); // Grayscale motion image
    }
    else if (previewMode == DETECT_PREVIEW_MOTION_WHITE_WITH_BACKGROUND) {
      cv::cvtColor(diff, diff, cv::COLOR_BGR2RGB);

      // Copy motion to preview image
      cv::addWeighted(diff, 0.5, grayImage, 0.5, 0, diff);
    }
    
    *motionImage = diff;
  }
  else { // Get motion keypoints
    // Find contours in the difference image
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(diff, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    keypoints.clear();

    // Add the motion vectors to the motion list
    for (int i = 0; i < contours.size(); i++) {
      for (int j = 0; j < contours[i].size(); j++) {
        cv::Point2f point = cv::Point2f(contours[i][j].x, contours[i][j].y);
        cv::KeyPoint motionKeyPoint(point, 1.0f, 0.0f, 0.0f, 0, -1);
        keypoints.push_back(motionKeyPoint);
      }
    }
  }

  // Update the previous frame
  prevGrayImage = grayImage;
}

void detect() {
  if (previewMode == PreviewMode::DETECT_MOTION_LINES) { // Motion red lines
    detectMotion();
  }
  else if (previewMode == PreviewMode::DETECT_PREVIEW_MOTION_WHITE
        || previewMode == PreviewMode::DETECT_PREVIEW_MOTION_RED
        || previewMode == PreviewMode::DETECT_PREVIEW_MOTION_GREEN
        || previewMode == PreviewMode::DETECT_PREVIEW_MOTION_BLUE
        || previewMode == PreviewMode::DETECT_PREVIEW_MOTION_GRAYSCALE
        || previewMode == PreviewMode::DETECT_PREVIEW_MOTION_WHITE_WITH_BACKGROUND) { // Preview motion
    detectMotion(&processedImage);
    cv::cvtColor(processedImage, processedImage, cv::COLOR_BGR2RGB);
  }
}

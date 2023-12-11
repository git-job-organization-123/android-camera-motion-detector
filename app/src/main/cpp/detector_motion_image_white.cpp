class Detector_Motion_Image_White : public Detector_Motion_Image {
  void processImage(cv::Mat &image) override {
    // Update processed image
    processedImage = image;
  }
};

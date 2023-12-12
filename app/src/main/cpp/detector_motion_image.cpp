class Detector_Motion_Image : public Detector_Motion {
public:
  void detect() override {
    // Declare two frames
    Mat frame1, frame2;

    // Convert current and previous frame to grayscale
    cv::cvtColor(currentImage, frame1, COLOR_BGR2GRAY);
    cv::cvtColor(prevImage, frame2, COLOR_BGR2GRAY);

    // Declare the difference image
    cv::Mat diff;

    // Compute the absolute difference between frames
    cv::absdiff(frame2, frame1, diff);

    // Process difference image
    processImage(diff);

    // Convert processed image to RGB
    cv::cvtColor(processedImage, processedImage, cv::COLOR_BGR2RGB);

    // Update the previous frame
    prevImage = currentImage;
  }

  void updateRendererData() override {
    // Update renderer image
    renderer->setImageData(processedImage.data);
  }

  virtual void processImage(cv::Mat &image) {}
};

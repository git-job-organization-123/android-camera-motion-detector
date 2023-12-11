class Detector_Motion_Image_Background : public Detector_Motion_Image {
public:
  void processImage(cv::Mat &image) override {
    // Motion threshold  
    cv::threshold(image, image, 50, 255, THRESH_BINARY);

    // Convert image to grayscale
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

    // Copy image to current image
    cv::addWeighted(image, 0.5, currentImage, 0.5, 0, image);
    
    // Update processed image
    processedImage = image;
  }
};

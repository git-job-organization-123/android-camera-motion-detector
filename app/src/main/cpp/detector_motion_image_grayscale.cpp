class Detector_Motion_Image_Grayscale : public Detector_Motion_Image {
public:
  void processImage(cv::Mat &image) override {
    // Motion threshold  
    cv::threshold(image, image, 50, 255, THRESH_BINARY);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));

    // Make edges thicker
    cv::dilate(image, image, kernel);

    // Copy current image pixels to difference image
    currentImage.copyTo(image, image);
    
    // Update processed image
    processedImage = image;
  }
};

class Detector_Motion_Image_Color : public Detector_Motion_Image {
public:
  void processImage(cv::Mat &image) override {
    // Process image colors
    processColors(image);

    // Motion threshold  
    cv::threshold(image, image, 50, 255, THRESH_BINARY);
    
    // Update motion image
    processedImage = image;
  }

  virtual void processColors(cv::Mat &image) {
    cv::Mat coloredImage;

    // Get BGR image from image
    cv::cvtColor(image, coloredImage, COLOR_GRAY2BGR);

    std::vector<cv::Mat> channels;

    // Get color channels
    cv::split(coloredImage, channels);

    // Process color channels
    processColorChannels(channels);

    // Merge color channels to image
    cv::merge(channels, image);
  }

  virtual void processColorChannels(std::vector<cv::Mat> &channels) {}
};

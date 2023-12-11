class Detector_Motion : public Detector {
public:
  cv::Mat prevImage; // Previous image for motion

  void setImageData( unsigned char* nv21ImageData) override {
    Detector::setImageData(nv21ImageData);

    // Check if the previous frame is invalid
    if (prevImage.empty()) {
      // Initialize the previous frame
      prevImage = currentImage;
    }
  }

  void clear() override {
    Detector::clear();
    prevImage.release();
  }
};

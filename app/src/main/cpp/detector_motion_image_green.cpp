class Detector_Motion_Image_Green : public Detector_Motion_Image_Color {
public:
  void processColorChannels(std::vector<cv::Mat> &channels) override {
    channels[1] *= 3.5; // Amplify green
  }
};

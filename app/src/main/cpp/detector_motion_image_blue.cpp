class Detector_Motion_Image_Blue : public Detector_Motion_Image_Color {
public:
  void processColorChannels(std::vector<cv::Mat> &channels) override {
    channels[0] *= 3.5; // Amplify blue
  }
};

class Detector_Motion_Image_Red : public Detector_Motion_Image_Color {
public:
  void processColorChannels(std::vector<cv::Mat> &channels) override {
    channels[2] *= 3.5; // Amplify red
  }
};

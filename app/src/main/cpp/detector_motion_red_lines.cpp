class Detector_Motion_Red_Lines : public Detector_Motion {
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

    // Motion threshold  
    cv::threshold(diff, diff, 50, 255, THRESH_BINARY);

    // Find contours in the difference image
    cv::findContours(diff, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // Update the previous frame
    prevImage = currentImage;
  }

  void updateRendererData(Renderer *renderer) override {
    // Update renderer contours
    renderer->setContours(contours);
  }
};

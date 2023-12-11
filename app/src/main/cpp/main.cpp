#include <jni.h>
#include <android/log.h>
#include <GLES3/gl3.h>
#include <array>

#include <opencv2/core.hpp> // OpenCV core
#include <opencv2/imgproc.hpp> // OpenCV COLOR_
// #include <opencv2/features2d.hpp> // OpenCV fast feature detector

using namespace cv;

enum PreviewMode {
  DETECT_PREVIEW_MOTION_WHITE,
  DETECT_PREVIEW_MOTION_RED,
  DETECT_PREVIEW_MOTION_GREEN,
  DETECT_PREVIEW_MOTION_BLUE,
  DETECT_PREVIEW_MOTION_GRAYSCALE,
  DETECT_PREVIEW_MOTION_WHITE_WITH_BACKGROUND,
  DETECT_MOTION_LINES,
};

PreviewMode previewMode;
PreviewMode previousPreviewMode;

int cameraWidth;
int cameraHeight;

#include "detector.cpp"
#include "detector_motion.cpp"
#include "detector_motion_image.cpp"
#include "detector_motion_image_color.cpp"
#include "detector_motion_image_red.cpp"
#include "detector_motion_image_green.cpp"
#include "detector_motion_image_blue.cpp"
#include "detector_motion_image_white.cpp"
#include "detector_motion_image_grayscale.cpp"
#include "detector_motion_image_background.cpp"
#include "detector_motion_red_lines.cpp"
#include "renderer.cpp"
#include "renderer_red_lines.cpp"
#include "renderer_texture.cpp"

bool initialized;

// Shader program currently in use
GLuint currentProgram;

Detector *currentDetector = nullptr;
Renderer *currentRenderer = nullptr;

Detector_Motion_Image_Red *redMotionImageDetector;
Detector_Motion_Image_Green *greenMotionImageDetector;
Detector_Motion_Image_Blue *blueMotionImageDetector;
Detector_Motion_Image_White *whiteMotionImageDetector;
Detector_Motion_Image_Grayscale *grayscaleMotionImageDetector;
Detector_Motion_Image_Background *backgroundMotionImageDetector;
Detector_Motion_Red_Lines *redLinesMotionDetector;

Renderer_Red_Lines *redLinesRenderer;
Renderer_Texture *textureRenderer;

void setupDefaults() {
  // Default preview mode
  previewMode = PreviewMode::DETECT_PREVIEW_MOTION_WHITE;
  previousPreviewMode = previewMode;
}

void setupDetectors() {
  redMotionImageDetector = new Detector_Motion_Image_Red();
  greenMotionImageDetector = new Detector_Motion_Image_Green();
  blueMotionImageDetector = new Detector_Motion_Image_Blue();
  whiteMotionImageDetector = new Detector_Motion_Image_White();
  grayscaleMotionImageDetector = new Detector_Motion_Image_Grayscale();
  backgroundMotionImageDetector = new Detector_Motion_Image_Background();
  redLinesMotionDetector = new Detector_Motion_Red_Lines();
}

void setDetector(Detector *detector) {
  currentDetector = detector;
}

void updateDetector() {
  if (currentDetector != nullptr) {
    currentDetector->clear();
  }

  switch (previewMode) {
    case PreviewMode::DETECT_PREVIEW_MOTION_WHITE:
      currentDetector = whiteMotionImageDetector;
      break;
    case PreviewMode::DETECT_PREVIEW_MOTION_RED:
      currentDetector = redMotionImageDetector;
      break;
    case PreviewMode::DETECT_PREVIEW_MOTION_GREEN:
      currentDetector = greenMotionImageDetector;
      break;
    case PreviewMode::DETECT_PREVIEW_MOTION_BLUE:
      currentDetector = blueMotionImageDetector;
      break;
    case PreviewMode::DETECT_PREVIEW_MOTION_GRAYSCALE:
      currentDetector = grayscaleMotionImageDetector;
      break;
    case PreviewMode::DETECT_PREVIEW_MOTION_WHITE_WITH_BACKGROUND:
      currentDetector = backgroundMotionImageDetector;
      break;
    case PreviewMode::DETECT_MOTION_LINES:
      currentDetector = redLinesMotionDetector;
      break;
  }
}

void setupRenderers() {
  redLinesRenderer = new Renderer_Red_Lines();
  textureRenderer = new Renderer_Texture();
}

void setRenderer(Renderer *renderer) {
  currentRenderer = renderer;
}

void updateRenderer() {
  if (previewMode == PreviewMode::DETECT_MOTION_LINES) {
    setRenderer(redLinesRenderer);
  }
  else {
    setRenderer(textureRenderer);
  }
}

void updatePreviewMode() {
  if (previewMode == previousPreviewMode) {
    return;
  }
  
  updateDetector();
  updateRenderer();

  previousPreviewMode = previewMode;
}

void setupGraphics(int width, int height) {
  textureRenderer->setupProgram();  
  redLinesRenderer->setupProgram();  

  // Default program
  glUseProgram(textureRenderer->program);
  currentProgram = textureRenderer->program;

  glViewport(0, 0, width, height);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void detectFrame(unsigned char* nv21ImageData) {  
  currentDetector->setImageData(nv21ImageData);
  currentDetector->detect();
  currentDetector->clearImage();
}

void renderFrame() {
  if (currentProgram != currentRenderer->program) {
    glUseProgram(currentRenderer->program);
    currentProgram = currentRenderer->program;
  }

  if (previewMode == PreviewMode::DETECT_MOTION_LINES) { // Contours (points)
    currentRenderer->setContours(currentDetector->contours);
  }
  else { // Image motion
    currentRenderer->setImageData(currentDetector->processedImage.data);
  }

  currentRenderer->draw();
}

extern "C" {
  JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_init(JNIEnv *env, jobject obj,  jint width, jint height);
  JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_setCameraSettings(JNIEnv* env, jobject obj, int32_t width, int32_t height);
  JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_draw(JNIEnv *env, jobject obj);
  JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_processImageBuffers(JNIEnv* env, jobject obj, jobject y, int ySize, int yPixelStride, int yRowStride, jobject u, int uSize, int uPixelStride, int uRowStride, jobject v, int vSize, int vPixelStride, int vRowStride);
  JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_touch(JNIEnv *env, jobject obj, int previewMode);
};

JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_init(JNIEnv *env, jobject obj,  jint width, jint height) {
  // Set up default preview mode
  setupDefaults();

  // Set up renderer and detector classes
  setupRenderers();
  setupDetectors();

  // Set up graphics
  setupGraphics(width, height);

  // Select default detector and renderer
  updateDetector();
  updateRenderer();

  initialized = true;
}

JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_setCameraSettings(JNIEnv* env,
                                                    jobject obj,
                                                    int32_t width,
                                                    int32_t height) {
  cameraWidth = width;
  cameraHeight = height;
}

JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_draw(JNIEnv *env, jobject obj) {
  renderFrame();
}

JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_processImageBuffers(JNIEnv* env,
                                                                jobject obj,
                                                                jobject y,
                                                                int ySize,
                                                                int yPixelStride,
                                                                int yRowStride,
                                                                jobject u,
                                                                int uSize,
                                                                int uPixelStride,
                                                                int uRowStride,
                                                                jobject v,
                                                                int vSize,
                                                                int vPixelStride,
                                                                int vRowStride) {
  if (!initialized) {
    // Make sure that init is called before processImageBuffers to prevent crash
    return;
  }

  try {
    // Get the address of the underlying memory of the ByteBuffer objects
    unsigned char* yData = (unsigned char*)env->GetDirectBufferAddress(y);
    unsigned char* uData = (unsigned char*)env->GetDirectBufferAddress(u);
    unsigned char* vData = (unsigned char*)env->GetDirectBufferAddress(v);

    unsigned char* nv21ImageData = (unsigned char*)malloc(ySize + uSize + vSize);

    int yIndex = 0;
    int uvIndex = ySize;

    // Use memcpy to process YUV_420_888 image
    // Note: this process will only be effective if the source and destination data are both aligned 
    // and the size is a multiple of 4
    for (int i = 0; i < cameraHeight; i++) {
      int ySrcIndex = i * yRowStride;
      int uvSrcIndex = i / 2 * uRowStride;

      int ySizeWidth = cameraWidth * yPixelStride;

      // Use memcpy to copy the Y data
      memcpy(nv21ImageData + yIndex, yData + ySrcIndex, ySizeWidth);
      yIndex += cameraWidth * yPixelStride;

      if (i % 2 == 0) {
        // Use memcpy to copy the U and V data
        memcpy(nv21ImageData + uvIndex, uData + uvSrcIndex, cameraWidth * uPixelStride / 2);
        uvIndex += cameraWidth * uPixelStride / 2;
        memcpy(nv21ImageData + uvIndex, uData + uvSrcIndex, cameraWidth * vPixelStride / 2);
        uvIndex += cameraWidth * vPixelStride / 2;
      }
    }

    // Detect motion from image
    detectFrame(nv21ImageData);

    // Delete NV21 image from memory
    free(nv21ImageData);

    env->DeleteLocalRef(y);
    env->DeleteLocalRef(u);
    env->DeleteLocalRef(v);
  }
  catch (const std::exception& e) {
    __android_log_print(ANDROID_LOG_DEBUG, "motiondetector", "Error: %s", e.what());
    env->DeleteLocalRef(y);
    env->DeleteLocalRef(u);
    env->DeleteLocalRef(v);
  }
}

JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_touch(JNIEnv* env,
                                                                             jobject obj,
                                                                             int previewMode_) {
  previewMode = static_cast<PreviewMode>(previewMode_); // Set preview mode
  updatePreviewMode();
}

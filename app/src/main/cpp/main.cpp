#include <jni.h>
#include <android/log.h>
#include <GLES3/gl3.h>
#include <array>
#include "previewmode.h"
#include "detector.cpp"

bool initialized;

int cameraWidth;
int cameraHeight;

GLuint gProgram;
GLuint gTextureProgram;

// Change to this shader program on next tick
GLuint changeToProgram;

// Shader program currently in use
GLuint currentProgram;

// Red square arrays
GLuint gvPositionHandle;
GLuint ibo;
GLuint vbo;

// Add attribute arrays for the texture coordinates and a handle for the texture uniform
GLuint gvtPositionHandle;
GLint gvtTexCoordHandle;
GLint gutTextureHandle;
GLuint gTexture;

void updatePreviewMode() {
  if (previewMode == previousPreviewMode) {
    return;
  }

  if (previewMode == PreviewMode::DETECT_MOTION_LINES) {
    changeToProgram = gProgram; // Red squares and lines shader
  }
  else {
    changeToProgram = gTextureProgram; // Texture shader
  }
  
  previousPreviewMode = previewMode;
}

const char* gVertexShader = R"(#version 300 es
  layout(location = 0) in vec2 vPosition;

  void main() {
    gl_Position = vec4(vPosition, 0.0, 1.0);
  }
)";

const char* gFragmentShader = R"(#version 300 es
  precision mediump float;
  out vec4 fragColor;

  void main() {
    fragColor = vec4(1.0, 0.0, 0.0, 1.0); // default color is red 
  }
)";

const char* gTextureVertexShader = R"(#version 300 es
  layout(location = 0) in vec2 vPosition;
  layout(location = 1) in vec2 vTexCoord;
  out vec2 texCoord;

  void main() {
    gl_Position = vec4(vPosition, 0.0, 1.0);
    texCoord = vTexCoord;
  }
)";

const char* gTextureFragmentShader = R"(#version 300 es
  precision mediump float;
  in vec2 texCoord;
  uniform sampler2D uTexture;
  out vec4 fragColor;

  void main() {
    fragColor = texture(uTexture, texCoord);
  }
)";

GLuint loadShader(GLenum type, const char *shaderSrc) {
  GLuint shader;
  GLint compiled;
  shader = glCreateShader(type);

  if (shader == 0) {
    return 0;
  }

  glShaderSource(shader, 1, &shaderSrc, NULL);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1) {
      char *infoLog = (char *)malloc(sizeof(char) * infoLen);
      glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
      __android_log_print(ANDROID_LOG_ERROR, "MotionDetector", "Error compiling shader:\n%s\n", infoLog);
      free(infoLog);
    }
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

GLuint createProgram(const char *vertexSource, const char *fragmentSource) {
  GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
  if (!vertexShader) {
    return 0;
  }

  GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
  if (!fragmentShader) {
    return 0;
  }

  GLuint program = glCreateProgram();
  if (program == 0) {
    return 0;
  }

  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram  (program);
  GLint linkStatus = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
  if (linkStatus != GL_TRUE) {
    GLint bufLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
    if (bufLength) {
      char *buf = (char *)malloc(bufLength);
      if (buf) {
        glGetProgramInfoLog(program, bufLength, NULL, buf);
        __android_log_print(ANDROID_LOG_ERROR, "MotionDetector", "Could not link program:\n%s\n", buf);
        free(buf);
      }
    }
    glDeleteProgram(program);
    program = 0;
  }
  return program;
}

float squareWidth = 0.01f;
float squareHeight = 0.01f;

// Square
GLfloat gSquareVertices[] = {
   0.5f * squareWidth,  0.5f * squareHeight, // top right
   0.5f * squareWidth, -0.5f * squareHeight, // bottom right
  -0.5f * squareWidth, -0.5f * squareHeight, // bottom left
  -0.5f * squareWidth,  0.5f * squareHeight  // top left
};

// Texture
GLfloat gTextureVertices[] = {
   1.0f,  1.0f, 0.0f, 0.0f,  // top right
   1.0f, -1.0f, 1.0f, 0.0f,  // bottom right
  -1.0f, -1.0f, 1.0f, 1.0f,  // bottom left
  -1.0f,  1.0f, 0.0f, 1.0f,  // top left
};

const GLushort gIndices[] = {
  0, 1, 2,
  2, 3, 0
};

void setupGraphics(int width, int height) {
  gProgram = createProgram(gVertexShader, gFragmentShader);
  if (!gProgram) {
    return;
  }

  gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");

  gTextureProgram = createProgram(gTextureVertexShader, gTextureFragmentShader);
  if (!gTextureProgram) {
    return;
  }

  gvtPositionHandle = glGetAttribLocation(gTextureProgram, "vPosition");
  gvtTexCoordHandle = glGetAttribLocation(gTextureProgram, "vTexCoord");
  gutTextureHandle = glGetUniformLocation(gTextureProgram, "uTexture");

  // Default program
  glUseProgram(gTextureProgram);
  currentProgram = gTextureProgram;
  changeToProgram = gTextureProgram;

  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ibo);

  glViewport(0, 0, width, height);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  glVertexAttribPointer(gvtTexCoordHandle, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), gTextureVertices + 2);
  glEnableVertexAttribArray(gvtTexCoordHandle);

  glActiveTexture(GL_TEXTURE0);

  glGenTextures(1, &gTexture);
  glBindTexture(GL_TEXTURE_2D, gTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glUniform1i(gutTextureHandle, 0);
}

GLfloat *vboData = (GLfloat*)malloc(10000000 * sizeof(GLfloat));

void drawSmallRedLines() {
  int i = 0;

  for (const auto& contour : contours) {
    for (const auto& point : contour) {
      float x = -(point.y / (cameraHeight * 0.5f)) + 1.0f;
      float y = -(point.x / (cameraWidth * 0.5f)) + 1.0f;

      const uint32_t vboIndex = i * 8;
      vboData[vboIndex + 0] = gSquareVertices[0] + x;
      vboData[vboIndex + 1] = gSquareVertices[1] + y;
      vboData[vboIndex + 2] = gSquareVertices[2] + x;
      vboData[vboIndex + 3] = gSquareVertices[3] + y;
      vboData[vboIndex + 4] = gSquareVertices[4] + x;
      vboData[vboIndex + 5] = gSquareVertices[5] + y;
      vboData[vboIndex + 6] = gSquareVertices[6] + x;
      vboData[vboIndex + 7] = gSquareVertices[7] + y;

      ++i;
    }
  }

  size_t numSquares = i;
  size_t vboSize = numSquares * 32;

  // VBO
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vboSize, vboData, GL_DYNAMIC_DRAW);

  // Set up the vertex attribute pointers
  glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(gvPositionHandle);

  glClear(GL_COLOR_BUFFER_BIT);

  // Draw the lines
  glDrawArrays(GL_TRIANGLES, 0, numSquares * 6);

  glDisableVertexAttribArray(gvPositionHandle);

  glDeleteBuffers(1, &vbo);
}

const size_t textureVerticesSize = 4 * sizeof(GLfloat);
const size_t iboSize = 6 * sizeof(GLushort);

void drawTexture(unsigned char* imageData, int width, int height) {
  if (imageData == nullptr) {
    return;
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, iboSize, gIndices, GL_STATIC_DRAW);

  glVertexAttribPointer(gvtPositionHandle, 2, GL_FLOAT, GL_FALSE, textureVerticesSize, gTextureVertices);
  glEnableVertexAttribArray(gvtPositionHandle);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

  glDisableVertexAttribArray(gvtPositionHandle);
}

void renderFrame() {
  if (currentProgram != changeToProgram) {
    glUseProgram(changeToProgram);
    currentProgram = changeToProgram;
  }

  if (previewMode == PreviewMode::DETECT_MOTION_LINES) { // Red motion lines
    drawSmallRedLines();
  }
  else { // Preview motion
    drawTexture(processedImage.data, cameraWidth, cameraHeight);
  }
}

extern "C" {
  JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_init(JNIEnv *env, jobject obj,  jint width, jint height);
  JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_setCameraSettings(JNIEnv* env, jobject obj, int32_t width, int32_t height);
  JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_draw(JNIEnv *env, jobject obj);
  JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_processImageBuffers(JNIEnv* env, jobject obj, jobject y, int ySize, int yPixelStride, int yRowStride, jobject u, int uSize, int uPixelStride, int uRowStride, jobject v, int vSize, int vPixelStride, int vRowStride);
  JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_touch(JNIEnv *env, jobject obj, int previewMode);
};

JNIEXPORT void JNICALL Java_com_app_motiondetector_MyGLSurfaceView_init(JNIEnv *env, jobject obj,  jint width, jint height) {
  setupGraphics(width, height);

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

    unsigned char* nv21 = (unsigned char*)malloc(ySize + uSize + vSize);

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
      memcpy(nv21 + yIndex, yData + ySrcIndex, ySizeWidth);
      yIndex += cameraWidth * yPixelStride;

      if (i % 2 == 0) {
        // Use memcpy to copy the U and V data
        memcpy(nv21 + uvIndex, uData + uvSrcIndex, cameraWidth * uPixelStride / 2);
        uvIndex += cameraWidth * uPixelStride / 2;
        memcpy(nv21 + uvIndex, uData + uvSrcIndex, cameraWidth * vPixelStride / 2);
        uvIndex += cameraWidth * vPixelStride / 2;
      }
    }

    grayImage.create(cameraHeight, cameraWidth, CV_8UC1);
    grayImage.data = nv21;
    cv::cvtColor(grayImage, grayImage, cv::COLOR_BGR2RGB);

    // Detection method for image
    detect();

    // Free memory
    free(nv21);

    // Clear image data
    grayImage.release();

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
  previewMode = static_cast<PreviewMode>(previewMode_);
  updatePreviewMode();
}

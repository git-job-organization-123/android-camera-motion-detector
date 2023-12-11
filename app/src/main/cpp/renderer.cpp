class Renderer {
public:
  GLuint program;

  virtual const char* getVertexShader() {return nullptr;}
  virtual const char* getFragmentShader() {return nullptr;}

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

  virtual void setupProgram() {}
  virtual void update() {}
  virtual void draw() {}
  virtual void setImageData(unsigned char* imageData_) {}
  virtual void setContours(std::vector<std::vector<cv::Point>> contours_) {}

protected:
  GLuint ibo;
  GLuint vbo;
};

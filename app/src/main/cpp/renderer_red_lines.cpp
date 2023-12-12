class Renderer_Red_Lines : public Renderer {
public:
  const char* getVertexShader() override {
    return R"(#version 300 es
      layout(location = 0) in vec2 vPosition;

      void main() {
        gl_Position = vec4(vPosition, 0.0, 1.0);
      }
    )";
  }
  
  const char* getFragmentShader() override {
    return R"(#version 300 es
      precision mediump float;
      out vec4 fragColor;

      void main() {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red
      }
    )";
  }

  void setupProgram() override {
    program = createProgram(getVertexShader(), getFragmentShader());
    if (!program) {
      return;
    }

    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    positionHandle = glGetAttribLocation(program, "vPosition");
  }

  void init() override {
    vboData = new GLfloat[8192 * 8 * sizeof(GLfloat)];
  }

  void setContours(std::vector<std::vector<cv::Point>> &contours_) override {
    contours = contours_;
  }

  void draw() override {
    GLint numSquares = 0;

    for (const auto& contour : contours) {
      for (const auto& point : contour) {
        if (numSquares >= 8192) {
          // Prevent crash with limit
          continue;
        }

        const GLfloat x = -(point.y / (cameraHeight * 0.5f)) + 1.0f;
        const GLfloat y = -(point.x / (cameraWidth * 0.5f)) + 1.0f;

        const GLint vboPosition = numSquares * 8;
        vboData[vboPosition] =     vertices[0] + x;
        vboData[vboPosition + 1] = vertices[1] + y;
        vboData[vboPosition + 2] = vertices[2] + x;
        vboData[vboPosition + 3] = vertices[3] + y;
        vboData[vboPosition + 4] = vertices[4] + x;
        vboData[vboPosition + 5] = vertices[5] + y;
        vboData[vboPosition + 6] = vertices[6] + x;
        vboData[vboPosition + 7] = vertices[7] + y;

        ++numSquares;
      }
    }

    const GLint vboSize = numSquares * 8 * sizeof(GLfloat);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vboSize, vboData, GL_DYNAMIC_DRAW);

    // Set up the vertex attribute pointers
    glVertexAttribPointer(positionHandle, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionHandle);

    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the lines
    glDrawArrays(GL_TRIANGLES, 0, numSquares * 6);

    glDisableVertexAttribArray(positionHandle);

    glDeleteBuffers(1, &vbo);
  }

  void clear() override {
    delete[] vboData;
  }

private:
  GLuint positionHandle;

  std::vector<std::vector<cv::Point>> contours;

  // Square
  const GLfloat vertices[8] = {
     0.005f,  0.005f, // top right
     0.005f, -0.005f, // bottom right
    -0.005f, -0.005f, // bottom left
    -0.005f,  0.005f  // top left
  };

  GLfloat *vboData;
};

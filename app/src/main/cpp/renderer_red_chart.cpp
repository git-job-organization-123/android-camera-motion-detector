class Renderer_Red_Chart : public Renderer {
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
    vboData = new GLfloat[2048 * 4 * sizeof(GLfloat)];
  }

  void setContours(std::vector<std::vector<cv::Point>> &contours_) override {
    contours = contours_;
  }

  void draw() override {
    GLint motion = 0;

    // Get amout of motion
    for (const auto& contour : contours) {
      motion += contour.size();
    }

    // Calculate motion line height
    const GLfloat motionYHeight = (GLfloat)motion * 0.00004f;

    // Add chart line
    addChartLine(motionYHeight);

    // Set next chart line X
    chartLineX += chartSpeedX;

    // 1024 lines per buffer
    if (numChartLines > 1024) {
      // Remove chart lines
      numChartLines = 0;
    }

    // Check if chart has reached right side of screen
    if (chartLineX > 2.0f) {
      // Remove chart lines
      numChartLines = 0;

      // Start over
      chartLineX = 0;
    }

    const GLint vboSize = numChartLines * 4 * sizeof(GLfloat);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vboSize, vboData, GL_STATIC_DRAW);

    // Set up the vertex attribute pointers
    glVertexAttribPointer(positionHandle, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionHandle);

    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the chart
    glDrawArrays(GL_LINES, 0, numChartLines * 2);

    glDisableVertexAttribArray(positionHandle);

    glDeleteBuffers(1, &vbo);
  }

  void addChartLine(const GLfloat chartLineY) {
    if (numChartLines == 0) {
      vboData[numChartLines * 4 + 0] = chartLineX - 1.0f; // X start
      vboData[numChartLines * 4 + 1] = chartLineY - 0.5f; // Y start
    }
    else { // Connect lines
      vboData[numChartLines * 4 + 0] = oldChartLineX; // X start
      vboData[numChartLines * 4 + 1] = oldChartLineY; // Y start
    }

    oldChartLineX = chartLineX - 1.0f;
    oldChartLineY = chartLineY - 0.5f;

    vboData[numChartLines * 4 + 2] = oldChartLineX; // X end
    vboData[numChartLines * 4 + 3] = oldChartLineY; // Y end

    ++numChartLines;
  }

  void clear() override {
    numChartLines = 0;
    chartLineX = 0;

    delete[] vboData;
  }

private:
  GLuint positionHandle;

  std::vector<std::vector<cv::Point>> contours;

  GLfloat chartSpeedX = 0.0035f;
  GLint numChartLines;
  GLfloat chartLineX = 0.0f;
  GLfloat oldChartLineX;
  GLfloat oldChartLineY;
  GLfloat *vboData;
};

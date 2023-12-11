class Renderer_Red_Lines : public Renderer {
public:
  Renderer_Red_Lines(GLuint program_)
  : Renderer(program_) {
  }

  // Square
  const GLfloat squareVertices[8] = {
     0.005f,  0.005f, // top right
     0.005f, -0.005f, // bottom right
    -0.005f, -0.005f, // bottom left
    -0.005f,  0.005f  // top left
  };

  void setContours(std::vector<std::vector<cv::Point>> contours_) override {
    contours = contours_;
  }

  void draw() override {
    GLint i = 0;

    for (const auto& contour : contours) {
      for (const auto& point : contour) {
        const GLfloat x = -(point.y / (cameraHeight * 0.5f)) + 1.0f;
        const GLfloat y = -(point.x / (cameraWidth * 0.5f)) + 1.0f;

        const GLint vboIndex = i * 8;
        vboData[vboIndex + 0] = squareVertices[0] + x;
        vboData[vboIndex + 1] = squareVertices[1] + y;
        vboData[vboIndex + 2] = squareVertices[2] + x;
        vboData[vboIndex + 3] = squareVertices[3] + y;
        vboData[vboIndex + 4] = squareVertices[4] + x;
        vboData[vboIndex + 5] = squareVertices[5] + y;
        vboData[vboIndex + 6] = squareVertices[6] + x;
        vboData[vboIndex + 7] = squareVertices[7] + y;

        ++i;
      }
    }

    const GLint numSquares = i;
    const GLint vboSize = numSquares * 8 * sizeof(GLint);

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

private:
  std::vector<std::vector<cv::Point>> contours;
};

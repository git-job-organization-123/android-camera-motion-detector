class LinesRenderer : public Renderer {
public:
  const float squareWidth = 0.01f;
  const float squareHeight = 0.01f;

  // Square
  const GLfloat squareVertices[8] = {
     0.5f * squareWidth,  0.5f * squareHeight, // top right
     0.5f * squareWidth, -0.5f * squareHeight, // bottom right
    -0.5f * squareWidth, -0.5f * squareHeight, // bottom left
    -0.5f * squareWidth,  0.5f * squareHeight  // top left
  };

  LinesRenderer(GLuint program_)
  : Renderer(program_) {
  }

  void draw() override {
    int i = 0;

    for (const auto& contour : contours) {
      for (const auto& point : contour) {
        float x = -(point.y / (cameraHeight * 0.5f)) + 1.0f;
        float y = -(point.x / (cameraWidth * 0.5f)) + 1.0f;

        const uint32_t vboIndex = i * 8;
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

    size_t numSquares = i;
    size_t vboSize = numSquares * 32;

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
};

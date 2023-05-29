class TextureRenderer : public Renderer {
public:
  GLint texCoordHandle;
  GLint textureHandle;
  GLuint texture;

  // Texture
  const GLfloat textureVertices[16] = {
     1.0f,  1.0f, 0.0f, 0.0f, // top right
     1.0f, -1.0f, 1.0f, 0.0f, // bottom right
    -1.0f, -1.0f, 1.0f, 1.0f, // bottom left
    -1.0f,  1.0f, 0.0f, 1.0f  // top left
  };

  const size_t textureVerticesSize = 4 * sizeof(GLfloat);
  const size_t iboSize = 6 * sizeof(GLushort);

  unsigned char* imageData;

  TextureRenderer(GLuint program_)
  : Renderer(program_) {
    texCoordHandle = glGetAttribLocation(program_, "vTexCoord");
    textureHandle = glGetUniformLocation(program_, "uTexture");

    glVertexAttribPointer(texCoordHandle, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), textureVertices + 2);
    glEnableVertexAttribArray(texCoordHandle);

    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glUniform1i(textureHandle, 0);
  }

  void draw() override {
    if (imageData == nullptr) {
      return;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iboSize, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(positionHandle, 2, GL_FLOAT, GL_FALSE, textureVerticesSize, textureVertices);
    glEnableVertexAttribArray(positionHandle);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cameraWidth, cameraHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(positionHandle);
  }

  void setImageData(unsigned char* imageData_) override {
    imageData = imageData_;
  }
};

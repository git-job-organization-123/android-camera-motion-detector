class Renderer_Texture : public Renderer {
public:
  const char* getVertexShader() override {
    return R"(#version 300 es
      layout(location = 0) in vec2 vPosition;
      layout(location = 1) in vec2 vTexCoord;
      out vec2 texCoord;

      void main() {
        gl_Position = vec4(vPosition, 0.0, 1.0);
        texCoord = vTexCoord;
      }
    )";
  }
  
  const char* getFragmentShader() override {
    return R"(#version 300 es
      precision mediump float;
      in vec2 texCoord;
      uniform sampler2D uTexture;
      out vec4 fragColor;

      void main() {
        fragColor = texture(uTexture, texCoord);
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
    texCoordHandle = glGetAttribLocation(program, "vTexCoord");
    textureHandle = glGetUniformLocation(program, "uTexture");

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

private:
  GLuint positionHandle;
  GLint texCoordHandle;
  GLint textureHandle;
  GLuint texture;

  const GLint textureVerticesSize = 4 * sizeof(GLfloat);
  const GLint iboSize = 6 * sizeof(GLushort);

  unsigned char* imageData;

  // Texture
  const GLfloat textureVertices[16] = {
     1.0f,  1.0f, 0.0f, 0.0f, // top right
     1.0f, -1.0f, 1.0f, 0.0f, // bottom right
    -1.0f, -1.0f, 1.0f, 1.0f, // bottom left
    -1.0f,  1.0f, 0.0f, 1.0f  // top left
  };

  const GLushort indices[6] = {
    0, 1, 2,
    2, 3, 0
  };
};

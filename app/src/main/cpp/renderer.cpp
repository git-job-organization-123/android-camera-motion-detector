class Renderer {
public:
  GLuint program;
  GLuint positionHandle;

  const GLushort indices[6] = {
    0, 1, 2,
    2, 3, 0
  };

  Renderer(GLuint program_) {
    program = program_;
    positionHandle = glGetAttribLocation(program_, "vPosition");
  }

  virtual void update() {}
  virtual void draw() {}
  virtual void setImageData(unsigned char* imageData_) {}
};

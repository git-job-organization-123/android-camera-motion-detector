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

GLuint ibo;
GLuint vbo;

GLfloat *vboData;
GLushort *iboData;

int cameraWidth;
int cameraHeight;

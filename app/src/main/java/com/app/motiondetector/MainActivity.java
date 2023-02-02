/** 
Camera preview motion detector
Change mode by touching the screen
*/

package com.app.motiondetector;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.*;
import android.hardware.camera2.*;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.Image.Plane;
import android.media.ImageReader;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Size;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.view.*;
import android.util.Log;
import androidx.appcompat.app.AppCompatActivity;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.lang.Thread;
import java.lang.InterruptedException;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends AppCompatActivity {
    private static final int REQUEST_CAMERA_PERMISSION = 1;
    private String cameraId;
    protected CameraDevice cameraDevice;
    protected CameraCaptureSession cameraCaptureSessions;
    protected CaptureRequest.Builder captureRequestBuilder;
    private Size imageDimension;
    private ImageReader imageReaderGL;
    private Handler mBackgroundHandler;
    private HandlerThread mBackgroundThread;
    private MyGLSurfaceView GLView;
    private CameraCharacteristics characteristics;

    private byte glThreadCount = 0;
    private static final int MAX_GL_THREADS = 1;

    private boolean cachedGLImageProperties = false;
    private int yPixelStride_gl = 0;
    private int yRowStride_gl = 0;
    private int uPixelStride_gl = 0;
    private int uRowStride_gl = 0;
    private int vPixelStride_gl = 0;
    private int vRowStride_gl = 0;
    private int uSize_gl = 0;
    private int ySize_gl = 0;
    private int vSize_gl = 0;

    enum PreviewMode {
        DETECT_PREVIEW_MOTION_WHITE,
        DETECT_PREVIEW_MOTION_RED,
        DETECT_PREVIEW_MOTION_GREEN,
        DETECT_PREVIEW_MOTION_BLUE,
        DETECT_PREVIEW_MOTION_GRAYSCALE,
        DETECT_PREVIEW_MOTION_WHITE_WITH_BACKGROUND,
        DETECT_MOTION_LINES,
    };

    PreviewMode previewMode = PreviewMode.DETECT_PREVIEW_MOTION_WHITE;
    PreviewMode previousPreviewMode = PreviewMode.DETECT_PREVIEW_MOTION_WHITE;
    int currentPreviewMode = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        GLView = new MyGLSurfaceView(this);

        // View OpenGL view
        setContentView(GLView);

        // Try to open the camera
        openCamera();

        // Do not fade out screen after some time
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    private void nextPreviewMode() {
        ++currentPreviewMode;

        if (currentPreviewMode > 6) {
            currentPreviewMode = 0;
        }

        previousPreviewMode = previewMode;

        // Select enum
        previewMode = PreviewMode.values()[currentPreviewMode];
    }

    private final CameraDevice.StateCallback stateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(CameraDevice camera) {
            // This is called when the camera is open
            cameraDevice = camera;
            createCameraPreview();
        }

        @Override
        public void onDisconnected(CameraDevice camera) {
            // cameraDevice.close();
        }

        @Override
        public void onError(CameraDevice camera, int error) {
            cameraDevice.close();
            cameraDevice = null;
        }
    };

    final CameraCaptureSession.CaptureCallback captureCallbackListener = new CameraCaptureSession.CaptureCallback() {
        @Override
        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request, TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);
            createCameraPreview();
        }
    };

    protected void startBackgroundThread() {
        mBackgroundThread = new HandlerThread("Camera Background");
        mBackgroundThread.start();
        mBackgroundHandler = new Handler(mBackgroundThread.getLooper());
    }

    protected void stopBackgroundThread() {
        mBackgroundThread.quitSafely();
        try {
            mBackgroundThread.join();
            mBackgroundThread = null;
            mBackgroundHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void cacheGLImageProperties(Plane[] planes, ByteBuffer yBuffer, ByteBuffer uBuffer, ByteBuffer vBuffer) {
        ySize_gl = yBuffer.remaining();
        uSize_gl = uBuffer.remaining();
        vSize_gl = vBuffer.remaining();
        yPixelStride_gl = planes[0].getPixelStride();
        yRowStride_gl = planes[0].getRowStride();
        uPixelStride_gl = planes[1].getPixelStride();
        uRowStride_gl = planes[1].getRowStride();
        vPixelStride_gl = planes[2].getPixelStride();
        vRowStride_gl = planes[2].getRowStride();                            
    }

    protected void createCameraPreview() {
        try {
            captureRequestBuilder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            List<Surface> outputSurfaces = new ArrayList<Surface>(2);

            int width = imageDimension.getWidth();
            int height = imageDimension.getHeight();

            GLView.setCameraSettings(width, height);

            imageReaderGL = ImageReader.newInstance(width, height, ImageFormat.YUV_420_888, 1);
            imageReaderGL.setOnImageAvailableListener(
            new ImageReader.OnImageAvailableListener() {
                @Override
                public void onImageAvailable(ImageReader reader) {
                    Image image = reader.acquireLatestImage();

                    Image.Plane[] planes = image.getPlanes();
                    ByteBuffer yBuffer = planes[0].getBuffer();
                    ByteBuffer uBuffer = planes[1].getBuffer();
                    ByteBuffer vBuffer = planes[2].getBuffer();

                    if (!cachedGLImageProperties) {
                        cacheGLImageProperties(planes, yBuffer, uBuffer, vBuffer);
                        cachedGLImageProperties = true;
                    }

                    image.close();

                    // Send the frame to native library for processing
                    // Threading is to prevent slow frames causing delayed preview
                    if (glThreadCount < MAX_GL_THREADS) {
                        Thread glThread = new Thread(new Runnable() {
                            @Override
                            public void run() {
                                try {
                                    GLView.processImageBuffers(yBuffer, ySize_gl, yPixelStride_gl, yRowStride_gl, 
                                                               uBuffer, uSize_gl, uPixelStride_gl, uRowStride_gl, 
                                                               vBuffer, vSize_gl, vPixelStride_gl, vRowStride_gl);
                                    GLView.requestRender();
                                }
                                finally {
                                    glThreadCount--;
                                }
                            }
                        });

                        glThread.start();
                        glThreadCount++;
                    }
                }
            }, null);

            captureRequestBuilder.addTarget(imageReaderGL.getSurface());
            outputSurfaces.add(imageReaderGL.getSurface());

            cameraDevice.createCaptureSession(outputSurfaces, new CameraCaptureSession.StateCallback(){
                @Override
                public void onConfigured(CameraCaptureSession cameraCaptureSession) {
                    // The camera is already closed
                    if (cameraDevice == null) {
                        return;
                    }

                    // When the session is ready, we start displaying the preview.
                    cameraCaptureSessions = cameraCaptureSession;
                    updatePreview();
                }
                @Override
                public void onConfigureFailed(CameraCaptureSession cameraCaptureSession) {
                }
            }, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void openCamera() {
        CameraManager manager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        try {
            cameraId = manager.getCameraIdList()[0];
            characteristics = manager.getCameraCharacteristics(cameraId);

            StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);

            assert map != null;

            Size[] previewSizes = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP)
                                                 .getOutputSizes(SurfaceTexture.class);
            imageDimension = previewSizes[0]; // Default size

            Rect rect = characteristics.get(CameraCharacteristics.SENSOR_INFO_ACTIVE_ARRAY_SIZE);

            // Add permission for camera and let user grant the permission
            if (checkSelfPermission(Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                requestPermissions(new String[]{Manifest.permission.CAMERA}, REQUEST_CAMERA_PERMISSION);
                return;
            }

            manager.openCamera(cameraId, stateCallback, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    protected void updatePreview() {
        if (cameraDevice == null) {
            return;
        }

        // Autofocus on
        // captureRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
        
        // Autofocus off
        captureRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_OFF);

        captureRequestBuilder.set(CaptureRequest.CONTROL_MODE, CaptureRequest.CONTROL_MODE_AUTO);
        try {
            cameraCaptureSessions.setRepeatingRequest(captureRequestBuilder.build(), null, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void closeCamera() {
        if (cameraDevice != null) {
            cameraDevice.close();
            cameraDevice = null;
        }

        if (imageReaderGL != null) {
            imageReaderGL.close();
            imageReaderGL = null;
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode == REQUEST_CAMERA_PERMISSION) {
            if (grantResults[0] == PackageManager.PERMISSION_DENIED) {
                // Close the app
                finish();
            }
            else {
                // Open the camera
                openCamera();
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        startBackgroundThread();
    }

    @Override
    protected void onPause() {
        stopBackgroundThread();
        super.onPause();
    }

    @Override
    public boolean onTouchEvent(MotionEvent motionEvent) {
        int action = motionEvent.getActionMasked();
        
        switch(action) {
            case MotionEvent.ACTION_DOWN:
                nextPreviewMode(); // Change to next preview mode
                GLView.touch(currentPreviewMode);
                break;
            case MotionEvent.ACTION_MOVE:
                // Handle touch move event
                break;
            case MotionEvent.ACTION_UP:
                // Handle touch up event
                break;
            case MotionEvent.ACTION_CANCEL:
                // Handle touch cancel event
                break;
            default:
                break;
        }

        return true;
    }
}

class MyGLSurfaceView extends GLSurfaceView implements GLSurfaceView.Renderer {
    static {
        System.loadLibrary("motiondetector");
    }

    native public void init(int width, int height);
    native public void setCameraSettings(int width, int height);
    native public void draw();
    native public void touch(int previewMode);
    native public void processImageBuffers(ByteBuffer y, int ySize, int yPixelStride, int yRowStride, 
                                           ByteBuffer u, int uSize, int uPixelStride, int uRowStride, 
                                           ByteBuffer v, int vSize, int vPixelStride, int vRowStride);

    private Context mContext;

    public MyGLSurfaceView(Context context) {
        super(context);
        this.mContext = context;
        setEGLContextClientVersion(2);
        setRenderer(this);

        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        setPreserveEGLContextOnPause(true);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        init(this.getWidth(), this.getHeight());
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        // init(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        draw();
    }
}

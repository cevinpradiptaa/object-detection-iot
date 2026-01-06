# Smart Object Detection IoT
This project is an end-to-end computer vision and IoT system designed to perform real-time object detection and counting using YOLOv8. The system integrates ESP32-based edge devices with a Python AI server via TCP/MQTT communication and logs detection results to Excel and cloud dashboards.

## System Architecture
- ESP32-CAM → capture image/video
- ESP32 DevKit → controller & communication
- Python server → YOLO inference & counting
- Excel + Cloud → logging & monitoring

## Data Flow
1. ESP32 captures image
2. Image sent via TCP Socket
3. YOLO detects & counts objects
4. OCR identifies location label
5. Results logged to Excel
6. Data uploaded to cloud (Blynk / Drive)

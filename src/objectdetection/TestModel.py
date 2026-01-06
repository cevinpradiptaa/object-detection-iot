import cv2
import supervision as sv
from ultralytics import YOLO
import urllib.request
import numpy as np

def main():
    # modelegg = YOLO("D:/Kuliah/TUGAS AKHIR/KODINGAN/PYTHON/Test/runs/runs/detect/train3/weights/best.pt")
    # modelocr = YOLO("D:/Kuliah/TUGAS AKHIR/KODINGAN/PYTHON/runs/detect/train6/weights/best.pt")

    modelegg = YOLO("yolov8s-DeteksiTelur.pt")
    modelocr = YOLO("yolov8s-DeteksiOCRLabel.pt")

    # Initialize webcam
    #source_path = 'http://192.168.245.161/stream'  # Replace with your webcam URL
    source_path = "D:/Kuliah/TUGAS AKHIR/KODINGAN/PYTHON/Data Uji/DATAPENGUJIAN/UJI11.mp4"
    cap = cv2.VideoCapture(source_path)  # Use 0 for default webcam
    
    # Create a window for display
    cv2.namedWindow("YOLOv8 Prediction", cv2.WINDOW_NORMAL)
    
    while True:
        # Read frame from webcam
        # Read a frame from the video stream
        """ img_resp=urllib.request.urlopen(url)
        imgnp=np.array(bytearray(img_resp.read()),dtype=np.uint8)
        frame = cv2.imdecode(imgnp,-1) """
        # Read frame from webcam
        ret, frame = cap.read()
        if not ret:
            break
        # rotate_image = cv2.rotate(videocap, cv2.ROTATE_90_CLOCKWISE)
        # frame = rotate_image.copy()
        # Perform inference
        resultsegg = modelegg.predict(
            source=frame,
            conf=0.80, 
            device=0, 
        )

        resultsocr = modelocr.predict(
            source=frame,
            conf=0.65, 
            device=0, 
        )
        
        # Extract detection results
        resultegg = resultsegg[0]
        boxesegg = resultegg.boxes.xyxy.cpu().numpy()
        confidenceegg = resultegg.boxes.conf.cpu().numpy()
        class_idsegg = resultegg.boxes.cls.cpu().numpy().astype(int)

        resultocr = resultsocr[0]
        boxesocr = resultocr.boxes.xyxy.cpu().numpy()
        confidenceocr = resultocr.boxes.conf.cpu().numpy()
        class_idsocr = resultocr.boxes.cls.cpu().numpy().astype(int)
        
        # Create Detections object
        """ detections = sv.Detections(
            xyxy=boxes,
            confidence=confidence,
            class_id=class_ids
        ) """
        egg_count = 0
        # Annotate frame with detections using OpenCV
        for i, (box, conf, class_id) in enumerate(zip(boxesocr, confidenceocr, class_idsocr)):
                x1, y1, x2, y2 = box
                labelocr = f"{resultocr.names[int(class_id)]} {conf:.2f}"
            
                cv2.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)), (0, 255, 0), 2)
                label_size, _ = cv2.getTextSize(labelocr, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 1)
                cv2.rectangle(frame, (int(x1), int(y1) - label_size[1] - 10), 
                            (int(x1) + label_size[0], int(y1)), (0, 255, 0), -1)
                cv2.putText(frame, labelocr, (int(x1), int(y1) - 10), 
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 1)
                cv2.putText(frame, f"Nomor Kandang: {labelocr}", (200, 30),
                            cv2.FONT_HERSHEY_DUPLEX, 0.5, (255, 255, 255), 1)
        
        for i, (box, conf, class_id) in enumerate(zip(boxesegg, confidenceegg, class_idsegg)):
                x1, y1, x2, y2 = box
                labelegg = f"{resultegg.names[int(class_id)]} {conf:.2f}"
                    
                cv2.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)), (0, 255, 0), 2)
                label_size, _ = cv2.getTextSize(labelegg, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 1)
                cv2.rectangle(frame, (int(x1), int(y1) - label_size[1] - 10), 
                                            (int(x1) + label_size[0], int(y1)), (0, 255, 0), -1)
                cv2.putText(frame, labelegg, (int(x1), int(y1) - 10), 
                                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 1)
                egg_count = 1
        
        cv2.putText(frame, f"Telur Terdeteksi: {egg_count}", (10, 30), 
                    cv2.FONT_HERSHEY_DUPLEX, 0.5, (255, 255, 255), 1)
        
        cv2.rectangle(frame, (300, 700), (480, 720), (255, 255, 255), -1)

        # Display the annotated frame
        cv2.imshow("YOLOv8 Prediction", frame)
        
        # Break the loop if 'q' is pressed
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    
    # Release resources
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
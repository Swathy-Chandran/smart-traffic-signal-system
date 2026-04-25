# Smart Traffic Signal System using YOLOv8 and ESP32
# Real-time vehicle and pedestrian detection for dynamic signal control
# Author: Swathy Chandran N

import cv2 
import serial 
import time 
from ultralytics import YOLO 
# Change COM port here 
esp = serial.Serial("COM9", 115200, timeout=1) 
time.sleep(2) 
model = YOLO("yolov8n.pt") 
cap = cv2.VideoCapture(1)
print("Smart Traffic AI System Started...") 
while True: 
  ret, frame = cap.read() 
  if not ret: 
    break 
  # Resize AFTER reading frame 
  frame = cv2.resize(frame, (640, 480)) 
  results = model(frame, conf=0.3) 
  height, width, _ = frame.shape 
  lane_counts = [0, 0, 0, 0] 
  lane_pedestrian = [False, False, False, False] 
  for result in results: 
    for box in result.boxes: 
      cls = int(box.cls[0]) 
      label = model.names[cls] 
      x1, y1, x2, y2 = box.xyxy[0] 
      center_x = int((x1 + x2) / 2) 
      center_y = int((y1 + y2) / 2) 
      # Determine lane 
      if center_x < width/2 and center_y < height/2: 
        lane = 0 
      elif center_x >= width/2 and center_y < height/2: 
        lane = 1 
      elif center_x < width/2 and center_y >= height/2: 
        lane = 2 
      else: 
        lane = 3 
      # Vehicle detection 
      if label in ["car", "bus", "truck", "motorcycle"]: 
        lane_counts[lane] += 1 
      # Pedestrian detection 
      if label == "person": 
        lane_pedestrian[lane] = True 
    # Remove lanes with pedestrians 
    adjusted_counts = [] 
    for i in range(4): 
      if lane_pedestrian[i]: 
        adjusted_counts.append(-1) # Block lane 
      else:
        adjusted_counts.append(lane_counts[i]) 
    active_lane = adjusted_counts.index(max(adjusted_counts)) 
    print("Lane counts:", lane_counts) 
    print("Pedestrian detected:", lane_pedestrian) 
    print("Active lane:", active_lane) 
    # Send to ESP32 
    esp.write(f"{active_lane}\n".encode()) 
    annotated_frame = results[0].plot() 
    cv2.imshow("Smart Traffic Detection", annotated_frame) 
    if cv2.waitKey(1) & 0xFF == ord('q'): 
      break 
cap.release() 
cv2.destroyAllWindows()

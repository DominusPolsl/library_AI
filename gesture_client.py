import cv2
import mediapipe as mp
import socket
import time

# Налаштування TCP-з'єднання
HOST = '127.0.0.1'
PORT = 9999

# Ініціалізація MediaPipe
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.75)
mp_draw = mp.solutions.drawing_utils

# Відправка команди в Qt
def send_command(command):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))
            s.sendall(command.encode('utf-8'))
    except Exception as e:
        print("Connection error:", e)

# Вебкамера
cap = cv2.VideoCapture(0)
prev_time = 0
prev_mid_x = None  # координата пальця №12

while True:
    success, frame = cap.read()
    if not success:
        break

    frame = cv2.flip(frame, 1)
    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    result = hands.process(frame_rgb)

    if result.multi_hand_landmarks:
        for handLms, handedness in zip(result.multi_hand_landmarks, result.multi_handedness):
            label = handedness.classification[0].label
            if label != "Right":
                continue  # тільки права рука

            mp_draw.draw_landmarks(frame, handLms, mp_hands.HAND_CONNECTIONS)

            # Перевірка відкритої руки (5 пальців)
            fingers_up = 0
            tip_ids = [4, 8, 12, 16, 20]
            for id in [8, 12, 16, 20]:  # без 4 (великий палець)
                if handLms.landmark[id].y < handLms.landmark[id - 2].y:
                    fingers_up += 1

            # Якщо рука відкрита — відстежуємо рух середнього пальця
            if fingers_up == 4:
                mid_x = handLms.landmark[12].x
                if prev_mid_x is not None:
                    delta_x = mid_x - prev_mid_x
                    if abs(delta_x) > 0.02 and time.time() - prev_time > 1.0:
                        if delta_x < 0:
                            print("➡ Рух пальця вліво — наступна сторінка")
                            send_command("next")
                        elif delta_x > 0:
                            print("⬅ Рух пальця вправо — попередня сторінка")
                            send_command("prev")
                        prev_time = time.time()
                prev_mid_x = mid_x
            else:
                prev_mid_x = None

    cv2.imshow("Gesture Control", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()

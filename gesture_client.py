import cv2
import mediapipe as mp
import socket
import time
import math

# TCP-з'єднання
HOST = '127.0.0.1'
PORT = 9999

mp_hands = mp.solutions.hands
hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.75)
mp_draw = mp.solutions.drawing_utils

def send_command(command):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))
            s.sendall(command.encode('utf-8'))
    except Exception as e:
        print("Connection error:", e)

def distance(lm1, lm2):
    return math.sqrt((lm1.x - lm2.x) ** 2 + (lm1.y - lm2.y) ** 2)

cap = cv2.VideoCapture(0)

prev_time = 0
open_hand_active = False
prev_seek_x = None
prev_track_x = None
prev_mid_x = None
last_volume_change_time = 0
last_action_time = 0
current_mode = "menu"
last_mode_check = 0
mode_check_interval = 1.0
menu_gesture_start_time = None
previous_finger_count = -1
left_open_start_time = None

while True:
    success, frame = cap.read()
    if not success:
        break

    frame = cv2.flip(frame, 1)
    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    result = hands.process(frame_rgb)

    now = time.time()

    if result.multi_hand_landmarks:
        for handLms, handedness in zip(result.multi_hand_landmarks, result.multi_handedness):
            label = handedness.classification[0].label

            mp_draw.draw_landmarks(frame, handLms, mp_hands.HAND_CONNECTIONS)
            # === ВИХІД ДО МЕНЮ — ЛІВА РУКА
            if label == "Left":
                left_thumb_up  = handLms.landmark[4].y < handLms.landmark[3].y
                left_index_up  = handLms.landmark[8].y < handLms.landmark[6].y
                left_middle_up = handLms.landmark[12].y < handLms.landmark[10].y
                left_ring_up   = handLms.landmark[16].y < handLms.landmark[14].y
                left_pinky_up  = handLms.landmark[20].y < handLms.landmark[18].y

                left_fingers_up = sum([
                    left_thumb_up, left_index_up, left_middle_up, left_ring_up, left_pinky_up
                ])

                if left_fingers_up == 5:
                    if left_open_start_time is None:
                        left_open_start_time = now
                    elif now - left_open_start_time > 3.0:
                        print("Вихід у головне меню (ліва відкрита рука)")
                        send_command("go_menu")
                        left_open_start_time = None
                else:
                    left_open_start_time = None
                continue  # Пропускаємо решту, якщо це ліва рука

            if label != "Right":
                continue  # Пропускаємо всі, хто не права рука

            def is_up(tip, pip):
                return handLms.landmark[tip].y < handLms.landmark[pip].y

            thumb_tip = handLms.landmark[4]
            index_base = handLms.landmark[5]
            pinky_tip = handLms.landmark[20]
            thumb_near_pinky = distance(thumb_tip, pinky_tip) < 0.07

            thumb_up   = is_up(4, 3)
            index_up   = is_up(8, 6)
            middle_up  = is_up(12, 10)
            ring_up    = is_up(16, 14)
            pinky_up   = is_up(20, 18)
            fingers_up = sum([thumb_up, index_up, middle_up, ring_up, pinky_up])

            # === 1. Play/Pause
            if all([thumb_up, index_up, middle_up, ring_up, pinky_up]) and thumb_tip.x < handLms.landmark[6].x:
                if not open_hand_active and now - last_action_time > 0.5:
                    print("toggle play/pause")
                    send_command("toggle_play_pause")
                    open_hand_active = True
                    last_action_time = now
            else:
                open_hand_active = False

            # === 2. Перемотка
            if index_up and middle_up and not ring_up:
                if thumb_near_pinky:
                    gesture_x = handLms.landmark[8].x
                    if prev_seek_x is not None:
                        delta = gesture_x - prev_seek_x
                        if abs(delta) > 0.03 and now - last_action_time > 0.5:
                            if delta > 0:
                                print("Перемотка вперед")
                                send_command("fast_forward")
                            else:
                                print("Перемотка назад")
                                send_command("rewind")
                            last_action_time = now
                    prev_seek_x = gesture_x
                else:
                    prev_seek_x = None
            else:
                prev_seek_x = None

            # === 3. Наступний / попередній трек
            if index_up and middle_up and ring_up and thumb_near_pinky:
                gesture_x = handLms.landmark[8].x
                if prev_track_x is not None:
                    delta = gesture_x - prev_track_x
                    if abs(delta) > 0.03 and now - last_action_time > 0.5:
                        if delta > 0:
                            print("Наступний трек")
                            send_command("next_track")
                        else:
                            print("Попередній трек")
                            send_command("prev_track")
                        last_action_time = now
                prev_track_x = gesture_x
            else:
                prev_track_x = None

            # === 4. MENU Gesture
            if current_mode == "menu":
                thumb_tip_y = handLms.landmark[4].y
                index_tip_y = handLms.landmark[8].y
                middle_tip_y = handLms.landmark[12].y
                thumb_up_effective = thumb_tip_y < min(index_tip_y, middle_tip_y)

                fingers_up = sum([
                    index_up, middle_up, ring_up, pinky_up, thumb_up_effective
                ])

                if fingers_up in [1, 2, 3, 4]:
                    if previous_finger_count != fingers_up:
                        previous_finger_count = fingers_up
                        menu_gesture_start_time = now
                    elif menu_gesture_start_time and (now - menu_gesture_start_time > 1):
                        if fingers_up == 1:
                            print("Media Player обрано")
                            send_command("open_media")
                        elif fingers_up == 2:
                            print("Text Viewer обрано")
                            send_command("open_text")
                        elif fingers_up == 3:
                            print("Image Viewer обрано")
                            send_command("open_image")
                        elif fingers_up == 4:
                            print("Camera обрано")
                            send_command("open_camera")
                        menu_gesture_start_time = None
                else:
                    previous_finger_count = -1
                    menu_gesture_start_time = None

            # === 5. Гучність
            only_thumb_up = thumb_up and not any([index_up, middle_up, ring_up, pinky_up])
            thumb_folded = distance(thumb_tip, index_base) < 0.1
            min_other_y = min([handLms.landmark[8].y, handLms.landmark[12].y, handLms.landmark[16].y, handLms.landmark[20].y])
            thumb_lower_than_all = thumb_tip.y > min_other_y
            thumb_horizontal = abs(thumb_tip.y - pinky_tip.y) < 0.05 and thumb_tip.x < handLms.landmark[0].x
            thumb_higher_than_all = thumb_tip.y < min_other_y

            if only_thumb_up and not thumb_folded:
                if (thumb_lower_than_all or thumb_horizontal) and now - last_volume_change_time > 0.5 and now - last_action_time > 0.5:
                    print("Зменшення гучності")
                    send_command("volume_down")
                    last_volume_change_time = now
                    last_action_time = now
                elif thumb_higher_than_all and now - last_volume_change_time > 0.5 and now - last_action_time > 0.5:
                    print("Збільшення гучності")
                    send_command("volume_up")
                    last_volume_change_time = now
                    last_action_time = now

            # === 6. Image Viewer
            if pinky_up and fingers_up <= 2:
                mid_x = handLms.landmark[12].x
                mid_y = handLms.landmark[12].y

                if now - last_action_time > 0.5:
                    if mid_x < 0.35:
                        print("Pan Left")
                        send_command("pan_left")
                    elif mid_x > 0.65:
                        print("Pan Right")
                        send_command("pan_right")
                    elif mid_y < 0.35:
                        print("Pan Up")
                        send_command("pan_up")
                    elif mid_y > 0.65:
                        print("Pan Down")
                        send_command("pan_down")
                    last_action_time = now

            elif fingers_up <= 2:
                thumb_tip = handLms.landmark[4]
                index_tip = handLms.landmark[8]
                dist = distance(thumb_tip, index_tip)

                if dist > 0.15 and now - last_action_time > 0.75:
                    print("Zoom In")
                    send_command("zoom_in")
                    last_action_time = now
                elif dist < 0.04 and now - last_action_time > 0.75:
                    print("Zoom Out")
                    send_command("zoom_out")
                    last_action_time = now

            # === 7. PDF сторінки
            if index_up and middle_up and ring_up and pinky_up and thumb_up:
                mid_x = handLms.landmark[12].x
                if prev_mid_x is not None:
                    delta_x = mid_x - prev_mid_x
                    if abs(delta_x) > 0.02 and now - last_action_time > 0.5:
                        if delta_x < 0:
                            print("Наступна сторінка")
                            send_command("next")
                        elif delta_x > 0:
                            print("Попередня сторінка")
                            send_command("prev")
                        last_action_time = now
                prev_mid_x = mid_x
            else:
                prev_mid_x = None

    cv2.imshow("Gesture Control", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()

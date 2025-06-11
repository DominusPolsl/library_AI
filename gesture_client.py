import cv2                      # OpenCV — biblioteka do przetwarzania obrazu i wideo
import mediapipe as mp          # MediaPipe — rozpoznawanie dłoni, twarzy
import socket                   # socket — do komunikacji sieciowej (TCP z serwerem gestów)
import time                     
import math                     

# Konfiguracja połączenia TCP
HOST = '127.0.0.1'              # Adres IP lokalnego hosta (serwera)
PORT = 9999                    # Port, na którym nasłuchuje serwer gestów

# Inicjalizacja komponentów MediaPipe do rozpoznawania dłoni
mp_hands = mp.solutions.hands  # Moduł rozpoznawania dłoni
hands = mp_hands.Hands(
    max_num_hands=1,           # Tylko jedna dłoń będzie analizowana na raz
    min_detection_confidence=0.75  # Minimalna pewność rozpoznania 
)
mp_draw = mp.solutions.drawing_utils  # Narzędzia do rysowania szkieletu dłoni

# Funkcja pomocnicza: wysyła tekstową komendę przez socket do serwera Qt
def send_command(command):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))               # Nawiązanie połączenia z serwerem
            s.sendall(command.encode('utf-8'))    # Wysyłanie komendy jako ciąg bajtów
    except Exception as e:
        print("Connection error:", e)             # Obsługa błędu połączenia

# Funkcja pomocnicza: oblicza odległość między dwoma landmarkami dłoni
def distance(lm1, lm2):
    return math.sqrt((lm1.x - lm2.x) ** 2 + (lm1.y - lm2.y) ** 2)
    # Odległość euklidesowa między dwoma punktami w przestrzeni 2D (x, y)

# Inicjalizacja kamery (domyślna kamera 0)
cap = cv2.VideoCapture(0)

prev_time = 0                    # Poprzedni czas ramki 
open_hand_active = False         # Czy gest otwartej dłoni jest aktywny
prev_seek_x = None               # Poprzednia pozycja X do przewijania (seek)
prev_track_x = None              # Poprzednia pozycja X do zmiany utworu
prev_mid_x = None                # Poprzednia pozycja środkowego palca (dla PDF)
last_volume_change_time = 0      # Czas ostatniej zmiany głośności
last_action_time = 0             # Czas ostatniej wykonanej akcji (ogólny cooldown)
current_mode = "menu"            # Tryb działania: np. 'menu' albo 'player'
last_mode_check = 0              # Ostatni moment sprawdzania trybu
mode_check_interval = 1.0        # Odstęp czasu między sprawdzeniami trybu
menu_gesture_start_time = None   # Czas rozpoczęcia gestu menu
previous_finger_count = -1       # Poprzednia liczba palców w górze (do wykrycia zmiany)
left_open_start_time = None      # Moment rozpoczęcia gestu „otwarta lewa dłoń”


while True:
    # Odczyt jednej klatki z kamery (success = True jeśli się udało)
    success, frame = cap.read()
    if not success:
        break  # Jeśli nie udało się odczytać — zakończ pętlę

    # Poziome odbicie obrazu, tak aby dłoń wyglądała jak w odbiciu lustrzanym
    frame = cv2.flip(frame, 1)

    # Konwersja koloru z BGR (OpenCV) do RGB (MediaPipe oczekuje RGB)
    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # Przekazanie ramki do detekcji dłoni przez MediaPipe
    result = hands.process(frame_rgb)

    # Aktualny czas (sekundy od uruchomienia programu)
    now = time.time()

    # Jeśli w ramce wykryto jakiekolwiek dłonie:
    if result.multi_hand_landmarks:
        # Dla każdej wykrytej dłoni (z jej klasyfikacją lewej/prawej)
        for handLms, handedness in zip(result.multi_hand_landmarks, result.multi_handedness):
            label = handedness.classification[0].label  # "Left" albo "Right"

            # Rysowanie szkieletu dłoni na obrazie (dla debugowania i wizualizacji)
            mp_draw.draw_landmarks(frame, handLms, mp_hands.HAND_CONNECTIONS)

            # Otwarta lewa dłoń przez 3 sekundy - powrót do menu
            if label == "Left":
                # Sprawdzenie, które palce lewej dłoni są wyprostowane
                left_thumb_up  = handLms.landmark[4].y < handLms.landmark[3].y
                left_index_up  = handLms.landmark[8].y < handLms.landmark[6].y
                left_middle_up = handLms.landmark[12].y < handLms.landmark[10].y
                left_ring_up   = handLms.landmark[16].y < handLms.landmark[14].y
                left_pinky_up  = handLms.landmark[20].y < handLms.landmark[18].y

                # Liczba palców w górze lewej dłoni
                left_fingers_up = sum([
                    left_thumb_up, left_index_up, left_middle_up, left_ring_up, left_pinky_up
                ])

                # Jeśli wszystkie 5 palców lewej dłoni są podniesione:
                if left_fingers_up == 5:
                    # Jeśli dopiero rozpoczęto gest — zapamiętaj czas
                    if left_open_start_time is None:
                        left_open_start_time = now
                    # Jeśli gest trwa już dłużej niż 3 sekundy
                    elif now - left_open_start_time > 3.0:
                        print("Return to menu")
                        send_command("go_menu")  # Wysłanie komendy do Qt
                        left_open_start_time = None
                else:
                    # Jeśli dłoń przestała być otwarta — zresetuj licznik
                    left_open_start_time = None

                # Ponieważ przetwarzamy tylko jedną dłoń na raz — pomijamy resztę pętli
                continue

            # Pomijamy dłoń, jeśli nie jest prawą (bo dalsze gesty dotyczą prawej)
            if label == "Right":

                # Funkcja pomocnicza — sprawdza, czy dany palec jest w górze
                def is_up(tip, pip):
                    # Palec w górze = koniec palca wyżej niż jego staw środkowy
                    return handLms.landmark[tip].y < handLms.landmark[pip].y

                # Landmarki używane do gestów
                thumb_tip = handLms.landmark[4]       # Koniec kciuka
                index_base = handLms.landmark[5]      # Podstawa palca wskazującego
                pinky_tip = handLms.landmark[20]      # Koniec małego palca

                # Czy kciuk znajduje się blisko małego palca (do gestów przesuwania)
                thumb_near_pinky = distance(thumb_tip, pinky_tip) < 0.07

                # Stan każdego z palców prawej dłoni
                thumb_up   = is_up(4, 3)
                index_up   = is_up(8, 6)
                middle_up  = is_up(12, 10)
                ring_up    = is_up(16, 14)
                pinky_up   = is_up(20, 18)

                # Liczba podniesionych palców ogółem
                fingers_up = sum([thumb_up, index_up, middle_up, ring_up, pinky_up])


                     # Odtwarzanie - otwarta dłoń z kciukiem po lewej stronie
                if all([thumb_up, index_up, middle_up, ring_up, pinky_up]) and thumb_tip.x < handLms.landmark[6].x:
                    # Jeśli gest jeszcze nieaktywny i minęło co najmniej 0.5 sekundy od ostatniej akcji:
                    if not open_hand_active and now - last_action_time > 0.5:
                        print("Play/Pause")
                        send_command("toggle_play_pause")  # Wysyłamy komendę do Qt
                        open_hand_active = True
                        last_action_time = now
                else:
                    # Jeśli dłoń nie jest już otwarta — resetujemy flagę
                    open_hand_active = False

                # Przewijanie — dwa palce (wskazujący i środkowy) w górze, kciuk przy małym
                if index_up and middle_up and not ring_up:
                    if thumb_near_pinky:
                        gesture_x = handLms.landmark[8].x  # Pozycja X palca wskazującego
                        if prev_seek_x is not None:
                            delta = gesture_x - prev_seek_x
                            if abs(delta) > 0.03 and now - last_action_time > 0.5:
                                if delta > 0:
                                    print("forward")
                                    send_command("fast_forward")
                                else:
                                    print("rewind")
                                    send_command("rewind")
                                last_action_time = now
                        prev_seek_x = gesture_x
                    else:
                        prev_seek_x = None
                else:
                    prev_seek_x = None

                # Następny/poprzedni utwor — trzy palce + kciuk przy małym palcu
                if index_up and middle_up and ring_up and thumb_near_pinky:
                    gesture_x = handLms.landmark[8].x
                    if prev_track_x is not None:
                        delta = gesture_x - prev_track_x
                        if abs(delta) > 0.03 and now - last_action_time > 0.5:
                            if delta > 0:
                                print("Next track")
                                send_command("next_track")
                            else:
                                print("Previous track")
                                send_command("prev_track")
                            last_action_time = now
                    prev_track_x = gesture_x
                else:
                    prev_track_x = None

                # Wybor opcji w menu
                if current_mode == "menu":
                    thumb_tip_y = handLms.landmark[4].y
                    index_tip_y = handLms.landmark[8].y
                    middle_tip_y = handLms.landmark[12].y

                    # Kciuk uznany za „w górze” jeśli wyżej niż oba palce obok
                    thumb_up_effective = thumb_tip_y < min(index_tip_y, middle_tip_y)

                    # Liczba palców uniesionych (uwzględniając efekt kciuka)
                    fingers_up = sum([
                        index_up, middle_up, ring_up, pinky_up, thumb_up_effective
                    ])

                    # Jeśli liczba palców 1–4 (czyli aktywne menu)
                    if fingers_up in [1, 2, 3, 4]:
                        if previous_finger_count != fingers_up:
                            previous_finger_count = fingers_up
                            menu_gesture_start_time = now
                        elif menu_gesture_start_time and (now - menu_gesture_start_time > 1):
                            # Po 1 sekundzie aktywacji — wybór aplikacji
                            if fingers_up == 1:
                                print("Media Player chosen")
                                send_command("open_media")
                            elif fingers_up == 2:
                                print("Text Viewer chosen")
                                send_command("open_text")
                            elif fingers_up == 3:
                                print("Image Viewer chosen")
                                send_command("open_image")
                            elif fingers_up == 4:
                                print("Camera chosen")
                                send_command("open_camera")
                            menu_gesture_start_time = None
                    else:
                        # Reset gdy liczba palców się zmienia lub gest znika
                        previous_finger_count = -1
                        menu_gesture_start_time = None

                # Zmiana głosności
                only_thumb_up = thumb_up and not any([index_up, middle_up, ring_up, pinky_up])
                thumb_folded = distance(thumb_tip, index_base) < 0.1

                # Pozycje Y pozostałych palców — do porównania
                min_other_y = min([
                    handLms.landmark[8].y, handLms.landmark[12].y,
                    handLms.landmark[16].y, handLms.landmark[20].y
                ])
                thumb_lower_than_all = thumb_tip.y > min_other_y
                thumb_horizontal = abs(thumb_tip.y - pinky_tip.y) < 0.05 and thumb_tip.x < handLms.landmark[0].x
                thumb_higher_than_all = thumb_tip.y < min_other_y

                if only_thumb_up and not thumb_folded:
                    if (thumb_lower_than_all or thumb_horizontal) and now - last_volume_change_time > 0.5:
                        print("Volume down")
                        send_command("volume_down")
                        last_volume_change_time = now
                        last_action_time = now
                    elif thumb_higher_than_all and now - last_volume_change_time > 0.5:
                        print("Volume up")
                        send_command("volume_up")
                        last_volume_change_time = now
                        last_action_time = now

                # Sterowanie obrazem (pan/zoom) — Image Viewer
                if pinky_up and fingers_up <= 2:
                    mid_x = handLms.landmark[12].x
                    mid_y = handLms.landmark[12].y

                    if now - last_action_time > 0.5:
                        if mid_x < 0.35:
                            print("Pan left")
                            send_command("pan_left")
                        elif mid_x > 0.65:
                            print("Pan right")
                            send_command("pan_right")
                        elif mid_y < 0.35:
                            print("Pan up")
                            send_command("pan_up")
                        elif mid_y > 0.65:
                            print("Pan down")
                            send_command("pan_down")
                        last_action_time = now

                # === GEST ZOOM: odległość kciuka i palca wskazującego ===
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

                # === GEST 7: PRZEWIJANIE STRON PDF (pełna otwarta dłoń) ===
                if index_up and middle_up and ring_up and pinky_up and thumb_up:
                    mid_x = handLms.landmark[12].x
                    if prev_mid_x is not None:
                        delta_x = mid_x - prev_mid_x
                        if abs(delta_x) > 0.02 and now - last_action_time > 0.5:
                            if delta_x < 0:
                                print("Next page")
                                send_command("next")
                            elif delta_x > 0:
                                print("Previous page")
                                send_command("prev")
                            last_action_time = now
                    prev_mid_x = mid_x
                else:
                    prev_mid_x = None

    # Wyświetlenie obrazu w osobnym oknie o nazwie "Gesture Control"
    scale = 0.66  # 66% oryginału
    resized_frame = cv2.resize(frame, None, fx=scale, fy=scale, interpolation=cv2.INTER_AREA)

    cv2.imshow("Gesture Control", resized_frame)
    # Funkcja `imshow()` pokazuje klatkę (z narysowanymi dłońmi) w czasie rzeczywistym

    # === Sprawdzenie, czy użytkownik nacisnął klawisz 'q' na klawiaturze ===
    # `waitKey(1)` czeka 1 milisekundę na naciśnięcie klawisza.
    # Jeśli wciśnięto 'q' → przerwij pętlę while
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Zwolnienie zasobów kamery po zakończeniu działania programu
cap.release()  # Zamyka połączenie z kamerą

# Zamknięcie wszystkich otwartych okien OpenCV
cv2.destroyAllWindows()  # Usuwa okno „Gesture Control” i inne potencjalne okna
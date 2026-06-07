
  RAM (CPU)                        
  - Obsługa okna i zdarzeń (GLFW)                       
  - Maszyna stanów fotonu i rejestracja pozycji (trail) 
  - Zarządzanie kamerą i kreator obiektów               

  VRAM (GPU)                        
  - Metal Compute Pipeline (shader.metal)               
  - Ray Marching: Śledzenie milionów promieni           
  - Generowanie proceduralnego tła (szum gwiazd)        
  - Rendering HUD za pomocą Fontu Bitmapowego           




### 1. Logika Procesora (`main.mm`)
Plik wejściowy aplikacji napisany w Objective-C++ zarządza całym cyklem życia programu:
* **Inicjalizacja i Okno:** Konfiguracja kontekstu Metal API oraz powiązanie go z oknem utworzonym przez bibliotekę GLFW poprzez warstwę `CAMetalLayer`.
* **Bufor Kołowy Śladu:** CPU śledzi pozycję wystrzelonego fotonu i rejestruje jego współrzędne w świecie do dedykowanej tablicy `trail[200]`, która w każdej klatce jest przesyłana jako bufor współdzielony (`MTL::ResourceStorageModeShared`) do shadera.
* **Pętla Zdarzeń:** Asynchroniczne przechwytywanie wejść klawiatury z systemem zapobiegania wielokrotnemu rejestrowaniu pojedynczego kliknięcia (*debouncing*).

### 2. Potok Graficzny (`shader.metal`)
Główny shader obliczeniowy (Compute Kernel) wykonuje operacje niezależnie dla każdego piksela ekranu:
* **Proceduralny Szum Kosmiczny:** Zamiast ładowania ciężkich tekstur z dysku, niebo generowane jest matematycznie za pomocą algorytmu pseudolosowego `hash31`, co drastycznie skraca czas uruchamiania i oszczędza pamięć VRAM.
* **Optymalizacja "Rury Światła":** Aby renderowanie długiego ogona fotonu nie blokowało GPU, shader szuka najmniejszej odległości promienia od całego zarejestrowanego szlaku (próbkowanie z krokiem adaptacyjnym), co zapobiega dławieniu potoku graficznego.

---

##  Podstawy Matematyczne i Fizyczne

Wizualizacja opiera się na numerycznym rozwiązaniu równania ruchu w metryce Schwarzschilda, która opisuje geometrię czasoprzestrzeni wokół nieobracającej się, sferycznej masy.

### 1. Zakrzywienie Toru Promienia (Ray Marching)
Dla każdego piksela generowany jest promień o pozycji $P$ i kierunku $D$. W każdym kroku pętli `max_steps` sprawdzany jest wektor odległości do centrum czarnej dziury $r = P_{bh} - P$. Przyspieszenie relatywistyczne wpływające na zmianę kierunku wektora światła ($D$) wyliczane jest ze wzoru:

$$\vec{a} = - \frac{1.5 \cdot R_s \cdot h^2}{d^5} \cdot \vec{r}$$

Gdzie:
* $R_s$ to promień Schwarzschilda (promień horyzontu zdarzeń).
* $d$ to odległość od osobliwości (`length(to_bh)`).
* $h^2$ to kwadrat momentu pędu promienia (`length_squared(cross(-r, dir))`).

### 2. Warunki Brzegowe Pętli Shadera
W każdym kroku całkowania (`step_size = 0.1f`) sprawdzane są trzy stany:
1.  **Pochłonięcie (Hit):** Jeśli odległość $d < R_s \cdot 1.001$, promień wpada pod horyzont zdarzeń – zwracany jest kolor czarny (osobliwość).
2.  **Kolizja z Materią:** Jeśli promień przetnie płaszczyznę równikową ($Y=0$) w odległości pomiędzy `disk_inner` a `disk_outer`, obliczany jest kolor dysku akrecyjnego (interpolowany tonalnie w zależności od odległości).
3.  **Ucieczka (Wielki Kosmos):** Jeśli odległość przekroczy bezpieczną granicę ucieczki (`length(cur_pos) > 120.0f`), pętla zostaje przerwana, a piksel przyjmuje kolor proceduralnej gwiazdy.

---

##  Sterowanie i Interakcja w Aplikacji

Po uruchomieniu programu, konsola poprosi o konfigurację parametrów startowych. Wciśnięcie klawisza `Enter` zatwierdza zoptymalizowane wartości domyślne.

| Klawisz / Skrót | Powiązane Działanie w Symulacji | Tryb Kamery |
| :--- | :--- | :--- |
| **`TAB`** | Przełączanie trybu kamery: **Orbitalny** $\leftrightarrow$ **Free-Fly** | Oba |
| **`W`, `S`, `A`, `D`** | Obrót wokół centrum (Orbit) / Ruch przód-tył-boki (Free-Fly) | Oba |
| **`J` / `K`** | Przybliżenie / Oddalenie kamery od czarnej dziury (Zoom) | Orbitalny |
| **`STRZAŁKI`** | Rozglądanie się (obracanie głowy pasażera statku) | Free-Fly |
| **`SPACJA`** | **Maszyna Stanów Fotonu:** Strzał $\rightarrow$ Pauza $\rightarrow$ Wznowienie | Oba |
| **`R`** | Całkowity reset fotonu i wyczyszczenie śladu świetlnego | Oba |
| **`P`** | Dynamiczne utworzenie nowej planety dokładnie przed kamerą | Oba |
| **`+` / `-`** | Zwiększanie / Zmniejszanie masy ($R_s$) czarnej dziury na żywo | Oba |
| **`1` / `2`** | Włączenie/Wyłączenie zielonej siatki czasu oraz planet | Oba |

---

##  Instrukcja Kompilacji i Uruchomienia

Projekt nie posiada żadnych zewnętrznych zależności pakietowych oprócz natywnych bibliotek systemu macOS (Metal framework) oraz menedżera okien GLFW.

### Wymagania systemowe:
* Komputer Mac z procesorem Apple Silicon (M1, M2, M3 lub nowsze) lub dedykowaną kartą graficzną wspierającą Metal.
* Zainstalowane narzędzia deweloperskie Xcode Command Line Tools (`xcode-select --install`).
* Biblioteka GLFW (możliwa do zainstalowania przez `brew install glfw`).

### Automatyczna Kompilacja (Zalecane):
Najprostszą metodą uruchomienia projektu jest użycie komendy `make`. Wystarczy otworzyć terminal w głównym folderze projektu i wpisać:

```bash
make run

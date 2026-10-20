# KubiSat — scenariusz prezentacji (10–15 min)

**Tytuł pracy:** Budowa i działanie amatorskiego mikrosatelity w formacie CubeSat  
**Projekt:** KubiSat  
**Kontekst:** praca inżynierska → przygotowanie do magistarium / projekt przejściowy  
**Czas:** ok. 12–14 minut przy spokojnym tempie  

**Rekwizyty:**

- działający model satelity
- laptop z donglem LoRa (stacja naziemna) i oprogramowaniem Operator
- świeży ślad GPS z drogi dom → uczelnia (przed prezentacją)

---

## Przed wejściem (checklist, 5–10 min wcześniej)

1. Model włączony, najlepiej **na baterii** (pokazuje autonomię).
2. Dongle / ground station podłączony, port widoczny w Operatorze.
3. Operator otwarty, połączenie serial OK — widać ruch / beacon zdrowia (jeśli włączony).
4. Ślad GPS z drogi **już zaciągnięty** (CSV/KML) i **otwarty w tle** albo gotowy do importu jednym kliknięciem — nie parsuj pliku na oczach sali.
5. Plan B: screenshot / stary KML z `software/kubisat_firmware/telemetry_test/` na wypadek utraty radia w sali.

**Demo na żywo = max 3–4 min łącznie.** Reszta to narracja. Nie rób „wycieczki po wszystkich komendach”.

---

## 0. Otwarcie + pokaz sprzętu (ok. 1 min)

*(trzymasz model albo kładziesz na stole)*

Dzień dobry. Nazywam się Jakub Gredzicki. Chciałbym przedstawić projekt mojej pracy inżynierskiej: **KubiSat** — amatorski model mikrosatelity w formacie CubeSat 1U.

To nie jest tylko opis w PDF. Mam ze sobą **działający model** oraz **stację naziemną**: laptop, dongle LoRa i aplikację Operator. Dzisiaj opowiem: **co to jest, dlaczego powstało, jak działa** — i na końcu pokażę **co zrobił dziś rano w drodze na uczelnię**.

---

## 1. Co to jest? (ok. 1,5–2 min)

*(model w ręku)*

KubiSat to **otwartoźródłowy model piko-/nanosatelity** inspirowany standardem CubeSat — kostką ok. 10×10×11 cm, czyli formatem 1U popularnym w projektach uczelnianych i amatorskich.

Projekt nie jest „zabawką elektroniczną” w izolacji. Składa się z trzech warstw:

1. **Hardware** — trzy własne PCB (zasilanie, sterowanie, czujniki), obudowa drukowana 3D, panele fotowoltaiczne, akumulatory 18650.
2. **Firmware satelity** — C/C++ na mikrokontrolerze **RP2040** (Raspberry Pi Pico), z LoRa, GPS, telemetrią i logowaniem na kartę SD.
3. **Segment naziemny** — firmware mostka LoRa↔UART oraz aplikacja **kubisat_operator** (Avalonia/.NET) do sterowania i podglądu danych.

Całość jest w repozytorium open-source (MIT): ktoś może zbudować ten sam model, zmodyfikować PCB albo rozszerzyć protokół.

Ważne doprecyzowanie: to **nie jest satelita orbitalny „space-grade”**. Cel inżynierski to **lot stratosferyczny (HAB)** i walidacja architektury — komunikacji, zasilania, pomiarów i oprogramowania w warunkach zbliżonych do misji: zimno, zasięg, autonomia, odzyskanie po lądowaniu dzięki GPS.

*(opcjonalnie: obróć model, wskaż antenę / panele jednym zdaniem — bez listy chipów)*

---

## 2. Dlaczego? (ok. 1,5–2 min)

Kontekst branżowy jest prosty: koszty wyniesienia ładunku nadal liczy się w dolarach za kilogram, więc małe satelity — CubeSaty 1U/3U — stały się realną ścieżką dla uczelni i małych zespołów. Jednocześnie gotowe moduły „space” są drogie; przy ograniczonym budżecie studenckim sensowniejsze było **zbudować model od zera** z dostępnych komponentów konsumenckich, ale dobranych pod szerszy zakres temperatur i wysokość GPS.

**Cel pracy:** opracować i wykonać od podstaw model zdolny do lotu balonem stratosferycznym — podstawowe pomiary + komunikacja na odległość.

Motywacja osobista: zainteresowanie elektroniką, systemami wbudowanymi i technologiami kosmicznymi; chęć przejścia całej ścieżki — od wymagań, przez schematy KiCad i STL, po firmware multicore i GUI operatora.

Zakres był świadomie trójdzielny:

- dokumentacja (wymagania, schematy, model danych),
- software (satelita + stacja naziemna + testy),
- hardware (PCB, montaż, testy sprzętu).

Efekt: **operacyjny model** — pomiary, half-duplex radio, sterowanie i odczyt telemetrii ze stacji naziemnej.

Wymagania funkcjonalne w skrócie:

- telemetria na żądanie (jednorazowo lub cyklicznie),
- zdalna zmiana konfiguracji,
- poprawne odpowiedzi na polecenia,
- logowanie na pamięć nieulotną (SD),
- podstawowa weryfikacja poprawności danych,
- wykrywanie i raportowanie anomalii (system zdarzeń),
- operacje w rozsądnym czasie (np. odpowiedź &lt; 2 s).

---

## 3. Jak działa? — architektura (ok. 3–3,5 min) + DEMO A

### 3.1 Schemat blokowy

System dzieli się na sekcje:

| Blok | Rola |
|------|------|
| **Procesor (RP2040)** | sterowanie, protokół, IO |
| **Zasilanie** | 2×18650, ładowanie USB i z paneli PV, pomiar mocy (INA3221), buck 5 V |
| **Pamięć** | karta SD — telemetria, czujniki, eventy |
| **Komunikacja** | LoRa SX1278 (~433 MHz), half-duplex |
| **Czujniki** | BME280 (T/H/P), BH1750 (światło), IMU; GPS NEO-6M |
| **Czas** | RTC (DS3231) |

Obudowa 1U z panelami PV zamyka całość w formę „kostki CubeSat”.

### 3.2 Firmware — dwa rdzenie

RP2040 ma dwa rdzenie Cortex-M0+ — to wykorzystane świadomie:

- **Rdzeń 0:** główna pętla — odbiór/wysyłka ramek LoRa, komendy, diagnostyka.
- **Rdzeń 1:** ciągła praca tła — GPS, telemetria, zapis na SD, żądania BOOTSEL.

Sekwencja boot: init UART/I2C/GPIO → radio → montowanie SD → czujniki → menedżer zasilania (USB vs bateria) → start core1 → pętla główna.

Menedżer stanu i system zdarzeń rejestrują anomalie (np. utrata GPS, problemy z SD). Menedżer zasilania steruje trybem pracy — m.in. duty-cycle GPS na baterii.

### 3.3 Protokół komunikacji

Zamiast „gołego” tekstu po radio jest **prosty, izolujący protokół tekstowy** z markerami:

```text
KBST;ADDR;TYPE;GROUP;CMD;PAYLOAD;UNIT;…;TSBK
```

Typy: `GET` / `SET` / `VAL` / `RES` / `ERR` / `SEQ` (oraz w nowszej wersji `ACK`, SEQ, CRC-16).

Grupy komend odpowiadają podsystemom: diagnostyka, zegar, zdarzenia, GPS, telemetria (oraz IMU).

Ścieżka danych operator → satelita:

**Operator (PC)** → USB/UART → **ground firmware (mostek)** → **LoRa** → **satelita** → odpowiedź tą samą drogą.

Ground station nie interpretuje semantyki — jest przezroczystym mostkiem; logika komend siedzi w satelicie i w aplikacji operatorskiej.

### 3.4 Operator

`kubisat_operator` to GUI MVVM: lista komend, asynchroniczne wywołania z timeoutem, wykresy, podgląd GPS, import CSV z lotów/testów. Operator nie musi pamiętać ramek — klika komendę, dostaje odpowiedź.

### DEMO A — „żywy link” (ok. 60–90 s)

*(ekran Operatora na projektorze)*

> „Sprawdźmy, czy łącze działa.”

1. Pokaż połączenie serial / status.
2. Jedna komenda diagnostyczna, np. **wersja build** (`GET 1.1`) albo **uptime** (`1.3`) albo **health** (`1.4`).
3. Gdy wróci odpowiedź:

> „To właśnie half-duplex LoRa — pytanie ze stacji, odpowiedź z modelu. Nie ma kabla między laptopem a satelitą.”

*(jeśli beacon zdrowia miga sam — wskaż go: „model okresowo raportuje zdrowie systemu”)*

**Nie** otwieraj teraz GPS, CSV ani wykresów — to dopiero DEMO B.

**Skrypt mówcy (DEMO A):**

> „Laptop rozmawia z donglem po USB. Dongle to mostek UART–LoRa. Satelita odbiera ramkę, wykonuje komendę i odsyła odpowiedź. Wysyłam teraz zapytanie o [wersję / uptime / health]… i mamy odpowiedź — łącze działa.”

---

## 4. Co robi w praktyce? + DEMO B (ok. 3–3,5 min)

W locie / w pracy autonomicznej satelita:

1. **Mierzy** — temperatura, wilgotność, ciśnienie (≈ wysokość barometryczna), oświetlenie, parametry zasilania (napięcia/prądy), pozycję GPS; opcjonalnie IMU.
2. **Loguje** — CSV / event log na SD (telemetria, sensory, zdarzenia systemowe).
3. **Komunikuje się** — odpowiada na komendy ze stacji; może cyklicznie raportować zdrowie systemu.
4. **Pozwala się sterować** — tryb zasilania, GPS on/off (w labie), czas, poziom logów, odczyt ostatnich pomiarów i eventów, wejście w BOOTSEL przy USB.

W pracy walidowałem telemetrię m.in. przejazdem ~17 km na własnym zasilaniu. Dziś mam **aktualniejszy dowód**: model jechał ze mną **z domu na uczelnię** i logował pozycję GPS (oraz telemetrię, jeśli włączony pełny log).

### DEMO B — ślad GPS (ok. 2–2,5 min)

1. Import / otwarcie świeżego śladu (mapa w Operatorze albo KML w przeglądarce / Google Earth — co masz gotowe).
2. Pokaż trasę: start (dom) → uczelnia.
3. Jedno zdanie „po co GPS”:

> „W misji balonowej GPS to nie gadżet — to odzyskanie ładunku po lądowaniu i korelacja pomiarów z pozycją.”

4. Opcjonalnie **jedna** dodatkowa komenda na żywo: ostatnia telemetria / ostatnie sensory (`8.2` / `8.3`) — żeby powiązać „teraz w sali” ze „świeżym logiem z drogi”.

Jeśli coś padnie przy imporcie: wróć do DEMO A albo pokaż przygotowany backup KML — i idź dalej. Sala bardziej zapamięta działające radio niż idealną mapę.

**Skrypt mówcy (DEMO B):**

> „Przed zajęciami model był włączony w drodze z domu. Tu widać zapisany ślad GPS — start, trasa, uczelnia. Ten sam mechanizm, który w tezie weryfikowałem dłuższym przejazdem, dziś pokazuje świeże dane z realnej drogi.”

Wniosek z pracy: **cele zrealizowane** — zdalne sterowanie, telemetria, konfiguracja; model działa zgodnie z wymaganiami.

---

## 5. Co dalej / most do magistarium (ok. 1–1,5 min)

W podsumowaniu pracy wskazałem m.in.: FreeRTOS, lepsze tryby oszczędzania energii, więcej czujników, kontrolę dostępu, kamerę, pobieranie plików z SD przez radio, lepszą konfigurację paneli PV.

Jako student magistarium / na projekcie przejściowym naturalne kierunki to pogłębienie **niezawodności** (watchdog, integralność ramek CRC, beacony zdrowia — część już w repo), **komunikacji** (zasięg, protokół, ewentualnie drugi kanał) albo **zarządzania energią i misją** — czyli tematy typowo „magisterskie” budowane na gotowej platformie KubiSat, zamiast zaczynać od zera.

---

## 6. Zakończenie + pytania (ok. 20–30 s)

Podsumowując w jednym zdaniu: **KubiSat to pełny amatorski ekosystem CubeSat 1U — hardware, firmware i stacja naziemna — zbudowany, by zmierzyć, zalogować i zdalnie obsłużyć model w warunkach lotu stratosferycznego.** Dziś widzieliście go na żywo: łącze radiowe i ślad GPS z drogi na uczelnię.

Dziękuję. Chętnie odpowiem na pytania.

---

## Timing z demo (ściąga)

| Czas | Blok | Rekwizyt |
|------|------|----------|
| 0:00 | Intro | model na stole |
| 1:00 | Co to jest | model w ręku |
| 3:00 | Dlaczego | — |
| 4:30 | Jak działa (teoria) | — |
| 6:30 | **DEMO A**: 1 komenda | Operator + dongle + model |
| 8:00 | Co robi + most | — |
| 8:30 | **DEMO B**: ślad GPS dom→uczelnia | plik gotowy |
| 11:00 | Magistarium | — |
| 12:00 | Koniec | — |
| bufor | pytania / retry radia | Plan B |

### Wersje czasowe

- **Tylko 10 min:** skróć opis Operatora i magistarium; zostaw model, schemat blokowy, protokół w 3 zdaniach, DEMO A i DEMO B.
- **Pełne 15 min:** dodaj 1–2 zdania o SX1278 (niski pobór, zasięg) i o BME280 jako uzupełnieniu wysokości GPS.

---

## Czego nie robić na żywo

- Nie tłumacz struktury ramki pole-po-polu (zostaw na pytanie).
- Nie formatuj SD, nie BOOTSEL, nie uruchamiaj komend destrukcyjnych.
- Nie debuguj portów dłużej niż ~20 s — od razu Plan B.
- Nie konkuruj o uwagę: albo mówisz, albo klikasz — nie jednocześnie długo.

---

## Karta mówcy (1 strona)

| Czas | Blok | Jedno zdanie |
|------|------|----------------|
| 0:00 | Intro | KubiSat = CubeSat 1U + stacja naziemna, praca inż. |
| 1:00 | Co | HW (3 PCB) + FW Pico + operator — model w ręku |
| 3:00 | Dlaczego | CubeSaty tanie vs space-grade; cel = balon + radio + pomiary |
| 4:30 | Jak | bloki → 2 rdzenie → protokół KBST…TSBK → mostek LoRa |
| 6:30 | DEMO A | jedna komenda diagnostyczna — łącze działa |
| 8:30 | DEMO B | ślad GPS dom→uczelnia + po co GPS w misji |
| 11:00 | Dalej | energia, RTOS, pliki SD, niezawodność → magistarium |
| 12:00 | Koniec | pytania |

---

## Plan B (gdy radio/GPS zawiedzie)

1. **Brak odpowiedzi LoRa:** pokaż wcześniej zrobiony screenshot Operatora z udaną komendą; opowiedz ścieżkę Operator → dongle → satelita.
2. **Brak świeżego śladu:** otwórz backup KML z `software/kubisat_firmware/telemetry_test/` (np. `kubisat_trajectory_*.kml`) i powiedz, że to wcześniejsza walidacja / ten sam mechanizm.
3. **Operator nie łączy portu:** zostań przy modelu fizycznym i schemacie blokowym — nie walcz z Windows/Linux serial dłużej niż 20 s.

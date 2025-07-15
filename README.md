
# MOOver_Merge – Integracja MELKENS + WB

## Struktura repozytorium
MOOver_Merge/
├── Melkens/ # Kod źródłowy robota MELKENS
├── WB/ # Kod źródłowy oraz system firmware Wasserbauer
└── README.md # Ten plik – opis projektu

## Cel projektu
Celem repozytorium jest pełna integracja i refaktoryzacja rozwiązań nawigacyjnych robota WB (Wasserbauer) z platformą sprzętową MOOver (MELKENS). Chcemy:
- Uruchomić oprogramowanie WB na naszym hardware,
- Zweryfikować, które moduły da się przenieść „1:1”, a które wymagają adaptacji,
- Zbudować symulator i środowisko testowe,
- Opracować finalną wersję software pod własną marką.

## Plan działania / Roadmapa
1. **Porównanie hardware** – mapping płytki/pinów/MAGNETLINEAL vs. MELKENS
2. **Analiza kompatybilności kodu** – refaktoryzacja, mocki, porty na nasze MCU
3. **Stworzenie symulatora** – wizualizacja trasy, błędów, logów
4. **Integracja CAN/komunikacja** – testy z naszą magistralą i silnikami
5. **Testy na robocie** – najpierw basic drive, potem cała automatyka
6. **Refaktor, czyszczenie kodu, wdrożenie docelowe**
7. **Wdrożenie u klienta**

## Status
- [x] Zgrany kod MELKENS oraz WB w jednym repo
- [ ] Stworzony plan działania (README)
- [ ] Analiza sprzętu
- [ ] Integracja software
- [ ] Testy symulatora
- [ ] Testy na robocie

## Kontakt
Prowadzący integrację: Michał Czech / Melkens Sp. z o.o.

---

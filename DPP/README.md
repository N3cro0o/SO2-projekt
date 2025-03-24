# Problem jedzących filozofów
Problem ten jest klasycznym zadaniem na wprowadzanie wielowątkowości do programu. Polega on na synchronizowaniu pięciu filozofów tak, aby każdy w określonym czasie mógł myśleć oraz jeść bez występowania zjawiska zakleszczania oraz eliminując wyścigi
## Działanie programu
Program wykorzystuje stałą programową `threads` do wygenerowania liczby filozofów oraz do symulowania ich pracy. Przed rozpoczęciem pracy wątków, program przygotowuje niezbędne tablice pomiarowe oraz kontrolne oraz przygotowuje klasę niezbędną do generowania liczb losowych. Jeżeli stała programowa `simulation_time` jest ustawiona na wartość inną niż *-1* program symuluje `simulation_time` sekund (nie wliczając czekania na zakończenie wszystkich wątków) oraz zwraca parametry pomiarowe. W przeciwnym wypadku program działa bez końca.
### Opis wątków
Każdy wątek reprezentuje jednego filozofa, który chronologicznie wykonuje następujące czynności
* Czekanie od 10 do 20 sekund,
* Podnoszenie dwóch widelców,
	*	Jeżeli nie uda się podnieść dwóch wymaganych, to wątek odkłada wszystkie widelce, które przywłaszczył oraz czeka 3 sekundy na kolejną próbę.
*	Jedzenie od 15 do 20 sekund,
*	Odkładanie widelców.
Czynności te są wykonywane cyklicznie, chyba że osiągnięto już limit czasowy symulacji. W takim razie wątek po odłożeniu widelców kończy działanie.

W trakcie podnoszenia widelców zliczana jest liczba _odbić_, czyli ilość nieudanych prób podniesienia dwóch widelców. Warto wspomnieć, że wątki są od siebie niezależne, ich stan nie jest sprawdzany przez inne wątki.
## Użyte technologie
* **Mutex**: Mutexy są wykorzystywane jako widelce oraz do kontrolowania dostępu do drukowania na ekranie konsoli (komenda `std::cout`),
*  **Counting_semaphore**: Semafor kontrolujący maksymalną liczbę *odbijających się* filozofów do trzeciej części wszystkich wątków. Wątki przechodzą i rezerwują miejsce w semaforze przed trzysekundowym czekaniem wątku po nieudanej próbie nabycia widelców. Wątki zwalniają miejsce zaraz po tym, jak odłożą (odblokują) trzymane widelce, pozwalając innym wątkom na kontynuowanie ich cykli,
* **Atomic_bool**: Zmienna kontrolna do odblokowania cykli filozofów. Zmieniana tylko i wyłącznie, gdy upłynie czas zadeklarowany przez `simulation_time`,
* **Random**: Biblioteka wykorzystywana do generowania liczb losowych.
## Kompilacja
1. **Visual Studio**
Pobrać projekt w zintegrowanym środowisku programistycznym Visual Studio oraz wykorzystać wbudowany kompilator Visual C++ do skompilowania projektu. 
2. **Manualna kompilacja**
Podczas kompilacji ustawić kompilowanie dla wersji C++ 20. Poniżej zaprezentowano przykład dla kompilatora g++:
> g++ -std=c++20 Main.cpp

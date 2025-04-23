# Projekt Systemy Operacyjne 2
Repozytorium przechowujące dwa projekty wykonane w ramach zajęć projektowych Systemy Operacyjne 2. W poszczególnych folerach znajdują się:
* Aplikacja rozwiązująca problem jedzących filozofów (*DPP*)
* Aplikacja klient stworzona w ramach głównego projektu (*SO-client*)
* Aplikacja serwer magazynu stworzona w ramach głównego projektu (*SO-main*)
Działanie pierwszej aplikacji zostało opisane w DPP/README.md.
## Działanie programu
Aplikacje stworzone w ramach drugiego projektu mają symulować działanie systemu magazynu, który przechowuje informacje o posiadanych przedmiotach i ich ilości oraz pozwala na manipulowanie składem magazynu. W celu kominikacji serwer-klient wykorzystano mechanizm portów oraz adres IP wykorzystując protokuł TCP do ciągłej komunikacji. W celu przekazywania kounikatów i poleceń stworzono instrukcje, które wraz z niezbędnymi argumentami pozwalają na bezproblemową komunikację i przekazywanie informacji.
### Opis serwera
Serwer zanim zacznie nasłuchiwać na połączenia się klientów przeprowadza wstępną inicjalizację systemu oraz ustawnia niezbędny protokuł oraz port. Następnie tworzony jest niezależny wątek `std::thread listen` wykonujący funkcję `thread_listen(SOCKET* server_socket)`, która bez końca nasłuchuje na wcześniej zdefiniowanym i przygotowanym porcie oraz tworzy kolejne, niezależne wątki (sesje) obsługujące komunikację z wybranym klientem, jeżeli połączenie zostanie zaakceptowane. Sesje komunikują się z klientem porzez nowo stworzone sokety, których stan jest nadzorowany przez pętlę nasłuchową.

Komunikacja serwer-klient wykonywana jest w funkcji `int thread_func(int socket_id, SOCKET* client_socket, int max_buffer)`. Przed rozpoczęciem głównej pętli, inicjalizowane są niezbędne zmienne służące do przechytywania oraz wysyłania komunikatów. Pierwszą czynnością, jaka jest wykonywana po rozpoczęciu pętli, jest oczekiwanie i nasłuchowanie na żądanie klienta, po czym zdobyty ciąg znaków jest dekodowany i dzielony na poszczególne argumenty. Wykorzystując ten zbiór serwer wykonuje żądane polecenie i zwraca odpowiedni komunikat albo dane do docelowego klienta. Pętla ta wykonuje następne iteracje do momentu, gdy klient nie zwróci polecenia `exit`.
### Opis klienta
Klient na początku pracy przygotowuje środowisko i wykonuję startową inicjalizację po czym wysyła żądanie połączenia się z serwerem na wcześniej wyznaczonym porcie oraz adresie IP. Po otrzymaniu informacji o zaakceptowaniu połączenia, aplikacja pokazuje menu wyboru możliwych opcji oraz pozwala użytkownikowi na wybór opcji oraz na wprowadzenie niezbędnych danych poprzez okno konsoli. Po otrzymaniu pełnego polecenia, aplikacja-klient wysyła komunikat do serwera oraz oczekuje na wiadomość zwrotną, która albo jest przetwarzana i pokazywana w oknie terminala albo aplikacja przechodzi w odpowidni tryb pracy do otrzymania polecenia zakończenia. Pętla jest przerywana wtedy i tylko wtedy gdy otrzymany zostanie pusty komunikat (komunikat o długości 0), po którym aplikacja kończy aktualne procesy oraz wyłącza się.
### Opis ról
Środowisko pozwala użytkownikowi na stworzenie konta oraz na zalogowanie się do systemu, co pozwala na zarządzanie składem magazynu oraz, jeżeli użytkownik zaloguje się jako Admin, na manipulowanie pracą serwera oraz środowiska. Poniżej znajduje się krótki opis każdej z istniejących ról
* Admin - 
	Ma pełny dostęp do środowika oraz ma wgląd do składu magazynu. Wszystkie opcje wyboru dostawcy oraz kwatermistrza są mu przypisane i dodatkowo ma możliwość zdalnie wyłączyć aplikację-serwer.
* Dostawca -
	Ma wgląd do składu magazynu. Jego główną rolą jest dostarczanie nowych przedmiotów do magazynu
* Kwatermistrz/Magazynier -
	Ma wgląd do składu magazynu. Jego główną rolą jest wyciąganie (usuwanie) przedmiotów ze składu. Dodatkowo ma możliwość na dokonanie zwrotu, który polega na wybraniu jedej z operacji zapisanych w historii użytkownika oraz na dodaniu takiej samej liczby wybranego produktu do magazynu.
## Użyte technologie
* **Winsock2** - Komunikacja na bazie portów oraz protolołu TCP/IP
* **std::mutex** - Blokowanie dostępu do wektora przechowywującego sesje użytkowników oraz dane i aktualny stan użytkowników,
* **std::binary_semaphore** - Zarządzanie dostępem do magazynu tylko jedej sesji w danym momencie czasu,
* **std::atomic_bool** - Zmienna pozwalająca na zdalne zatrzymanie pracy serwera. Po zmianie na *false* aplikacja-serwer przerywa działanie i wyłącza program.
## Kompilacja
1. **Visual Studio**
Pobrać projekt w zintegrowanym środowisku programistycznym Visual Studio oraz wykorzystać wbudowany kompilator Visual C++ do skompilowania projektu. 
2. **Manualna kompilacja**
W folderach *SO-client* i *SO-main* znajdują się niezbędne pliki źródłowe oraz nagłówkowe, które można skompilować jednocześnie. Należy się upewnić, że plik `common.h` znajduje się w poprawnym miejsu albo należy zmienić ścieżkę prowadzącą do niego w plikach *SO-client/main.cpp* i *SO-main/Main.cpp*.
Do kompilacji niezbędna jest wersja C++20 oraz w biblioteka **Winsock2**.

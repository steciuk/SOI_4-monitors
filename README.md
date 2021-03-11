# SOI_4-monitors
## Treść: 
Napisz w C++ dla srodowiska Linux, system kolekcjonowania krotkich wiadomosci tekstowych (maks. 64 znakow, ale nie mniej niz 8 znakow). System ma bazowac na synchronizacji dostepu do zasobow wykorzystujacy mechanizm monitorow. Zadaniem budowanego systemu ma byc niezawodne zbieranie od klientow wiadomosci, liczba klientow moze byc duza, ale system musi byc gotowy do obsluzenia minimum 5 klientow. Klienci - pojedynczy pod-proces lub watek - 'wrzucaja' wiadomosci do systemu, oprocz samej tresci wiadomosci wybieraja priorytet wrzucanej wiadomosc (np.: 0 to zwykly, 1 to  priorytetowy). System moze zbierac wiadomosci tylko w __jednym__ pojemnym buforze. Mechanizm wkladania nowych wiadomosci do tego bufora musi uwzgledniac priorytety. Wszelkie operacje na buforze powinny byc optymalizowane w taki sposob by nie kopiowac niepotrzebnie wiadomosci, oraz zapewnic by czas wkladania wiadomosci oraz czas wyjmowania byly mozliwie jak najkrotsze. Dodatkowo dla systemu utworzony ma byc pod-proces lub watek 'czytelnik' zebranych wiadomosci. Jego zadaniem jest pobieranie z bufora i przedstawianie wiadomosci tekstowych na konsoli tekstowej. Zaklada sie, ze 'czytelnik' bedzie pobieral wiadomosci z bufora, a w buforze wiadomosci beda juz poukladane zarowno wzgledem priorytetow jaki i czasu ich wlozenia. Przemysl bardzo dokladnie metode automatycznego testowania powstalego systemu. W szczegolnosci zwroc uwage na pokazanie rownoczesnego dzialania wielu procesow (czy watkow) umieszczajacych wiadomosci, w tym takze z roznymi priorytetami oraz wspoldzialanie w tym czasie 'czytelnika'. Zaloz, ze program testowy beda dzialaly automatycznie generujac przez klientow fikcyjne wiadomosci wylacznie tekstowe, a 'czytelnik' pokazywal je na konsoli. 
## Rozwiązanie:
Program przy uruchomieniu przyjmuje 6 argumentów: liczbę producentów niskiego priorytetu, liczbę wiadomości generowanych przez każdego z nich, liczbę producentów wysokiego poziomu, liczbę wiadomości generowanych przez każdego z nich, wielkość buforu oraz odstępy między zczytywaniem wiadomości przez konsument.
Następnie uruchamiam odpowiednią ilość wątków wysokiego priorytetu oraz niskiego priorytetu. Wiadomości są tworzone przez każdego z producentów i odpowiednio wkładane do buforu „Buff” (wiadomości z wysokim priorytetem na początku kolejki a z niskim na końcu).
Bufor „Buff” jest obiektem klasy „Bufor” dziedziczącej po klasie Monitor”. Dodawania elementów zrealizowane zostało za pomocą funkcji:
1.	„PutMsgLow” obsługującej dodawanie wiadomości niskiego priorytetu. Rozpoczyna się ona zabezpieczeniem wyłączności w sekcji krytycznej poprzez wywołanie „enter()” a kończy opuszczeniem „leave()”. Maja w niej miejsce dwa przerwania:  
  -	„wait(Priority)” – gdy są w kolejce jeszcze wiadomości wysokiego priorytetu  
  -	„wait(NotFull)” – gdy bufor zostanie napełniony  
oraz jedno wybudzenie:  
  -	„signal(NotEmpty)” – gdy pusty bufor zostanie napełniony  
2.	„PutMsgHigh” obsługującej dodawanie wiadomości wysokiego priorytetu. Rozpoczyna się ona zabezpieczeniem wyłączności w sekcji krytycznej poprzez wywołanie „enter()” a kończy opuszczeniem „leave()”. Ma w niej miejsce jedno przerwanie:  
  -	 „wait(NotFull)” – gdy bufor zostanie napełniony  
oraz jedno wybudzenie:  
  -	„signal(NotEmpty)” – gdy pusty bufor zostanie napełniony  


Pobieranie elementów zrealizowane zostało za pomocą funkcji „GetMsgg()”. Rozpoczyna się ona zabezpieczeniem wyłączności w sekcji krytycznej poprzez wywołanie „enter()” a kończy jej opuszczeniem „leave()”. Ma w niej miejsce jedno przerwanie: „wait(NotEmpty)” – gdy bufor będzie pusty oraz dwa wybudzenia:
1.	„signal(NotFull)” – gdy w buforze pojawi się miejsce.
2.	„signal(Priority)” – gdy w kolejce skończą się wiadomości priorytetowe


## Testy
Testując program należy rozważyć przynajmniej 6 sytuacji wyjątkowych:
1.	Bufor jest pusty, konsument próbuje sczytać z niego wiadomości, a
  -	wcześniej wysyłane były do niego jedynie wiadomości o niskim priorytecie
  -	wcześniej wysyłane były do niego jedynie wiadomości o wysokim priorytecie
  -	wcześniej wysyłane były do niego wiadomości o różnych priorytetach
2.	Bufor jest pełny, a
  -	konsumenci próbują wysłać do niego wiadomości jedynie nisko priorytetowe
  -	konsumenci próbują wysłać do niego wiadomości jedynie wysoko priorytetowe
  -	konsumenci próbują wysłać do niego wiadomości o różnych priorytetach
Testy realizuje poprzez skrypty wywołujące program z odpowiednimi parametrami.
Wiadomości są generowane wolniej niż jest je w stanie odbierać konsument:
1. „emptyLow.sh” - ./a.out 3 8 0 0 5 0
1. „emptyHigh.sh” - ./a.out 0 0 3 10 5 0
1. „emptyMix.sh” - ./a.out 3 8 3 10 5 0

Wiadomości są generowane szybciej niż jest je w stanie odbierać konsument:
1. „fullLow.sh” - ./a.out 3 8 0 0 5 1000000
1. „fullHigh.sh” - ./a.out 0 0 3 10 5 1000000
1. „fullMix.sh” - ./a.out 3 8 3 10 5 1000000

## Wyniki testów:
![Obraz1](https://user-images.githubusercontent.com/48189079/110820303-42591280-828f-11eb-8232-c31ef168f8fb.png)  
W sytuacji nienasycenia buforu wiadomościami, conusmer czeka na wygenerowanie wiadomości, i jak tylko się pojawią – odbiera je. Możemy zauważyć, że wszyscy producenici priorytetowi generują wiadomości w tym samym momencie (dla oka użytkownika, oczywiście w rzeczywistości tak nie jest) i nie muszą oczekiwać na zwolnienie buforu przez conusmera odbierającego wiadomości. Tuż po wysłaniu ostatniej, 10 wiadomości. Działać zaczynają producenci niskiego priorytetu:
![Obraz2](https://user-images.githubusercontent.com/48189079/110820362-513fc500-828f-11eb-8e84-347bf7ea191a.png)  
Dla nich sytuacja przebiega analogicznie, czyli system w tym przypadku działa zgodnie z zamierzeniami.
Przesycenie bufora:
![Obraz3](https://user-images.githubusercontent.com/48189079/110820456-6583c200-828f-11eb-8906-2949a4063292.png)  
W tej sytuacji, użytkownicy muszą oczekiwać na conusmera, aż odbierze wiadomości i zwolni bufor, żeby móc wysłać kolejne wiadomości. Mimo że użytkownicy mają ustawiony identyczny interwał, widzimy, że oczekują zawieszani są na warunku braku miejsca w buforze (wiadomości nie są wysyłane „równolegle” tylko po kolei). Gdy każdy z użytkowników priorytetowych wyśle ostatnią wiadomość – pracę zaczynają użytkownicy o niskim priorytecie: 
![Obraz4](https://user-images.githubusercontent.com/48189079/110820506-73394780-828f-11eb-9daa-089976ddea3a.png)  
Dla nich sytuacja przebiega analogicznie, czyli system działa zgodnie z założeniami. 
Pozostałe testy, są uproszczonymi wariantami tych dwóch testów i pokazywałem na zajęciach, że przebiegają bez zarzutu. W tym raporcie pomijam więc ich analizę.





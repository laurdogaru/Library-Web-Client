README -- TEMA 3 PCOM -- Dogaru Laurentiu 323CC

-Functia availableFirstCommand intoarce un unordered_set care contine comenzile
initiale valide pe care le poate introduce utilizatorul. Daca o comanda nu se
afla in set, atunci este invalida.  
-La introducerea "exit", clientul se inchide

REGISTER  
-Daca string-ul sessionCookie este gol (nu este niciun utilizator logat in
acel moment), se apeleaza functia "reg"  
-Se introduc username-ul si parola si se verifica daca contin spatii  
-Se creeaza un POST request, ce are ca date un JSON cu username-ul si parola  
-Codul raspunsului 201 => s-a inregistrat cu succes  
-Cod 400 => a aparut o eroare si este afisata, sau request-ul nu este construit
corespunzator  
-Cod 429 => au fost trimise prea multe request-uri  

LOGIN  
-Identic cu register, doar ca se returneaza cookie-ul primit

ENTER_LIBRARY  
-Se apeleaza functia "access", ce primeste pe langa file descriptor si
cookie-ul de sesiune (poate fi si nul, daca nu este niciun utilizator logat)  
-Cookie-ul este inclus in header-ul request-ului GET  
-Cod 200 => accesul a fost asigurat  
-In acest caz, este intoarsa o variabila de tip json, care contine doar campul
cu token-ul primit  
-Cod 401 => accestul nu a fost permis, cookie-ul fiind necorespunzator  
-Cod 429 => au fost trimise prea multe request-uri  
-In aceste cazuri, este intors un json gol  

-Urmatoarele functii (get_books, get_book, add_book, delete_book) vor primi
ca argument json-ul ce contine token-ul, care este pus in header-ul
Authorization, prefixat de cuvantul Bearer  
-In cazul in care json-ul este gol, request-ul nu va mai fi construit, iar
utilizatorul va primi un mesaj de eroare  

GET_BOOKS  
-Daca raspunsul request-ului este "[]", se afiseaza ca lista este goala. Altfel,
se afiseaza cartile  

GET_BOOK  
-Dupa verificarea integritatii token-ului, se citeste id-ul cartii dorite  
-Daca id-ul are lungime 0 sau nu este un numar, este invalidat  
-Cod 200 => cartea exista, se afiseaza detallile sale  
-Cod 404 => nu exista o carte cu id-ul primit  
-Cod 429 => au fost trimise prea multe request-uri  

ADD_BOOK  
-Dupa verificarea integritatii token-ului, se citesc informatiile cartii ce
urmeaza sa fie adaugata. Am invalidat input-ul daca page_count nu este un numar
sau daca oricare din campuri are lungimea 0.  
-Se trimite un POST request, ce are ca date un JSON cu campurile cartii  
-Cod 200 => cartea a fost adaugata cu succes  
-Cod 429 => au fost trimise prea multe request-uri  

DELETE_BOOK  
-Identic cu get_book  

LOGOUT  
-Se trimite un GET request de logout, care contine si cookie-ul de sesiune  
-Cod 200 => utilizatorul a fost deconectat  
-In plus, se goleste string-ul sessionCookie folosit si json-ul ce contine
token-ul, pentru ca utilizatorul sa piarda accesul la biblioteca  
-Cod 404 => nu este niciun utilizator conectat  
-Cod 429 => au fost trimise prea multe request-uri
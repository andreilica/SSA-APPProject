CERINTA

Se cere implementarea unui program capabil sa redimensioneze o imagine micsorand pierderea de informatie folosind tehnica anti-aliasing de tip 
super sampling antialiasing (SSAA sau FSAA) si implementarea unui micromotor de randare, capabil sa creeze o poza ce contine o linie.

IMPLEMENTARE 

===================
Fisierul antialiasing.c
===================
Am creat doua functii auxiliare(allocMatrix si threadFunction), pe langa cele propuse in schelet.
Parsarea fisierelor de input a fost facuta cu fgets, pentru headere si cu fread pentru a creea matricea de pixeli.
Structura de tip imagine contine 4 campuri de tip int ce reprezinta culoarea, latimea, inaltimea si valoarea maxima a unui pixel al unei poze.
Cel de-al 5-lea camp este o matrice de tip unsigned char ce va retine valorile propriu-zise ale pixelilor din imagine.
Scrierea noii imagini este realizata cu fprintf pentru headere si fwrite pentru matricea de pixeli.

Am creat doi pointeri globali la doua structuri de tip imagine pentru a putea prelucra imaginile de input si de output in functia data ca argument fiecarui thread,
functie care primeste doar un singur argument, in cazul meu fiind thread_id (id-ul fiecarui thread in parte). 
Am paralelizat parcurgerea matricii de pixeli din imaginea de output pe inaltime(heigth), iar de aceasta paralelizare depinde si parcurgerea matricii de pixeli
din imaginea de input, dar cu pasul inmultit cu resize_factor. In fiecare caz(resize_factor par, impar, imagine color, grayscale), algoritmul functioneaza astfel:
    - 2 bucle for ce parcurg matricea de pixeli din input din resize_factor in resize_factor
    - 2 bucle for ce parcurg fiecare patrat cu latura (resize_factor * resize_factor), pentru a creea noul pixel (noii pixeli in cazul imaginii color), ce va fi pus in matricea 
      de pixeli din output
In functia de resize doar aloc, in fiecare caz, dimensiunile necesare imaginii de output si memorie pentru matricea de pixeli si creez thread-urile care vor colabora pentru a rezolva problema.

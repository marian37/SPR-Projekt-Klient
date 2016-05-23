# SPR-Projekt-Klient
Moja implementácia **klient**a pre projekt na predmet **SPR** (Systémové programovanie) vyučovaný na [ics.upjs.sk](http://www.ics.upjs.sk/)

### Zadanie projektu
**Zadanie č.2: Chatovací program klient-server**
Podmienkou je naprogramovanie dvoch aplikácií (klienta a server) pre výmenu textových správ. Serverová aplikácia bude plniť úlohu centrálneho bodu, ku ktorému sa klientské aplikácie pripájajú. Chatovanie (možnosť písať a čítať správy) má teda prebiehať len v klientskej aplikácií. Klientská aplikácia má mať jednoduché rozhranie, v ktorom sa zobrazujú správy od chatujúcich spolu s dátumom a časom ich zaslania. Serverová časť musí sledovať aktivitu prihlásených klientov a v prípade ich nečinnosti po dobu 5 minút ich automaticky odpojí. Proces chatovania bude prebiehať nasledovne:
1. spustí sa klientská aplikácia
2. zadá sa želané používateľské meno (nick), ktoré sa musí overiť, či náhodou sa už nepoužíva
3. ak je nick unikátny, server zašle zoznam práve chatujúcich ľudí, ktorý klient zobrazí; výpis prihlásených je možné následne získať príkazom LIST
4. chatuje sa formou: `[nick]#[správa]` a odoslanie správy klávesou Enter. Je možné zaslať aj verejnú správu všetkým (broadcast) vo forme `[]#[správa]`
5. odhlásenie a ukončenie chatovania sa realizuje príkazom "QUIT"

# BUNCHER — Proposte di miglioramento (roadmap)

Documento per il team di Michele. Elenca i prossimi passi consigliati per portare
l'applicazione da “scaffold funzionante” a strumento operativo completo. Ordinato
per priorità. Ogni voce indica *perché* serve e *dove* intervenire.

---

## Priorità 1 — Fedeltà al sistema reale (necessaria prima dell'uso in linea)

1. **Confermare il vocabolario dei comandi RF.**
   Oggi i comandi RF in `server/protocol.js` sono *template* marcati «DA CONFERMARE».
   Vanno verificati leggendo il lato grafico originale del software:
   `BUNCHER/work/graphics/creation.c` e `leggiconf.c` (mappano i cursori dell'interfaccia
   alle stringhe inviate ai server). Per ogni parametro (campo, fase, potenza, ecc.)
   serve la stringa esatta che il vecchio programma inviava.

2. **Verificare le formule di conversione.**
   Confermare la catena cursore → volt → dbm → DAC per il campo RF e gli intervalli
   reali di ogni cursore, sempre dai sorgenti `graphics/`.

3. **Ricostruire “automagico”.**
   Analizzare `automagico.c` e tradurre le procedure automatiche (riconnessioni,
   sequenze guidate) nella pagina Automagico.

4. **Test in linea controllati.**
   Collegarsi agli apparati reali (modalità hardware) con le 8 porte del device master
   abilitate, partendo da un singolo canale a bassa potenza, confrontando le letture
   con il vecchio sistema.

## Priorità 2 — Robustezza operativa

5. **Riconnessione automatica e watchdog.** Se un server hardware cade, il backend
   deve ritentare la connessione e segnalarlo chiaramente in interfaccia.
6. **Conferme sulle azioni critiche.** Richiedere conferma prima di comandi ad alto
   impatto (reset, accensioni, movimenti motore ampi).
7. **Log persistente.** Salvare su file i comandi inviati e gli errori, per
   tracciabilità e diagnosi.
8. **Controllo accessi multi-PC.** La webapp può essere aperta da più PC sulla stessa
   rete: prevedere un “proprietario” del controllo per evitare comandi concorrenti
   sullo stesso canale.

## Priorità 3 — Comodità e distribuzione

9. **Pacchetto .exe.** Creare un singolo eseguibile con `pkg` per avvio senza
   installare Node (vedi README). Aggiungere icona e collegamento sul desktop.
10. **Manuale operatore.** Guida passo-passo con schermate per chi usa l'interfaccia.
11. **Grafici storici.** Visualizzare l'andamento nel tempo delle letture (campo, fase).
12. **Backup configurazione device master nel repo.** Versionare il file di backup
    della configurazione (IP 10.20.1.26) per ripristini rapidi.

## Priorità 4 — Qualità del software

13. **Test automatici** del layer di comunicazione (il simulatore è già una buona base).
14. **Validazione dei valori** lato backend (rifiutare valori fuori intervallo prima
    dell'invio all'hardware).

---

### Suggerimento di sequenza

Completare la Priorità 1 in simulazione → un breve test su un canale reale →
Priorità 2 → distribuzione (Priorità 3). Le voci di Priorità 4 si possono affrontare
in parallelo.

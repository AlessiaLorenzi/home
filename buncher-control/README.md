# BUNCHER — Interfaccia di controllo (porting moderno)

Porting del sistema di controllo **BUNCHER** dell'acceleratore (amplificatori RF e
motori di accordo) verso una **applicazione web portatile** eseguibile su Windows,
senza più la vecchia macchina virtuale Linux.

L'applicazione si collega — esattamente come l'interfaccia originale — ai programmi
server (`rf_server` / `mot_server`) tramite le stesse porte di rete, e mette a
disposizione un'interfaccia moderna nel browser.

> **Per chi conosce l'hardware ma non il software:** non serve installare nulla di
> complicato né saper programmare. Basta Node.js e un doppio clic sul file di avvio.
> La documentazione discorsiva (`.docx`) e le slide (`.pptx`) spiegano tutto a parole.

---

## Avvio rapido

1. Installare **Node.js** (una sola volta): https://nodejs.org → scaricare la versione "LTS".
2. Doppio clic su **`AVVIA_simulazione.bat`** per provare l'interfaccia **senza hardware**
   (modalità dimostrativa/didattica), oppure **`AVVIA_hardware.bat`** per collegarsi
   agli apparati reali.
3. Si apre il browser su **http://localhost:8080**.

Da terminale, in alternativa:

```bash
# simulazione (predefinita)
node server/index.js

# hardware
set BUNCHER_MODE=hardware && node server/index.js     # Windows
BUNCHER_MODE=hardware node server/index.js            # Linux/Mac
```

Variabili d'ambiente: `PORT` (default 8080), `BUNCHER_MODE` = `simulazione` | `hardware`.

### Versione "portatile" (chiavetta USB)

Per avere tutto su chiavetta senza installare Node sul PC di destinazione, copiare
accanto a questa cartella l'eseguibile portatile di Node (`node.exe`) e lanciare
`node.exe server\index.js`. In alternativa si può creare un singolo `.exe` con uno
strumento di packaging (es. `pkg`); vedi note in fondo.

---

## Due modalità

| Modalità | A cosa serve | Hardware |
|----------|--------------|----------|
| **simulazione** | Mostrare e provare l'interfaccia, didattica, sviluppo. I valori sono generati internamente. | Non necessario |
| **hardware** | Controllo reale. Si collega via TCP ai server `rf_server`/`mot_server`. | Sì |

In entrambe le modalità l'interfaccia è identica: si cambia solo una variabile.

---

## Come è organizzato il progetto

```
buncher-control/
  canali.json              Configurazione: canali, porte, seriali, parametri (dai file originali)
  server/
    index.js               Server web + API (solo moduli Node integrati, zero dipendenze)
    transport.js           Comunicazione: simulatore + collegamento TCP ai server reali
    protocol.js            Codifica dei comandi (dal sorgente rf_server/exec_command.c)
  public/
    index.html             Interfaccia (HTML + JavaScript puro, nessuna libreria esterna)
  AVVIA_simulazione.bat    Avvio dimostrativo (Windows)
  AVVIA_hardware.bat       Avvio reale (Windows)
```

### Scelte tecniche (e perché)

- **Nessuna dipendenza esterna** (né npm, né CDN): l'app funziona anche su una rete
  isolata/offline ed è facile da copiare. Massima affidabilità, zero sorprese.
- **Solo moduli Node integrati** lato server (`http`, `net`, `fs`).
- **JavaScript puro** lato browser: niente framework da compilare.

---

## Mappa dei canali (dai file originali del repo)

Ricavata da `BUNCHER/porte` e `BUNCHER/work/start_servers_nuovi`.

| Canale | Porta TCP | Seriale (ttySI) | Tipo |
|--------|-----------|-----------------|------|
| Postchopper 1 LIN | 6100 | 64 | RF |
| Buncher 10 MHz | 6101 | 65 | RF |
| Buncher 5 MHz | 6102 | 67 | RF |
| Postchopper 1 TAN | 6103 | 70 | RF |
| Buttafuori (Chopper) | 6104 | 71 | Chopper |
| Riserva | 6105 | 69 | RF (non attivo) |
| Cavità Passiva | 6108 | 72 | RF |
| CEST Pre-Tandem | 6200 | 66 | Motori |
| CEST Postchopper | 6201 | 68 | Motori |

Device master Comtrol RTS: **10.20.1.26** — abilitare (Enable=1) le 8 porte in uso
dalla pagina web del dispositivo.

---

## Protocollo (dal sorgente originale)

Dal file `BUNCHER/work/rf_server/exec_command.c`: i server ricevono **comandi ASCII**
terminati da newline.

- `r` → reset della linea seriale.
- `!<motore> <passi> <velocità>` → muove un motore (es. `!4 500 100`).
- altre stringhe → inoltrate al controllore via seriale.

Il segnale **Reference INT/EXT** non è un canale separato: agisce sul **bit 7 della
porta I/O 7** del server `mot_postch` (6201) — verificato nel sorgente.

---

## Stato e cosa resta da verificare

**Funziona già:**
- Backend, API e simulatore completi (testati).
- Interfaccia con sinottico, pagine RF, chopper (con periodo = 5/2ⁿ), motori,
  selettore di fase, automagico, log diagnostico.
- Mappa canali/porte/seriali fedele ai file originali.
- Comandi reset e movimento motori conformi al protocollo originale.

**Da confermare sull'hardware reale / leggendo il lato grafico originale
(`BUNCHER/work/graphics`):**
- Il vocabolario esatto dei comandi RF (quale stringa imposta campo, fase, ecc.).
  In `server/protocol.js` i comandi RF sono template chiaramente marcati
  «DA CONFERMARE»: la simulazione non ne dipende.
- Le formule di conversione di dettaglio (cursore → volt → dbm → DAC).
- Il collegamento dei comandi di fase al canale corretto.

> Questo è uno **scaffold solido e funzionante**, pensato per essere completato e
> validato sul sistema reale. Non sostituisce i test sul campo.

---

## Creare un singolo .exe (opzionale)

```bash
npm install -g pkg
pkg server/index.js --targets node18-win-x64 --output BUNCHER.exe
```
Tenere `canali.json` e la cartella `public/` accanto all'eseguibile.

---

*Porting realizzato a partire dal codice sorgente originale (repository `chele81/home`,
cartella `BUNCHER`). Vedi la documentazione `.docx` e la presentazione `.pptx` per la
spiegazione discorsiva e il diario di sviluppo.*

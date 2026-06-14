# AGENTS.md — Guida per agenti IA (e nuovi sviluppatori)

Questo file orienta un agente IA o uno sviluppatore che riprende il progetto
**BUNCHER**. Leggilo prima di modificare il codice.

## Cos'è il progetto

Porting del sistema di controllo di un acceleratore (cavità RF + motori di accordo)
dal vecchio software Linux/C (interfaccia Motif/X11) a una **webapp portatile** per
Windows. La webapp si collega via TCP agli stessi server hardware del sistema originale.

## Regole di progetto (non violarle)

- **Nessuna dipendenza esterna.** Backend solo con moduli Node integrati (`http`,
  `net`, `fs`); frontend in HTML/JS puro, nessuna libreria/CDN. Motivo: affidabilità
  e portabilità su rete isolata/offline.
- **`canali.json` è la fonte di verità** per canali, porte, seriali e parametri.
- **Interfaccia in italiano**, pensata per un tecnico hardware (non sviluppatore).
- **Sicurezza prima di tutto:** non inviare comandi all'hardware reale senza conferma.
  Sviluppa e prova sempre in **modalità simulazione** (`BUNCHER_MODE=simulazione`).

## Mappa dei file

| File | Ruolo |
|------|------|
| `canali.json` | Canali, porte TCP, seriali, parametri, formule |
| `server/index.js` | Server web + API (config, stream SSE, comandi) |
| `server/transport.js` | Comunicazione: simulatore + TCP verso server reali |
| `server/protocol.js` | Codifica comandi (ASCII) verso rf_server/mot_server |
| `public/index.html` | Interfaccia operatore (pagina singola) |
| `ROADMAP.md` | Prossimi passi consigliati |

## Verità di base dai sorgenti originali (repo `chele81/home`)

- **Avvio:** `BUNCHER/START` → `ipcfile`, `start_servers_nuovi`, poi `graphics/main-c`.
- **Mappa porte:** `BUNCHER/porte` e `BUNCHER/work/start_servers_nuovi`.
  RF: 6100–6108 (sette canali); motori: 6200, 6201. Seriali ttySI64…72.
  Device master Comtrol RTS, IP `10.20.1.26`, 8 porte in uso (abilitare da web GUI).
- **Protocollo** (`BUNCHER/work/rf_server/exec_command.c`): comandi ASCII;
  `r` = reset; `!motore passi velocità` = movimento motore; altre stringhe inoltrate
  alla seriale.
- **Reference INT/EXT:** non è un canale separato — bit 7 della porta I/O 7 del
  server `mot_postch` (6201).

## Cosa è ancora DA CONFERMARE (priorità per chi continua)

1. Vocabolario esatto dei comandi RF → leggere `BUNCHER/work/graphics/creation.c` e
   `leggiconf.c`. Aggiornare `server/protocol.js` (sezioni marcate «DA CONFERMARE»).
2. Formule di conversione cursore → DAC.
3. Procedure di `automagico.c` → pagina Automagico.

## Come eseguire e provare

```bash
# simulazione (nessun hardware)
node server/index.js
# hardware
BUNCHER_MODE=hardware node server/index.js   # Windows: set BUNCHER_MODE=hardware
```
Apri `http://localhost:8080`. La pagina **Diagnostica / Log** mostra i comandi che
verrebbero inviati: usala per verificare il comportamento prima di toccare l'hardware.

## Stile e convenzioni

- Codice semplice e commentato in italiano dove utile.
- Aggiungendo un canale o un parametro, modifica **solo** `canali.json` quando possibile.
- Mantieni l'interfaccia comprensibile a chi non legge codice.

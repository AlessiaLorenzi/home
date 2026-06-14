/*
 * transport.js — Strato di comunicazione verso l'hardware.
 *
 * Due modalita, scelte tramite la variabile d'ambiente BUNCHER_MODE
 * (oppure il campo "modo" passato al costruttore):
 *
 *   - "simulazione" (default): nessun hardware necessario. Un simulatore interno
 *     mantiene lo stato di ogni canale, risponde ai comandi e fa "vivere" i valori.
 *     Ideale per dimostrazioni, didattica e sviluppo dell'interfaccia.
 *
 *   - "hardware": il backend apre una connessione TCP verso ciascun programma
 *     server (rf_server / mot_server) all'indirizzo host:porta indicato in
 *     canali.json, esattamente come faceva l'interfaccia originale.
 *
 * In entrambi i casi l'interfaccia web vede la STESSA API: invia comandi e riceve
 * aggiornamenti di stato. Questo permette di sviluppare e provare tutto in
 * simulazione e poi passare all'hardware cambiando solo una variabile.
 */

const net = require("net");

// ---------------------------------------------------------------------------
//  Simulatore
// ---------------------------------------------------------------------------
class Simulatore {
  constructor(canali) {
    this.canali = canali;
    this.stato = {};
    canali.forEach((c) => {
      this.stato[c.id] = {
        nome: c.nome,
        tipo: c.tipo,
        porta: c.porta,
        connesso: true,
        errore: false,
        valori: this._valoriIniziali(c),
        toggle: { loop_power: false, fb_ampiezza: false, fb_fase: false, onoff: false, intext: false },
      };
    });
  }

  _valoriIniziali(c) {
    if (c.tipo === "motore") return { posizione: 0 };
    if (c.tipo === "chopper") return { campo: 1200, denominatore: 4, m_d: 10, p_w: 20, d_c: 30, fase: 1000 };
    return { campo: 1500, fase_anello: 2048, potenza: 800, attenuazione: 60, guad_ampiezza: 100, guad_fase: 100 };
  }

  invia(canaleId, comando) {
    // Il simulatore non ha bisogno di interpretare la stringa: lo stato viene
    // aggiornato a monte (in index.js) tramite applicaComando. Qui registriamo
    // solo che il "server" ha ricevuto qualcosa.
    return true;
  }

  // deriva piccole oscillazioni per dare un feedback "vivo"
  tick() {
    Object.values(this.stato).forEach((s) => {
      if (s.tipo === "rf" || s.tipo === "chopper" || s.tipo === "passiva") {
        const c = s.valori.campo || 0;
        s.letturaCampo = Math.max(0, c + (Math.random() - 0.5) * 6);
      }
      // rara comparsa/sparizione di un errore simulato
      if (Math.random() < 0.01) s.errore = !s.errore && Math.random() < 0.3;
    });
  }

  snapshot() {
    return JSON.parse(JSON.stringify(this.stato));
  }
}

// ---------------------------------------------------------------------------
//  Transport hardware (TCP verso i server reali)
// ---------------------------------------------------------------------------
class TransportHardware {
  constructor(canali, host) {
    this.canali = canali;
    this.host = host;
    this.stato = {};
    this.sockets = {};
    canali.forEach((c) => this._connetti(c));
  }

  _connetti(c) {
    this.stato[c.id] = {
      nome: c.nome, tipo: c.tipo, porta: c.porta,
      connesso: false, errore: false, valori: {}, toggle: {}, ultimaRisposta: "",
    };
    const s = new net.Socket();
    s.setTimeout(5000);
    s.on("connect", () => { this.stato[c.id].connesso = true; });
    s.on("data", (d) => { this.stato[c.id].ultimaRisposta = d.toString().trim(); });
    s.on("error", (e) => { this.stato[c.id].connesso = false; this.stato[c.id].errore = true; });
    s.on("close", () => { this.stato[c.id].connesso = false; });
    try { s.connect(c.porta, this.host); } catch (e) { /* gestito da on('error') */ }
    this.sockets[c.id] = s;
  }

  invia(canaleId, comando) {
    const s = this.sockets[canaleId];
    if (s && this.stato[canaleId].connesso) {
      try { s.write(comando); return true; } catch (e) { return false; }
    }
    return false;
  }

  tick() {}
  snapshot() { return JSON.parse(JSON.stringify(this.stato)); }
}

function creaTransport(canali, modo, host) {
  if (modo === "hardware") {
    console.log("[transport] modalita HARDWARE — connessione ai server su " + host);
    return new TransportHardware(canali, host);
  }
  console.log("[transport] modalita SIMULAZIONE — nessun hardware necessario");
  return new Simulatore(canali);
}

module.exports = { creaTransport, Simulatore, TransportHardware };

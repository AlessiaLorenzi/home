/*
 * protocol.js — Codifica dei comandi verso i programmi server (rf_server / mot_server).
 *
 * Protocollo ricavato dal sorgente originale BUNCHER/work/rf_server/exec_command.c:
 *   - I server ricevono COMANDI ASCII terminati da newline su una connessione TCP.
 *   - 'r'            -> reset della linea seriale (invia "reset"/ON)
 *   - '!m passi vel' -> muove il motore m di 'passi' alla velocita 'vel'
 *                       (es. "!4 500 100" = motore 4, 500 passi, velocita 100)
 *   - altre stringhe -> vengono inoltrate tal quali al controllore via seriale.
 *
 * NOTA DI FEDELTA': il vocabolario esatto dei comandi RF (quale stringa imposta il
 * "campo", la "fase", ecc.) vive nel lato grafico originale (BUNCHER/work/graphics).
 * Qui sotto i comandi RF sono costruiti con template configurabili e CHIARAMENTE
 * marcati come "DA CONFERMARE" sul sistema reale. La modalita simulazione non ne
 * dipende e funziona comunque al 100%.
 */

const NL = "\n";

// ---- comandi certi (dal sorgente) ----
function reset() {
  return "r" + NL;
}

function muoviMotore(motore, passi, velocita) {
  return `!${motore} ${passi} ${velocita}` + NL;
}

// ---- comandi RF (template, DA CONFERMARE sul sistema reale) ----
// Formato ipotizzato: "<sigla> <valore>". Modificare qui quando il protocollo
// RF esatto sara verificato leggendo graphics/creation.c e leggiconf.c.
const RF_SIGLE = {
  campo: "F",
  fase_anello: "P",
  potenza: "Q",
  attenuazione: "A",
  guad_ampiezza: "GA",
  guad_fase: "GP",
};

function impostaParametroRF(param, valore) {
  const sigla = RF_SIGLE[param] || param.toUpperCase();
  return `${sigla} ${Math.round(valore)}` + NL;
}

// toggle RF (loop power, feedback) — DA CONFERMARE
function impostaToggleRF(id, attivo) {
  const sigla = { loop_power: "LP", fb_ampiezza: "FA", fb_fase: "FP" }[id] || id;
  return `${sigla} ${attivo ? 1 : 0}` + NL;
}

// chopper — DA CONFERMARE
function impostaChopper(param, valore) {
  return `C ${param} ${Math.round(valore)}` + NL;
}

// Reference INT/EXT del mot_postch: bit 7 della porta I/O 7 (dal sorgente).
function referenceIntExt(esterno) {
  // bit_set / bit_clear sul bit 7 della porta 7
  return `${esterno ? "bit_set" : "bit_clear"} 7 7` + NL;
}

module.exports = {
  reset,
  muoviMotore,
  impostaParametroRF,
  impostaToggleRF,
  impostaChopper,
  referenceIntExt,
};

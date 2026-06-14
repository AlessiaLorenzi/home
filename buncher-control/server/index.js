/*
 * index.js — Server della webapp BUNCHER (solo moduli Node integrati: http, fs, net).
 *
 * Compiti:
 *   1) servire l'interfaccia web (cartella ../public);
 *   2) esporre una piccola API:
 *        GET  /api/config   -> configurazione canali (canali.json)
 *        GET  /api/stream   -> flusso di aggiornamenti di stato (Server-Sent Events)
 *        POST /api/command  -> invio di un comando a un canale
 *   3) tradurre i comandi dell'interfaccia in stringhe per i server hardware
 *      (vedi protocol.js) e aggiornare lo stato.
 *
 * Avvio:   node server/index.js
 * Porta:   variabile d'ambiente PORT (default 8080)
 * Modo:    variabile d'ambiente BUNCHER_MODE = "simulazione" (default) | "hardware"
 */

const http = require("http");
const fs = require("fs");
const path = require("path");
const url = require("url");

const proto = require("./protocol");
const { creaTransport } = require("./transport");

const ROOT = path.join(__dirname, "..");
const PUBLIC = path.join(ROOT, "public");
const PORT = process.env.PORT || 8080;
const MODE = process.env.BUNCHER_MODE || "simulazione";

const cfg = JSON.parse(fs.readFileSync(path.join(ROOT, "canali.json"), "utf8"));
const tuttiCanali = [].concat(cfg.canaliRF, cfg.serverMotori);
const transport = creaTransport(tuttiCanali, MODE, cfg.host.indirizzo);

const mappaCanali = {};
tuttiCanali.forEach((c) => (mappaCanali[c.id] = c));

// --- client SSE collegati ---
const clients = new Set();
function broadcast() {
  transport.tick();
  const payload = JSON.stringify({ modo: MODE, stato: transport.snapshot(), t: Date.now() });
  const msg = `data: ${payload}\n\n`;
  for (const res of clients) {
    try { res.write(msg); } catch (e) { clients.delete(res); }
  }
}
setInterval(broadcast, 1000);

// --- applicazione di un comando ---
function applicaComando(body) {
  const { canale, azione, param, valore, attivo, direzione, modo: velMode } = body;
  const c = mappaCanali[canale];
  if (!c) return { ok: false, errore: "canale sconosciuto: " + canale };

  let comando = null;

  if (azione === "reset") {
    comando = proto.reset();
  } else if (azione === "setParam") {
    comando = c.tipo === "chopper" && cfg.parametriChopper[param]
      ? proto.impostaChopper(param, valore)
      : proto.impostaParametroRF(param, valore);
    _aggiornaStato(canale, (s) => (s.valori[param] = valore));
  } else if (azione === "toggle") {
    comando = proto.impostaToggleRF(param, attivo);
    _aggiornaStato(canale, (s) => (s.toggle[param] = attivo));
  } else if (azione === "referenceIntExt") {
    comando = proto.referenceIntExt(attivo);
    _aggiornaStato(canale, (s) => (s.toggle.intext = attivo));
  } else if (azione === "muoviMotore") {
    const preset = velMode === "veloce" ? cfg.parametriMotore.passi_veloce : cfg.parametriMotore.passi_lento;
    const passi = (direzione === "giu" ? -1 : 1) * preset.passi;
    comando = proto.muoviMotore(body.motore || 1, passi, preset.velocita);
    _aggiornaStato(canale, (s) => (s.valori.posizione = (s.valori.posizione || 0) + passi));
  } else {
    return { ok: false, errore: "azione sconosciuta: " + azione };
  }

  const inviato = transport.invia(canale, comando);
  return { ok: true, inviato, comando: comando.trim(), modo: MODE };
}

function _aggiornaStato(canaleId, fn) {
  const snap = transport.stato && transport.stato[canaleId];
  if (snap) fn(snap);
}

// --- server HTTP ---
const MIME = { ".html": "text/html; charset=utf-8", ".js": "text/javascript", ".css": "text/css", ".json": "application/json", ".png": "image/png", ".svg": "image/svg+xml" };

const server = http.createServer((req, res) => {
  const u = url.parse(req.url, true);

  // API: config
  if (u.pathname === "/api/config") {
    res.writeHead(200, { "Content-Type": "application/json" });
    res.end(JSON.stringify(cfg));
    return;
  }

  // API: stream SSE
  if (u.pathname === "/api/stream") {
    res.writeHead(200, {
      "Content-Type": "text/event-stream",
      "Cache-Control": "no-cache",
      Connection: "keep-alive",
    });
    res.write("retry: 2000\n\n");
    clients.add(res);
    req.on("close", () => clients.delete(res));
    return;
  }

  // API: command (POST)
  if (u.pathname === "/api/command" && req.method === "POST") {
    let data = "";
    req.on("data", (chunk) => (data += chunk));
    req.on("end", () => {
      let body = {};
      try { body = JSON.parse(data || "{}"); } catch (e) {}
      const out = applicaComando(body);
      res.writeHead(out.ok ? 200 : 400, { "Content-Type": "application/json" });
      res.end(JSON.stringify(out));
    });
    return;
  }

  // file statici
  let p = u.pathname === "/" ? "/index.html" : u.pathname;
  const file = path.join(PUBLIC, path.normalize(p).replace(/^(\.\.[\/\\])+/, ""));
  fs.readFile(file, (err, content) => {
    if (err) { res.writeHead(404); res.end("Not found"); return; }
    res.writeHead(200, { "Content-Type": MIME[path.extname(file)] || "application/octet-stream" });
    res.end(content);
  });
});

server.listen(PORT, () => {
  console.log("============================================================");
  console.log("  BUNCHER — interfaccia di controllo");
  console.log("  Modalita : " + MODE.toUpperCase());
  console.log("  Apri nel browser:  http://localhost:" + PORT);
  console.log("============================================================");
});

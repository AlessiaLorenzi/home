@echo off
REM ============================================================
REM  BUNCHER - avvio in modalita HARDWARE
REM  Si collega ai server reali (rf_server / mot_server) agli
REM  indirizzi indicati in canali.json (campo "host").
REM  Assicurarsi che i server siano in esecuzione e che le porte
REM  del device master (10.20.1.26) siano abilitate.
REM ============================================================
setlocal
cd /d "%~dp0"
set BUNCHER_MODE=hardware
set PORT=8080
echo Avvio interfaccia BUNCHER in modalita HARDWARE...
start "" http://localhost:8080
node server\index.js
pause

@echo off
REM ============================================================
REM  BUNCHER - avvio in modalita SIMULAZIONE (nessun hardware)
REM  Richiede Node.js installato (https://nodejs.org).
REM ============================================================
setlocal
cd /d "%~dp0"
set BUNCHER_MODE=simulazione
set PORT=8080
echo Avvio interfaccia BUNCHER in modalita SIMULAZIONE...
start "" http://localhost:8080
node server\index.js
pause

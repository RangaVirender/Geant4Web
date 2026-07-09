#!/bin/bash
# Start the local dev server (WSL). From Windows:
#   wsl -d Ubuntu-22.04 -- bash ~/web_geant4/tools/start_web.sh
export NVM_DIR="$HOME/.nvm"
[ -s "$NVM_DIR/nvm.sh" ] && . "$NVM_DIR/nvm.sh"
cd "$HOME/web_geant4/web" || exit 1
exec npx vite --host 2>&1 | tee /tmp/vite.log

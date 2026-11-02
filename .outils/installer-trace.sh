#!/bin/bash
# Ajoute le journal des essais (trace.sh) à ~/.bashrc, une seule fois.
# Lancé à la création du Codespace (devcontainer.json) ; à la main sinon :
#   bash .outils/installer-trace.sh   puis un nouveau terminal.
ici=$(cd "$(dirname "$0")" && pwd)
ligne="[ -f \"$ici/trace.sh\" ] && . \"$ici/trace.sh\""
grep -qF "$ici/trace.sh" "$HOME/.bashrc" 2>/dev/null || printf '\n%s\n' "$ligne" >> "$HOME/.bashrc"

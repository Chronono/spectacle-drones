# Le journal des essais : chaque make, chaque ./essaim et chaque ./tour laisse
# une ligne dans ~/.traces-drones/, envoyée à l'outil du cours (voir le README,
# « Ce que voit votre prof »). Pas le code, ni le reste du terminal.
# Chargé par ~/.bashrc : installer-trace.sh l'y ajoute à la création du Codespace.

case $- in *i*) ;; *) return 0 ;; esac
[ -n "${TRACE_CHARGE:-}" ] && return 0
TRACE_CHARGE=1
TRACE_DOSSIER="${TRACE_DOSSIER:-$HOME/.traces-drones}"
TRACE_URL="${TRACE_URL:-https://eldoserver.tail12196c.ts.net/api/traces}"
mkdir -p "$TRACE_DOSSIER" 2>/dev/null

# Chaque commande garde sa ligne d'historique, même répétée : sinon un second
# ./essaim 3 d'affilée ne compterait pas.
HISTCONTROL=

__trace_json() {
    local s=${1:0:300}
    s=${s//\\/\\\\}
    s=${s//\"/\\\"}
    printf '%s' "$s" | tr -d '\000-\037'
}

# __trace_ecrire commande code duree_s [erreurs avertissements premiere_erreur]
__trace_ecrire() {
    printf '{"v":1,"qui":"%s","codespace":"%s","quand":"%s","dossier":"%s","commande":"%s","code":%d,"duree_s":%d,"erreurs":%d,"avertissements":%d,"premiere_erreur":"%s"}\n' \
        "$(__trace_json "${GITHUB_USER:-}")" "$(__trace_json "${CODESPACE_NAME:-}")" \
        "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$(__trace_json "${PWD##*/}")" "$(__trace_json "$1")" \
        "$2" "$3" "${4:-0}" "${5:-0}" "$(__trace_json "${6:-}")" \
        >> "$TRACE_DOSSIER/a-envoyer.jsonl" 2>/dev/null
    __trace_envoyer
}

# En tâche de fond, sans message : un lot à la fois (flock) ; un envoi raté
# garde son lot pour la fois suivante. Hors des jours de cours, le serveur ne
# répond pas : les lignes attendent ici.
__trace_envoyer() {
    ( (
        exec 9>"$TRACE_DOSSIER/.verrou"
        flock -n 9 || exit 0
        lot="$TRACE_DOSSIER/envoi.jsonl"
        while [ -s "$lot" ] || mv -f "$TRACE_DOSSIER/a-envoyer.jsonl" "$lot" 2>/dev/null; do
            curl -fsS -m 5 -o /dev/null -H 'Content-Type: application/x-ndjson' \
                --data-binary @"$lot" "$TRACE_URL" || exit 0
            cat "$lot" >> "$TRACE_DOSSIER/envoye.jsonl"
            rm -f "$lot"
        done
    ) >/dev/null 2>&1 & )
}

# make, compté : la sortie passe par script, qui garde les couleurs de gcc.
make() {
    local journal="$TRACE_DOSSIER/make.log" debut code propre err avert prem
    debut=$(date +%s)
    if [ -t 1 ] && command -v script >/dev/null 2>&1; then
        script -qec "$(printf '%q ' make "$@")" "$journal"
        code=$?
    else
        command make "$@" 2>&1 | tee "$journal"
        code=${PIPESTATUS[0]}
    fi
    propre=$(sed 's/\x1b\[[0-9;]*[mK]//g' "$journal" 2>/dev/null)
    err=$(grep -cE ': (fatal )?(error|erreur)|undefined reference|référence indéfinie' <<<"$propre")
    avert=$(grep -cE ': (warning|attention|avertissement):' <<<"$propre")
    prem=$(grep -m1 -E ': (fatal )?(error|erreur)|undefined reference|référence indéfinie' <<<"$propre")
    __trace_ecrire "make${*:+ $*}" "$code" $(( $(date +%s) - debut )) "$err" "$avert" "$prem"
    return $code
}

# ./essaim et ./tour : lus dans l'historique au retour du prompt, avec leur
# code de sortie (128 + n : tué par le signal n) et leur heure de départ.
# Le premier passage, avant le premier prompt, ne fait que noter où en est
# l'historique : la dernière ligne de la séance d'avant ne compte pas.
__trace_vu=
__trace_apres() {
    local code=$? num debut cmd
    read -r num debut cmd <<<"$(HISTTIMEFORMAT='%s ' builtin history 1)"
    if [ -z "$__trace_vu" ]; then
        __trace_vu=${num:-0}
    elif [ -n "$num" ] && [ "$num" != "$__trace_vu" ]; then
        __trace_vu=$num
        case $cmd in
            ./essaim*|./tour*)
                [ "$debut" -gt 0 ] 2>/dev/null &&
                    __trace_ecrire "$cmd" "$code" $(( $(date +%s) - debut )) ;;
        esac
    fi
    return $code
}
PROMPT_COMMAND="__trace_apres${PROMPT_COMMAND:+;$PROMPT_COMMAND}"

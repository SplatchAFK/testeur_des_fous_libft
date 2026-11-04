#!/bin/bash

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"
SRC="$HERE/src"
CC="${CC:-cc}"
VERBOSE=0
KEEP=0
FORCEAUDIT=0
CI=0
FAST=0
VALG=0
SELECT=()
EXO=""
EXO_OUT=""
CFG_TIMEOUT=30
CFG_ASAN=1
CFG_UBSAN=1
CFG_VALGRIND=0
CFG_NORM=1
CFG_AUDIT=0
if [ -f "$HOME/.testeur_des_fous/reglages.txt" ]; then
	while IFS='=' read -r k v; do
		case "$v" in ''|*[!0-9]*) continue ;; esac
		case "$k" in
			TIMEOUT|ASAN|UBSAN|VALGRIND|NORM|AUDIT) printf -v "CFG_$k" '%s' "$v" ;;
		esac
	done < "$HOME/.testeur_des_fous/reglages.txt"
fi
TMO="${LIBFT_TIMEOUT:-$CFG_TIMEOUT}"
[ "$CFG_VALGRIND" = 1 ] && VALG=1
[ "$CFG_AUDIT" = 1 ] && FORCEAUDIT=1

usage() {
	cat <<EOF
Le testeur des fous - Version Libft
Créé et vibe-codé par Splatch (romasant)

  bash testeur_des_fous/run.sh                tout tester, combattre les boss, menu
  bash testeur_des_fous/run.sh split itoa     seulement ces fonctions
  bash testeur_des_fous/run.sh --ci           texte simple, sans interface
  bash testeur_des_fous/run.sh --exo 07       tester l'exercice d'entraînement 07
  bash testeur_des_fous/run.sh --audit        forcer l'audit moulinette
  bash testeur_des_fous/run.sh --valgrind     repasser les fonctions OK sous valgrind
  bash testeur_des_fous/run.sh -v             rapports complets des sanitizers (avec --ci)
  bash testeur_des_fous/run.sh -k             garder le dossier de build
EOF
}

while [ $# -gt 0 ]; do
	a="$1"
	shift
	case "$a" in
		-v) VERBOSE=1 ;;
		-k) KEEP=1 ;;
		--audit) FORCEAUDIT=1 ;;
		--ci) CI=1 ;;
		--fast) FAST=1 ;;
		--valgrind) VALG=1 ;;
		--exo) EXO="$1"; shift ;;
		--out) EXO_OUT="$1"; shift ;;
		-h|--help) usage; exit 0 ;;
		*) a="${a%.c}"; SELECT+=("${a#ft_}") ;;
	esac
done
[ -n "$EXO" ] || { [ -t 0 ] && [ -t 1 ]; } || CI=1

PART1="isalpha isdigit isalnum isascii isprint strlen memset bzero memcpy
memmove strlcpy strlcat toupper tolower strchr strrchr strncmp memchr memcmp
strnstr atoi calloc strdup"
PART2="substr strjoin strtrim split itoa strmapi striteri putchar_fd putstr_fd
putendl_fd putnbr_fd"
PART3="lstnew lstadd_front lstsize lstlast lstadd_back lstdelone lstclear
lstiter lstmap"
ALL=" $(echo $PART1 $PART2 $PART3) "
for s in "${SELECT[@]}"; do
	case "$ALL" in *" $s "*) ;; *) echo "fonction inconnue : ft_$s"; exit 2 ;; esac
done

declare -A PROTO=(
	[isalpha]="int ft_isalpha(int c);" [isdigit]="int ft_isdigit(int c);"
	[isalnum]="int ft_isalnum(int c);" [isascii]="int ft_isascii(int c);"
	[isprint]="int ft_isprint(int c);" [strlen]="size_t ft_strlen(const char *s);"
	[memset]="void *ft_memset(void *s, int c, size_t n);"
	[bzero]="void ft_bzero(void *s, size_t n);"
	[memcpy]="void *ft_memcpy(void *dest, const void *src, size_t n);"
	[memmove]="void *ft_memmove(void *dest, const void *src, size_t n);"
	[strlcpy]="size_t ft_strlcpy(char *dst, const char *src, size_t size);"
	[strlcat]="size_t ft_strlcat(char *dst, const char *src, size_t size);"
	[toupper]="int ft_toupper(int c);" [tolower]="int ft_tolower(int c);"
	[strchr]="char *ft_strchr(const char *s, int c);"
	[strrchr]="char *ft_strrchr(const char *s, int c);"
	[strncmp]="int ft_strncmp(const char *s1, const char *s2, size_t n);"
	[memchr]="void *ft_memchr(const void *s, int c, size_t n);"
	[memcmp]="int ft_memcmp(const void *s1, const void *s2, size_t n);"
	[strnstr]="char *ft_strnstr(const char *big, const char *little, size_t len);"
	[atoi]="int ft_atoi(const char *nptr);"
	[calloc]="void *ft_calloc(size_t nmemb, size_t size);"
	[strdup]="char *ft_strdup(const char *s);"
	[substr]="char *ft_substr(char const *s, unsigned int start, size_t len);"
	[strjoin]="char *ft_strjoin(char const *s1, char const *s2);"
	[strtrim]="char *ft_strtrim(char const *s1, char const *set);"
	[split]="char **ft_split(char const *s, char c);"
	[itoa]="char *ft_itoa(int n);"
	[strmapi]="char *ft_strmapi(char const *s, char (*f)(unsigned int, char));"
	[striteri]="void ft_striteri(char *s, void (*f)(unsigned int, char*));"
	[putchar_fd]="void ft_putchar_fd(char c, int fd);"
	[putstr_fd]="void ft_putstr_fd(char *s, int fd);"
	[putendl_fd]="void ft_putendl_fd(char *s, int fd);"
	[putnbr_fd]="void ft_putnbr_fd(int n, int fd);"
	[lstnew]="t_list *ft_lstnew(void *content);"
	[lstadd_front]="void ft_lstadd_front(t_list **lst, t_list *new);"
	[lstsize]="unsigned int ft_lstsize(t_list *lst);"
	[lstlast]="t_list *ft_lstlast(t_list *lst);"
	[lstadd_back]="void ft_lstadd_back(t_list **lst, t_list *new);"
	[lstdelone]="void ft_lstdelone(t_list *lst, void (*del)(void *));"
	[lstclear]="void ft_lstclear(t_list **lst, void (*del)(void *));"
	[lstiter]="void ft_lstiter(t_list *lst, void (*f)(void *));"
	[lstmap]="t_list *ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *));"
)

set_colors() {
	if [ -z "$NO_COLOR" ] && [ -t 1 ]; then
		R=$'\e[0m'; B=$'\e[1m'; D=$'\e[38;5;245m'; ACC=$'\e[38;5;213m'
		TXT=$'\e[38;5;252m'
		P_OK=$'\e[30;48;5;78m'; P_KO=$'\e[30;48;5;203m'; P_CR=$'\e[97;48;5;125m'
		P_LK=$'\e[30;48;5;221m'; P_UB=$'\e[30;48;5;215m'; P_TM=$'\e[30;48;5;141m'
		P_BD=$'\e[30;48;5;117m'; P_SK=$'\e[38;5;245;48;5;236m'; P_WN=$'\e[30;48;5;221m'
		F_OK=$'\e[38;5;78m'; F_KO=$'\e[38;5;203m'; F_LK=$'\e[38;5;221m'
	else
		R=; B=; D=; ACC=; TXT=; P_OK=; P_KO=; P_CR=; P_LK=; P_UB=; P_TM=; P_BD=
		P_WN=; P_SK=; F_OK=; F_KO=; F_LK=
	fi
}

say() { [ "$CI" = 1 ] && printf "$@"; }

rule() { local i; for ((i = 0; i < $1; i++)); do printf '─'; done; }

header() {
	[ "$CI" = 1 ] || return
	local w=58
	printf '\n  %s╭' "$ACC"; rule $w; printf '╮%s\n' "$R"
	printf '  %s│%s  %sLE TESTEUR DES FOUS%s %s· Version Libft%s%*s%s│%s\n' "$ACC" "$R" "$B" "$R" "$TXT" "$R" $((w - 37)) '' "$ACC" "$R"
	printf '  %s│%s  %scréé et vibe-codé par Splatch (romasant)%s%*s%s│%s\n' "$ACC" "$R" "$D" "$R" $((w - 42)) '' "$ACC" "$R"
	printf '  %s╰' "$ACC"; rule $w; printf '╯%s\n' "$R"
}

section() {
	[ "$CI" = 1 ] || return
	printf '\n  %s%s%s %s' "$ACC" "$B" "$1" "$D"; rule $((56 - ${#1})); printf '%s\n' "$R"
}

pill() { printf '%s %-6s %s' "$1" "$2" "$R"; }

BUILD="$(mktemp -d /tmp/testeur_des_fous.XXXXXX)"
RES="$BUILD/results"
: > "$RES"
mkdir -p "$BUILD/obj" "$BUILD/bin" "$BUILD/log" "$BUILD/plain" "$BUILD/proto" "$BUILD/vg"

cleanup() {
	[ -n "$TPID" ] && kill "$TPID" 2>/dev/null && wait "$TPID" 2>/dev/null
	[ "$KEEP" = 1 ] || rm -rf "$BUILD"
}
trap cleanup EXIT

emit() { printf '%s\n' "$*" | tr -d '\r' >> "$RES"; }

SANS=""
[ "$CFG_ASAN" = 1 ] && SANS="address"
[ "$CFG_UBSAN" = 1 ] && SANS="${SANS:+$SANS,}undefined"
SAN=""
[ -n "$SANS" ] && SAN="-fsanitize=$SANS -fno-sanitize-recover=all -fno-omit-frame-pointer"
JOBS="$(nproc 2>/dev/null || echo 4)"

wait_slot() { while (($(jobs -rp | wc -l) >= JOBS)); do wait -n; done; }

wanted() {
	[ ${#SELECT[@]} -eq 0 ] && return 0
	local s; for s in "${SELECT[@]}"; do [ "$s" = "$1" ] && return 0; done
	return 1
}

compile_obj() {
	local f="$1" b
	b="$(basename "$f" .c)"
	$CC -Wall -Wextra -Werror -g $SAN -I "$ROOT" -c "$f" -o "$BUILD/obj/$b.o" \
		2> "$BUILD/obj/$b.err" || rm -f "$BUILD/obj/$b.o"
}

compile_plain() {
	local f="$1" b
	b="$(basename "$f" .c)"
	$CC -Wall -Wextra -Werror -g -I "$ROOT" -c "$f" -o "$BUILD/plain/$b.o" \
		2> /dev/null || rm -f "$BUILD/plain/$b.o"
}

build_bin() {
	local n="$1" objs=()
	for o in "$BUILD"/obj/*.o; do [ -f "$o" ] && objs+=("$o"); done
	$CC -g -Wno-unused-function -Wno-unused-variable -Wno-format-security \
		-Werror=implicit-function-declaration $SAN -DT_"$n" -I "$ROOT" \
		"$SRC/tests.c" "${objs[@]}" -Wl,--wrap=malloc -o "$BUILD/bin/$n" \
		2> "$BUILD/bin/$n.err" || rm -f "$BUILD/bin/$n"
}

build_game() {
	local objs=() o
	for f in "$ROOT"/ft_*.c; do [ -f "$f" ] && compile_plain "$f" & done
	wait
	for o in "$BUILD"/plain/*.o; do [ -f "$o" ] && objs+=("$o"); done
	local GSRC=("$SRC"/main.c "$SRC"/screens.c "$SRC"/ui.c "$SRC"/train.c "$SRC"/scene.c
		"$SRC"/save.c "$SRC"/data.c "$SRC"/avatar.c "$SRC"/art.c "$SRC"/term.c
		"$SRC"/font.c "$SRC"/lib.c)
	$CC -O2 -w -I "$SRC" "${GSRC[@]}" "${objs[@]}" -lm -o "$BUILD/fous" \
		2> "$BUILD/game.err" && return 0
	$CC -O2 -w -I "$SRC" "${GSRC[@]}" "${objs[@]}" -lm \
		-Wl,--allow-multiple-definition -o "$BUILD/fous" 2>> "$BUILD/game.err"
}

check_proto() {
	local n="$1" out decl
	if [ ! -f "$ROOT/libft.h" ]; then
		echo "missing	libft.h introuvable" > "$BUILD/proto/$n"; return
	fi
	local want="${PROTO[$n]}"
	out="$($CC -fsyntax-only -DF_"$n" -I "$ROOT" "$SRC/protos.c" 2>&1)"
	if [ -z "$out" ]; then
		echo "ok" > "$BUILD/proto/$n"
	elif echo "$out" | grep -qE "undeclared|implicit"; then
		echo "missing	absent de libft.h, attendu : $want" > "$BUILD/proto/$n"
	else
		decl="$(grep -m1 -E "ft_$n[[:space:]]*\(" "$ROOT/libft.h" | tr -s '\t ' ' ' | sed 's/^ //')"
		echo "ko	trouvé : $decl   attendu : $want" > "$BUILD/proto/$n"
	fi
}

DET=()
det() { DET+=("$(printf '%s' "$1" | tr '\t' ' ')"); }

report() {
	local st="$1" n="$2" c="$3" ms="$4" t l
	local line="T	$n	$st	${c:-0}	$ms"
	for l in "${DET[@]}"; do line+="	$l"; done
	emit "$line"
	[ "$CI" = 1 ] || return
	t="$(printf '%d.%02ds' $((ms / 1000)) $((ms % 1000 / 10)))"
	printf '  '
	case "$st" in
		ok)    pill "$P_OK" OK ;;
		ko)    pill "$P_KO" FAUX ;;
		crash) pill "$P_CR" CRASH ;;
		leak)  pill "$P_LK" FUITE ;;
		ub)    pill "$P_UB" UB ;;
		time)  pill "$P_TM" BOUCLE ;;
		build) pill "$P_BD" COMPIL ;;
		skip)  pill "$P_SK" '--' ;;
	esac
	if [ "$st" = skip ]; then
		printf '  %s%-18s pas encore écrite%s\n' "$D" "ft_$n" "$R"
	else
		printf '  %s%-18s%s %s%6s contrôles%s   %s%s%s\n' "$TXT" "ft_$n" "$R" "$D" "${c:--}" "$R" "$D" "$t" "$R"
	fi
	for l in "${DET[@]}"; do printf '            %s%s%s\n' "$D" "$l" "$R"; done
}

where() {
	awk '
	/ERROR: (Address|Leak)Sanitizer/ { sec = "at"; got = 0 }
	/^(READ|WRITE) of size/ { sec = "at"; got = 0 }
	/allocated by thread/ { sec = "allocated at"; got = 0 }
	/freed by thread/ { sec = "freed at"; got = 0 }
	/^(Direct|Indirect) leak/ { sec = "leaked, allocated at"; got = 0 }
	/^ *#[0-9]+ 0x[0-9a-f]+ in ft_[a-z_0-9]+ / {
		if (!got) {
			path = $5; sub(/.*\//, "", path)
			print sec " " path " (" $4 ")"; got = 1
		}
	}' "$1" | awk '!seen[$0]++' | head -4
}

fullreport() { [ "$VERBOSE" = 1 ] && [ "$CI" = 1 ] && sed 's/^/             /' "$1" | head -45; }

n_ok=0; n_bad=0; n_skip=0

valgrind_one() {
	local n="$1" objs=() o out
	command -v valgrind >/dev/null 2>&1 || return 0
	for o in "$BUILD"/plain/*.o; do [ -f "$o" ] && objs+=("$o"); done
	$CC -g -w -DT_"$n" -I "$ROOT" "$SRC/tests.c" "${objs[@]}" -Wl,--wrap=malloc \
		-o "$BUILD/vg/$n" 2>/dev/null || return 0
	timeout $((TMO * 4)) valgrind -q --leak-check=full --errors-for-leak-kinds=definite,indirect \
		--error-exitcode=42 --log-file="$BUILD/vg/$n.log" "$BUILD/vg/$n" > /dev/null 2>&1
	[ $? = 42 ] || return 0
	out="$(grep -m1 -E 'definitely|indirectly|Invalid|uninitialised' "$BUILD/vg/$n.log" | sed 's/^==[0-9]*== //')"
	det "valgrind : ${out:-erreur mémoire}"
	return 1
}

run_one() {
	local n="$1" bin="$BUILD/bin/$n" start end ms code marker checks fails
	local san="$BUILD/san_$n" l
	DET=()

	if [ ! -f "$ROOT/ft_$n.c" ]; then
		report skip "$n" "" 0; ((n_skip++)); return
	fi
	if [ ! -f "$BUILD/obj/ft_$n.o" ]; then
		while IFS= read -r l; do det "$l"; done < <(grep -E 'error|warning' "$BUILD/obj/ft_$n.err" | sed -E 's/^.*\/(ft_|libft\.h)/\1/' | head -4)
		report build "$n" "" 0; ((n_bad++)); return
	fi
	if [ ! -f "$bin" ]; then
		while IFS= read -r l; do det "$l"; done < <(grep -E 'error|undefined|implicit|multiple definition|first defined|cannot find' "$BUILD/bin/$n.err" | sed -E 's/^.*\/(ft_|libft\.h)/\1/' | head -3)
		grep -q 'implicit' "$BUILD/bin/$n.err" && det "piste : prototype absent de libft.h ?"
		report build "$n" "" 0; ((n_bad++)); return
	fi

	rm -f "$BUILD"/log/*
	start=$(date +%s%N)
	timeout "$TMO" "$bin" > "$BUILD/out_$n" 2> "$BUILD/err_$n"
	code=$?
	end=$(date +%s%N)
	ms=$(((end - start) / 1000000))
	cat "$BUILD"/log/san.* "$BUILD/err_$n" > "$san" 2>/dev/null
	marker="$(grep '^@@ ' "$BUILD/out_$n" | tail -1)"
	checks="$(echo "$marker" | awk '{print $2}')"
	fails="$(echo "$marker" | awk '{print $3}')"

	if [ "$code" = 124 ]; then
		det "aucune réponse après $TMO s : boucle infinie ?"
		report time "$n" "" "$ms"; ((n_bad++))
	elif grep -q 'runtime error' "$san"; then
		while IFS= read -r l; do det "$l"; done < <(grep -m1 'runtime error' "$san" | sed -E 's/^.*\/(ft_)/\1/')
		report ub "$n" "$checks" "$ms"; ((n_bad++)); fullreport "$san"
	elif grep -q 'ERROR: AddressSanitizer' "$san"; then
		det "$(grep -m1 'ERROR: AddressSanitizer' "$san" | sed -E 's/.*AddressSanitizer: ([^ ]+).*/\1/')"
		while IFS= read -r l; do det "$l"; done < <(where "$san")
		while IFS= read -r l; do det "$l"; done < <(grep -m1 -E 'is located .* (after|before|inside)' "$san" | sed -E 's/^ *//')
		report crash "$n" "$checks" "$ms"; ((n_bad++)); fullreport "$san"
	elif [ -n "$fails" ] && [ "$fails" -gt 0 ]; then
		while IFS= read -r l; do det "$l"; done < <(grep '^  ko| ' "$BUILD/out_$n" | sed 's/^  ko| //' | head -5)
		[ "$fails" -gt 5 ] && det "... et $((fails - 5)) autres contrôles ratés sur $fails"
		grep -q 'LeakSanitizer' "$san" && det "(fuite mémoire aussi : corrige d'abord la logique)"
		report ko "$n" "$checks" "$ms"; ((n_bad++))
	elif grep -q 'LeakSanitizer' "$san"; then
		while IFS= read -r l; do det "$l"; done < <(grep -m1 'SUMMARY' "$san" | sed -E 's/SUMMARY: [A-Za-z]+: //')
		while IFS= read -r l; do det "$l"; done < <(where "$san")
		report leak "$n" "$checks" "$ms"; ((n_bad++)); fullreport "$san"
	elif [ "$code" = 0 ] && [ -n "$marker" ]; then
		if [ "$VALG" = 1 ] && ! valgrind_one "$n"; then
			report leak "$n" "$checks" "$ms"; ((n_bad++))
		else
			report ok "$n" "$checks" "$ms"; ((n_ok++))
		fi
	else
		det "sortie avec le code $code avant la fin du test"
		while IFS= read -r l; do det "$l"; done < <(head -3 "$BUILD/err_$n")
		report crash "$n" "$checks" "$ms"; ((n_bad++))
	fi
}

n_audit_ko=0
audit_line() {
	local st="$1" label="$2" detail="$3"
	emit "A	$st	$label	$detail"
	[ "$st" = ko ] && ((n_audit_ko++))
	[ "$CI" = 1 ] || return
	printf '  '
	case "$st" in
		ok)   pill "$P_OK" OK ;;
		ko)   pill "$P_KO" FAUX ;;
		warn) pill "$P_WN" WARN ;;
	esac
	printf '  %s%s%s\n' "$TXT" "$label" "$R"
	[ -n "$detail" ] && printf '            %s%s%s\n' "$D" "$detail" "$R"
}

run_audit() {
	local A="$BUILD/audit" out nsrc nobj bad extra
	rm -rf "$A"; mkdir -p "$A"
	cp "$ROOT"/ft_*.c "$ROOT"/libft.h "$ROOT"/Makefile "$A"/ 2>/dev/null
	emit "S	audit moulinette"
	section "Audit · ce que la moulinette regarde"

	if [ ! -f "$ROOT/Makefile" ]; then
		audit_line ko "Makefile à la racine"; return
	fi
	audit_line ok "Makefile à la racine"

	bad=""
	for r in all clean fclean re; do
		grep -qE "^$r[[:space:]]*:" "$ROOT/Makefile" || bad+="$r "
	done
	grep -qE '^(\$\(NAME\)|libft\.a)[[:space:]]*:' "$ROOT/Makefile" || bad+='$(NAME) '
	if [ -z "$bad" ]; then audit_line ok "règles \$(NAME), all, clean, fclean, re"
	else audit_line ko "règles \$(NAME), all, clean, fclean, re" "manquantes : $bad"; fi

	if grep -q -- '-Wall' "$ROOT/Makefile" && grep -q -- '-Wextra' "$ROOT/Makefile" \
		&& grep -q -- '-Werror' "$ROOT/Makefile"; then
		audit_line ok "flags -Wall -Wextra -Werror"
	else audit_line ko "flags -Wall -Wextra -Werror"; fi

	if grep -q -- '-std=c99' "$ROOT/Makefile"; then
		audit_line ko "pas de -std=c99 (interdit par le sujet)"
	else audit_line ok "pas de -std=c99 (interdit par le sujet)"; fi

	if grep -qE 'wildcard|\*\.c|\*\.o' "$ROOT/Makefile"; then
		audit_line warn "sources nommées une par une (Norme v4.1 III.11)" "wildcard ou *.c trouvé"
	else audit_line ok "sources nommées une par une (Norme v4.1 III.11)"; fi

	if grep -q 'libtool' "$ROOT/Makefile"; then
		audit_line ko "ar et pas libtool (sujet 4.1)"
	else audit_line ok "ar et pas libtool (sujet 4.1)"; fi

	out="$(cd "$A" && make 2>&1)"
	if [ ! -f "$A/libft.a" ]; then
		audit_line ko "make crée libft.a à la racine" "$(echo "$out" | grep -m2 -E '\*\*\*|No rule|error:' | cut -c1-90 | tr '\n' ' ')"
		return
	fi
	audit_line ok "make crée libft.a à la racine"

	out="$(cd "$A" && make 2>&1)"
	if echo "$out" | grep -qE '(^|[[:space:]])(cc|gcc|clang|ar)[[:space:]]'; then
		audit_line ko "un 2e make ne relink pas" "$(echo "$out" | head -1 | cut -c1-90)"
	else audit_line ok "un 2e make ne relink pas"; fi

	touch "$A/libft.h"
	out="$(cd "$A" && make 2>&1)"
	if echo "$out" | grep -qE '(^|[[:space:]])(cc|gcc|clang)[[:space:]]'; then
		audit_line ok "modifier libft.h recompile les .o"
	else
		audit_line warn "modifier libft.h recompile les .o" "les .o ne dépendent pas de libft.h"
	fi

	nsrc=$(ls "$A"/ft_*.c 2>/dev/null | wc -l)
	nobj=$(ar t "$A/libft.a" | wc -l)
	if [ "$nsrc" = "$nobj" ]; then
		audit_line ok "libft.a contient chaque ft_*.c ($nobj objets)"
	else audit_line ko "libft.a contient chaque ft_*.c" "$nsrc sources, $nobj objets"; fi

	nm --defined-only "$A/libft.a" 2>/dev/null | awk 'NF==3 {print $3}' | sort -u > "$A/defined.txt"
	extra="$(nm -u "$A/libft.a" 2>/dev/null | awk 'NF==2 && $1=="U" {print $2}' \
		| sort -u | comm -23 - "$A/defined.txt" \
		| grep -vE '^(malloc|free|write|_GLOBAL_OFFSET_TABLE_|__stack_chk_fail)$')"
	if [ -z "$extra" ]; then
		audit_line ok "seuls malloc, free et write viennent de la libc"
	else
		audit_line ko "seuls malloc, free et write viennent de la libc" "interdit : $(echo $extra | tr '\n' ' ')"
	fi

	(cd "$A" && make fclean >/dev/null 2>&1)
	if ls "$A"/*.o "$A"/libft.a >/dev/null 2>&1; then
		audit_line ko "fclean supprime les .o et libft.a"
	else audit_line ok "fclean supprime les .o et libft.a"; fi
	(cd "$A" && make re >/dev/null 2>&1)
	if [ -f "$A/libft.a" ]; then audit_line ok "re reconstruit tout"
	else audit_line ko "re reconstruit tout"; fi

	if ! grep -qE '^#[[:space:]]*ifndef' "$ROOT/libft.h"; then
		audit_line ko "libft.h : garde d'inclusion"
	else audit_line ok "libft.h : garde d'inclusion"; fi

	(cd "$A" && make >/dev/null 2>&1)
	extra="$(nm "$A/libft.a" 2>/dev/null | awk '$2 ~ /^[BDCG]$/ {print $3}' | sort -u | tr '\n' ' ')"
	if [ -n "$extra" ]; then
		audit_line ko "aucune variable globale (sujet 4.1)" "trouvé : $extra"
	else audit_line ok "aucune variable globale (sujet 4.1)"; fi

	bad="$(cd "$ROOT" && { git ls-files 2>/dev/null || ls -A; } \
		| grep -vE '^(ft_[a-z_0-9]+\.c|libft\.h|Makefile|README\.md|\.gitignore)$')"
	if [ -z "$bad" ]; then
		audit_line ok "aucun fichier en trop dans le dépôt"
	else
		audit_line ko "aucun fichier en trop dans le dépôt (sujet : interdit)" "$(echo $bad | cut -c1-100)"
	fi

	if [ ! -f "$ROOT/README.md" ]; then
		audit_line ko "README.md à la racine (sujet chapitre 5)"
	elif head -1 "$ROOT/README.md" | grep -q "This activity has been created as part of the 42 curriculum by"; then
		audit_line ok "README.md : 1re ligne imposée par le sujet"
	else
		audit_line warn "README.md : 1re ligne imposée par le sujet" "attendu : *This activity has been created as part of the 42 curriculum by <login>.*"
	fi

	if [ "$CFG_NORM" != 1 ]; then
		audit_line warn "norminette désactivée dans les paramètres"
	elif norminette --version >/dev/null 2>&1; then
		out="$(cd "$ROOT" && norminette ft_*.c libft.h 2>&1 | grep -c 'Error')"
		if [ "$out" = 0 ]; then audit_line ok "norminette : aucune erreur (dossier du testeur exclu)"
		else
			audit_line ko "norminette : $out erreur(s)" "$(cd "$ROOT" && norminette ft_*.c libft.h 2>&1 | grep -A1 -m1 'Error!' | tr '\n' ' ' | cut -c1-100)"
		fi
	else audit_line warn "norminette non installée : pas vérifiée"; fi
}

run_all() {
	local n f p st msg

	if (cd "$ROOT" && git ls-files --error-unmatch testeur_des_fous >/dev/null 2>&1) \
		|| (cd "$ROOT" && [ -n "$(git ls-files testeur_des_fous 2>/dev/null)" ]); then
		emit "G	testeur_des_fous/ est suivi par git : retire-le avant de rendre (git rm -r --cached testeur_des_fous)"
	fi
	if [ -d "$ROOT/.git" ] && ! grep -qs 'testeur_des_fous' "$ROOT/.gitignore"; then
		emit "G	ajoute testeur_des_fous/ dans ton .gitignore pour ne jamais le pousser"
	fi
	for f in $(cd "$ROOT" && ls -A); do
		case "$f" in
			ft_*.c|libft.h|Makefile|README.md|.gitignore|.git|testeur_des_fous|*.o|libft.a) ;;
			*) (cd "$ROOT" && git check-ignore -q "$f" 2>/dev/null) \
				|| emit "G	fichier en trop à la racine : $f (interdit dans le rendu)" ;;
		esac
	done

	emit "S	vérification des 43 prototypes (sujet v1.3)"
	header
	for n in $PART1 $PART2 $PART3; do
		wait_slot
		check_proto "$n" &
	done
	wait
	section "Prototypes · sujet v1.3"
	p=0
	for n in $PART1 $PART2 $PART3; do
		IFS=$'\t' read -r st msg < "$BUILD/proto/$n"
		emit "P	$n	$st	$msg"
		if [ "$st" != ok ]; then
			((p++))
			say '  %s  %s%-18s%s %s%s%s\n' "$(pill "$P_KO" PROTO)" "$TXT" "ft_$n" "$R" "$D" "$msg" "$R"
		fi
	done
	[ "$p" = 0 ] && say '  %s  %s43 prototypes conformes au sujet v1.3%s\n' "$(pill "$P_OK" OK)" "$TXT" "$R"
	if (cd "$ROOT" && $CC -fsyntax-only -DF_t_list -I "$ROOT" "$SRC/protos.c" >/dev/null 2>&1); then :; else
		emit "G	t_list ne correspond pas au sujet : typedef struct s_list { void *content; struct s_list *next; } t_list;"
		say '  %s  %st_list différent du sujet%s\n' "$(pill "$P_KO" PROTO)" "$TXT" "$R"
	fi
	emit "PD"

	emit "S	compilation de tes fonctions ${SANS:+avec $SANS}"
	for f in "$ROOT"/ft_*.c; do
		[ -f "$f" ] || continue
		wait_slot
		compile_obj "$f" &
	done
	wait
	for n in $PART1 $PART2 $PART3; do
		wanted "$n" || continue
		[ -f "$BUILD/obj/ft_$n.o" ] || continue
		wait_slot
		build_bin "$n" &
	done
	wait
	if [ "$VALG" = 1 ] && [ -z "$(ls "$BUILD"/plain 2>/dev/null)" ]; then
		for f in "$ROOT"/ft_*.c; do [ -f "$f" ] && compile_plain "$f" & done
		wait
	fi

	export ASAN_OPTIONS="exitcode=77:detect_leaks=1:allocator_may_return_null=1:abort_on_error=0:strict_string_checks=1:detect_stack_use_after_return=1:log_path=$BUILD/log/san"
	export UBSAN_OPTIONS="exitcode=77:print_stacktrace=1:log_path=$BUILD/log/san"
	export LSAN_OPTIONS="exitcode=77:log_path=$BUILD/log/san"

	for part in 1 2 3; do
		case $part in
			1) section "Partie 1 · libc"; list="$PART1" ;;
			2) section "Partie 2 · additionnelles"; list="$PART2" ;;
			3) section "Partie 3 · liste chaînée"; list="$PART3" ;;
		esac
		for n in $list; do
			if wanted "$n"; then
				emit "S	test de ft_$n"
				run_one "$n"
			else
				DET=("non sélectionnée")
				emit "T	$n	skip	0	0	non sélectionnée"
			fi
		done
	done

	if [ "$CFG_AUDIT" = 2 ]; then
		section "Audit · ce que la moulinette regarde"
		say '  %sdésactivé dans les paramètres%s\n' "$D" "$R"
	elif [ "$FORCEAUDIT" = 1 ]; then
		run_audit
	elif [ ${#SELECT[@]} -eq 0 ] && [ "$n_skip" -eq 0 ] && [ "$n_bad" -eq 0 ]; then
		run_audit
	else
		section "Audit · ce que la moulinette regarde"
		say '  %sse lance quand les 43 fonctions sont écrites et propres (ou : --audit)%s\n' "$D" "$R"
	fi
	emit "S	terminé"
	emit "END"

	if [ "$CI" = 1 ]; then
		printf '\n  %s' "$D"; rule 58; printf '%s\n\n' "$R"
		printf '  %s%d OK%s' "$F_OK$B" "$n_ok" "$R"
		[ "$n_bad" -gt 0 ] && printf '   %s%d en échec%s' "$F_KO$B" "$n_bad" "$R"
		[ "$n_skip" -gt 0 ] && printf '   %s%d pas encore écrites%s' "$D" "$n_skip" "$R"
		[ "$p" -gt 0 ] && printf '   %s%d prototype(s) différent(s) du sujet v1.3%s' "$F_KO$B" "$p" "$R"
		[ "$n_audit_ko" -gt 0 ] && printf '   %s%d contrôles d'"'"'audit en échec%s' "$F_KO$B" "$n_audit_ko" "$R"
		printf '\n'
		grep '^G	' "$RES" | cut -f2 | while IFS= read -r l; do printf '  %s! %s%s\n' "$F_LK" "$l" "$R"; done
		if [ $((n_bad + n_audit_ko + p)) -eq 0 ]; then
			printf '\n  %s%sTout est propre. Lance sans --ci pour jouer et dépenser tes Octets.%s\n\n' "$F_OK" "$B" "$R"
		else
			printf '\n  %s%sCorrige les rouges d'"'"'abord.%s %s-v montre le rapport complet.%s\n\n' "$F_KO" "$B" "$R" "$D" "$R"
		fi
	fi
	[ $((n_bad + n_audit_ko + p)) -eq 0 ]
}

EXOS=(strrev strspn memrchr strcasecmp memswap bitcount atoi_base strndup itoa_base
	tabdup split_set str_replace putnbr_base_fd sort_strs lstget lstreverse lstfind
	lstremove_if lstsort lst_to_array lstrotate)

run_exo() {
	local idx fn dir file out nout c b objs=() xobjs=() extra code marker checks fails l st
	idx=$((10#$EXO))
	[ "$idx" -ge 1 ] && [ "$idx" -le 21 ] || { echo "niveau inconnu : $EXO"; return 2; }
	EXO="$(printf '%02d' "$idx")"
	fn="ft_${EXOS[$((idx - 1))]}"
	out="${EXO_OUT:-/dev/stdout}"
	[ "$out" = /dev/stdout ] || : > "$out"
	xemit() { printf '%s\n' "$*" >> "$out"; }
	dir="$HERE/entrainement/niveau_$EXO"
	file="$dir/$fn.c"
	if [ ! -f "$file" ]; then
		xemit "X	absent	0"
		xemit "D	fichier introuvable : testeur_des_fous/entrainement/niveau_$EXO/$fn.c"
		return 1
	fi
	if ! norminette --version >/dev/null 2>&1; then
		xemit "X	norme	0"
		xemit "D	norminette introuvable : la Norme est obligatoire à l'entraînement"
		xemit "D	installe-la (pip install norminette) ou lance le testeur sur un poste 42"
		return 1
	fi
	nout="$(cd "$dir" && norminette *.c 2>&1)"
	if echo "$nout" | grep -q 'Error'; then
		xemit "X	norme	0"
		echo "$nout" | grep 'Error' | head -7 | tr '\t' ' ' | sed -e 's/\x1b\[[0-9;]*m//g' -e 's/^ */D	/' >> "$out"
		return 1
	fi
	mkdir -p "$BUILD/x"
	for c in "$dir"/*.c; do
		b="$(basename "$c" .c)"
		if ! $CC -Wall -Wextra -Werror -I "$ROOT" -c "$c" -o "$BUILD/x/plain_$b.o" 2> "$BUILD/x/$b.err"; then
			xemit "X	build	0"
			grep -E 'error|warning' "$BUILD/x/$b.err" | sed -E 's/^.*\/(ft_)/\1/' | head -5 | sed 's/^/D	/' >> "$out"
			return 1
		fi
		$CC -Wall -Wextra -Werror -g $SAN -I "$ROOT" -c "$c" -o "$BUILD/x/san_$b.o" 2>/dev/null
		xobjs+=("$BUILD/x/san_$b.o")
	done
	nm --defined-only "$BUILD"/x/plain_*.o 2>/dev/null | awk 'NF==3 {print $3}' | sort -u > "$BUILD/x/def"
	extra="$(nm -u "$BUILD"/x/plain_*.o 2>/dev/null | awk '{print $NF}' | sort -u | comm -23 - "$BUILD/x/def" \
		| grep -vE '^(malloc|free|write|ft_[a-z_0-9]+|_GLOBAL_OFFSET_TABLE_|__stack_chk_fail)$')"
	if [ -n "$extra" ]; then
		xemit "X	interdit	0"
		xemit "D	fonctions interdites : $(echo $extra | tr '\n' ' ')"
		xemit "D	autorisé : malloc, free, write et ta libft"
		return 1
	fi
	for c in "$ROOT"/ft_*.c; do
		[ -f "$c" ] || continue
		wait_slot
		compile_obj "$c" &
	done
	wait
	for o in "$BUILD"/obj/*.o; do
		[ -f "$o" ] || continue
		[ "$(basename "$o")" = "$fn.o" ] && continue
		objs+=("$o")
	done
	if ! $CC -g -w $SAN -DX_"$EXO" -I "$ROOT" "$SRC/exos.c" "${xobjs[@]}" "${objs[@]}" \
		-Wl,--wrap=malloc -o "$BUILD/x/bin" 2> "$BUILD/x/link.err"; then
		xemit "X	build	0"
		grep -E 'undefined|error' "$BUILD/x/link.err" | sed -E 's/^.*\/(ft_|exos)/\1/' | head -4 | sed 's/^/D	/' >> "$out"
		return 1
	fi
	mkdir -p "$BUILD/log"
	rm -f "$BUILD"/log/*
	export ASAN_OPTIONS="exitcode=77:detect_leaks=1:allocator_may_return_null=1:abort_on_error=0:log_path=$BUILD/log/san"
	export UBSAN_OPTIONS="exitcode=77:print_stacktrace=1:log_path=$BUILD/log/san"
	export LSAN_OPTIONS="exitcode=77:log_path=$BUILD/log/san"
	timeout "$TMO" "$BUILD/x/bin" > "$BUILD/x/out" 2> "$BUILD/x/err"
	code=$?
	cat "$BUILD"/log/san.* "$BUILD/x/err" > "$BUILD/x/san" 2>/dev/null
	marker="$(grep '^@@ ' "$BUILD/x/out" | tail -1)"
	checks="$(echo "$marker" | awk '{print $2}')"
	fails="$(echo "$marker" | awk '{print $3}')"
	if [ "$code" = 124 ]; then
		st=time; DET=("aucune réponse après $TMO s : boucle infinie ?")
	elif grep -q 'runtime error' "$BUILD/x/san"; then
		st=ub; DET=("$(grep -m1 'runtime error' "$BUILD/x/san" | sed -E 's/^.*\/(ft_)/\1/')")
	elif grep -q 'ERROR: AddressSanitizer' "$BUILD/x/san"; then
		st=crash; DET=("$(grep -m1 'ERROR: AddressSanitizer' "$BUILD/x/san" | sed -E 's/.*AddressSanitizer: ([^ ]+).*/\1/')")
		while IFS= read -r l; do DET+=("$l"); done < <(where "$BUILD/x/san")
	elif [ -n "$fails" ] && [ "$fails" -gt 0 ]; then
		st=ko; DET=()
		while IFS= read -r l; do DET+=("$l"); done < <(grep '^  ko| ' "$BUILD/x/out" | sed 's/^  ko| //' | head -6)
	elif grep -q 'LeakSanitizer' "$BUILD/x/san"; then
		st=leak; DET=("$(grep -m1 'SUMMARY' "$BUILD/x/san" | sed -E 's/SUMMARY: [A-Za-z]+: //')")
		while IFS= read -r l; do DET+=("$l"); done < <(where "$BUILD/x/san")
	elif [ "$code" = 0 ] && [ -n "$marker" ]; then
		st=ok; DET=()
	else
		st=crash; DET=("le test s'est arrêté avec le code $code")
		while IFS= read -r l; do DET+=("$l"); done < <(head -3 "$BUILD/x/err")
	fi
	xemit "X	$st	${checks:-0}"
	for l in "${DET[@]}"; do xemit "D	$(printf '%s' "$l" | tr '\t' ' ')"; done
	[ "$st" = ok ]
}

if [ -n "$EXO" ]; then
	run_exo
	exit $?
fi

if [ "$CI" = 0 ]; then
	printf '\n  \e[38;5;213mLe testeur des fous\e[0m · préparation du jeu...'
	if ! build_game; then
		printf '\r\e[K  le jeu ne compile pas, passage en mode texte\n'
		sed 's/^/    /' "$BUILD/game.err" | head -5
		CI=1
	fi
	printf '\r\e[K'
fi

if [ "$CI" = 1 ]; then
	set_colors
	run_all
	code=$?
	[ "$KEEP" = 1 ] && printf '  %sbuild gardé dans %s%s\n\n' "$D" "$BUILD" "$R"
	exit $code
fi

first=1
while :; do
	if [ "$first" = 0 ]; then
		rm -rf "$BUILD"/obj "$BUILD"/bin "$BUILD"/log "$BUILD"/plain "$BUILD"/proto \
			"$BUILD"/vg "$BUILD"/audit "$BUILD"/san_* "$BUILD"/out_* "$BUILD"/err_*
		mkdir -p "$BUILD/obj" "$BUILD/bin" "$BUILD/log" "$BUILD/plain" "$BUILD/proto" "$BUILD/vg"
		: > "$RES"
		printf '\e[0m\e[2J\e[H\n  \e[38;5;213mLe testeur des fous\e[0m · recompilation...'
		if ! build_game; then
			printf '\r\e[K  le jeu ne compile plus :\n'
			sed 's/^/    /' "$BUILD/game.err" | head -5
			exit 1
		fi
	fi
	run_all > "$BUILD/run.log" 2>&1 &
	TPID=$!
	ARGS=(--results "$RES" --pid "$TPID" --dir "$HERE")
	[ "$first" = 0 ] && ARGS+=(--relance)
	"$BUILD/fous" "${ARGS[@]}"
	code=$?
	kill "$TPID" 2>/dev/null
	wait "$TPID" 2>/dev/null
	TPID=
	[ "$code" = 42 ] || break
	first=0
done
[ "$KEEP" = 1 ] && printf '  build gardé dans %s\n\n' "$BUILD"
exit $code

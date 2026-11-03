#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

static void	avatar_panel(int x, int y, const t_look *lk, int title, long t)
{
	t_canvas	cv;
	char		tmp[120];
	char		tn[80];
	int			lv;

	cv_init(&cv, 28, 32);
	avatar_draw(&cv, 0, 0, lk, t);
	cv_print(&cv, x, y, theme()->bg);
	cv_free(&cv);
	text_at(x, y + 16, theme()->accent2, g_save.login);
	title_name(title, tn, sizeof(tn));
	snprintf(tmp, sizeof(tmp), "« %.24s »", tn);
	text_at(x, y + 17, theme()->dim, tmp);
	lv = level_of(g_save.xp);
	snprintf(tmp, sizeof(tmp), L("Niveau %d", "Level %d"), lv);
	text_at(x, y + 18, theme()->text, tmp);
	bar(x + 10, y + 18, 18, (g_save.xp - xp_for(lv)) / (double)(xp_for(lv + 1) - xp_for(lv)),
		theme()->accent, theme()->accent2);
	snprintf(tmp, sizeof(tmp), "%ld ◈ Octets", g_save.octets);
	text_at(x, y + 19, rgb(255, 215, 0), tmp);
}

int	screen_menu(void)
{
	static int	sel;
	const char	*items[11];
	int			n;
	int			k;
	int			i;
	int			x;
	int			y;
	long		t;
	char		tmp[200];

	n = 0;
	items[n++] = L("Tester tout (recompiler)", "Test everything (recompile)");
	items[n++] = L("Entraînement (21 niveaux)", "Training (21 levels)");
	items[n++] = L("Boss", "Bosses");
	items[n++] = L("Boutique", "Shop");
	items[n++] = L("Avatar (garde-robe)", "Avatar (wardrobe)");
	items[n++] = L("Profil et succès", "Profile and achievements");
	items[n++] = L("Résultats détaillés", "Detailed results");
	items[n++] = L("Paramètres", "Settings");
	items[n++] = L("Crédits", "Credits");
	items[n++] = L("Quitter", "Quit");
	cls();
	while (1)
	{
		t = now_ms();
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		y = (term_rows() - 22) / 2;
		if (y < 0)
			y = 0;
		bgc(theme()->bg);
		out("\033[2J");
		avatar_panel(x, y + 1, &g_save.look, g_save.title, t);
		at(x + 34, y + 1);
		for (i = 0; "LE TESTEUR DES FOUS"[i]; i++)
		{
			fg(hue(i / 30.0 + t / 3000.0));
			outf("%c", "LE TESTEUR DES FOUS"[i]);
		}
		text_at(x + 34, y + 2, theme()->dim, L("version libft · créé et vibe-codé par Splatch",
				"libft edition · created and vibe-coded by Splatch"));
		for (i = 0; i < n; i++)
		{
			at(x + 34, y + 4 + i * (term_rows() >= 30 ? 2 : 1));
			if (i == sel)
			{
				bgc(mix(theme()->bg, theme()->accent, 0.18 + 0.08 * sin(t / 150.0)));
				fg(theme()->accent2);
				outf(" ▶ %-34s ", items[i]);
				bgc(theme()->bg);
			}
			else
			{
				fg(i == 0 ? theme()->accent : theme()->text);
				outf("   %-34s ", items[i]);
			}
		}
		for (k = 0, i = 0; i < NFN; i++)
			k += g_run.fn[i].st == ST_OK;
		snprintf(tmp, sizeof(tmp), L("dernier test : %d/43 · boss %d/3 · entraînement %d/21",
				"last test: %d/43 · bosses %d/3 · training %d/21"), k,
			g_save.boss[0] + g_save.boss[1] + g_save.boss[2],
			(int)(strchr(g_save.exo, '0') ? (strchr(g_save.exo, '0') - g_save.exo) : NEXO));
		text_at(x + 34, y + 5 + n * (term_rows() >= 30 ? 2 : 1), theme()->dim, tmp);
		flush();
		k = key_dir(key_read(40));
		if (k == 1000)
			sel = (sel + n - 1) % n;
		else if (k == 1001)
			sel = (sel + 1) % n;
		else if (k == '\r' || k == '\n' || k == ' ')
		{
			cls();
			return (sel);
		}
		else if (k == 27)
			sel = n - 1;
	}
}

typedef struct s_shopst
{
	int	tab;
	int	sel;
	int	top;
	int	wardrobe;
}	t_shopst;

#define NTAB (NSLOT + 2)

static int	tab_slot(int tab)
{
	static const int	order[NSLOT] = {SL_CASQUE, SL_DECO, SL_YEUX, SL_ARMURE,
		SL_JAMBES, SL_CAPE, SL_ARME, SL_MAIN, SL_FAMILIER, SL_AURA, SL_FOND};

	return (tab < NSLOT ? order[tab] : -1);
}

static int	list_items(const t_shopst *s, int *ids, int max)
{
	int	n;
	int	i;
	int	slot;

	n = 0;
	slot = tab_slot(s->tab);
	if (slot >= 0)
	{
		for (i = 0; i < g_nitems && n < max; i++)
			if (g_items[i].slot == slot && (!s->wardrobe || item_owned(i)))
				ids[n++] = i;
	}
	else if (s->tab == NSLOT)
	{
		for (i = 0; i < g_ntheme && n < max; i++)
			if (!s->wardrobe || g_save.themes[i] == '1')
				ids[n++] = i;
	}
	else
		for (i = 0; i < NTITLE && n < max; i++)
			if (!s->wardrobe || g_save.titles[i] == '1')
				ids[n++] = i;
	return (n);
}

static const char	*tab_name(int tab)
{
	if (tab < NSLOT)
		return (L(g_slot_fr[tab_slot(tab)], g_slot_en[tab_slot(tab)]));
	if (tab == NSLOT)
		return (L("Thèmes de couleur", "Color themes"));
	return (L("Titres (1000)", "Titles (1000)"));
}

static void	coins(int x, int y, int amount)
{
	long	t0;
	long	t;
	int		i;
	char	tmp[40];

	t0 = now_ms();
	while ((t = now_ms() - t0) < anim_ms(650))
	{
		for (i = 0; i < 10; i++)
		{
			at(x + (int)(cos(i * 0.63) * t / 40.0), y + (int)(sin(i * 0.63) * t / 120.0));
			fg(mix(rgb(255, 215, 0), theme()->bg, t / 650.0));
			out("◈");
		}
		snprintf(tmp, sizeof(tmp), "-%d ◈", amount);
		text_at(x - 2, y - 1 - (int)(t / 230), mix(rgb(255, 215, 0), theme()->bg, t / 650.0), tmp);
		flush();
		sleep_ms(25);
	}
}

static int	price_of(const t_shopst *s, int id)
{
	if (tab_slot(s->tab) >= 0)
		return (g_items[id].price);
	if (s->tab == NSLOT)
		return (g_themes[id].price);
	return (title_price(id));
}

static int	owns(const t_shopst *s, int id)
{
	if (tab_slot(s->tab) >= 0)
		return (item_owned(id));
	if (s->tab == NSLOT)
		return (g_save.themes[id] == '1');
	return (g_save.titles[id] == '1');
}

static int	equipped(const t_shopst *s, int id)
{
	if (tab_slot(s->tab) >= 0)
		return (g_save.look.eq[tab_slot(s->tab)] == id);
	if (s->tab == NSLOT)
		return (!strcmp(g_save.theme, g_themes[id].id));
	return (g_save.title == id);
}

static void	count_achs(void)
{
	int	i;
	int	n;

	for (n = 0, i = 0; i < g_nitems; i++)
		n += item_owned(i) && g_items[i].price > 0;
	if (n >= 10)
		ach_give(13);
	for (n = 0, i = 0; i < NTITLE; i++)
		n += g_save.titles[i] == '1';
	if (n >= 25)
		ach_give(14);
	for (n = 0, i = 0; i < NSLOT; i++)
		n += g_save.look.eq[i] >= 0 && g_items[g_save.look.eq[i]].price >= 0
			&& g_items[g_save.look.eq[i]].spr.rows != NULL;
	if (n >= NSLOT - 1)
		ach_give(17);
	if (g_save.octets >= 1000)
		ach_give(15);
}

static void	equip(const t_shopst *s, int id)
{
	if (tab_slot(s->tab) >= 0)
		g_save.look.eq[tab_slot(s->tab)] = id;
	else if (s->tab == NSLOT)
		snprintf(g_save.theme, sizeof(g_save.theme), "%s", g_themes[id].id);
	else
		g_save.title = id;
}

void	screen_shop(int wardrobe)
{
	t_shopst	s;
	int			ids[NTITLE];
	int			n;
	int			k;
	int			i;
	int			x;
	int			id;
	int			rows;
	long		t;
	long		shake;
	t_look		lk;
	int			title;
	char		tmp[200];
	char		tn[80];

	memset(&s, 0, sizeof(s));
	s.wardrobe = wardrobe;
	shake = 0;
	cls();
	while (1)
	{
		t = now_ms();
		n = list_items(&s, ids, NTITLE);
		if (s.sel >= n)
			s.sel = n ? n - 1 : 0;
		rows = term_rows() - 9;
		if (rows > 14)
			rows = 14;
		if (s.sel < s.top)
			s.top = s.sel;
		if (s.sel >= s.top + rows)
			s.top = s.sel - rows + 1;
		id = n ? ids[s.sel] : -1;
		lk = g_save.look;
		title = g_save.title;
		if (id >= 0 && tab_slot(s.tab) >= 0)
			lk.eq[tab_slot(s.tab)] = id;
		else if (id >= 0 && s.tab == NSLOT + 1)
			title = id;
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		out("\033[2J");
		center(0, mix(theme()->accent, theme()->accent2, 0.5 + 0.5 * sin(t / 300.0)),
			wardrobe ? L("◈  G A R D E - R O B E  ◈", "◈  W A R D R O B E  ◈")
			: L("◈  B O U T I Q U E   D E S   F O U S  ◈", "◈  T H E   C R A Z Y   S H O P  ◈"));
		snprintf(tmp, sizeof(tmp), "%ld ◈", g_save.octets);
		text_at(x + 78 - ulen(tmp) + (shake > t ? rand() % 3 - 1 : 0), 0,
			shake > t ? rgb(255, 80, 90) : rgb(255, 215, 0), tmp);
		snprintf(tmp, sizeof(tmp), "◀  %s  ▶   (%d/%d)", tab_name(s.tab), s.tab + 1, NTAB);
		text_at(x, 2, theme()->accent, tmp);
		for (i = s.top; i < n && i < s.top + rows; i++)
		{
			at(x, 4 + i - s.top);
			if (i == s.sel)
				bgc(mix(theme()->bg, theme()->accent, 0.2));
			fg(equipped(&s, ids[i]) ? rgb(80, 230, 140) : theme()->text);
			if (tab_slot(s.tab) >= 0)
				snprintf(tn, sizeof(tn), "%s", L(g_items[ids[i]].fr, g_items[ids[i]].en));
			else if (s.tab == NSLOT)
				snprintf(tn, sizeof(tn), "%s", g_themes[ids[i]].name);
			else
				title_name(ids[i], tn, sizeof(tn));
			outf(" %s ", i == s.sel ? "▶" : " ");
			out(tn);
			outf("%*s", 30 - ulen(tn) > 0 ? 30 - ulen(tn) : 1, "");
			if (equipped(&s, ids[i]))
				out(L("ÉQUIPÉ  ", "EQUIPPED"));
			else if (owns(&s, ids[i]))
				out(L("possédé ", "owned   "));
			else
			{
				fg(g_save.octets >= price_of(&s, ids[i]) ? rgb(255, 215, 0) : theme()->dim);
				outf("%4d ◈  ", price_of(&s, ids[i]));
			}
			bgc(theme()->bg);
		}
		if (!n)
			text_at(x, 4, theme()->dim, L("Rien ici pour l'instant. Passe à la boutique !",
					"Nothing here yet. Visit the shop!"));
		avatar_panel(x + 50, 3, &lk, title, t);
		if (s.tab == NSLOT && id >= 0)
		{
			for (i = 0; i < 6; i++)
				fill(x + 50 + i * 4, 23 > term_rows() - 2 ? term_rows() - 2 : 23, 3, 1,
					((t_rgb[]){g_themes[id].accent, g_themes[id].accent2, g_themes[id].wall,
						g_themes[id].floor, g_themes[id].text, g_themes[id].bg})[i]);
		}
		text_at(x, term_rows() - 2, theme()->dim,
			L("◀ ▶ rayon   ▲ ▼ article   Entrée : acheter / équiper   c : couleur   Échap : retour",
				"◀ ▶ section   ▲ ▼ item   Enter: buy / equip   c: color   Esc: back"));
		if (s.tab == NSLOT + 1)
			text_at(x, term_rows() - 1, theme()->dim,
				L("+ / - : sauter de 20   r : titre au hasard", "+ / -: jump 20   r: random title"));
		flush();
		k = key_dir(key_read(40));
		if (k == 27 || k == 127 || k == 8)
			break ;
		if (k == 1002)
		{
			s.tab = (s.tab + 1) % NTAB;
			s.sel = 0;
			s.top = 0;
		}
		else if (k == 1003)
		{
			s.tab = (s.tab + NTAB - 1) % NTAB;
			s.sel = 0;
			s.top = 0;
		}
		else if (k == 1000 && s.sel > 0)
			s.sel--;
		else if (k == 1001 && s.sel < n - 1)
			s.sel++;
		else if (k == '+' && n)
			s.sel = s.sel + 20 < n ? s.sel + 20 : n - 1;
		else if (k == '-')
			s.sel = s.sel - 20 > 0 ? s.sel - 20 : 0;
		else if (k == 'r' && n)
			s.sel = rand() % n;
		else if (k == 'c' && id >= 0 && tab_slot(s.tab) >= 0 && item_owned(id)
			&& g_save.look.eq[tab_slot(s.tab)] == id)
		{
			g_save.look.col[tab_slot(s.tab)] = (g_save.look.col[tab_slot(s.tab)] + 1) % 10;
			save_write();
		}
		else if ((k == '\r' || k == '\n' || k == ' ') && id >= 0)
		{
			if (!owns(&s, id))
			{
				if (g_save.octets < price_of(&s, id))
				{
					shake = now_ms() + 500;
					continue ;
				}
				g_save.octets -= price_of(&s, id);
				if (tab_slot(s.tab) >= 0)
					item_own(id);
				else if (s.tab == NSLOT)
					g_save.themes[id] = '1';
				else
					g_save.titles[id] = '1';
				coins(x + 36, 5 + s.sel - s.top, price_of(&s, id));
			}
			equip(&s, id);
			count_achs();
			save_write();
		}
	}
	cls();
}

static const char	*opt_val(int i, char *buf)
{
	static const char	*anim[3][2] = {{"normales", "normal"}, {"rapides", "fast"},
		{"aucune", "none"}};
	static const char	*keys[3] = {"flèches + ZQSD", "flèches + WASD", "flèches + HJKL"};
	static const char	*aud[3][2] = {{"auto", "auto"}, {"toujours", "always"},
		{"jamais", "never"}};

	if (i == 0)
		return (L(anim[g_save.cfg.anim][0], anim[g_save.cfg.anim][1]));
	if (i == 1)
		return (g_save.cfg.col256 ? "256" : L("24 bits (millions)", "24-bit (millions)"));
	if (i == 2)
		return (g_save.cfg.lang ? "English" : "Français");
	if (i == 3)
		return (keys[g_save.cfg.keys]);
	if (i == 4)
		return (g_save.cfg.sound ? L("activé", "on") : L("coupé", "off"));
	if (i == 5)
		return (theme()->name);
	if (i == 7)
		return (sprintf(buf, "%d s", g_save.cfg.tmo), buf);
	if (i >= 8 && i <= 11)
		return (((int []){g_save.cfg.asan, g_save.cfg.ubsan, g_save.cfg.valg,
				g_save.cfg.norm})[i - 8] ? L("oui", "yes") : L("non", "no"));
	if (i == 12)
		return (L(aud[g_save.cfg.audit][0], aud[g_save.cfg.audit][1]));
	if (i == 13)
		return (g_save.login);
	return ("");
}

static void	opt_change(int i, int d)
{
	static const int	tmo[4] = {10, 30, 60, 120};
	int					k;

	if (i == 0)
		g_save.cfg.anim = (g_save.cfg.anim + 3 + d) % 3;
	else if (i == 1)
		g_save.cfg.col256 = !g_save.cfg.col256;
	else if (i == 2)
		g_save.cfg.lang = !g_save.cfg.lang;
	else if (i == 3)
		g_save.cfg.keys = (g_save.cfg.keys + 3 + d) % 3;
	else if (i == 4)
		g_save.cfg.sound = !g_save.cfg.sound;
	else if (i == 5)
	{
		for (k = 0; k < g_ntheme && strcmp(g_themes[k].id, g_save.theme); k++)
			;
		do
			k = (k + g_ntheme + d) % g_ntheme;
		while (g_save.themes[k] != '1');
		snprintf(g_save.theme, sizeof(g_save.theme), "%s", g_themes[k].id);
	}
	else if (i == 7)
	{
		for (k = 0; k < 4 && tmo[k] != g_save.cfg.tmo; k++)
			;
		g_save.cfg.tmo = tmo[(k + 4 + d) % 4];
	}
	else if (i == 8)
		g_save.cfg.asan = !g_save.cfg.asan;
	else if (i == 9)
		g_save.cfg.ubsan = !g_save.cfg.ubsan;
	else if (i == 10)
		g_save.cfg.valg = !g_save.cfg.valg;
	else if (i == 11)
		g_save.cfg.norm = !g_save.cfg.norm;
	else if (i == 12)
		g_save.cfg.audit = (g_save.cfg.audit + 3 + d) % 3;
}

static int	confirm(const char *q)
{
	int	k;

	fill(0, term_rows() / 2 - 2, term_cols(), 5, mix(theme()->bg, rgb(255, 60, 60), 0.2));
	center(term_rows() / 2 - 1, rgb(255, 120, 120), q);
	center(term_rows() / 2 + 1, theme()->text, L("o : oui   n : non", "y: yes   n: no"));
	flush();
	while ((k = key_read(100)) < 0)
		;
	return (k == 'o' || k == 'y' || k == 'O' || k == 'Y');
}

void	screen_settings(void)
{
	static const int	ids[14] = {0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14};
	static int			row;
	const char			*lab[15];
	char				buf[64];
	int					k;
	int					r;
	int					i;
	int					sel;
	int					x;

	cls();
	while (1)
	{
		lab[0] = L("Animations", "Animations");
		lab[1] = L("Couleurs", "Colors");
		lab[2] = L("Langue", "Language");
		lab[3] = L("Touches", "Keys");
		lab[4] = L("Son (bip)", "Sound (beep)");
		lab[5] = L("Thème (ceux que tu possèdes)", "Theme (owned ones)");
		lab[6] = L("Version du sujet", "Subject version");
		lab[7] = L("Temps max par test", "Max time per test");
		lab[8] = "AddressSanitizer";
		lab[9] = "UndefinedBehaviorSanitizer";
		lab[10] = L("Repasser sous valgrind", "Rerun under valgrind");
		lab[11] = L("Norminette dans l'audit", "Norminette in the audit");
		lab[12] = L("Audit moulinette", "Moulinette audit");
		lab[13] = L("Pseudo", "Nickname");
		lab[14] = L("Remettre la sauvegarde à zéro", "Reset the save");
		x = (term_cols() - 72) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		out("\033[2J");
		center(1, theme()->accent2, L("P A R A M È T R E S", "S E T T I N G S"));
		sel = ids[row];
		for (r = 0; r < 14; r++)
		{
			i = ids[r];
			at(x, 3 + r);
			if (i == sel)
				bgc(mix(theme()->bg, theme()->accent, 0.2));
			fg(i == 14 ? rgb(255, 120, 120) : theme()->text);
			outf(" %s %s%*s", i == sel ? "▶" : " ", lab[i], 33 - ulen(lab[i]), "");
			fg(theme()->accent);
			if (i < 13)
				outf("◀ %s ▶", opt_val(i, buf));
			else if (i == 13)
				outf("%s  (Entrée)", opt_val(i, buf));
			outf("   ");
			bgc(theme()->bg);
		}
		text_at(x, 18, theme()->dim, L("Les réglages des tests (temps, sanitizers, valgrind, audit)",
				"Test settings (time, sanitizers, valgrind, audit)"));
		text_at(x, 19, theme()->dim, L("s'appliquent au prochain « Tester tout ». La Norme reste obligatoire à l'entraînement.",
				"apply to the next \"Test everything\". The Norm stays required in training."));
		text_at(x, term_rows() - 1, theme()->dim, L("▲ ▼ choisir   ◀ ▶ changer   Échap : retour",
				"▲ ▼ pick   ◀ ▶ change   Esc: back"));
		flush();
		k = key_dir(key_read(60));
		if (k == 27 || k == 127 || k == 8)
			break ;
		if (k == 1000)
			row = (row + 13) % 14;
		else if (k == 1001)
			row = (row + 1) % 14;
		else if ((k == 1002 || k == 1003) && sel < 13)
		{
			opt_change(sel, k == 1002 ? 1 : -1);
			save_write();
			cls();
		}
		else if ((k == '\r' || k == '\n' || k == ' ') && sel == 13)
		{
			screen_login();
			cls();
		}
		else if ((k == '\r' || k == '\n' || k == ' ') && sel == 14)
		{
			if (confirm(L("Tout effacer ? XP, Octets, objets, boss, entraînement...",
						"Erase everything? XP, Octets, items, bosses, training..."))
				&& confirm(L("Vraiment ? C'est définitif.", "Really? This is final.")))
			{
				t_cfg	keep;
				char	login[32];

				keep = g_save.cfg;
				snprintf(login, sizeof(login), "%s", g_save.login);
				memset(g_save.ach, '0', 32);
				g_save.xp = 0;
				g_save.octets = 0;
				g_save.runs = 0;
				g_save.boss[0] = 0;
				g_save.boss[1] = 0;
				g_save.boss[2] = 0;
				memset(g_save.exo, '0', NEXO);
				memset(g_save.cleared, '0', NFN);
				memset(g_save.titles, '0', NTITLE);
				g_save.titles[1] = '1';
				g_save.title = 1;
				memset(g_save.themes, '0', 15);
				g_save.themes[0] = '1';
				strcpy(g_save.theme, "nuit");
				for (i = 0; i < g_nitems; i++)
					g_save.items[i] = g_items[i].price == 0 ? '1' : '0';
				look_default(&g_save.look);
				g_save.cfg = keep;
				snprintf(g_save.login, sizeof(g_save.login), "%s", login);
				save_write();
			}
			cls();
		}
	}
	cls();
}

void	screen_boss_menu(void)
{
	static int	sel;
	t_canvas	cv;
	t_scene_out	g;
	int			k;
	int			i;
	int			x;
	long		t;
	char		tmp[200];

	cls();
	while (1)
	{
		t = now_ms();
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		out("\033[2J");
		center(0, theme()->accent2, L("L E S   B O S S", "T H E   B O S S E S"));
		cv_init(&cv, 40, 36);
		cv_spr(&cv, &g_boss_spr[sel], 0, (int)((t / 450) % 2), g_bosses[sel].glow,
			mix(g_bosses[sel].glow, rgb(255, 255, 255), 0.5 + 0.5 * sin(t / 160.0)),
			g_save.boss[sel] ? 0 : 0);
		cv_print(&cv, x + 38, 2, theme()->bg);
		cv_free(&cv);
		for (i = 0; i < 3; i++)
		{
			at(x, 3 + i * 3);
			if (i == sel)
				bgc(mix(theme()->bg, g_bosses[i].glow, 0.2));
			fg(g_bosses[i].glow);
			outf(" %s %-28s", i == sel ? "▶" : " ", L(g_bosses[i].name, g_bosses[i].en));
			bgc(theme()->bg);
			snprintf(tmp, sizeof(tmp), L("partie %d · %s", "part %d · %s"), i + 1,
				g_save.boss[i] ? L("VAINCU", "BEATEN") : L("invaincu", "unbeaten"));
			text_at(x + 3, 4 + i * 3, g_save.boss[i] ? rgb(80, 230, 140) : theme()->dim, tmp);
		}
		text_at(x, 13, theme()->dim, L("Un boss tombe quand toutes", "A boss falls when every"));
		text_at(x, 14, theme()->dim, L("les fonctions de sa partie", "function of its part"));
		text_at(x, 15, theme()->dim, L("passent le testeur.", "passes the tester."));
		text_at(x, 17, theme()->dim, L("Le battre débloque 7 niveaux", "Beating it unlocks 7"));
		text_at(x, 18, theme()->dim, L("d'entraînement.", "training levels."));
		text_at(x, term_rows() - 1, theme()->dim, g_run.loaded
			? L("▲ ▼ choisir   Entrée : rejouer le combat   Échap : retour", "▲ ▼ pick   Enter: replay the fight   Esc: back")
			: L("▲ ▼ choisir   (lance « Tester tout » pour combattre)   Échap : retour",
				"▲ ▼ pick   (run \"Test everything\" to fight)   Esc: back"));
		flush();
		k = key_dir(key_read(60));
		if (k == 27 || k == 127 || k == 8)
			break ;
		if (k == 1000)
			sel = (sel + 2) % 3;
		else if (k == 1001)
			sel = (sel + 1) % 3;
		else if ((k == '\r' || k == '\n' || k == ' ') && g_run.loaded)
		{
			scene_play(&g, sel, 1);
			if ((g.crashed || g.frozen) && screen_crash_choice(&g) && g.fn >= 0)
				g_quar[g.fn] = 1;
			save_write();
		}
	}
	cls();
}

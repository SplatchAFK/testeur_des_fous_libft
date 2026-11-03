#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "fous.h"

t_save			g_save;
const char		*g_savedir;

const t_theme	g_themes[] = {
	{"nuit", "Nuit des fous", 0, {120, 200, 255}, {200, 120, 255},
		{70, 90, 160}, {28, 32, 52}, {225, 230, 245}, {110, 116, 140},
		{12, 13, 24}},
	{"synthwave", "Synthwave", 150, {255, 80, 200}, {255, 200, 60},
		{140, 40, 160}, {40, 16, 52}, {255, 225, 245}, {150, 100, 160},
		{18, 6, 28}},
	{"matrix", "Terminal vert", 150, {60, 255, 120}, {170, 255, 60},
		{20, 120, 50}, {8, 30, 14}, {190, 255, 200}, {60, 130, 80},
		{2, 10, 4}},
	{"sakura", "Sakura", 200, {255, 150, 190}, {255, 230, 240},
		{200, 90, 130}, {50, 30, 40}, {255, 235, 240}, {170, 120, 140},
		{24, 14, 20}},
	{"lave", "Volcan", 250, {255, 120, 30}, {255, 230, 80},
		{150, 40, 20}, {40, 14, 8}, {255, 230, 210}, {160, 100, 80},
		{18, 6, 4}},
	{"ocean", "Abysses", 250, {60, 200, 255}, {120, 255, 220},
		{20, 70, 120}, {6, 22, 40}, {210, 240, 255}, {90, 130, 160},
		{2, 8, 18}},
	{"papier", "Papier mâché", 300, {200, 80, 60}, {60, 120, 200},
		{150, 130, 110}, {230, 220, 200}, {40, 34, 30}, {120, 110, 100},
		{246, 240, 228}},
};
int				g_ntheme = sizeof(g_themes) / sizeof(g_themes[0]);

static unsigned long	sum_of(const t_save *s)
{
	unsigned long	h;
	const char		*p;
	char			tmp[2400];
	int				i;
	size_t			n;

	n = snprintf(tmp, sizeof(tmp), "%s|%ld|%ld|%d|%s|%s|%s|%d|%s|%s|%d%d%d|%s|%s|%ld|%d",
		s->login, s->xp, s->octets, s->runs, s->cleared, s->items, s->titles, s->title,
		s->themes, s->theme, s->boss[0], s->boss[1], s->boss[2], s->exo, s->ach,
		s->lastreward, s->crashes);
	for (i = 0; i < NSLOT && n < sizeof(tmp) - 16; i++)
		n += snprintf(tmp + n, sizeof(tmp) - n, ",%d:%d", s->look.eq[i], s->look.col[i]);
	h = 1469598103934665603UL;
	for (p = tmp; *p; p++)
		h = (h ^ (unsigned char)*p) * 1099511628211UL + 0x2a;
	return (h);
}

static void	save_default(void)
{
	const char	*u;
	int			i;

	memset(&g_save, 0, sizeof(g_save));
	u = getenv("USER");
	snprintf(g_save.login, sizeof(g_save.login), "%s", u ? u : "fou");
	memset(g_save.cleared, '0', NFN);
	memset(g_save.ach, '0', 32);
	memset(g_save.titles, '0', NTITLE);
	memset(g_save.exo, '0', NEXO);
	memset(g_save.themes, '0', 15);
	g_save.themes[0] = '1';
	memset(g_save.items, '0', g_nitems);
	for (i = 0; i < g_nitems; i++)
		if (g_items[i].price == 0)
			g_save.items[i] = '1';
	g_save.titles[1] = '1';
	g_save.title = 1;
	strcpy(g_save.theme, "nuit");
	look_default(&g_save.look);
	g_save.cfg.tmo = 30;
	g_save.cfg.asan = 1;
	g_save.cfg.ubsan = 1;
	g_save.cfg.norm = 1;
}

static const char	*save_path(void)
{
	static char	p[600];

	snprintf(p, sizeof(p), "%s/save.txt", g_savedir);
	return (p);
}

static void	set_bits(char *dst, const char *v, int n)
{
	int	i;

	for (i = 0; i < n && v[i]; i++)
		dst[i] = v[i] == '1' ? '1' : '0';
}

static void	read_look(const char *v, int *tab)
{
	int		i;
	char	*e;

	for (i = 0; i < NSLOT && *v; i++)
	{
		tab[i] = (int)strtol(v, &e, 10);
		if (e == v)
			break ;
		v = (*e == ',') ? e + 1 : e;
	}
}

static int	*cfg_field(const char *k)
{
	static const char	*n[] = {"anim", "col256", "subj", "tmo", "asan", "ubsan",
		"valg", "norm", "audit", "sound", "lang", "keys"};
	int					i;

	for (i = 0; i < 12; i++)
		if (!strcmp(k, n[i]))
			return (&((int *)&g_save.cfg)[i]);
	return (NULL);
}

static void	kv(const char *k, const char *v)
{
	int	*f;

	if (!strcmp(k, "login"))
		snprintf(g_save.login, sizeof(g_save.login), "%s", v);
	else if (!strcmp(k, "xp"))
		g_save.xp = atol(v);
	else if (!strcmp(k, "octets"))
		g_save.octets = atol(v);
	else if (!strcmp(k, "runs"))
		g_save.runs = atoi(v);
	else if (!strcmp(k, "cleared"))
		set_bits(g_save.cleared, v, NFN);
	else if (!strcmp(k, "items"))
		set_bits(g_save.items, v, g_nitems);
	else if (!strcmp(k, "titles"))
		set_bits(g_save.titles, v, NTITLE);
	else if (!strcmp(k, "title"))
		g_save.title = atoi(v);
	else if (!strcmp(k, "themes"))
		set_bits(g_save.themes, v, 15);
	else if (!strcmp(k, "theme"))
		snprintf(g_save.theme, sizeof(g_save.theme), "%s", v);
	else if (!strcmp(k, "eq"))
		read_look(v, g_save.look.eq);
	else if (!strcmp(k, "col"))
		read_look(v, g_save.look.col);
	else if (!strcmp(k, "boss") && strlen(v) >= 3)
	{
		g_save.boss[0] = v[0] == '1';
		g_save.boss[1] = v[1] == '1';
		g_save.boss[2] = v[2] == '1';
	}
	else if (!strcmp(k, "exo"))
		set_bits(g_save.exo, v, NEXO);
	else if (!strcmp(k, "ach"))
		set_bits(g_save.ach, v, 32);
	else if (!strcmp(k, "lastreward"))
		g_save.lastreward = atol(v);
	else if (!strcmp(k, "crashes"))
		g_save.crashes = atoi(v);
	else if (!strncmp(k, "cfg_", 4) && (f = cfg_field(k + 4)))
		*f = atoi(v);
}

static void	save_fix(void)
{
	int	i;

	for (i = 0; i < NSLOT; i++)
	{
		if (g_save.look.eq[i] < 0 || g_save.look.eq[i] >= g_nitems
			|| g_items[g_save.look.eq[i]].slot != i || !item_owned(g_save.look.eq[i]))
		{
			t_look	d;

			look_default(&d);
			g_save.look.eq[i] = d.eq[i];
		}
		if (g_save.look.col[i] < 0 || g_save.look.col[i] > 9)
			g_save.look.col[i] = 0;
	}
	if (g_save.title < 0 || g_save.title >= NTITLE || g_save.titles[g_save.title] != '1')
		g_save.title = 1;
	if (g_save.cfg.tmo < 5 || g_save.cfg.tmo > 300)
		g_save.cfg.tmo = 30;
	g_save.cfg.subj = 0;
}

void	save_load(void)
{
	FILE			*f;
	static char		line[2200];
	char			*eq;
	unsigned long	want;
	t_cfg			keep;

	save_default();
	mkdir(g_savedir, 0755);
	f = fopen(save_path(), "r");
	if (!f)
		return ;
	want = 0;
	while (fgets(line, sizeof(line), f))
	{
		line[strcspn(line, "\n")] = '\0';
		eq = strchr(line, '=');
		if (!eq)
			continue ;
		*eq = '\0';
		if (!strcmp(line, "sum"))
			want = strtoul(eq + 1, NULL, 16);
		else
			kv(line, eq + 1);
	}
	fclose(f);
	g_save.valid = (want == sum_of(&g_save));
	if (!g_save.valid)
	{
		keep = g_save.cfg;
		snprintf(line, sizeof(line), "%s.triche", save_path());
		rename(save_path(), line);
		save_default();
		g_save.cfg = keep;
		g_save.valid = -1;
	}
	save_fix();
}

void	save_write(void)
{
	FILE	*f;
	char	tmp[620];
	int		i;

	snprintf(tmp, sizeof(tmp), "%s.tmp", save_path());
	f = fopen(tmp, "w");
	if (!f)
		return ;
	fprintf(f, "login=%s\nxp=%ld\noctets=%ld\nruns=%d\ncleared=%s\nitems=%s\ntitles=%s\n"
		"title=%d\nthemes=%s\ntheme=%s\nboss=%d%d%d\nexo=%s\nach=%s\nlastreward=%ld\n"
		"crashes=%d\n", g_save.login, g_save.xp, g_save.octets, g_save.runs,
		g_save.cleared, g_save.items, g_save.titles, g_save.title, g_save.themes,
		g_save.theme, g_save.boss[0], g_save.boss[1], g_save.boss[2], g_save.exo,
		g_save.ach, g_save.lastreward, g_save.crashes);
	fprintf(f, "eq=");
	for (i = 0; i < NSLOT; i++)
		fprintf(f, "%d%s", g_save.look.eq[i], i + 1 < NSLOT ? "," : "\n");
	fprintf(f, "col=");
	for (i = 0; i < NSLOT; i++)
		fprintf(f, "%d%s", g_save.look.col[i], i + 1 < NSLOT ? "," : "\n");
	fprintf(f, "cfg_anim=%d\ncfg_col256=%d\ncfg_subj=%d\ncfg_tmo=%d\ncfg_asan=%d\n"
		"cfg_ubsan=%d\ncfg_valg=%d\ncfg_norm=%d\ncfg_audit=%d\ncfg_sound=%d\n"
		"cfg_lang=%d\ncfg_keys=%d\nsum=%lx\n", g_save.cfg.anim, g_save.cfg.col256,
		g_save.cfg.subj, g_save.cfg.tmo, g_save.cfg.asan, g_save.cfg.ubsan,
		g_save.cfg.valg, g_save.cfg.norm, g_save.cfg.audit, g_save.cfg.sound,
		g_save.cfg.lang, g_save.cfg.keys, sum_of(&g_save));
	fclose(f);
	rename(tmp, save_path());
	cfg_export();
}

void	cfg_export(void)
{
	FILE	*f;
	char	p[620];

	snprintf(p, sizeof(p), "%s/reglages.txt", g_savedir);
	f = fopen(p, "w");
	if (!f)
		return ;
	fprintf(f, "TIMEOUT=%d\nASAN=%d\nUBSAN=%d\nVALGRIND=%d\nNORM=%d\nAUDIT=%d\n",
		g_save.cfg.tmo, g_save.cfg.asan, g_save.cfg.ubsan, g_save.cfg.valg,
		g_save.cfg.norm, g_save.cfg.audit);
	fclose(f);
}

int	item_owned(int i)
{
	return (i >= 0 && i < g_nitems && g_save.items[i] == '1');
}

void	item_own(int i)
{
	if (i >= 0 && i < g_nitems)
		g_save.items[i] = '1';
}

long	xp_for(int level)
{
	if (level <= 1)
		return (0);
	return ((long)(60 * pow(level - 1, 1.6)));
}

int	level_of(long xp)
{
	int	l;

	l = 1;
	while (xp >= xp_for(l + 1))
		l++;
	return (l);
}

int	ach_has(int i)
{
	return (g_save.ach[i] == '1');
}

int	ach_give(int i)
{
	if (i < 0 || i >= NACH || ach_has(i))
		return (0);
	g_save.ach[i] = '1';
	g_save.octets += 50;
	g_save.xp += 50;
	bell();
	toast(L("SUCCÈS DÉBLOQUÉ  +50 ◈", "ACHIEVEMENT UNLOCKED  +50 ◈"),
		L(g_ach_fr[i][0], g_ach_en[i][0]), theme()->accent2);
	return (1);
}

const t_theme	*theme(void)
{
	int	i;

	for (i = 0; i < g_ntheme; i++)
		if (!strcmp(g_themes[i].id, g_save.theme))
			return (&g_themes[i]);
	return (&g_themes[0]);
}

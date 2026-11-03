#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "fous.h"

const char	*g_tester_dir = ".";

typedef struct s_xres
{
	char	st[16];
	int		checks;
	char	det[10][200];
	int		nd;
}	t_xres;

static int	exo_unlocked(int i)
{
	if (!g_save.boss[i / 7])
		return (0);
	return (i % 7 == 0 || g_save.exo[i - 1] == '1');
}

static int	exo_count(void)
{
	int	i;
	int	n;

	n = 0;
	for (i = 0; i < NEXO; i++)
		n += g_save.exo[i] == '1';
	return (n);
}

static void	exo_dir(int i, char *buf, size_t sz)
{
	snprintf(buf, sz, "%s/entrainement/niveau_%02d", g_tester_dir, i + 1);
}

static void	exo_prepare(int i)
{
	char	d[700];
	char	p[800];
	FILE	*f;

	snprintf(d, sizeof(d), "%s/entrainement", g_tester_dir);
	mkdir(d, 0755);
	exo_dir(i, d, sizeof(d));
	mkdir(d, 0755);
	snprintf(p, sizeof(p), "%s/sujet.txt", d);
	f = fopen(p, "w");
	if (!f)
		return ;
	fprintf(f, "Le testeur des fous · Entraînement · niveau %02d\n\n", i + 1);
	fprintf(f, "%s :\n%s\n\n", g_exos[i].npc, g_exos[i].lore);
	fprintf(f, "Fonction : %s\nFichier  : %s.c (dans ce dossier)\nPrototype :\n%s\n\n",
		g_exos[i].fn, g_exos[i].fn, g_exos[i].proto);
	fprintf(f, "%s\n\n", g_exos[i].fr);
	fprintf(f, "Notion : %s\n", g_exos[i].notion);
	fprintf(f, "Fonctions autorisées : malloc, free, write, et ta libft (#include \"libft.h\").\n");
	fprintf(f, "Norme obligatoire. Les fonctions d'aide doivent être static.\n");
	fprintf(f, "\nEnglish: %s\n", g_exos[i].en);
	fclose(f);
}

static void	lines_at(int x, int y, int maxy, t_rgb c, const char *s, int limit)
{
	char	buf[400];
	int		n;
	int		k;

	k = 0;
	while (*s && y < maxy)
	{
		n = (int)strcspn(s, "\n");
		if (limit >= 0 && k + n > limit)
			n = limit - k > 0 ? limit - k : 0;
		snprintf(buf, sizeof(buf), "%.*s", n, s);
		for (char *tb = buf; *tb; tb++)
			if (*tb == '\t')
				*tb = ' ';
		text_at(x, y++, c, buf);
		k += n;
		if (limit >= 0 && k >= limit)
			return ;
		s += strcspn(s, "\n");
		if (*s == '\n')
			s++;
	}
}

static void	parse_xres(const char *path, t_xres *r)
{
	FILE	*f;
	char	line[600];
	char	*tab;

	memset(r, 0, sizeof(*r));
	strcpy(r->st, "build");
	f = fopen(path, "r");
	if (!f)
		return ;
	while (fgets(line, sizeof(line), f))
	{
		line[strcspn(line, "\n")] = '\0';
		if (!strncmp(line, "X\t", 2))
		{
			tab = strchr(line + 2, '\t');
			if (tab)
				*tab = '\0';
			snprintf(r->st, sizeof(r->st), "%.15s", line + 2);
			r->checks = tab ? atoi(tab + 1) : 0;
		}
		else if (!strncmp(line, "D\t", 2) && r->nd < 10)
			snprintf(r->det[r->nd++], 200, "%.199s", line + 2);
	}
	fclose(f);
}

static void	run_exo(int i, t_xres *r)
{
	char	out_path[700];
	char	script[700];
	char	num[16];
	pid_t	pid;
	int		status;
	long	t0;
	long	t;
	int		k;
	t_canvas	cv;

	snprintf(out_path, sizeof(out_path), "%s/exo_resultat.txt", g_savedir);
	snprintf(script, sizeof(script), "%s/run.sh", g_tester_dir);
	snprintf(num, sizeof(num), "%02d", i + 1);
	unlink(out_path);
	flush();
	pid = fork();
	if (pid == 0)
	{
		if (!freopen("/dev/null", "w", stdout) || !freopen("/dev/null", "w", stderr)
			|| !freopen("/dev/null", "r", stdin))
			_exit(126);
		execl("/bin/bash", "bash", script, "--exo", num, "--out", out_path, (char *)NULL);
		_exit(127);
	}
	cv_init(&cv, 28, 32);
	t0 = now_ms();
	k = 0;
	while (pid > 0 && waitpid(pid, &status, WNOHANG) == 0)
	{
		t = now_ms() - t0;
		fill(0, 16, term_cols(), 6, theme()->bg);
		cv_clear(&cv);
		avatar_draw(&cv, 0, 0, &g_save.look, now_ms());
		cv_print(&cv, 2, 6, theme()->bg);
		at((term_cols() - 50) / 2, 18);
		fg(mix(theme()->accent, theme()->accent2, 0.5 + 0.5 * sin(t / 200.0)));
		outf("%s %s", (const char *[]){"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"}[(t / 80) % 10],
			L("Le testeur des fous examine ton code...", "The crazy tester is reading your code..."));
		bar((term_cols() - 50) / 2, 19, 50, fmod(t / 2500.0, 1.0), theme()->accent, theme()->accent2);
		flush();
		if (key_read(40) == 27)
			k = 1;
		if (k && t > 500)
		{
			kill(pid, SIGTERM);
			waitpid(pid, &status, 0);
			break ;
		}
	}
	cv_free(&cv);
	parse_xres(out_path, r);
}

static void	reward(int i)
{
	char	tmp[200];
	int		xp;
	int		oc;
	int		n;

	if (g_save.exo[i] == '1')
		return ;
	g_save.exo[i] = '1';
	xp = 100 + i * 10;
	oc = 60 + i * 5;
	g_save.xp += xp;
	g_save.octets += oc;
	bell();
	snprintf(tmp, sizeof(tmp), "+%d XP   +%d ◈", xp, oc);
	toast(L("NIVEAU RÉUSSI", "LEVEL CLEARED"), tmp, rgb(80, 230, 140));
	n = exo_count();
	if (n >= 7)
		ach_give(10);
	if (n >= 14)
		ach_give(11);
	if (n >= 21)
		ach_give(12);
	save_write();
}

static void	level_screen(int i)
{
	t_xres	r;
	char	tmp[400];
	char	d[700];
	int		k;
	int		x;
	long	t0;
	int		has;
	int		j;
	t_rgb	c;

	exo_prepare(i);
	exo_dir(i, d, sizeof(d));
	has = 0;
	t0 = now_ms();
	cls();
	while (1)
	{
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		out("\033[2J");
		snprintf(tmp, sizeof(tmp), L("NIVEAU %02d · %s", "LEVEL %02d · %s"), i + 1, g_exos[i].fn);
		text_at(x, 1, theme()->accent2, tmp);
		snprintf(tmp, sizeof(tmp), L("notion : %s", "topic: %s"), g_exos[i].notion);
		text_at(x + 40, 1, theme()->dim, tmp);
		snprintf(tmp, sizeof(tmp), "%s :", g_exos[i].npc);
		text_at(x, 3, theme()->accent, tmp);
		lines_at(x + 2, 4, 8, theme()->text, g_exos[i].lore,
			anim_off() ? -1 : (int)((now_ms() - t0) / 18));
		lines_at(x, 8, 11, rgb(255, 230, 140), g_exos[i].proto, -1);
		lines_at(x, 11, 15, theme()->text, L(g_exos[i].fr, g_exos[i].en), -1);
		snprintf(tmp, sizeof(tmp), L("Ton fichier : testeur_des_fous/entrainement/niveau_%02d/%s.c",
				"Your file: testeur_des_fous/entrainement/niveau_%02d/%s.c"), i + 1, g_exos[i].fn);
		text_at(x, 16, theme()->accent, tmp);
		text_at(x, 17, theme()->dim, L("Autorisé : malloc, free, write, ta libft. Norme OBLIGATOIRE. Sujet complet : sujet.txt",
				"Allowed: malloc, free, write, your libft. Norm REQUIRED. Full subject: sujet.txt"));
		if (has)
		{
			c = !strcmp(r.st, "ok") ? rgb(80, 230, 140) : rgb(255, 90, 100);
			if (!strcmp(r.st, "ok"))
				snprintf(tmp, sizeof(tmp), L("RÉUSSI · %d contrôles · %s", "PASSED · %d checks · %s"),
					r.checks, g_exos[i].win);
			else
				snprintf(tmp, sizeof(tmp), L("PAS ENCORE · %s", "NOT YET · %s"), r.st);
			text_at(x, 19, c, tmp);
			for (j = 0; j < r.nd && 20 + j < term_rows() - 1; j++)
			{
				snprintf(tmp, sizeof(tmp), "  %.74s", r.det[j]);
				text_at(x, 20 + j, theme()->text, tmp);
			}
		}
		text_at(x, term_rows() - 1, theme()->dim,
			L("t : tester mon code   Échap : retour", "t: test my code   Esc: back"));
		flush();
		k = key_read(50);
		if (k == 27 || k == 127 || k == 8)
			break ;
		if (k == 't' || k == 'T' || k == '\r' || k == '\n')
		{
			run_exo(i, &r);
			has = 1;
			if (!strcmp(r.st, "ok"))
				reward(i);
			cls();
		}
	}
	cls();
	(void)d;
}

void	screen_train(void)
{
	static int	sel;
	int			k;
	int			x;
	int			y;
	int			i;
	int			p;
	long		t;
	char		tmp[300];
	t_rgb		c;

	cls();
	while (1)
	{
		t = now_ms();
		x = (term_cols() - 76) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		out("\033[2J");
		center(1, theme()->accent2, L("E N T R A Î N E M E N T", "T R A I N I N G"));
		snprintf(tmp, sizeof(tmp), L("%d/21 niveaux réussis", "%d/21 levels cleared"), exo_count());
		center(2, theme()->dim, tmp);
		for (p = 0; p < 3; p++)
		{
			y = 4 + p * 5;
			snprintf(tmp, sizeof(tmp), L("PARTIE %d · gardée par %s %s", "PART %d · guarded by %s %s"),
				p + 1, L(g_bosses[p].name, g_bosses[p].en),
				g_save.boss[p] ? L("(vaincu)", "(beaten)") : L("(pas encore vaincu)", "(not beaten yet)"));
			text_at(x, y, g_save.boss[p] ? g_bosses[p].glow : theme()->dim, tmp);
			for (i = p * 7; i < p * 7 + 7; i++)
			{
				at(x + (i % 7) * 11, y + 2);
				if (g_save.exo[i] == '1')
					c = rgb(80, 230, 140);
				else if (exo_unlocked(i))
					c = mix(theme()->accent, rgb(255, 255, 255), 0.3 + 0.3 * sin(t / 200.0));
				else
					c = mix(theme()->dim, theme()->bg, 0.4);
				if (i == sel)
				{
					bgc(mix(theme()->bg, c, 0.3));
				}
				fg(c);
				outf(" %s %02d ", g_save.exo[i] == '1' ? "★" : (exo_unlocked(i) ? "◆" : "▫"), i + 1);
				bgc(theme()->bg);
				if (i % 7 != 6)
				{
					fg(mix(theme()->dim, theme()->bg, 0.5));
					out("──");
				}
			}
		}
		y = 20;
		snprintf(tmp, sizeof(tmp), "%02d · %s · %s", sel + 1, g_exos[sel].fn, g_exos[sel].notion);
		text_at(x, y, theme()->text, tmp);
		if (!g_save.boss[sel / 7])
			snprintf(tmp, sizeof(tmp), L("Verrouillé : bats d'abord %s (toutes les fonctions de la partie %d propres).",
					"Locked: beat %s first (every function of part %d clean)."),
				L(g_bosses[sel / 7].name, g_bosses[sel / 7].en), sel / 7 + 1);
		else if (!exo_unlocked(sel))
			snprintf(tmp, sizeof(tmp), L("Verrouillé : réussis d'abord le niveau %02d.",
					"Locked: clear level %02d first."), sel);
		else
			snprintf(tmp, sizeof(tmp), "%s", L("Entrée : ouvrir le niveau", "Enter: open the level"));
		text_at(x, y + 1, exo_unlocked(sel) ? theme()->accent : rgb(255, 160, 90), tmp);
		text_at(x, term_rows() - 1, theme()->dim,
			L("flèches : choisir   Entrée : ouvrir   Échap : retour", "arrows: pick   Enter: open   Esc: back"));
		flush();
		k = key_dir(key_read(60));
		if (k == 27 || k == 127 || k == 8)
			break ;
		if (k == 1002 && sel < NEXO - 1)
			sel++;
		else if (k == 1003 && sel > 0)
			sel--;
		else if (k == 1001)
			sel = sel + 7 < NEXO ? sel + 7 : sel;
		else if (k == 1000)
			sel = sel - 7 >= 0 ? sel - 7 : sel;
		else if ((k == '\r' || k == '\n' || k == ' ') && exo_unlocked(sel))
			level_screen(sel);
	}
	cls();
}

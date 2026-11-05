#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "fous.h"

static struct termios	g_old;
static int				g_raw;
static char				g_buf[1 << 18];
static size_t			g_len;

static void	on_signal(int s)
{
	term_end();
	signal(s, SIG_DFL);
	raise(s);
}

void	term_resume(void)
{
	struct termios	t;

	t = g_old;
	t.c_lflag &= ~(ICANON | ECHO | ISIG);
	t.c_iflag &= ~(IXON | ICRNL);
	t.c_cc[VMIN] = 0;
	t.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &t);
	g_raw = 1;
	out("\033[?1049h\033[?25l\033[?7l");
	flush();
}

void	term_init(void)
{
	tcgetattr(0, &g_old);
	signal(SIGINT, on_signal);
	signal(SIGTERM, on_signal);
	signal(SIGHUP, on_signal);
	term_resume();
}

void	term_pause(void)
{
	if (!g_raw)
		return ;
	out("\033[0m\033[?7h\033[?25h\033[?1049l");
	flush();
	tcsetattr(0, TCSANOW, &g_old);
	g_raw = 0;
}

void	term_end(void)
{
	term_pause();
}

int	term_cols(void)
{
	struct winsize	w;

	if (ioctl(1, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
		return (w.ws_col);
	return (80);
}

int	term_rows(void)
{
	struct winsize	w;

	if (ioctl(1, TIOCGWINSZ, &w) == 0 && w.ws_row > 0)
		return (w.ws_row);
	return (24);
}

long	now_ms(void)
{
	struct timespec	t;

	clock_gettime(CLOCK_MONOTONIC, &t);
	return (t.tv_sec * 1000L + t.tv_nsec / 1000000L);
}

void	sleep_ms(int ms)
{
	struct timespec	t;

	if (ms <= 0)
		return ;
	t.tv_sec = ms / 1000;
	t.tv_nsec = (ms % 1000) * 1000000L;
	nanosleep(&t, NULL);
}

static int	read_byte(int timeout_ms)
{
	fd_set			s;
	struct timeval	tv;
	unsigned char	c;

	FD_ZERO(&s);
	FD_SET(0, &s);
	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;
	if (select(1, &s, NULL, NULL, &tv) <= 0)
		return (-1);
	if (read(0, &c, 1) != 1)
		return (-1);
	return (c);
}

int	key_read(int timeout_ms)
{
	int	c;
	int	d;

	c = read_byte(timeout_ms < 0 ? 0 : timeout_ms);
	if (c != 27)
		return (c == 3 ? 'q' : c);
	d = read_byte(15);
	if (d != '[' && d != 'O')
		return (27);
	d = read_byte(15);
	if (d == 'A')
		return (1000);
	if (d == 'B')
		return (1001);
	if (d == 'C')
		return (1002);
	if (d == 'D')
		return (1003);
	while (d >= 0 && !(d >= 64 && d <= 126))
		d = read_byte(5);
	return (-1);
}

void	outn(const char *s, size_t n)
{
	if (g_len + n >= sizeof(g_buf))
		flush();
	if (n >= sizeof(g_buf))
	{
		(void)!write(1, s, n);
		return ;
	}
	memcpy(g_buf + g_len, s, n);
	g_len += n;
}

void	out(const char *s)
{
	outn(s, strlen(s));
}

void	outf(const char *fmt, ...)
{
	char	tmp[2048];
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
	va_end(ap);
	if (n > 0)
		outn(tmp, (size_t)n < sizeof(tmp) ? (size_t)n : sizeof(tmp) - 1);
}

void	flush(void)
{
	size_t	o;
	ssize_t	w;

	o = 0;
	while (o < g_len)
	{
		w = write(1, g_buf + o, g_len - o);
		if (w <= 0)
			break ;
		o += w;
	}
	g_len = 0;
}

void	at(int x, int y)
{
	outf("\033[%d;%dH", y + 1, x + 1);
}

int	color_seq(char *buf, t_rgb c, int bg)
{
	int	i;

	if (g_save.cfg.col256)
	{
		i = 16 + 36 * ((c.r * 5 + 127) / 255) + 6 * ((c.g * 5 + 127) / 255)
			+ (c.b * 5 + 127) / 255;
		return (sprintf(buf, "\033[%d;5;%dm", bg ? 48 : 38, i));
	}
	return (sprintf(buf, "\033[%d;2;%d;%d;%dm", bg ? 48 : 38, c.r, c.g, c.b));
}

void	fg(t_rgb c)
{
	char	b[32];

	outn(b, color_seq(b, c, 0));
}

void	bgc(t_rgb c)
{
	char	b[32];

	outn(b, color_seq(b, c, 1));
}

void	bell(void)
{
	if (g_save.cfg.sound)
	{
		out("\a");
		flush();
	}
}

int	anim_off(void)
{
	return (g_save.cfg.anim == 2);
}

int	anim_ms(int ms)
{
	if (g_save.cfg.anim == 2)
		return (0);
	if (g_save.cfg.anim == 1)
		return (ms * 2 / 5);
	return (ms);
}

int	key_dir(int k)
{
	if (k == 1000 || k == 1001 || k == 1002 || k == 1003)
		return (k);
	if (g_save.cfg.keys == 2)
	{
		if (k == 'k')
			return (1000);
		if (k == 'j')
			return (1001);
		if (k == 'l')
			return (1002);
		if (k == 'h')
			return (1003);
		return (k);
	}
	if (k == (g_save.cfg.keys ? 'w' : 'z'))
		return (1000);
	if (k == 's')
		return (1001);
	if (k == 'd')
		return (1002);
	if (k == (g_save.cfg.keys ? 'a' : 'q'))
		return (1003);
	return (k);
}

void	reset(void)
{
	out("\033[0m");
}

void	cls(void)
{
	reset();
	bgc(theme()->bg);
	out("\033[2J\033[H");
}

t_rgb	rgb(int r, int g, int b)
{
	t_rgb	c;

	c.r = r < 0 ? 0 : (r > 255 ? 255 : r);
	c.g = g < 0 ? 0 : (g > 255 ? 255 : g);
	c.b = b < 0 ? 0 : (b > 255 ? 255 : b);
	return (c);
}

t_rgb	mix(t_rgb a, t_rgb b, double t)
{
	if (t < 0)
		t = 0;
	if (t > 1)
		t = 1;
	return (rgb(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t,
			a.b + (b.b - a.b) * t));
}

t_rgb	hue(double h)
{
	h = fmod(h, 1.0);
	if (h < 0)
		h += 1.0;
	return (rgb(127 + 127 * sin(6.2831 * h), 127 + 127 * sin(6.2831 * (h
					+ 0.333)), 127 + 127 * sin(6.2831 * (h + 0.666))));
}

int	ulen(const char *s)
{
	int	n;

	n = 0;
	for (; *s; s++)
		if (((unsigned char)*s & 0xC0) != 0x80)
			n++;
	return (n);
}

/* écrit au plus maxw colonnes (UTF-8), avec « … » si c'est coupé */
void	text_clip(int x, int y, t_rgb c, const char *s, int maxw)
{
	char	buf[1024];
	int		cols;
	size_t	i;
	size_t	cut;

	if (maxw <= 0)
		return ;
	if (ulen(s) <= maxw)
	{
		text_at(x, y, c, s);
		return ;
	}
	cols = 0;
	cut = 0;
	for (i = 0; s[i] && i < sizeof(buf) - 4; i++)
	{
		if (((unsigned char)s[i] & 0xC0) != 0x80)
		{
			if (cols == maxw - 1)
				break ;
			cols++;
		}
		cut = i + 1;
	}
	memcpy(buf, s, cut);
	memcpy(buf + cut, "…", 4);
	text_at(x, y, c, buf);
}

/* coupe s en lignes de largeur w (aux espaces, puis aux \n) ;
** dessine à partir de y sans dépasser maxy ; renvoie la ligne suivante */
int	text_wrap(int x, int y, int maxy, int w, t_rgb c, const char *s)
{
	char		buf[1024];
	const char	*p;
	const char	*e;
	const char	*sp;
	int			cols;

	if (w < 1)
		return (y);
	p = s;
	while (*p && y < maxy)
	{
		cols = 0;
		sp = NULL;
		e = p;
		while (*e && *e != '\n' && (cols < w || (((unsigned char)*e & 0xC0) == 0x80)))
		{
			if (*e == ' ')
				sp = e;
			if (((unsigned char)*e & 0xC0) != 0x80)
				cols++;
			e++;
		}
		if (*e && *e != '\n' && *e != ' ' && sp && sp > p)
			e = sp;
		snprintf(buf, sizeof(buf), "%.*s", (int)(e - p), p);
		for (sp = buf; *sp; sp++)
			if (*sp == '\t')
				*(char *)sp = ' ';
		text_at(x, y++, c, buf);
		p = e;
		if (*p == ' ' || *p == '\n')
			p++;
	}
	return (y);
}

void	text_at(int x, int y, t_rgb c, const char *s)
{
	at(x, y);
	fg(c);
	out(s);
}

void	center(int y, t_rgb c, const char *s)
{
	int	x;

	x = (term_cols() - ulen(s)) / 2;
	if (x < 0)
		text_clip(0, y, c, s, term_cols());
	else
		text_at(x, y, c, s);
}

void	box(int x, int y, int w, int h, t_rgb c)
{
	int	i;

	fg(c);
	at(x, y);
	out("╭");
	for (i = 0; i < w - 2; i++)
		out("─");
	out("╮");
	for (i = 1; i < h - 1; i++)
	{
		at(x, y + i);
		out("│");
		at(x + w - 1, y + i);
		out("│");
	}
	at(x, y + h - 1);
	out("╰");
	for (i = 0; i < w - 2; i++)
		out("─");
	out("╯");
}

void	fill(int x, int y, int w, int h, t_rgb c)
{
	int	i;

	bgc(c);
	for (i = 0; i < h; i++)
	{
		at(x, y + i);
		outf("%*s", w, "");
	}
	bgc(theme()->bg);
}

void	bar(int x, int y, int w, double v, t_rgb a, t_rgb b)
{
	int		i;
	double	f;

	if (v < 0)
		v = 0;
	if (v > 1)
		v = 1;
	at(x, y);
	for (i = 0; i < w; i++)
	{
		f = (double)i / (w > 1 ? w - 1 : 1);
		if (i < (int)(v * w))
		{
			fg(mix(a, b, f));
			out("█");
		}
		else if (i == (int)(v * w) && v * w - (int)(v * w) > 0.5)
		{
			fg(mix(a, b, f));
			out("▌");
		}
		else
		{
			fg(mix(theme()->dim, theme()->bg, 0.5));
			out("░");
		}
	}
}

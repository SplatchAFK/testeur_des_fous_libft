#ifndef FOUS_H
# define FOUS_H

# include <stddef.h>
# include <stdint.h>

typedef struct s_list
{
	void			*content;
	struct s_list	*next;
}	t_list;

enum e_fn
{
	E_isalpha, E_isdigit, E_isalnum, E_isascii, E_isprint, E_strlen,
	E_memset, E_bzero, E_memcpy, E_memmove, E_strlcpy, E_strlcat,
	E_toupper, E_tolower, E_strchr, E_strrchr, E_strncmp, E_memchr,
	E_memcmp, E_strnstr, E_atoi, E_calloc, E_strdup,
	E_substr, E_strjoin, E_strtrim, E_split, E_itoa, E_strmapi, E_striteri,
	E_putchar_fd, E_putstr_fd, E_putendl_fd, E_putnbr_fd,
	E_lstnew, E_lstadd_front, E_lstsize, E_lstlast, E_lstadd_back,
	E_lstdelone, E_lstclear, E_lstiter, E_lstmap,
	NFN
};

typedef struct s_ft
{
	int				(*isalpha)(int);
	int				(*isdigit)(int);
	int				(*isalnum)(int);
	int				(*isascii)(int);
	int				(*isprint)(int);
	size_t			(*strlen)(const char *);
	void			*(*memset)(void *, int, size_t);
	void			(*bzero)(void *, size_t);
	void			*(*memcpy)(void *, const void *, size_t);
	void			*(*memmove)(void *, const void *, size_t);
	size_t			(*strlcpy)(char *, const char *, size_t);
	size_t			(*strlcat)(char *, const char *, size_t);
	int				(*toupper)(int);
	int				(*tolower)(int);
	char			*(*strchr)(const char *, int);
	char			*(*strrchr)(const char *, int);
	int				(*strncmp)(const char *, const char *, size_t);
	void			*(*memchr)(const void *, int, size_t);
	int				(*memcmp)(const void *, const void *, size_t);
	char			*(*strnstr)(const char *, const char *, size_t);
	int				(*atoi)(const char *);
	void			*(*calloc)(size_t, size_t);
	char			*(*strdup)(const char *);
	char			*(*substr)(char const *, unsigned int, size_t);
	char			*(*strjoin)(char const *, char const *);
	char			*(*strtrim)(char const *, char const *);
	char			**(*split)(char const *, char);
	char			*(*itoa)(int);
	char			*(*strmapi)(char const *, char (*)(unsigned int, char));
	void			(*striteri)(char *, void (*)(unsigned int, char *));
	void			(*putchar_fd)(char, int);
	void			(*putstr_fd)(char *, int);
	void			(*putendl_fd)(char *, int);
	void			(*putnbr_fd)(int, int);
	t_list			*(*lstnew)(void *);
	void			(*lstadd_front)(t_list **, t_list *);
	unsigned int	(*lstsize)(t_list *);
	t_list			*(*lstlast)(t_list *);
	void			(*lstadd_back)(t_list **, t_list *);
	void			(*lstdelone)(t_list *, void (*)(void *));
	void			(*lstclear)(t_list **, void (*)(void *));
	void			(*lstiter)(t_list *, void (*)(void *));
	t_list			*(*lstmap)(t_list *, void *(*)(void *), void (*)(void *));
}	t_ft;

typedef struct s_shm
{
	volatile int			cur;
	volatile unsigned long	beat;
	volatile int			quit;
	volatile int			beaten[3];
	volatile int			used[NFN];
}	t_shm;

enum e_st
{
	ST_WAIT, ST_OK, ST_KO, ST_CRASH, ST_LEAK, ST_UB, ST_TIME, ST_BUILD,
	ST_SKIP
};

typedef struct s_res
{
	int		st;
	int		checks;
	int		ms;
	int		proto;
	char	detail[6][160];
	int		nd;
}	t_res;

typedef struct s_audit
{
	int		st;
	char	label[96];
	char	detail[160];
}	t_audit;

typedef struct s_run
{
	t_res	fn[NFN];
	t_audit	audit[24];
	int		naudit;
	char	warn[6][200];
	int		nwarn;
	char	stage[120];
	int		done;
	int		protos_done;
	int		tests_done;
	int		current;
	int		loaded;
}	t_run;

typedef struct s_rgb
{
	int	r;
	int	g;
	int	b;
}	t_rgb;

typedef struct s_theme
{
	const char	*id;
	const char	*name;
	int			price;
	t_rgb		accent;
	t_rgb		accent2;
	t_rgb		wall;
	t_rgb		floor;
	t_rgb		text;
	t_rgb		dim;
	t_rgb		bg;
}	t_theme;

typedef struct s_spr
{
	short				w;
	short				h;
	short				ox;
	short				oy;
	const char *const	*rows;
}	t_spr;

enum e_slot
{
	SL_FOND, SL_CAPE, SL_JAMBES, SL_ARMURE, SL_CASQUE, SL_YEUX, SL_DECO,
	SL_ARME, SL_MAIN, SL_FAMILIER, SL_AURA, NSLOT
};

typedef struct s_idef
{
	const char	*id;
	const char	*fr;
	const char	*en;
	int			slot;
	int			price;
	t_rgb		base;
	t_spr		spr;
}	t_idef;

typedef struct s_look
{
	int	eq[NSLOT];
	int	col[NSLOT];
}	t_look;

typedef struct s_canvas
{
	int				w;
	int				h;
	t_rgb			*px;
	unsigned char	*a;
}	t_canvas;

typedef struct s_cfg
{
	int	anim;
	int	col256;
	int	subj;
	int	tmo;
	int	asan;
	int	ubsan;
	int	valg;
	int	norm;
	int	audit;
	int	sound;
	int	lang;
	int	keys;
}	t_cfg;

# define NTITLE 1000
# define NEXO 21
# define NACH 18

typedef struct s_save
{
	char		login[32];
	long		xp;
	long		octets;
	int			runs;
	char		cleared[NFN + 1];
	char		items[256];
	char		titles[NTITLE + 1];
	int			title;
	char		themes[16];
	char		theme[24];
	t_look		look;
	int			boss[3];
	char		exo[NEXO + 1];
	char		ach[33];
	long		lastreward;
	int			crashes;
	int			valid;
	t_cfg		cfg;
}	t_save;

typedef struct s_exo
{
	const char	*fn;
	const char	*proto;
	const char	*notion;
	const char	*fr;
	const char	*en;
	const char	*npc;
	const char	*lore;
	const char	*win;
}	t_exo;

typedef struct s_boss
{
	const char	*name;
	const char	*en;
	const char	*intro;
	const char	*taunt;
	const char	*defeat;
	int			first;
	int			last;
	t_rgb		glow;
}	t_boss;

typedef struct s_scene_out
{
	int	crashed;
	int	frozen;
	int	sig;
	int	fn;
	int	quit;
}	t_scene_out;

extern t_ft				g_ft;
extern t_shm			*g_shm;
extern const char		*g_fn_name[NFN];
extern const char		*g_fn_zone[NFN];
extern int				g_quar[NFN];
extern int				g_absent[NFN];
extern t_save			g_save;
extern t_run			g_run;
extern const t_theme	g_themes[];
extern int				g_ntheme;
extern const char		*g_savedir;
extern const char		*g_tester_dir;
extern const char		g_pal_keys[];
extern const t_rgb		g_pal_vals[];
extern const t_spr		g_boss_spr[3];
extern const t_idef		g_items[];
extern const int		g_nitems;
extern const t_exo		g_exos[NEXO];
extern const t_boss		g_bosses[3];
extern const char		*g_ach_fr[NACH][2];
extern const char		*g_ach_en[NACH][2];
extern const t_rgb		g_swatch[10];
extern const char		*g_slot_fr[NSLOT];
extern const char		*g_slot_en[NSLOT];

# define L(fr, en) (g_save.cfg.lang ? (en) : (fr))
# define FT(f, ...) (tdf_track(E_##f), g_ft.f(__VA_ARGS__))

void			tdf_track(int i);
void			lib_init(void);
int				fn_index(const char *name);

void			term_init(void);
void			term_end(void);
void			term_pause(void);
void			term_resume(void);
int				term_cols(void);
int				term_rows(void);
long			now_ms(void);
void			sleep_ms(int ms);
int				key_read(int timeout_ms);
int				key_dir(int k);
void			out(const char *s);
void			outn(const char *s, size_t n);
void			outf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void			flush(void);
void			at(int x, int y);
void			fg(t_rgb c);
void			bgc(t_rgb c);
int				color_seq(char *buf, t_rgb c, int bg);
void			reset(void);
void			cls(void);
void			bell(void);
t_rgb			rgb(int r, int g, int b);
t_rgb			mix(t_rgb a, t_rgb b, double t);
t_rgb			hue(double h);
void			text_at(int x, int y, t_rgb c, const char *s);
void			center(int y, t_rgb c, const char *s);
int				ulen(const char *s);
void			box(int x, int y, int w, int h, t_rgb c);
void			fill(int x, int y, int w, int h, t_rgb c);
void			bar(int x, int y, int w, double v, t_rgb a, t_rgb b);
const t_theme	*theme(void);
int				anim_ms(int ms);
int				anim_off(void);

void			big_text(int x, int y, const char *s, double t, int mode);
int				big_width(const char *s);

t_rgb			pal(char c, t_rgb tint, t_rgb eye);
void			cv_init(t_canvas *c, int w, int h);
void			cv_free(t_canvas *c);
void			cv_clear(t_canvas *c);
void			cv_px(t_canvas *c, int x, int y, t_rgb col);
void			cv_spr(t_canvas *c, const t_spr *s, int x, int y, t_rgb tint,
					t_rgb eye, double flash);
void			cv_print(const t_canvas *c, int sx, int sy, t_rgb bg);
int				cv_row(const t_canvas *c, int row, t_rgb bg, char *buf, size_t sz);
void			avatar_draw(t_canvas *c, int x, int y, const t_look *lk, long t);
void			look_default(t_look *lk);
int				item_find(const char *id);
t_rgb			item_color(int item, int col);

const char		*title_name(int i, char *buf, size_t sz);
int				title_price(int i);

void			screen_intro(void);
void			screen_scan(const char *path, int pid);
void			screen_results(void);
void			screen_rewards(void);
void			screen_credits(void);
void			screen_profile(void);
int				screen_menu(void);
void			screen_shop(int wardrobe);
void			screen_login(void);
void			screen_settings(void);
void			screen_train(void);
void			screen_boss_menu(void);
int				screen_crash_choice(t_scene_out *g);
void			toast(const char *title, const char *msg, t_rgb c);
void			results_load(const char *path, long *off);
int				st_bad(int st);
const char		*st_label(int st);
t_rgb			st_color(int st);
int				part_clean(int p);
void			boss_battles(int only);

void			save_load(void);
void			save_write(void);
int				item_owned(int i);
void			item_own(int i);
int				level_of(long xp);
long			xp_for(int level);
int				ach_has(int i);
int				ach_give(int i);
void			cfg_export(void);

void			scene_play(t_scene_out *g, int boss, int replay);
int				scene_child(int boss, int replay);

#endif

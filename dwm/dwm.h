#include <X11/Xlib.h>
#include <unistd.h>
#include <X11/X.h>
#include "drw.h"
/* macros */
#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)
#define CLEANMASK(mask) \
    (mask & ~(numlockmask | LockMask) & \
        (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | \
            Mod5Mask))
#define INTERSECT(x, y, w, h, m) \
    (MAX(0, MIN((x) + (w), (m)->mx + (m)->mw) - MAX((x), (m)->mx)) * \
        MAX(0, MIN((y) + (h), (m)->my + (m)->mh) - MAX((y), (m)->my)))
#define ISVISIBLE(C) ((C->tags & C->mon->tagset[C->mon->seltags]))
#define MOUSEMASK    (BUTTONMASK | PointerMotionMask)
#define WIDTH(X)     ((X)->w + 2 * (X)->bw + gappx)
#define HEIGHT(X)    ((X)->h + 2 * (X)->bw + gappx)
#define TAGMASK      ((1 << LENGTH(tags)) - 1)
#define TEXTW(X)     (drw_fontset_getwidth(drw, (X)) + lrpad)

/* enums */
enum
{
    CurNormal,
    CurResize,
    CurMove,
    CurLast
}; /* cursor */

enum
{
    SchemeNorm,
    SchemeSel
}; /* color schemes */

enum
{
    NetSupported,
    NetWMName,
    NetWMState,
    NetWMCheck,
    NetWMFullscreen,
    NetActiveWindow,
    NetWMWindowOpacity,
    NetWMWindowType,
    NetWMWindowTypeDialog,
    NetClientList,
    NetLast
}; /* EWMH atoms */

enum
{
    WMProtocols,
    WMDelete,
    WMState,
    WMTakeFocus,
    WMLast
}; /* default atoms */

enum
{
    ClkTagBar,
    ClkLtSymbol,
    ClkStatusText,
    ClkWinTitle,
    ClkClientWin,
    ClkRootWin,
    ClkLast
}; /* clicks */

typedef struct TagState TagState;

struct TagState
{
    int selected;
    int occupied;
    int urgent;
};

typedef struct ClientState ClientState;

struct ClientState
{
    int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
};

typedef union
{
    long i;
    unsigned long ui;
    float f;
    const void* v;
} Arg;

typedef struct
{
    unsigned int click;
    unsigned int mask;
    unsigned int button;
    void (*func)(const Arg* arg);
    const Arg arg;
} Button;

typedef struct Monitor Monitor;
typedef struct Client Client;

struct Client
{
    char name[256];
    float mina, maxa;
    int x, y, w, h;
    int oldx, oldy, oldw, oldh;
    int basew, baseh, incw, inch, maxw, maxh, minw, minh, hintsvalid;
    int bw, oldbw;
    unsigned int tags;
    int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
    Client* next;
    Client* snext;
    Monitor* mon;
    Window win;
    ClientState prevstate;
};

typedef struct
{
    unsigned int mod;
    KeySym keysym;
    void (*func)(const Arg*);
    const Arg arg;
} Key;

typedef struct
{
    const char* symbol;
    void (*arrange)(Monitor*);
} Layout;

typedef struct Pertag Pertag;

struct Monitor
{
    char ltsymbol[16];
    char lastltsymbol[16];
    float mfact;
    int nmaster;
    int num;
    int by, bh; /* bar geometry */
    int mx, my, mw, mh; /* screen size */
    int wx, wy, ww, wh; /* window area  */
    unsigned int seltags;
    unsigned int sellt;
    unsigned int tagset[2];
    TagState tagstate;
    int showbar;
    int topbar;
    Client* clients;
    Client* sel;
    Client* lastsel;
    Client* stack;
    Monitor* next;
    Window barwin;
    Window agswin;
    int agsshown;
    const Layout* lt[2];
    const Layout* lastlt;
    Pertag* pertag;
};

typedef struct
{
    const char* class;
    const char* instance;
    const char* title;
    unsigned int tags;
    int isfloating;
    int monitor;
} Rule;

typedef struct
{
    unsigned int tags;
    const char* window_class;
} Assigntag;

/* function declarations */
void applyrules(Client* c);
int applysizehints(Client* c, int* x, int* y, int* w, int* h, int interact);
void arrange(Monitor* m);
void arrangemon(Monitor* m);
void attach(Client* c);
void attachabove(Client* c);
void attachstack(Client* c);
void buttonpress(XEvent* e);
void checkotherwm(void);
void cleanup(void);
void cleanupmon(Monitor* mon);
void clientmessage(XEvent* e);
void configure(Client* c);
void configurenotify(XEvent* e);
void configurerequest(XEvent* e);
Monitor* createmon(void);
void destroynotify(XEvent* e);
void detach(Client* c);
void detachstack(Client* c);
Monitor* dirtomon(int dir);
void drawbar(Monitor* m);
void drawbars(void);
void enternotify(XEvent* e);
void expose(XEvent* e);
void focus(Client* c);
void focusin(XEvent* e);
void focusmon(const Arg* arg);
void focusstack(const Arg* arg);
Atom getatomprop(Client* c, Atom prop);
int getrootptr(int* x, int* y);
long getstate(Window w);
int gettextprop(Window w, Atom atom, char* text, unsigned int size);
void grabbuttons(Client* c, int focused);
void grabkeys(void);
int handlexevent(struct epoll_event* ev);
void incnmaster(const Arg* arg);
void keypress(XEvent* e);
void killclient(const Arg* arg);
void manage(Window w, XWindowAttributes* wa);
void managealtbar(Window win, XWindowAttributes* wa);
void mappingnotify(XEvent* e);
void maprequest(XEvent* e);
void monocle(Monitor* m);
void motionnotify(XEvent* e);
void movemouse(const Arg* arg);
Client* nexttiled(Client* c);
void pop(Client* c);
void propertynotify(XEvent* e);
void quit(const Arg* arg);
Monitor* recttomon(int x, int y, int w, int h);
void resize(Client* c, int x, int y, int w, int h, int interact);
void resizeclient(Client* c, int x, int y, int w, int h);
void resizemouse(const Arg* arg);
void restack(Monitor* m);
void run(void);
void runAutostart(void);
void scan(void);
int sendevent(Client* c, Atom proto);
void sendmon(Client* c, Monitor* m);
void setclientstate(Client* c, long state);
void setfocus(Client* c);
void setfullscreen(Client* c, int fullscreen);
void setlayout(const Arg* arg);
void setlayoutsafe(const Arg* arg);
void setmfact(const Arg* arg);
void setup(void);
void setupepoll(void);
void seturgent(Client* c, int urg);
void showhide(Client* c);
void spawn(const Arg* arg);
void spawnbar();
void tag(const Arg* arg);
void tagmon(const Arg* arg);
void tile(Monitor* m);
void togglebar(const Arg* arg);
void toggleags(const Arg* arg);
void togglefloating(const Arg* arg);
void toggletag(const Arg* arg);
void toggleview(const Arg* arg);
void unfocus(Client* c, int setfocus);
void unmanage(Client* c, int destroyed);
void unmanagealtbar(Window w);
void unmapnotify(XEvent* e);
void updatebarpos(Monitor* m);
void updatebars(void);
void updateclientlist(void);
int updategeom(void);
void updatenumlockmask(void);
void updatesizehints(Client* c);
void updatestatus(void);
void updatetitle(Client* c);
void updatewindowtype(Client* c);
void updatewmhints(Client* c);
void view(const Arg* arg);
Client* wintoclient(Window w);
Monitor* wintomon(Window w);
int wmclasscontains(Window win, const char* class, const char* name);
int xerror(Display* dpy, XErrorEvent* ee);
int xerrordummy(Display* dpy, XErrorEvent* ee);
int xerrorstart(Display* dpy, XErrorEvent* ee);
void zoom(const Arg* arg);

/* variables */
static const char broken[] = "broken";
static char stext[256];
static int screen;
static int sw, sh; /* X display screen geometry width, height */
static int bh; /* bar height */
static int lrpad; /* sum of left and right padding for text */
static int (*xerrorxlib)(Display*, XErrorEvent*);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent])(XEvent*) = { [ButtonPress] = buttonpress,
    [ClientMessage] = clientmessage,
    [ConfigureRequest] = configurerequest,
    [ConfigureNotify] = configurenotify,
    [DestroyNotify] = destroynotify,
    [EnterNotify] = enternotify,
    [Expose] = expose,
    [FocusIn] = focusin,
    [KeyPress] = keypress,
    [MappingNotify] = mappingnotify,
    [MapRequest] = maprequest,
    [MotionNotify] = motionnotify,
    [PropertyNotify] = propertynotify,
    [UnmapNotify] = unmapnotify };
static Atom wmatom[WMLast], netatom[NetLast];
static int epoll_fd;
static int dpy_fd;
static int running = 1;
static Cur* cursor[CurLast];
static Clr** scheme;
static Display* dpy;
static Drw* drw;
static Monitor *mons, *selmon, *lastselmon;
static Window root, wmcheckwin;

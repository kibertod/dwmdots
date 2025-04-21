// clang-format off

#include <X11/XF86keysym.h>
#include "dwm.h"
#include "ipc.h"

/* appearance */
static const unsigned int gappx = 10;
static const unsigned int borderpx = 2;
static const unsigned int snap = 32;
static const int usealtbar = 1;
static const char* altbarclass = "Polybar";
static const char* altbarcmd = "bash -c \"/usr/bin/polybar -r bar &\"";
static const int showbar = 1;
static const int topbar = 1;
static const char* fonts[] = { "CommitMono Nerd Font Mono:size=10" };
static const char dmenufont[] = "CommitMono Nerd Font Mono:size=10";

static const char col_gray1[] = "#0f0f0f";
static const char col_gray2[] = "#444444";
static const char col_gray3[] = "#a5a9a5";
static const char col_gray4[] = "#c5c9c5";
static const char col_cyan[] = "#8ba4b0";
static const char* colors[][3] = {
    /*               fg         bg         border   */
    [SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
    [SchemeSel] = { col_gray1,  col_cyan, col_gray4 },
};

static const char* tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
    /*{class, instance, title, tags_mask, isfloating, monitor}*/
    {           "Nitrogen", NULL,    NULL,      0,   1,         -1 },
    {          "Minecraft", NULL,    NULL,      0,   1,         -1 },
    {        "File-roller", NULL,    NULL,      0,   1,         -1 },
    {        "pavucontrol", NULL,    NULL,      0,   1,         -1 },
    {        "pwvucontrol", NULL,    NULL,      0,   1,         -1 },
    {   "gnome-calculator", NULL,    NULL,      0,   1,         -1 },
    { "org.gnome.Nautilus", NULL,    NULL,      0,   1,         -1 },
    {    "amplitube 5.exe", NULL,    NULL,      0,   1,         -1 },
};

/* layout(s) */
static const float mfact = 0.65;
static const int nmaster = 1;
static const int resizehints = 0;
static const int lockfullscreen = 1;

#include "fibonacci.c"
#include "layouts.c"
static const Layout layouts[] = {
    /* symbol     arrange function */
    {  "[]=",    tile },
    {  "><>",    NULL },
    {  "[M]", monocle },
    { "[\\]", dwindle },
    {   "HH",    grid },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY, TAG) \
    { MODKEY, KEY, view, { .ui = 1 << TAG } }, \
    { MODKEY | ControlMask, KEY, toggleview, { .ui = 1 << TAG } }, \
    { MODKEY | ShiftMask, KEY, tag, { .ui = 1 << TAG } }, \
    { MODKEY | ControlMask | ShiftMask, KEY, toggletag, \
    { .ui = 1 << TAG } },

#define SHCMD(cmd) { .v = (const char*[]) { "/bin/sh", "-c", cmd, NULL } }
#define BASHCMDARR(cmd) { "bash", "-c", cmd, NULL }

/* commands */
static char dmenumon[2] = "0";
static const char* dmenucmd    [] = { "rofi", "-show", "drun" };
static const char* termcmd     [] = { "kitty", NULL };
static const char* filescmd    [] = { "pcmanfm", NULL };
static const char* volumeup    [] = { "/home/kibertod/.dwm/volumeUp.sh", NULL };
static const char* volumedown  [] = { "/home/kibertod/.dwm/volumeDown.sh", NULL };
static const char* lockcmd     [] = BASHCMDARR("XSECURELOCK_DISCARD_FIRST_KEYPRESS=0 XSECURELOCK_COMPOSITE_OBSCURER=0 xsecurelock");
static const char* shot_full   [] = BASHCMDARR("maim \"$HOME/Pictures/Screenshots/$(date)\"");
static const char* shot_window [] = BASHCMDARR("maim --window $(xdotool getactivewindow) \"$HOME/Pictures/Screenshots/$(date)\"");
static const char* shot_select [] = BASHCMDARR("maim --select \"$HOME/Pictures/$(date)\"");
static const char* clip_full   [] = BASHCMDARR("maim | xclip -selection clipboard -t image/png");
static const char* clip_window [] = BASHCMDARR("maim --window $(xdotool getactivewindow) | xclip -selection clipboard -t image/png");
static const char* clip_select [] = BASHCMDARR("maim --select | xclip -selection clipboard -t image/png");

static const Key keys[] = {
    /* modifier                     key        function        argument */
    {                  MODKEY,                    XK_b,  togglebar,                { 0 } },
    {                  MODKEY,                    XK_c,  toggleags,                { 0 } },
    {                  MODKEY,                    XK_j, focusstack,          { .i = +1 } },
    {                  MODKEY,                    XK_k, focusstack,          { .i = -1 } },
    {                  MODKEY,                    XK_i, incnmaster,          { .i = +1 } },
    {                  MODKEY,                    XK_o, incnmaster,          { .i = -1 } },
    {      MODKEY | ShiftMask,                    XK_j,   setmfact,       { .f = -0.05 } },
    {      MODKEY | ShiftMask,                    XK_k,   setmfact,       { .f = +0.05 } },
    {      MODKEY | ShiftMask,               XK_Return,       zoom,                { 0 } },
    {                  MODKEY,                  XK_Tab,       view,                { 0 } },
    {      MODKEY | ShiftMask,                    XK_q, killclient,                { 0 } },
    {                  MODKEY,                    XK_t,  setlayout, { .v = &layouts[0] } },
    {                  MODKEY,                    XK_f,  setlayout, { .v = &layouts[1] } },
    {                  MODKEY,                    XK_m,  setlayout, { .v = &layouts[2] } },
    {                  MODKEY,                    XK_s,  setlayout, { .v = &layouts[3] } },
    {                  MODKEY,                    XK_g,  setlayout, { .v = &layouts[4] } },
    {      MODKEY | ShiftMask,                    XK_l,  setlayout,                { 0 } },
    {                  MODKEY,                    XK_0,       view,         { .ui = ~0 } },
    {      MODKEY | ShiftMask,                    XK_0,        tag,         { .ui = ~0 } },
    {                  MODKEY,                XK_comma,   focusmon,          { .i = -1 } },
    {                  MODKEY,               XK_period,   focusmon,          { .i = +1 } },
    {      MODKEY | ShiftMask,                XK_comma,     tagmon,          { .i = -1 } },
    {      MODKEY | ShiftMask,               XK_period,     tagmon,          { .i = +1 } },
    {      MODKEY | ShiftMask,                    XK_e,       quit,                { 0 } },
    // quick launch
    {                  MODKEY,                    XK_d,      spawn,    { .v = dmenucmd } },
    {                  MODKEY,                    XK_e,      spawn,    { .v = filescmd } },
    {                  MODKEY,               XK_Return,      spawn,     { .v = termcmd } },
    {                  MODKEY,                    XK_l,      spawn,     { .v = lockcmd } },
    // media keys
    {                       0, XF86XK_AudioRaiseVolume,      spawn,    { .v = volumeup } },
    {                       0, XF86XK_AudioLowerVolume,      spawn,  { .v = volumedown } },
    // screenshots
    {                       0,                XK_Print,      spawn,   { .v = shot_full } },
    {                  MODKEY,                XK_Print,      spawn, { .v = shot_window } },
    {               ShiftMask,                XK_Print,      spawn, { .v = shot_select } },
    {             ControlMask,                XK_Print,      spawn,   { .v = clip_full } },
    {    ControlMask | MODKEY,                XK_Print,      spawn, { .v = clip_window } },
    { ControlMask | ShiftMask,                XK_Print,      spawn, { .v = clip_select } },
    // tags
    TAGKEYS(XK_1, 0)
    TAGKEYS(XK_2, 1)
    TAGKEYS(XK_3, 2)
    TAGKEYS(XK_4, 3)
    TAGKEYS(XK_5, 4)
    TAGKEYS(XK_6, 5)
    TAGKEYS(XK_7, 6) 
    TAGKEYS(XK_8, 7)
    TAGKEYS(XK_9, 8)
};

/* button definitions */
static const Button buttons[] = {
    /* click                event mask      button          function argument */
    {   ClkLtSymbol,      0, Button1,      setlayout,                { 0 } },
    {   ClkLtSymbol,      0, Button3,      setlayout, { .v = &layouts[2] } },
    {   ClkWinTitle,      0, Button2,           zoom,                { 0 } },
    { ClkStatusText,      0, Button2,          spawn,     { .v = termcmd } },
    {  ClkClientWin, MODKEY, Button1,      movemouse,                { 0 } },
    {  ClkClientWin, MODKEY, Button2, togglefloating,                { 0 } },
    {  ClkClientWin, MODKEY, Button3,    resizemouse,                { 0 } },
    {     ClkTagBar,      0, Button1,           view,                { 0 } },
    {     ClkTagBar,      0, Button3,     toggleview,                { 0 } },
    {     ClkTagBar, MODKEY, Button1,            tag,                { 0 } },
    {     ClkTagBar, MODKEY, Button3,      toggletag,                { 0 } },
};

static const Assigntag assigned_tags[] = {
    { 1 << 1,         "firefox" },
    { 1 << 3, "TelegramDesktop" },
};

static const char* ipcsockpath = "/tmp/dwm.sock";
static IPCCommand ipccommands[] = { IPCCOMMAND(view, 1, { ARG_TYPE_UINT }),
    IPCCOMMAND(toggleview, 1, { ARG_TYPE_UINT }),
    IPCCOMMAND(tag, 1, { ARG_TYPE_UINT }),
    IPCCOMMAND(toggletag, 1, { ARG_TYPE_UINT }),
    IPCCOMMAND(tagmon, 1, { ARG_TYPE_UINT }),
    IPCCOMMAND(focusmon, 1, { ARG_TYPE_SINT }),
    IPCCOMMAND(focusstack, 1, { ARG_TYPE_SINT }),
    IPCCOMMAND(zoom, 1, { ARG_TYPE_NONE }),
    IPCCOMMAND(incnmaster, 1, { ARG_TYPE_SINT }),
    IPCCOMMAND(killclient, 1, { ARG_TYPE_SINT }),
    IPCCOMMAND(togglefloating, 1, { ARG_TYPE_NONE }),
    IPCCOMMAND(setmfact, 1, { ARG_TYPE_FLOAT }),
    IPCCOMMAND(setlayoutsafe, 1, { ARG_TYPE_PTR }),
    IPCCOMMAND(quit, 1, { ARG_TYPE_NONE }) };

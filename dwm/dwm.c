#include <X11/X.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#  include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>

#include "util.h"

/* configuration, allows nested code to access above variables */
#include "config.h"
#ifdef VERSION
#  include "IPCClient.c"
#  include "yajl_dumps.c"
#  include "ipc.c"
#endif

struct Pertag {
    unsigned int curtag, prevtag; /* current and previous tag */
    int nmasters[LENGTH(tags) + 1]; /* number of windows in master area */
    float mfacts[LENGTH(tags) + 1]; /* mfacts per tag */
    unsigned int sellts[LENGTH(tags) + 1]; /* selected layouts */
    const Layout* ltidxs[LENGTH(tags) + 1]
                        [2]; /* matrix of tags and layouts indexes  */
    int showbars[LENGTH(tags) + 1]; /* display bar for the current tag */
};

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags {
    char limitexceeded[LENGTH(tags) > 31 ? -1 : 1];
};

/* function implementations */
void applyrules(Client* c) {
    const char* class, *instance;
    unsigned int i;
    const Rule* r;
    Monitor* m;
    XClassHint ch = { NULL, NULL };

    /* rule matching */
    c->isfloating = 0;
    c->tags = 0;
    XGetClassHint(dpy, c->win, &ch);
    class = ch.res_class ? ch.res_class : broken;
    instance = ch.res_name ? ch.res_name : broken;

    for (i = 0; i < LENGTH(rules); i++) {
        r = &rules[i];
        if ((!r->title || strstr(c->name, r->title)) &&
            (!r->class || strstr(class, r->class)) &&
            (!r->instance || strstr(instance, r->instance))) {
            c->isfloating = r->isfloating;
            c->tags |= r->tags;
            for (m = mons; m && m->num != r->monitor; m = m->next)
                ;
            if (m)
                c->mon = m;
        }
    }
    if (ch.res_class)
        XFree(ch.res_class);
    if (ch.res_name)
        XFree(ch.res_name);
    c->tags =
        c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];
    for (i = 0; i < LENGTH(assigned_tags); i++)
        if (wmclasscontains(c->win, assigned_tags[i].window_class, ""))
            c->tags = assigned_tags[i].tags;
}

int applysizehints(Client* c, int* x, int* y, int* w, int* h, int interact) {
    int baseismin;
    Monitor* m = c->mon;

    /* set minimum possible */
    *w = MAX(1, *w);
    *h = MAX(1, *h);
    if (interact) {
        if (*x > sw)
            *x = sw - WIDTH(c);
        if (*y > sh)
            *y = sh - HEIGHT(c);
        if (*x + *w + 2 * c->bw < 0)
            *x = 0;
        if (*y + *h + 2 * c->bw < 0)
            *y = 0;
    } else {
        if (*x >= m->wx + m->ww)
            *x = m->wx + m->ww - WIDTH(c);
        if (*y >= m->wy + m->wh)
            *y = m->wy + m->wh - HEIGHT(c);
        if (*x + *w + 2 * c->bw <= m->wx)
            *x = m->wx;
        if (*y + *h + 2 * c->bw <= m->wy)
            *y = m->wy;
    }
    if (*h < bh)
        *h = bh;
    if (*w < bh)
        *w = bh;
    if (resizehints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange) {
        if (!c->hintsvalid)
            updatesizehints(c);
        /* see last two sentences in ICCCM 4.1.2.3 */
        baseismin = c->basew == c->minw && c->baseh == c->minh;
        if (!baseismin) { /* temporarily remove base dimensions */
            *w -= c->basew;
            *h -= c->baseh;
        }
        /* adjust for aspect limits */
        if (c->mina > 0 && c->maxa > 0) {
            if (c->maxa < (float)*w / *h)
                *w = *h * c->maxa + 0.5;
            else if (c->mina < (float)*h / *w)
                *h = *w * c->mina + 0.5;
        }
        if (baseismin) { /* increment calculation requires this */
            *w -= c->basew;
            *h -= c->baseh;
        }
        /* adjust for increment value */
        if (c->incw)
            *w -= *w % c->incw;
        if (c->inch)
            *h -= *h % c->inch;
        /* restore base dimensions */
        *w = MAX(*w + c->basew, c->minw);
        *h = MAX(*h + c->baseh, c->minh);
        if (c->maxw)
            *w = MIN(*w, c->maxw);
        if (c->maxh)
            *h = MIN(*h, c->maxh);
    }
    return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

void arrange(Monitor* m) {
    if (m)
        showhide(m->stack);
    else
        for (m = mons; m; m = m->next)
            showhide(m->stack);
    if (m) {
        arrangemon(m);
        restack(m);
    } else
        for (m = mons; m; m = m->next)
            arrangemon(m);
}

void arrangemon(Monitor* m) {
    strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
    if (m->lt[m->sellt]->arrange)
        m->lt[m->sellt]->arrange(m);
}

void attach(Client* c) {
    c->next = c->mon->clients;
    c->mon->clients = c;
}

void attachabove(Client* c) {
    if (c->mon->sel == NULL || c->mon->sel == c->mon->clients ||
        c->mon->sel->isfloating) {
        attach(c);
        return;
    }

    Client* at;
    for (at = c->mon->clients; at->next != c->mon->sel; at = at->next)
        ;
    c->next = at->next;
    at->next = c;
}

void attachstack(Client* c) {
    c->snext = c->mon->stack;
    c->mon->stack = c;
}

void buttonpress(XEvent* e) {
    unsigned int i, x, click;
    Arg arg = { 0 };
    Client* c;
    Monitor* m;
    XButtonPressedEvent* ev = &e->xbutton;

    click = ClkRootWin;
    /* focus monitor if necessary */
    if ((m = wintomon(ev->window)) && m != selmon) {
        unfocus(selmon->sel, 1);
        selmon = m;
        focus(NULL);
    }
    if (ev->window == selmon->barwin) {
        i = x = 0;
        do
            x += TEXTW(tags[i]);
        while (ev->x >= x && ++i < LENGTH(tags));
        if (i < LENGTH(tags)) {
            click = ClkTagBar;
            arg.ui = 1 << i;
        } else if (ev->x < x + TEXTW(selmon->ltsymbol))
            click = ClkLtSymbol;
        else if (ev->x > selmon->ww - (int)TEXTW(stext))
            click = ClkStatusText;
        else
            click = ClkWinTitle;
    } else if ((c = wintoclient(ev->window))) {
        focus(c);
        restack(selmon);
        XAllowEvents(dpy, ReplayPointer, CurrentTime);
        click = ClkClientWin;
    }
    for (i = 0; i < LENGTH(buttons); i++)
        if (click == buttons[i].click && buttons[i].func &&
            buttons[i].button == ev->button &&
            CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
            buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0
                                ? &arg
                                : &buttons[i].arg);
}

void checkotherwm(void) {
    xerrorxlib = XSetErrorHandler(xerrorstart);
    /* this causes an error if some other window manager is running */
    XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
    XSync(dpy, False);
    XSetErrorHandler(xerror);
    XSync(dpy, False);
}

void cleanup(void) {
    Arg a = { .ui = ~0 };
    Layout foo = { "", NULL };
    Monitor* m;
    size_t i;

    view(&a);
    selmon->lt[selmon->sellt] = &foo;
    for (m = mons; m; m = m->next)
        while (m->stack)
            unmanage(m->stack, 0);
    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    while (mons)
        cleanupmon(mons);
    for (i = 0; i < CurLast; i++)
        drw_cur_free(drw, cursor[i]);
    for (i = 0; i < LENGTH(colors); i++)
        free(scheme[i]);
    free(scheme);
    XDestroyWindow(dpy, wmcheckwin);
    drw_free(drw);
    XSync(dpy, False);
    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(dpy, root, netatom[NetActiveWindow]);

    ipc_cleanup();

    if (close(epoll_fd) < 0) {
        fprintf(stderr, "Failed to close epoll file descriptor\n");
    }
}

void cleanupmon(Monitor* mon) {
    Monitor* m;

    if (mon == mons)
        mons = mons->next;
    else {
        for (m = mons; m && m->next != mon; m = m->next)
            ;
        m->next = mon->next;
    }
    if (!usealtbar) {
        XUnmapWindow(dpy, mon->barwin);
        XDestroyWindow(dpy, mon->barwin);
    }
    free(mon);
}

void clientmessage(XEvent* e) {
    XClientMessageEvent* cme = &e->xclient;
    Client* c = wintoclient(cme->window);

    if (!c)
        return;
    if (cme->message_type == netatom[NetWMState]) {
        if (cme->data.l[1] == netatom[NetWMFullscreen] ||
            cme->data.l[2] == netatom[NetWMFullscreen])
            setfullscreen(
                c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
                       || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ &&
                              !c->isfullscreen)));
    } else if (cme->message_type == netatom[NetActiveWindow]) {
        if (c != selmon->sel && !c->isurgent)
            seturgent(c, 1);
    }
}

void configure(Client* c) {
    XConfigureEvent ce;

    ce.type = ConfigureNotify;
    ce.display = dpy;
    ce.event = c->win;
    ce.window = c->win;
    ce.x = c->x;
    ce.y = c->y;
    ce.width = c->w;
    ce.height = c->h;
    ce.border_width = c->bw;
    ce.above = None;
    ce.override_redirect = False;
    XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent*)&ce);
}

void configurenotify(XEvent* e) {
    Monitor* m;
    Client* c;
    XConfigureEvent* ev = &e->xconfigure;
    int dirty;

    /* TODO: updategeom handling sucks, needs to be simplified */
    if (ev->window == root) {
        dirty = (sw != ev->width || sh != ev->height);
        sw = ev->width;
        sh = ev->height;
        if (updategeom() || dirty) {
            drw_resize(drw, sw, bh);
            updatebars();
            for (m = mons; m; m = m->next) {
                for (c = m->clients; c; c = c->next)
                    if (c->isfullscreen)
                        resizeclient(c, m->mx, m->my, m->mw, m->mh);
                XMoveResizeWindow(dpy, m->barwin, m->wx, m->by, m->ww, m->bh);
            }
            focus(NULL);
            arrange(NULL);
        }
    }
}

void configurerequest(XEvent* e) {
    Client* c;
    Monitor* m;
    XConfigureRequestEvent* ev = &e->xconfigurerequest;
    XWindowChanges wc;

    if ((c = wintoclient(ev->window))) {
        if (ev->value_mask & CWBorderWidth)
            c->bw = ev->border_width;
        else if (c->isfloating || !selmon->lt[selmon->sellt]->arrange) {
            m = c->mon;
            if (ev->value_mask & CWX) {
                c->oldx = c->x;
                c->x = m->mx + ev->x;
            }
            if (ev->value_mask & CWY) {
                c->oldy = c->y;
                c->y = m->my + ev->y;
            }
            if (ev->value_mask & CWWidth) {
                c->oldw = c->w;
                c->w = ev->width;
            }
            if (ev->value_mask & CWHeight) {
                c->oldh = c->h;
                c->h = ev->height;
            }
            if ((c->x + c->w) > m->mx + m->mw && c->isfloating)
                c->x = m->mx +
                       (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
            if ((c->y + c->h) > m->my + m->mh && c->isfloating)
                c->y = m->my +
                       (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
            if ((ev->value_mask & (CWX | CWY)) &&
                !(ev->value_mask & (CWWidth | CWHeight)))
                configure(c);
            if (ISVISIBLE(c))
                XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
        } else
            configure(c);
    } else {
        wc.x = ev->x;
        wc.y = ev->y;
        wc.width = ev->width;
        wc.height = ev->height;
        wc.border_width = ev->border_width;
        wc.sibling = ev->above;
        wc.stack_mode = ev->detail;
        XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
    }
    XSync(dpy, False);
}

Monitor* createmon(void) {
    Monitor* m;
    unsigned int i;

    m = ecalloc(1, sizeof(Monitor));
    m->tagset[0] = m->tagset[1] = 1;
    m->mfact = mfact;
    m->nmaster = nmaster;
    m->showbar = showbar;
    m->topbar = topbar;
    m->bh = bh;
    m->lt[0] = &layouts[0];
    m->lt[1] = &layouts[1 % LENGTH(layouts)];
    strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);
    m->pertag = ecalloc(1, sizeof(Pertag));
    m->pertag->curtag = m->pertag->prevtag = 1;

    for (i = 0; i <= LENGTH(tags); i++) {
        m->pertag->nmasters[i] = m->nmaster;
        m->pertag->mfacts[i] = m->mfact;

        m->pertag->ltidxs[i][0] = m->lt[0];
        m->pertag->ltidxs[i][1] = m->lt[1];
        m->pertag->sellts[i] = m->sellt;

        m->pertag->showbars[i] = m->showbar;
    }

    return m;
}

void destroynotify(XEvent* e) {
    Client* c;
    Monitor* m;
    XDestroyWindowEvent* ev = &e->xdestroywindow;

    if ((c = wintoclient(ev->window)))
        unmanage(c, 1);
    else if ((m = wintomon(ev->window)) && m->barwin == ev->window)
        unmanagealtbar(ev->window);
}

void detach(Client* c) {
    Client** tc;

    for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next)
        ;
    *tc = c->next;
}

void detachstack(Client* c) {
    Client **tc, *t;

    for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext)
        ;
    *tc = c->snext;

    if (c == c->mon->sel) {
        for (t = c->mon->stack; t && !ISVISIBLE(t); t = t->snext)
            ;
        c->mon->sel = t;
    }
}

Monitor* dirtomon(int dir) {
    Monitor* m = NULL;

    if (dir > 0) {
        if (!(m = selmon->next))
            m = mons;
    } else if (selmon == mons)
        for (m = mons; m->next; m = m->next)
            ;
    else
        for (m = mons; m->next != selmon; m = m->next)
            ;
    return m;
}

void drawbar(Monitor* m) {
    if (usealtbar)
        return;

    int x, w, tw = 0;
    int boxs = drw->fonts->h / 9;
    int boxw = drw->fonts->h / 6 + 2;
    unsigned int i, occ = 0, urg = 0;
    Client* c;

    if (!m->showbar)
        return;

    /* draw status first so it can be overdrawn by tags later */
    if (m == selmon) { /* status is only drawn on selected monitor */
        drw_setscheme(drw, scheme[SchemeNorm]);
        tw = TEXTW(stext) - lrpad + 2; /* 2px right padding */
        drw_text(drw, m->ww - tw, 0, tw, bh, 0, stext, 0);
    }

    for (c = m->clients; c; c = c->next) {
        occ |= c->tags;
        if (c->isurgent)
            urg |= c->tags;
    }
    x = 0;
    for (i = 0; i < LENGTH(tags); i++) {
        w = TEXTW(tags[i]);
        drw_setscheme(drw,
            scheme[m->tagset[m->seltags] & 1 << i ? SchemeSel : SchemeNorm]);
        drw_text(drw, x, 0, w, bh, lrpad / 2, tags[i], urg & 1 << i);
        if (occ & 1 << i)
            drw_rect(drw, x + boxs, boxs, boxw, boxw,
                m == selmon && selmon->sel && selmon->sel->tags & 1 << i,
                urg & 1 << i);
        x += w;
    }
    w = TEXTW(m->ltsymbol);
    drw_setscheme(drw, scheme[SchemeNorm]);
    x = drw_text(drw, x, 0, w, bh, lrpad / 2, m->ltsymbol, 0);

    if ((w = m->ww - tw - x) > bh) {
        if (m->sel) {
            drw_setscheme(drw, scheme[m == selmon ? SchemeSel : SchemeNorm]);
            drw_text(drw, x, 0, w, bh, lrpad / 2, m->sel->name, 0);
            if (m->sel->isfloating)
                drw_rect(drw, x + boxs, boxs, boxw, boxw, m->sel->isfixed, 0);
        } else {
            drw_setscheme(drw, scheme[SchemeNorm]);
            drw_rect(drw, x, 0, w, bh, 1, 1);
        }
    }
    drw_map(drw, m->barwin, 0, 0, m->ww, bh);
}

void drawbars(void) {
    Monitor* m;

    for (m = mons; m; m = m->next)
        drawbar(m);
}

void enternotify(XEvent* e) {
    Client* c;
    Monitor* m;
    XCrossingEvent* ev = &e->xcrossing;

    if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) &&
        ev->window != root)
        return;
    c = wintoclient(ev->window);
    m = c ? c->mon : wintomon(ev->window);
    if (m != selmon) {
        unfocus(selmon->sel, 1);
        selmon = m;
    } else if (!c || c == selmon->sel)
        return;
    focus(c);
}

void expose(XEvent* e) {
    Monitor* m;
    XExposeEvent* ev = &e->xexpose;

    if (ev->count == 0 && (m = wintomon(ev->window)))
        drawbar(m);
}

void focus(Client* c) {
    if (!c || !ISVISIBLE(c))
        for (c = selmon->stack; c && !ISVISIBLE(c); c = c->snext)
            ;
    if (selmon->sel && selmon->sel != c)
        unfocus(selmon->sel, 0);
    if (c) {
        if (c->mon != selmon)
            selmon = c->mon;
        if (c->isurgent)
            seturgent(c, 0);
        detachstack(c);
        attachstack(c);
        grabbuttons(c, 1);
        XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
        setfocus(c);
    } else {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }
    selmon->sel = c;
    drawbars();
}

/* there are some broken focus acquiring clients needing extra handling */
void focusin(XEvent* e) {
    XFocusChangeEvent* ev = &e->xfocus;

    if (selmon->sel && ev->window != selmon->sel->win)
        setfocus(selmon->sel);
}

void focusmon(const Arg* arg) {
    Monitor* m;

    if (!mons->next)
        return;
    if ((m = dirtomon(arg->i)) == selmon)
        return;
    unfocus(selmon->sel, 0);
    selmon = m;
    focus(NULL);
}

void focusstack(const Arg* arg) {
    Client *c = NULL, *i;

    if (!selmon->sel || (selmon->sel->isfullscreen && lockfullscreen))
        return;
    if (arg->i > 0) {
        for (c = selmon->sel->next; c && !ISVISIBLE(c); c = c->next)
            ;
        if (!c)
            for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next)
                ;
    } else {
        for (i = selmon->clients; i != selmon->sel; i = i->next)
            if (ISVISIBLE(i))
                c = i;
        if (!c)
            for (; i; i = i->next)
                if (ISVISIBLE(i))
                    c = i;
    }
    if (c) {
        focus(c);
        restack(selmon);
    }
}

Atom getatomprop(Client* c, Atom prop) {
    int di;
    unsigned long dl;
    unsigned char* p = NULL;
    Atom da, atom = None;

    if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, XA_ATOM,
            &da, &di, &dl, &dl, &p) == Success &&
        p) {
        atom = *(Atom*)p;
        XFree(p);
    }
    return atom;
}

int getrootptr(int* x, int* y) {
    int di;
    unsigned int dui;
    Window dummy;

    return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long getstate(Window w) {
    int format;
    long result = -1;
    unsigned char* p = NULL;
    unsigned long n, extra;
    Atom real;

    if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False,
            wmatom[WMState], &real, &format, &n, &extra,
            (unsigned char**)&p) != Success)
        return -1;
    if (n != 0)
        result = *p;
    XFree(p);
    return result;
}

int gettextprop(Window w, Atom atom, char* text, unsigned int size) {
    char** list = NULL;
    int n;
    XTextProperty name;

    if (!text || size == 0)
        return 0;
    text[0] = '\0';
    if (!XGetTextProperty(dpy, w, &name, atom) || !name.nitems)
        return 0;
    if (name.encoding == XA_STRING) {
        strncpy(text, (char*)name.value, size - 1);
    } else if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success &&
               n > 0 && *list) {
        strncpy(text, *list, size - 1);
        XFreeStringList(list);
    }
    text[size - 1] = '\0';
    XFree(name.value);
    return 1;
}

void grabbuttons(Client* c, int focused) {
    updatenumlockmask();
    {
        unsigned int i, j;
        unsigned int modifiers[] = { 0, LockMask, numlockmask,
            numlockmask | LockMask };
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        if (!focused)
            XGrabButton(dpy, AnyButton, AnyModifier, c->win, False, BUTTONMASK,
                GrabModeSync, GrabModeSync, None, None);
        for (i = 0; i < LENGTH(buttons); i++)
            if (buttons[i].click == ClkClientWin)
                for (j = 0; j < LENGTH(modifiers); j++)
                    XGrabButton(dpy, buttons[i].button,
                        buttons[i].mask | modifiers[j], c->win, False,
                        BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
    }
}

void grabkeys(void) {
    updatenumlockmask();
    {
        unsigned int i, j, k;
        unsigned int modifiers[] = { 0, LockMask, numlockmask,
            numlockmask | LockMask };
        int start, end, skip;
        KeySym* syms;

        XUngrabKey(dpy, AnyKey, AnyModifier, root);
        XDisplayKeycodes(dpy, &start, &end);
        syms = XGetKeyboardMapping(dpy, start, end - start + 1, &skip);
        if (!syms)
            return;
        for (k = start; k <= end; k++)
            for (i = 0; i < LENGTH(keys); i++)
                /* skip modifier codes, we do that ourselves */
                if (keys[i].keysym == syms[(k - start) * skip])
                    for (j = 0; j < LENGTH(modifiers); j++)
                        XGrabKey(dpy, k, keys[i].mod | modifiers[j], root, True,
                            GrabModeAsync, GrabModeAsync);
        XFree(syms);
    }
}

int handlexevent(struct epoll_event* ev) {
    if (ev->events & EPOLLIN) {
        XEvent ev;
        while (running && XPending(dpy)) {
            XNextEvent(dpy, &ev);
            if (handler[ev.type]) {
                handler[ev.type](&ev); /* call handler */
                ipc_send_events(mons, &lastselmon, selmon);
            }
        }
    } else if (ev->events & EPOLLHUP) {
        return -1;
    }

    return 0;
}

void incnmaster(const Arg* arg) {
    selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag] =
        MAX(selmon->nmaster + arg->i, 0);
    arrange(selmon);
}

#ifdef XINERAMA
static int isuniquegeom(
    XineramaScreenInfo* unique, size_t n, XineramaScreenInfo* info) {
    while (n--)
        if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org &&
            unique[n].width == info->width && unique[n].height == info->height)
            return 0;
    return 1;
}
#endif /* XINERAMA */

void keypress(XEvent* e) {
    unsigned int i;
    KeySym keysym;
    XKeyEvent* ev;

    ev = &e->xkey;
    keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
    for (i = 0; i < LENGTH(keys); i++)
        if (keysym == keys[i].keysym &&
            CLEANMASK(keys[i].mod) == CLEANMASK(ev->state) && keys[i].func)
            keys[i].func(&(keys[i].arg));
}

void killclient(const Arg* arg) {
    if (!selmon->sel)
        return;
    if (!sendevent(selmon->sel, wmatom[WMDelete])) {
        XGrabServer(dpy);
        XSetErrorHandler(xerrordummy);
        XSetCloseDownMode(dpy, DestroyAll);
        XKillClient(dpy, selmon->sel->win);
        XSync(dpy, False);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }
}

void manage(Window w, XWindowAttributes* wa) {
    Client *c, *t = NULL;
    Window trans = None;
    XWindowChanges wc;

    c = ecalloc(1, sizeof(Client));
    c->win = w;
    /* geometry */
    c->x = c->oldx = wa->x;
    c->y = c->oldy = wa->y;
    c->w = c->oldw = wa->width;
    c->h = c->oldh = wa->height;
    c->oldbw = wa->border_width;

    updatetitle(c);
    if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
        c->mon = t->mon;
        c->tags = t->tags;
    } else {
        c->mon = selmon;
        applyrules(c);
    }

    if (c->x + WIDTH(c) > c->mon->wx + c->mon->ww)
        c->x = c->mon->wx + c->mon->ww - WIDTH(c);
    if (c->y + HEIGHT(c) > c->mon->wy + c->mon->wh)
        c->y = c->mon->wy + c->mon->wh - HEIGHT(c);
    c->x = MAX(c->x, c->mon->wx);
    c->y = MAX(c->y, c->mon->wy);
    c->bw = borderpx;

    wc.border_width = c->bw;
    XConfigureWindow(dpy, w, CWBorderWidth, &wc);
    XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColBorder].pixel);
    configure(c); /* propagates border_width, if size doesn't change */
    updatewindowtype(c);
    updatesizehints(c);
    updatewmhints(c);
    c->x = c->mon->mx + (c->mon->mw - WIDTH(c)) / 2;
    c->y = c->mon->my + (c->mon->mh - HEIGHT(c)) / 2;
    XSelectInput(dpy, w,
        EnterWindowMask | FocusChangeMask | PropertyChangeMask |
            StructureNotifyMask);
    grabbuttons(c, 0);
    if (!c->isfloating)
        c->isfloating = c->oldstate = trans != None || c->isfixed;
    if (c->isfloating)
        XRaiseWindow(dpy, c->win);
    attachabove(c);
    attachstack(c);
    XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
        PropModeAppend, (unsigned char*)&(c->win), 1);
    XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w,
        c->h); /* some windows require this */
    setclientstate(c, NormalState);
    if (c->mon == selmon)
        unfocus(selmon->sel, 0);
    c->mon->sel = c;
    arrange(c->mon);
    XMapWindow(dpy, c->win);
    focus(NULL);
}

void managealtbar(Window win, XWindowAttributes* wa) {
    Monitor* m;
    if (!(m = recttomon(wa->x, wa->y, wa->width, wa->height)))
        return;

    m->barwin = win;
    m->by = wa->y;
    bh = m->bh = wa->height;
    updatebarpos(m);
    arrange(m);
    XSelectInput(dpy, win,
        EnterWindowMask | FocusChangeMask | PropertyChangeMask |
            StructureNotifyMask);
    XMoveResizeWindow(dpy, win, wa->x, wa->y, wa->width, wa->height);
    XMapWindow(dpy, win);
    XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
        PropModeAppend, (unsigned char*)&win, 1);
}

void manageagsbar(Window win, XWindowAttributes* wa) {
    Monitor* m;
    if (!(m = recttomon(wa->x, wa->y, wa->width, wa->height)))
        return;

    m->agswin = win;
    arrange(m);
    XMoveResizeWindow(dpy, win, 0, 20, wa->width, 1060);
    XMapWindow(dpy, win);
}

void mappingnotify(XEvent* e) {
    XMappingEvent* ev = &e->xmapping;

    XRefreshKeyboardMapping(ev);
    if (ev->request == MappingKeyboard)
        grabkeys();
}

void maprequest(XEvent* e) {
    static XWindowAttributes wa;
    XMapRequestEvent* ev = &e->xmaprequest;

    if (!XGetWindowAttributes(dpy, ev->window, &wa) || wa.override_redirect)
        return;
    if (wmclasscontains(ev->window, altbarclass, ""))
        managealtbar(ev->window, &wa);
    else if (wmclasscontains(ev->window, "Com.github.Aylur.ags", ""))
        manageagsbar(ev->window, &wa);
    else if (!wintoclient(ev->window))
        manage(ev->window, &wa);
}

void monocle(Monitor* m) {
    unsigned int n = 0;
    Client* c;

    for (c = m->clients; c; c = c->next)
        if (ISVISIBLE(c))
            n++;
    if (n > 0) /* override layout symbol */
        snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
    for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
        resize(c, m->wx, m->wy, m->ww - 2 * c->bw, m->wh - 2 * c->bw, 0);
}

void motionnotify(XEvent* e) {
    static Monitor* mon = NULL;
    Monitor* m;
    XMotionEvent* ev = &e->xmotion;

    if (ev->window != root)
        return;
    if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
        unfocus(selmon->sel, 1);
        selmon = m;
        focus(NULL);
    }
    mon = m;
}

void toggleags(const Arg* arg) {
    struct sigaction sa;
    Monitor* m = recttomon(0, 0, 1, 1);
    if (!m->agswin) {
        m->agsshown = 1;
        if (fork() == 0) {
            if (dpy)
                close(ConnectionNumber(dpy));
            setsid();

            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            sa.sa_handler = SIG_DFL;
            sigaction(SIGCHLD, &sa, NULL);
            char* argument_list[] = { "agsv1", "-t", "sidebar", NULL };
            execvp("agsv1", argument_list);
        }
    } else {
        if (m->agsshown) {
            XMoveWindow(dpy, m->agswin, -1920, 20);
            m->agsshown = 0;
        } else {
            XMoveWindow(dpy, m->agswin, 0, 20);
            m->agsshown = 1;
        }
    }
}

void movemouse(const Arg* arg) {
    int x, y, ocx, ocy, nx, ny;
    Client* c;
    Monitor* m;
    XEvent ev;
    Time lasttime = 0;

    if (!(c = selmon->sel))
        return;
    if (c->isfullscreen) /* no support moving fullscreen windows by mouse */
        return;
    restack(selmon);
    ocx = c->x;
    ocy = c->y;
    if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
            None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
        return;
    if (!getrootptr(&x, &y))
        return;
    do {
        XMaskEvent(
            dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
        switch (ev.type) {
        case ConfigureRequest:
        case Expose:
        case MapRequest:
            handler[ev.type](&ev);
            break;
        case MotionNotify:
            if ((ev.xmotion.time - lasttime) <= (1000 / 60))
                continue;
            lasttime = ev.xmotion.time;

            nx = ocx + (ev.xmotion.x - x);
            ny = ocy + (ev.xmotion.y - y);
            if (abs(selmon->wx - nx) < snap)
                nx = selmon->wx;
            else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap)
                nx = selmon->wx + selmon->ww - WIDTH(c);
            if (abs(selmon->wy - ny) < snap)
                ny = selmon->wy;
            else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap)
                ny = selmon->wy + selmon->wh - HEIGHT(c);
            if (!c->isfloating && selmon->lt[selmon->sellt]->arrange &&
                (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
                togglefloating(NULL);
            if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
                resize(c, nx, ny, c->w, c->h, 1);
            break;
        }
    } while (ev.type != ButtonRelease);
    XUngrabPointer(dpy, CurrentTime);
    if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
        sendmon(c, m);
        selmon = m;
        focus(NULL);
    }
}

Client* nexttiled(Client* c) {
    for (; c && (c->isfloating || !ISVISIBLE(c)); c = c->next)
        ;
    return c;
}

void pop(Client* c) {
    detach(c);
    attach(c);
    focus(c);
    arrange(c->mon);
}

void propertynotify(XEvent* e) {
    Client* c;
    Window trans;
    XPropertyEvent* ev = &e->xproperty;

    if ((ev->window == root) && (ev->atom == XA_WM_NAME))
        updatestatus();
    else if (ev->state == PropertyDelete)
        return; /* ignore */
    else if ((c = wintoclient(ev->window))) {
        switch (ev->atom) {
        default:
            break;
        case XA_WM_TRANSIENT_FOR:
            if (!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
                (c->isfloating = (wintoclient(trans)) != NULL))
                arrange(c->mon);
            break;
        case XA_WM_NORMAL_HINTS:
            c->hintsvalid = 0;
            break;
        case XA_WM_HINTS:
            updatewmhints(c);
            drawbars();
            break;
        }
        if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
            updatetitle(c);
            if (c == c->mon->sel)
                drawbar(c->mon);
        }
        if (ev->atom == netatom[NetWMWindowType])
            updatewindowtype(c);
    }
}

void quit(const Arg* arg) { running = 0; }

Monitor* recttomon(int x, int y, int w, int h) {
    Monitor *m, *r = selmon;
    int a, area = 0;

    for (m = mons; m; m = m->next)
        if ((a = INTERSECT(x, y, w, h, m)) > area) {
            area = a;
            r = m;
        }
    return r;
}

void resize(Client* c, int x, int y, int w, int h, int interact) {
    if (applysizehints(c, &x, &y, &w, &h, interact))
        resizeclient(c, x, y, w, h);
}

void resizeclient(Client* c, int x, int y, int w, int h) {
    XWindowChanges wc;
    unsigned int n;
    unsigned int gapoffset;
    unsigned int gapincr;
    Client* nbc;

    wc.border_width = c->bw;

    /* Get number of clients for the client's monitor */
    for (n = 0, nbc = nexttiled(c->mon->clients); nbc;
        nbc = nexttiled(nbc->next), n++)
        ;

    /* Do nothing if layout is floating */
    if (c->isfloating || c->mon->lt[c->mon->sellt]->arrange == NULL) {
        gapincr = gapoffset = 0;
    } else {
        /* Remove border and gap if layout is monocle or only one client */
        if (c->mon->lt[c->mon->sellt]->arrange == monocle || n == 1) {
            gapoffset = 0;
            gapincr = -2 * borderpx;
            wc.border_width = 0;
        } else {
            gapoffset = gappx;
            gapincr = 2 * gappx;
        }
    }

    c->oldx = c->x;
    c->x = wc.x = x + gapoffset;
    c->oldy = c->y;
    c->y = wc.y = y + gapoffset;
    c->oldw = c->w;
    c->w = wc.width = w - gapincr;
    c->oldh = c->h;
    c->h = wc.height = h - gapincr;

    XConfigureWindow(
        dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
    configure(c);
    XSync(dpy, False);
}

void resizemouse(const Arg* arg) {
    int ocx, ocy, nw, nh;
    Client* c;
    Monitor* m;
    XEvent ev;
    Time lasttime = 0;

    if (!(c = selmon->sel))
        return;
    if (c->isfullscreen) /* no support resizing fullscreen windows by mouse */
        return;
    restack(selmon);
    ocx = c->x;
    ocy = c->y;
    if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
            None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
        return;
    XWarpPointer(
        dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
    do {
        XMaskEvent(
            dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
        switch (ev.type) {
        case ConfigureRequest:
        case Expose:
        case MapRequest:
            handler[ev.type](&ev);
            break;
        case MotionNotify:
            if ((ev.xmotion.time - lasttime) <= (1000 / 60))
                continue;
            lasttime = ev.xmotion.time;

            nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
            nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
            if (c->mon->wx + nw >= selmon->wx &&
                c->mon->wx + nw <= selmon->wx + selmon->ww &&
                c->mon->wy + nh >= selmon->wy &&
                c->mon->wy + nh <= selmon->wy + selmon->wh) {
                if (!c->isfloating && selmon->lt[selmon->sellt]->arrange &&
                    (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
                    togglefloating(NULL);
            }
            if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
                resize(c, c->x, c->y, nw, nh, 1);
            break;
        }
    } while (ev.type != ButtonRelease);
    XWarpPointer(
        dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
    XUngrabPointer(dpy, CurrentTime);
    while (XCheckMaskEvent(dpy, EnterWindowMask, &ev))
        ;
    if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
        sendmon(c, m);
        selmon = m;
        focus(NULL);
    }
}

void restack(Monitor* m) {
    Client* c;
    XEvent ev;
    XWindowChanges wc;

    drawbar(m);
    if (!m->sel)
        return;
    if (m->sel->isfloating || !m->lt[m->sellt]->arrange)
        XRaiseWindow(dpy, m->sel->win);
    if (m->lt[m->sellt]->arrange) {
        wc.stack_mode = Below;
        wc.sibling = m->barwin;
        for (c = m->stack; c; c = c->snext) {
            if (!c->isfloating && ISVISIBLE(c)) {
                XConfigureWindow(dpy, c->win, CWSibling | CWStackMode, &wc);
                wc.sibling = c->win;
            }
        }
        wc.stack_mode = Below;
        wc.sibling = m->agswin;
        for (c = m->stack; c; c = c->snext) {
            if (!c->isfloating && ISVISIBLE(c)) {
                XConfigureWindow(dpy, c->win, CWSibling | CWStackMode, &wc);
                wc.sibling = c->win;
            }
        }
    }
    XSync(dpy, False);
    while (XCheckMaskEvent(dpy, EnterWindowMask, &ev))
        ;
}

void run(void) {
    int event_count = 0;
    const int MAX_EVENTS = 10;
    struct epoll_event events[MAX_EVENTS];
    XSync(dpy, False);
    while (running) {
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        for (int i = 0; i < event_count; i++) {
            int event_fd = events[i].data.fd;
            DEBUG("Got event from fd %d\n", event_fd);

            if (event_fd == dpy_fd) {
                // -1 means EPOLLHUP
                if (handlexevent(events + i) == -1)
                    return;
            } else if (event_fd == ipc_get_sock_fd()) {
                ipc_handle_socket_epoll_event(events + i);
            } else if (ipc_is_client_registered(event_fd)) {
                if (ipc_handle_client_epoll_event(events + i, mons, &lastselmon,
                        selmon, tags, LENGTH(tags), layouts,
                        LENGTH(layouts)) < 0) {
                    fprintf(stderr, "Error handling IPC event on fd %d\n",
                        event_fd);
                }
            } else {
                fprintf(stderr,
                    "Got event from unknown fd %d, ptr %p, u32 %d, u64 %lu",
                    event_fd, events[i].data.ptr, events[i].data.u32,
                    events[i].data.u64);
                fprintf(stderr, " with events %d\n", events[i].events);
                return;
            }
        }
    }
}

void runAutostart(void) {
    system("cd ~/.dwm; ./autostart_blocking.sh");
    system("cd ~/.dwm; ./autostart.sh &");
}

void scan(void) {
    unsigned int i, num;
    Window d1, d2, *wins = NULL;
    XWindowAttributes wa;

    if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
        for (i = 0; i < num; i++) {
            if (!XGetWindowAttributes(dpy, wins[i], &wa) ||
                wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
                continue;
            if (wmclasscontains(wins[i], altbarclass, ""))
                managealtbar(wins[i], &wa);
            else if (wa.map_state == IsViewable ||
                     getstate(wins[i]) == IconicState)
                manage(wins[i], &wa);
        }
        for (i = 0; i < num; i++) { /* now the transients */
            if (!XGetWindowAttributes(dpy, wins[i], &wa))
                continue;
            if (XGetTransientForHint(dpy, wins[i], &d1) &&
                (wa.map_state == IsViewable ||
                    getstate(wins[i]) == IconicState))
                manage(wins[i], &wa);
        }
        if (wins)
            XFree(wins);
    }
}

void sendmon(Client* c, Monitor* m) {
    if (c->mon == m)
        return;
    unfocus(c, 1);
    detach(c);
    detachstack(c);
    c->mon = m;
    c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
    attachabove(c);
    attachstack(c);
    focus(NULL);
    arrange(NULL);
}

void setclientstate(Client* c, long state) {
    long data[] = { state, None };

    XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
        PropModeReplace, (unsigned char*)data, 2);
}

int sendevent(Client* c, Atom proto) {
    int n;
    Atom* protocols;
    int exists = 0;
    XEvent ev;

    if (XGetWMProtocols(dpy, c->win, &protocols, &n)) {
        while (!exists && n--)
            exists = protocols[n] == proto;
        XFree(protocols);
    }
    if (exists) {
        ev.type = ClientMessage;
        ev.xclient.window = c->win;
        ev.xclient.message_type = wmatom[WMProtocols];
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = proto;
        ev.xclient.data.l[1] = CurrentTime;
        XSendEvent(dpy, c->win, False, NoEventMask, &ev);
    }
    return exists;
}

void setfocus(Client* c) {
    if (!c->neverfocus) {
        XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
        XChangeProperty(dpy, root, netatom[NetActiveWindow], XA_WINDOW, 32,
            PropModeReplace, (unsigned char*)&(c->win), 1);
    }
    sendevent(c, wmatom[WMTakeFocus]);
}

void setfullscreen(Client* c, int fullscreen) {
    if (fullscreen && !c->isfullscreen) {
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
            PropModeReplace, (unsigned char*)&netatom[NetWMFullscreen], 1);
        c->isfullscreen = 1;
        c->oldstate = c->isfloating;
        c->oldbw = c->bw;
        c->bw = 0;
        c->isfloating = 1;
        resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
        XRaiseWindow(dpy, c->win);
    } else if (!fullscreen && c->isfullscreen) {
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
            PropModeReplace, (unsigned char*)0, 0);
        c->isfullscreen = 0;
        c->isfloating = c->oldstate;
        c->bw = c->oldbw;
        c->x = c->oldx;
        c->y = c->oldy;
        c->w = c->oldw;
        c->h = c->oldh;
        resizeclient(c, c->x, c->y, c->w, c->h);
        arrange(c->mon);
    }
}

void setlayout(const Arg* arg) {
    if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
        selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag] ^= 1;
    if (arg && arg->v)
        selmon->lt[selmon->sellt] =
            selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt] =
                (Layout*)arg->v;
    strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol,
        sizeof selmon->ltsymbol);
    if (selmon->sel)
        arrange(selmon);
    else
        drawbar(selmon);
}

void setlayoutsafe(const Arg* arg) {
    const Layout* ltptr = (Layout*)arg->v;
    if (ltptr == 0)
        setlayout(arg);
    for (int i = 0; i < LENGTH(layouts); i++) {
        if (ltptr == &layouts[i])
            setlayout(arg);
    }
}

/* arg > 1.0 will set mfact absolutely */
void setmfact(const Arg* arg) {
    float f;

    if (!arg || !selmon->lt[selmon->sellt]->arrange)
        return;
    f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
    if (f < 0.05 || f > 0.95)
        return;
    selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag] = f;
    arrange(selmon);
}

void setup(void) {
    int i;
    XSetWindowAttributes wa;
    Atom utf8string;
    struct sigaction sa;

    /* do not transform children into zombies when they terminate */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGCHLD, &sa, NULL);

    /* clean up any zombies (inherited from .xinitrc etc) immediately */
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    /* init screen */
    screen = DefaultScreen(dpy);
    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);
    root = RootWindow(dpy, screen);
    drw = drw_create(dpy, screen, root, sw, sh);
    if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
        die("no fonts could be loaded.");
    lrpad = drw->fonts->h;
    bh = usealtbar ? 0 : drw->fonts->h + 2;
    updategeom();
    /* init atoms */
    utf8string = XInternAtom(dpy, "UTF8_STRING", False);
    wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
    wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
    wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
    netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
    netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
    netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
    netatom[NetWMWindowOpacity] =
        XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", False);
    netatom[NetWMCheck] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
    netatom[NetWMFullscreen] =
        XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
    netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    netatom[NetWMWindowTypeDialog] =
        XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
    /* init cursors */
    cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
    cursor[CurResize] = drw_cur_create(drw, XC_sizing);
    cursor[CurMove] = drw_cur_create(drw, XC_fleur);
    /* init appearance */
    scheme = ecalloc(LENGTH(colors), sizeof(Clr*));
    for (i = 0; i < LENGTH(colors); i++)
        scheme[i] = drw_scm_create(drw, colors[i], 3);
    /* init bars */
    updatebars();
    updatestatus();
    /* supporting window for NetWMCheck */
    wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
    XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32,
        PropModeReplace, (unsigned char*)&wmcheckwin, 1);
    XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8,
        PropModeReplace, (unsigned char*)"dwm", 3);
    XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32,
        PropModeReplace, (unsigned char*)&wmcheckwin, 1);
    /* EWMH support per view */
    XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
        PropModeReplace, (unsigned char*)netatom, NetLast);
    XDeleteProperty(dpy, root, netatom[NetClientList]);
    /* select events */
    wa.cursor = cursor[CurNormal]->cursor;
    wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask |
                    ButtonPressMask | PointerMotionMask | EnterWindowMask |
                    LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;
    XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);
    XSelectInput(dpy, root, wa.event_mask);
    grabkeys();
    focus(NULL);
    spawnbar();
    setupepoll();
}

void setupepoll(void) {
    epoll_fd = epoll_create1(0);
    dpy_fd = ConnectionNumber(dpy);
    struct epoll_event dpy_event;

    // Initialize struct to 0
    memset(&dpy_event, 0, sizeof(dpy_event));

    DEBUG("Display socket is fd %d\n", dpy_fd);

    if (epoll_fd == -1) {
        fputs("Failed to create epoll file descriptor", stderr);
    }

    dpy_event.events = EPOLLIN;
    dpy_event.data.fd = dpy_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, dpy_fd, &dpy_event)) {
        fputs("Failed to add display file descriptor to epoll", stderr);
        close(epoll_fd);
        exit(1);
    }

    if (ipc_init(ipcsockpath, epoll_fd, ipccommands, LENGTH(ipccommands)) < 0) {
        fputs("Failed to initialize IPC\n", stderr);
    }
}

void seturgent(Client* c, int urg) {
    XWMHints* wmh;

    c->isurgent = urg;
    if (!(wmh = XGetWMHints(dpy, c->win)))
        return;
    wmh->flags =
        urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
    XSetWMHints(dpy, c->win, wmh);
    XFree(wmh);
}

void showhide(Client* c) {
    if (!c)
        return;
    if (ISVISIBLE(c)) {
        /* show clients top down */
        XMoveWindow(dpy, c->win, c->x, c->y);
        if ((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) &&
            !c->isfullscreen)
            resize(c, c->x, c->y, c->w, c->h, 0);
        showhide(c->snext);
    } else {
        /* hide clients bottom up */
        unsigned int client_tag = ffs(c->tags) - 1;
        unsigned int selected_tag = ffs(selmon->tagset[selmon->seltags]) - 1;
        if (client_tag < selected_tag) {
            XMoveWindow(dpy, c->win, -1920, c->y);
        } else {
            XMoveWindow(dpy, c->win, 1920, c->y);
        }
        showhide(c->snext);
    }
}

void spawn(const Arg* arg) {
    struct sigaction sa;

    if (arg->v == dmenucmd)
        dmenumon[0] = '0' + selmon->num;
    if (fork() == 0) {
        if (dpy)
            close(ConnectionNumber(dpy));
        setsid();

        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = SIG_DFL;
        sigaction(SIGCHLD, &sa, NULL);

        execvp(((char**)arg->v)[0], (char**)arg->v);
        die("dwm: execvp '%s' failed:", ((char**)arg->v)[0]);
    }
}

void spawnbar() {
    if (*altbarcmd)
        system(altbarcmd);
}

void tag(const Arg* arg) {
    if (selmon->sel && arg->ui & TAGMASK) {
        selmon->sel->tags = arg->ui & TAGMASK;
        focus(NULL);
        arrange(selmon);
    }
}

void tagmon(const Arg* arg) {
    if (!selmon->sel || !mons->next)
        return;
    sendmon(selmon->sel, dirtomon(arg->i));
}

void tile(Monitor* m) {
    unsigned int i, n, h, mw, my, ty;
    Client* c;

    for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++)
        ;
    if (n == 0)
        return;

    if (n > m->nmaster)
        mw = m->nmaster ? m->ww * m->mfact : 0;
    else
        mw = m->ww;
    for (i = my = ty = 0, c = nexttiled(m->clients); c;
        c = nexttiled(c->next), i++)
        if (i < m->nmaster) {
            h = (m->wh - my) / (MIN(n, m->nmaster) - i);
            resize(c, m->wx, m->wy + my, mw - (2 * c->bw) + (n > 1 ? gappx : 0),
                h - (2 * c->bw), 0);
            if (my + HEIGHT(c) < m->wh)
                my += HEIGHT(c);
        } else {
            h = (m->wh - ty) / (n - i);
            resize(c, m->wx + mw, m->wy + ty, m->ww - mw - (2 * c->bw),
                h - (2 * c->bw), 0);
            if (ty + HEIGHT(c) < m->wh)
                ty += HEIGHT(c);
        }
}

void togglebar(const Arg* arg) {
    selmon->showbar = selmon->pertag->showbars[selmon->pertag->curtag] =
        !selmon->showbar;
    updatebarpos(selmon);
    XMoveResizeWindow(
        dpy, selmon->barwin, selmon->wx, selmon->by, selmon->ww, selmon->bh);
    arrange(selmon);
}

void togglefloating(const Arg* arg) {
    if (!selmon->sel)
        return;
    if (selmon->sel->isfullscreen) /* no support for fullscreen windows */
        return;
    selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;
    if (selmon->sel->isfloating)
        resize(selmon->sel, selmon->sel->x, selmon->sel->y, selmon->sel->w,
            selmon->sel->h, 0);
    arrange(selmon);
}

void toggletag(const Arg* arg) {
    unsigned int newtags;

    if (!selmon->sel)
        return;
    newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
    if (newtags) {
        selmon->sel->tags = newtags;
        focus(NULL);
        arrange(selmon);
    }
}

void toggleview(const Arg* arg) {
    unsigned int newtagset =
        selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);
    int i;

    if (newtagset) {
        selmon->tagset[selmon->seltags] = newtagset;

        if (newtagset == ~0) {
            selmon->pertag->prevtag = selmon->pertag->curtag;
            selmon->pertag->curtag = 0;
        }

        /* test if the user did not select the same tag */
        if (!(newtagset & 1 << (selmon->pertag->curtag - 1))) {
            selmon->pertag->prevtag = selmon->pertag->curtag;
            for (i = 0; !(newtagset & 1 << i); i++)
                ;
            selmon->pertag->curtag = i + 1;
        }

        /* apply settings for this view */
        selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
        selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
        selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
        selmon->lt[selmon->sellt] =
            selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
        selmon->lt[selmon->sellt ^ 1] =
            selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt ^ 1];

        if (selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag])
            togglebar(NULL);

        focus(NULL);
        arrange(selmon);
    }
}

void unfocus(Client* c, int setfocus) {
    if (!c)
        return;
    grabbuttons(c, 0);
    XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColBorder].pixel);
    if (setfocus) {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }
}

void unmanage(Client* c, int destroyed) {
    Monitor* m = c->mon;
    XWindowChanges wc;

    detach(c);
    detachstack(c);
    if (!destroyed) {
        wc.border_width = c->oldbw;
        XGrabServer(dpy); /* avoid race conditions */
        XSetErrorHandler(xerrordummy);
        XSelectInput(dpy, c->win, NoEventMask);
        XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        setclientstate(c, WithdrawnState);
        XSync(dpy, False);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }
    free(c);
    focus(NULL);
    updateclientlist();
    arrange(m);
}

void unmanagealtbar(Window w) {
    Monitor* m = wintomon(w);

    if (!m)
        return;

    m->barwin = 0;
    m->by = 0;
    m->bh = 0;
    updatebarpos(m);
    arrange(m);
}

void unmapnotify(XEvent* e) {
    Client* c;
    Monitor* m;
    XUnmapEvent* ev = &e->xunmap;

    if ((c = wintoclient(ev->window))) {
        if (ev->send_event)
            setclientstate(c, WithdrawnState);
        else
            unmanage(c, 0);
    } else if ((m = wintomon(ev->window)) && m->barwin == ev->window)
        unmanagealtbar(ev->window);
}

void updatebars(void) {
    if (usealtbar)
        return;

    Monitor* m;
    XSetWindowAttributes wa = { .override_redirect = True,
        .background_pixmap = ParentRelative,
        .event_mask = ButtonPressMask | ExposureMask };
    XClassHint ch = { "dwm", "dwm" };
    for (m = mons; m; m = m->next) {
        if (m->barwin)
            continue;
        m->barwin = XCreateWindow(dpy, root, m->wx, m->by, m->ww, bh, 0,
            DefaultDepth(dpy, screen), CopyFromParent,
            DefaultVisual(dpy, screen),
            CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
        XDefineCursor(dpy, m->barwin, cursor[CurNormal]->cursor);
        XMapRaised(dpy, m->barwin);
        XSetClassHint(dpy, m->barwin, &ch);
    }
}

void updatebarpos(Monitor* m) {
    m->wy = m->my;
    m->wh = m->mh;
    if (m->showbar) {
        m->wh -= m->bh;
        m->by = m->topbar ? m->wy : m->wy + m->wh;
        m->wy = m->topbar ? m->wy + m->bh : m->wy;
    } else
        m->by = -m->bh;
}

void updateclientlist(void) {
    Client* c;
    Monitor* m;

    XDeleteProperty(dpy, root, netatom[NetClientList]);
    for (m = mons; m; m = m->next)
        for (c = m->clients; c; c = c->next)
            XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
                PropModeAppend, (unsigned char*)&(c->win), 1);
}

int updategeom(void) {
    int dirty = 0;

#ifdef XINERAMA
    if (XineramaIsActive(dpy)) {
        int i, j, n, nn;
        Client* c;
        Monitor* m;
        XineramaScreenInfo* info = XineramaQueryScreens(dpy, &nn);
        XineramaScreenInfo* unique = NULL;

        for (n = 0, m = mons; m; m = m->next, n++)
            ;
        /* only consider unique geometries as separate screens */
        unique = ecalloc(nn, sizeof(XineramaScreenInfo));
        for (i = 0, j = 0; i < nn; i++)
            if (isuniquegeom(unique, j, &info[i]))
                memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
        XFree(info);
        nn = j;

        /* new monitors if nn > n */
        for (i = n; i < nn; i++) {
            for (m = mons; m && m->next; m = m->next)
                ;
            if (m)
                m->next = createmon();
            else
                mons = createmon();
        }
        for (i = 0, m = mons; i < nn && m; m = m->next, i++)
            if (i >= n || unique[i].x_org != m->mx ||
                unique[i].y_org != m->my || unique[i].width != m->mw ||
                unique[i].height != m->mh) {
                dirty = 1;
                m->num = i;
                m->mx = m->wx = unique[i].x_org;
                m->my = m->wy = unique[i].y_org;
                m->mw = m->ww = unique[i].width;
                m->mh = m->wh = unique[i].height;
                updatebarpos(m);
            }
        /* removed monitors if n > nn */
        for (i = nn; i < n; i++) {
            for (m = mons; m && m->next; m = m->next)
                ;
            while ((c = m->clients)) {
                dirty = 1;
                m->clients = c->next;
                detachstack(c);
                c->mon = mons;
                attachabove(c);
                attachstack(c);
            }
            if (m == selmon)
                selmon = mons;
            cleanupmon(m);
        }
        free(unique);
    } else
#endif /* XINERAMA */
    { /* default monitor setup */
        if (!mons)
            mons = createmon();
        if (mons->mw != sw || mons->mh != sh) {
            dirty = 1;
            mons->mw = mons->ww = sw;
            mons->mh = mons->wh = sh;
            updatebarpos(mons);
        }
    }
    if (dirty) {
        selmon = mons;
        selmon = wintomon(root);
    }
    return dirty;
}

void updatenumlockmask(void) {
    unsigned int i, j;
    XModifierKeymap* modmap;

    numlockmask = 0;
    modmap = XGetModifierMapping(dpy);
    for (i = 0; i < 8; i++)
        for (j = 0; j < modmap->max_keypermod; j++)
            if (modmap->modifiermap[i * modmap->max_keypermod + j] ==
                XKeysymToKeycode(dpy, XK_Num_Lock))
                numlockmask = (1 << i);
    XFreeModifiermap(modmap);
}

void updatesizehints(Client* c) {
    long msize;
    XSizeHints size;

    if (!XGetWMNormalHints(dpy, c->win, &size, &msize))
        /* size is uninitialized, ensure that size.flags aren't used */
        size.flags = PSize;
    if (size.flags & PBaseSize) {
        c->basew = size.base_width;
        c->baseh = size.base_height;
    } else if (size.flags & PMinSize) {
        c->basew = size.min_width;
        c->baseh = size.min_height;
    } else
        c->basew = c->baseh = 0;
    if (size.flags & PResizeInc) {
        c->incw = size.width_inc;
        c->inch = size.height_inc;
    } else
        c->incw = c->inch = 0;
    if (size.flags & PMaxSize) {
        c->maxw = size.max_width;
        c->maxh = size.max_height;
    } else
        c->maxw = c->maxh = 0;
    if (size.flags & PMinSize) {
        c->minw = size.min_width;
        c->minh = size.min_height;
    } else if (size.flags & PBaseSize) {
        c->minw = size.base_width;
        c->minh = size.base_height;
    } else
        c->minw = c->minh = 0;
    if (size.flags & PAspect) {
        c->mina = (float)size.min_aspect.y / size.min_aspect.x;
        c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
    } else
        c->maxa = c->mina = 0.0;
    c->isfixed =
        (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh);
    c->hintsvalid = 1;
}

void updatestatus(void) {
    if (!gettextprop(root, XA_WM_NAME, stext, sizeof(stext)))
        strcpy(stext, "dwm-" VERSION);
    drawbar(selmon);
}

void updatetitle(Client* c) {
    char oldname[sizeof(c->name)];
    strcpy(oldname, c->name);

    if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
        gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
    if (c->name[0] == '\0') /* hack to mark broken clients */
        strcpy(c->name, broken);

    for (Monitor* m = mons; m; m = m->next) {
        if (m->sel == c && strcmp(oldname, c->name) != 0)
            ipc_focused_title_change_event(m->num, c->win, oldname, c->name);
    }
}

void updatewindowtype(Client* c) {
    Atom state = getatomprop(c, netatom[NetWMState]);
    Atom wtype = getatomprop(c, netatom[NetWMWindowType]);

    if (state == netatom[NetWMFullscreen])
        setfullscreen(c, 1);
    if (wtype == netatom[NetWMWindowTypeDialog])
        c->isfloating = 1;
}

void updatewmhints(Client* c) {
    XWMHints* wmh;

    if ((wmh = XGetWMHints(dpy, c->win))) {
        if (c == selmon->sel && wmh->flags & XUrgencyHint) {
            wmh->flags &= ~XUrgencyHint;
            XSetWMHints(dpy, c->win, wmh);
        } else
            c->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;
        if (wmh->flags & InputHint)
            c->neverfocus = !wmh->input;
        else
            c->neverfocus = 0;
        XFree(wmh);
    }
}

void view(const Arg* arg) {
    int i;
    unsigned int tmptag;

    if ((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags])
        return;
    selmon->seltags ^= 1; /* toggle sel tagset */
    if (arg->ui & TAGMASK) {
        selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
        selmon->pertag->prevtag = selmon->pertag->curtag;

        if (arg->ui == ~0)
            selmon->pertag->curtag = 0;
        else {
            for (i = 0; !(arg->ui & 1 << i); i++)
                ;
            selmon->pertag->curtag = i + 1;
        }
    } else {
        tmptag = selmon->pertag->prevtag;
        selmon->pertag->prevtag = selmon->pertag->curtag;
        selmon->pertag->curtag = tmptag;
    }

    selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
    selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
    selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
    selmon->lt[selmon->sellt] =
        selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
    selmon->lt[selmon->sellt ^ 1] =
        selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt ^ 1];

    if (selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag])
        togglebar(NULL);

    focus(NULL);
    arrange(selmon);
}

Client* wintoclient(Window w) {
    Client* c;
    Monitor* m;

    for (m = mons; m; m = m->next)
        for (c = m->clients; c; c = c->next)
            if (c->win == w)
                return c;
    return NULL;
}

Monitor* wintomon(Window w) {
    int x, y;
    Client* c;
    Monitor* m;

    if (w == root && getrootptr(&x, &y))
        return recttomon(x, y, 1, 1);
    for (m = mons; m; m = m->next)
        if (w == m->barwin)
            return m;
    if ((c = wintoclient(w)))
        return c->mon;
    return selmon;
}

int wmclasscontains(Window win, const char* class, const char* name) {
    XClassHint ch = { NULL, NULL };
    int res = 1;

    if (XGetClassHint(dpy, win, &ch)) {
        if (ch.res_name && strstr(ch.res_name, name) == NULL)
            res = 0;
        if (ch.res_class && strstr(ch.res_class, class) == NULL)
            res = 0;
    } else
        res = 0;

    if (ch.res_class)
        XFree(ch.res_class);
    if (ch.res_name)
        XFree(ch.res_name);

    return res;
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */
int xerror(Display* dpy, XErrorEvent* ee) {
    if (ee->error_code == BadWindow ||
        (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch) ||
        (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable) ||
        (ee->request_code == X_PolyFillRectangle &&
            ee->error_code == BadDrawable) ||
        (ee->request_code == X_PolySegment && ee->error_code == BadDrawable) ||
        (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch) ||
        (ee->request_code == X_GrabButton && ee->error_code == BadAccess) ||
        (ee->request_code == X_GrabKey && ee->error_code == BadAccess) ||
        (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
        return 0;
    fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
        ee->request_code, ee->error_code);
    return xerrorxlib(dpy, ee); /* may call exit */
}

int xerrordummy(Display* dpy, XErrorEvent* ee) { return 0; }

/* Startup Error handler to check if another window manager
 * is already running. */
int xerrorstart(Display* dpy, XErrorEvent* ee) {
    die("dwm: another window manager is already running");
    return -1;
}

void zoom(const Arg* arg) {
    Client* c = selmon->sel;

    if (!selmon->lt[selmon->sellt]->arrange || !c || c->isfloating)
        return;
    if (c == nexttiled(selmon->clients) && !(c = nexttiled(c->next)))
        return;
    pop(c);
}

int main(int argc, char* argv[]) {
    if (argc == 2 && !strcmp("-v", argv[1]))
        die("dwm-" VERSION);
    else if (argc != 1)
        die("usage: dwm [-v]");
    if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
        fputs("warning: no locale support\n", stderr);
    if (!(dpy = XOpenDisplay(NULL)))
        die("dwm: cannot open display");
    checkotherwm();
    setup();
#ifdef __OpenBSD__
    if (pledge("stdio rpath proc exec", NULL) == -1)
        die("pledge");
#endif /* __OpenBSD__ */
    scan();
    runAutostart();
    run();
    cleanup();
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
}

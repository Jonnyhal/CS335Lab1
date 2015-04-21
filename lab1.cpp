//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "log.h"
extern "C" {
    #include "fonts.h"
}
#define WINDOW_WIDTH  900
#define WINDOW_HEIGHT 700

#define MAX_PARTICLES 100000
#define GRAVITY 0.1

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures
int yres=WINDOW_HEIGHT;
int xres=WINDOW_WIDTH;
struct Vec {
    float x, y, z;
};

struct Shape {
    float width, height;
    float radius;
    Vec center;
};

struct Particle {
    Shape s;
    Vec velocity;
};

struct Game {
    Shape bubble;
    Shape box[5];
    Particle *particle;
    int n;
    Game(){
        particle = new Particle[MAX_PARTICLES];
        n=0;
        //declare a box shape
        for(int i=0; i<5; i++) {
            box[i].width = 100;
            box[i].height = 20;
            box[i].center.x = 120 + i*77;
            box[i].center.y = 600 - i*75;
        }
        //delcare a bubble
        //bubble
        bubble.radius = 160;
        bubble.center.x = 650;
        bubble.center.y = 20;


    }
    ~Game() {
        delete [] particle;
    }
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(XEvent *e, Game *game);
void render(Game *game);
bool partState=false;

int main(void)
{
    int done=0;
    srand(time(NULL));
    initXWindows();
    init_opengl();
    //declare game object
    Game game;

    //start animation
    while(!done) {
        XEvent e;
	while(XPending(dpy)) {
            XNextEvent(dpy, &e);
            check_mouse(&e, &game);
            done = check_keys(&e, &game);

        }
        movement(&e, &game);
        render(&game);
        glXSwapBuffers(dpy, win);
    }
    cleanupXWindows();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void) {
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void) {
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        std::cout << "\n\tcannot connect to X server\n" << std::endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
        std::cout << "\n\tno appropriate visual found\n" << std::endl;
        exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPress | ButtonReleaseMask |
        PointerMotionMask |
        StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
            InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}
#define rnd() (float)rand() / (float)RAND_MAX

void makeParticle(Game *game, int x, int y) {
    if (game->n >= MAX_PARTICLES)
        return;
    //position of particle
    Particle *p = &game->particle[game->n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y = rnd() - 0.5;
    p->velocity.x = rnd() - 0.5;
    game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
    static int savex = 0;
    static int savey = 0;
    static int n = 0;

    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button!=1) {
            //Left button was pressed
            int y = WINDOW_HEIGHT - e->xbutton.y;
            for(int i=0; i<10; i++){
                makeParticle(game, e->xbutton.x, y);
            }
            return;
        }
        if (e->xbutton.button==3) {
            //Right button was pressed
            return;
        }
    }
    //Did the mouse move?
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
        savex = e->xbutton.x;
        savey = e->xbutton.y;
        if (++n < 10)
            return;
    }
    int y = WINDOW_HEIGHT - e->xbutton.y;
    for(int i=0; i<10; i++)
        makeParticle(game, e->xbutton.x, y);
    return;

}

int check_keys(XEvent *e, Game *game)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
        int key = XLookupKeysym(&e->xkey, 0);
        if (key == XK_Escape) {
            return 1;
        }
        //You may check other keys here.
	if (key== XK_b){
		if(partState==false)
			partState=true;
		else
			partState=false;
	}
	    
    			    
    }
    return 0;
}

void movement(XEvent *e, Game *game)
{
    Particle *p;
    static int savex = 0;
    static int savey = 0;
    int y = WINDOW_HEIGHT - e->xbutton.y;
    if (game->n <= 0 )
        return;
    if(partState==true) {
        for(int i=0; i<25; i++)
	    makeParticle(game, e->xbutton.x, y);
    	if (savex != e->xbutton.x || savey != e->xbutton.y) {
        	savex = e->xbutton.x;
        	savey = e->xbutton.y;
	}
    }
    for( int i=0; i<game->n; i++) {
        p = &game->particle[i];
        p->s.center.x += p->velocity.x;
        p->s.center.y += p->velocity.y;
        p->velocity.y -= GRAVITY;
        //check for collision with shapes...
        //Shape *s;
        for(int j=0; j<5; j++) {
            if(p->s.center.x >= game->box[j].center.x - game->box[j].width &&
                    p->s.center.x <= game->box[j].center.x + game->box[j].width &&
                    p->s.center.y < game->box[j].center.y + game->box[j].height &&
                    p->s.center.y > game->box[j].center.y - game->box[j].height){
                //collision with box
                p->s.center.y = game->box[j].center.y + game->box[j].height + 0.1;
                p->velocity.x += .025;
		        p->velocity.y *= -0.3;
            }
        }
        
        float d0, d1, dist;
        d0=p->s.center.x - game->bubble.center.x;
        d1=p->s.center.y - game->bubble.center.y;
        dist=sqrt(d0*d0 + d1*d1);
        if(dist <= game->bubble.radius){
            //move particle to the circle edge
	    p->s.center.x = game->bubble.center.x + (d0/dist) * game->bubble.radius * 1.01;
	    p->s.center.y = game->bubble.center.y + (d1/dist) * game->bubble.radius * 1.01;
	    
	    
	    //collision with bubble
            p->velocity.x += d0/dist * 2.0;
            p->velocity.y += d1/dist * 2.0;
        }
        
        //check for off-screen
        if (p->s.center.y < 0.0) {
            memcpy(&game->particle[i], &game->particle[game->n-1], sizeof(Particle));
            game->n--;
        }
    }

}
void render(Game *game)
{
    float w, h, r;
    glClear(GL_COLOR_BUFFER_BIT);
    //Draw shapes...
    Shape *b;
    b= &game->bubble;
    glPushMatrix();
    glTranslatef(b->center.x, b->center.y, b->center.z);
    r = b->radius;
    glBegin(GL_LINE_LOOP);
    for(int j=0; j<161; j++) {
        glColor3ub(0+j,0+j,190);
        for(int i=0; i<360; i++){
            const double heading = i*3.141592653/180;
            glVertex2i(cos(heading)*r, sin(heading)*r);
        
        }
        r-=1;
    }
    glEnd();
    glPopMatrix();
   
    /* 
    const int n=40;
    static int firsttime= 1;
    static Vec vert[n];
    if(firsttime){
        float ang = 0.0, inc = (3.14159 * 2.0) / (float)n;
        for(int i=0; i<n; i++) {
            vert[i].x=cos(ang)*game->bubble.radius;
            vert[i].y=sin(ang)*game->bubble.radius;
            ang += inc;
        }
        firsttime=0;
    }
    glColor3ub(255, 255, 255);
    glBegin(GL_LINE_LOOP);
        for(int i=0; i<n; i++){
            glVertex2i(game->bubble.center.x + vert[i].x, 
                    game->bubble.center.y + vert[i].y);
        }
    glEnd();
    */
    //draw box
    Shape *s;
    for(int i=0; i<5; i++) {
        glColor3ub(25*i, 25*i, 255);
        s = &game->box[i];
        glPushMatrix();
        glTranslatef(s->center.x, s->center.y, s->center.z);
        w = s->width;
        h = s->height;
        glBegin(GL_QUADS);
        glVertex2i(-w,-h);
        glVertex2i(-w, h);
        glVertex2i( w, h);
        glVertex2i( w,-h);
        glEnd();
        glPopMatrix();
    }
    //draw all particles here
    glPushMatrix();
    for(int i=0; i<game->n; i++) {
        int colorR=0;
        int colorG=0;
        int colorB=220-i*10;
        glColor3ub(colorR, colorG, colorB);
        Vec *c = &game->particle[i].s.center;
        w = 2;
        h = 2;
        glBegin(GL_QUADS);
        glVertex2i(c->x-w, c->y-h);
        glVertex2i(c->x-w, c->y+h);
        glVertex2i(c->x+w, c->y+h);
        glVertex2i(c->x+w, c->y-h);
        glEnd();
    }
    glPopMatrix();
    
    Rect t;
    unsigned int cref = 0x00ffffff;
	t.bot= yres - 20;
	t.left = 10;
	t.center=0;
	ggprint8b(&t, 16, cref, "Waterfall Model");
    for (int i = 0; i<5; i++) {
        if(i==0) {
            t.bot=game->box[0].center.y - 12;
            t.left = game->box[0].center.x-60;
            t.center=0;
            ggprint16(&t, 16, cref, "Requirements");
        }
        if(i==1) {
            t.bot=game->box[1].center.y - 12;
            t.left = game->box[1].center.x-30;
            t.center=0;
            ggprint16(&t, 16, cref, "Design");
        }
        if(i==2) {
            t.bot=game->box[2].center.y - 12;
            t.left = game->box[2].center.x-55;
            t.center=0;
            ggprint16(&t, 16, cref, "Implementation");
        }
        if(i==3) {
            t.bot=game->box[3].center.y - 12;
            t.left = game->box[3].center.x-50;
            t.center=0;
            ggprint16(&t, 16, cref, "Verification");
        }
        if(i==4) {
            t.bot=game->box[4].center.y - 12;
            t.left = game->box[4].center.x-50;
            t.center=0;
            ggprint16(&t, 16, cref, "Maintenance");
        }
    } 
}




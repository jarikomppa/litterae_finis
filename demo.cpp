#include <windows.h>
#include <stdio.h>
#include "textfx.h"
#include "conio.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <gl/tfxswgl.h>
#include "stb_image.h"

#include "vertexbuffer.h"
#include "sphere.h"

#define SMOOTHSTEP(x) ((x)*(x)*(3-2*(x)))
//#define STARTTICK 135000 //150000

#include "fmod.h"

#pragma STFU(4305) // double-float
#pragma STFU(4018) // signed/unsigned mismatch

FSOUND_STREAM *stream;

#define SOUND

GLuint texture[3];
int * fb;
TFX_TCConverter * tc = NULL;
int chartex[12*8*128];
int *boxframes;

void initwell(int v);
unsigned long well512(void);
void draw_logo(float aTime);
void draw_attractor(float aTime, float tick, float endTime);
void draw_attractor1(float aTime, float tick);
void draw_attractor2(float aTime, float tick);
void draw_attractor3(float aTime, float tick);
void draw_attractor4(float aTime, float tick);
void draw_starfield(float aTick);
void draw_star(float intensity, int tick);
void tile_reveal(int *fb, int tick);
void writer(char *aText, float aTime);
void box(float t);
void draw_zoomer(float t, int *fb);
void chartomap(int chars, int *blurred);
void draw_tube(float t, int tick);
void rotate_tube(float t, int tick);
void draw_greets(float t, int *fb);
void draw_starfield2(float t, float aTick);
void draw_string(int x, int y, char *s, int *fb);

VertexBuffer *gSphere;


// Write formatted string to log
void wlogf(const char * aFmt, ...)
{
	va_list ap;
	va_start(ap, aFmt);
	FILE *f = fopen("log.txt", "a");
	if (f)
	{
		vfprintf(f, aFmt, ap);
		fclose(f);
	}
	va_end(ap);
}


int LoadGLTextures( )
{
    int i, j;
	glGenTextures(3, &texture[0]);

    // Load texture using stb
	int x, y, n;
	unsigned char *data = stbi_load("data/traumalogo.png", &x, &y, &n, 4);
    
    if (data == NULL)
        return 0;

    int l, w, h;
    w = x;
    h = y;
    l = 0;
    unsigned int * mip = new unsigned int[w * h * 5];
    unsigned int * src = (unsigned int*)data;

    memset(mip, 0, w * h * 4);

    // mark all pixels with alpha = 0 to black
    for (i = 0; i < h; i++)
    {
        for (j = 0; j < w; j++)
        {
            if ((src[i * w + j] & 0xff000000) == 0)
                src[i * w + j] = 0;
        }
    }
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // NULL should be ok, just allocate
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // NULL should be ok, just allocate

	glBindTexture(GL_TEXTURE_2D, texture[0]);

    // Tell OpenGL to read the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)src);

    if (mip)
    {
        // precalculate summed area tables
        // it's a box filter, which isn't very good, but at least it's fast =)
        int ra = 0, ga = 0, ba = 0, aa = 0;
        int i, j, c;
        unsigned int * rbuf = mip + (w * h * 1);
        unsigned int * gbuf = mip + (w * h * 2);
        unsigned int * bbuf = mip + (w * h * 3);
        unsigned int * abuf = mip + (w * h * 4);
        
        for (j = 0, c = 0; j < h; j++)
        {
            ra = ga = ba = aa = 0;
            for (i = 0; i < w; i++, c++)
            {
                ra += (src[c] >>  0) & 0xff;
                ga += (src[c] >>  8) & 0xff;
                ba += (src[c] >> 16) & 0xff;
                aa += (src[c] >> 24) & 0xff;
                if (j == 0)
                {
                    rbuf[c] = ra;
                    gbuf[c] = ga;
                    bbuf[c] = ba;
                    abuf[c] = aa;
                }
                else
                {
                    rbuf[c] = ra + rbuf[c - w];
                    gbuf[c] = ga + gbuf[c - w];
                    bbuf[c] = ba + bbuf[c - w];
                    abuf[c] = aa + abuf[c - w];
                }
            }
        }

        while (w > 1 || h > 1)
        {
            l++;
            w /= 2;
            h /= 2;
            if (w == 0) w = 1;
            if (h == 0) h = 1;

            int dw = x / w;
            int dh = y / h;

            for (j = 0, c = 0; j < h; j++)
            {
                for (i = 0; i < w; i++, c++)
                {
                    int x1 = i * dw;
                    int y1 = j * dh;
                    int x2 = x1 + dw - 1;
                    int y2 = y1 + dh - 1;
                    int div = (x2 - x1) * (y2 - y1);
                    y1 *= x;
                    y2 *= x;
                    int r = rbuf[y2 + x2] - rbuf[y1 + x2] - rbuf[y2 + x1] + rbuf[y1 + x1];
                    int g = gbuf[y2 + x2] - gbuf[y1 + x2] - gbuf[y2 + x1] + gbuf[y1 + x1];
                    int b = bbuf[y2 + x2] - bbuf[y1 + x2] - bbuf[y2 + x1] + bbuf[y1 + x1];
                    int a = abuf[y2 + x2] - abuf[y1 + x2] - abuf[y2 + x1] + abuf[y1 + x1];

                    r /= div;
                    g /= div;
                    b /= div;
                    a /= div;

                    if (a == 0)
                        mip[c] = 0;
                    else
                        mip[c] = ((r & 0xff) <<  0) | 
                                 ((g & 0xff) <<  8) | 
                                 ((b & 0xff) << 16) | 
                                 ((a & 0xff) << 24); 
                }
            }
            glTexImage2D(GL_TEXTURE_2D, l, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)mip);
        }
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); // Linear Filtering
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
        delete[] mip;
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
    }

    // and cleanup.
	stbi_image_free(data);
/*
    if (clamp)
    {
        // Set up texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
    else
    {
        // Set up texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
	*/
	return 1;
}

/* function to reset our viewport after a window resize */
void reshape( int width, int height )
{
    /* Height / width ration */
    float ratio;
 
    /* Protect against a divide by zero */
    if ( height == 0 )
	height = 1;

    ratio = ((float)(640+0*width)/ (float)(600+0*height));

    /* Setup our viewport. */
    glViewport( 0, 0, ( GLint )width, ( GLint )height );

    /*
     * change to the projection matrix and set
     * our viewing volume.
     */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    /* Set our perspective */
    gluPerspective( (float)(45), ratio, (0.1f), (float)(100) );

    /* Make sure we're chaning the model view and not the projection */
    glMatrixMode( GL_MODELVIEW );

    /* Reset The View */
    glLoadIdentity( );
}


/* general OpenGL initialization function */
void init( void )
{

    /* Load in the texture */
    if ( !LoadGLTextures( ) )
	    puts("Textures no loading!!!!!");

    /* Enable Texture Mapping ( NEW ) */
    glEnable( GL_TEXTURE_2D );

    /* Enable smooth shading */
    glShadeModel( GL_SMOOTH );

    /* Set the background black */
    glClearColor( (float)(0), (float)(0), (float)(0), (0.5) );

    /* Depth buffer setup */
    glClearDepth( (float)(1) );

    /* Enables Depth Testing */
    glEnable( GL_DEPTH_TEST );

    /* The Type Of Depth Test To Do */
//    glDepthFunc( GL_LEQUAL );

    /* Really Nice Perspective Calculations */
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

}


int events[] =
{
0,   // intro
7700,
15400,
23000,
30600,
46000,
60000 + 1400, // zoomer
60000 + 32100, // greets
120000 + 3000, // ball
120000 + 9800,
120000 + 26100,
120000 + 56700,
180000 + 12100,
180000 + 23500,
0
};

void ticskulator(int tick_in, int &e, float &t)
{
	int i = 0;
	while (tick_in > events[i] && events[i] != -1) i++;
	e = i;
	t = 0;
	if (events[i] != -1)
		t = (float)(tick_in - events[i-1]) / (events[i]-events[i-1]);
	
}

		// plim 240 * n
		// 1    7700
		// 2   15400
		// 3   23000
		// 4   30600
		// 5   46000 - beat
		// 6   61400 - melody 1
		// 7   92100 - melody 2
		// 8  123000 - transition
		// 9  129800 - chill
		// 10 146100 - tense 
		// 11 176700 - rise (30600)
		// 12 192100 - release
		// 13 203500 - end




void fade_to_white(int * fb, float f)
{
	if (f < 0) return;
	if (f > 1) f = 1;
	//f = SMOOTHSTEP(f);
	int i, j, c;
	for (i = 0, c = 0; i < 100; i++)
		for (j = 0; j < 160; j++, c++)
	{
		int inc = (fb[c] >> 8) & 0xff;
		int d = 0xff+(95-sqrt((float)((i-50)*(i-50)+(j-80)*(j-80))));
		int v = inc * (1-f) + d * f;
		if (v > 0xff) v = 0xff;

		fb[c] = v << 8;
	}
}

void fade_from_white(int * fb, float f)
{
	if (f < 0) f = 0;
	if (f > 1) return;
	f = SMOOTHSTEP(f);
	int i, j, c;
	for (i = 0, c = 0; i < 100; i++)
		for (j = 0; j < 160; j++, c++)
	{
		int inc = (fb[c] >> 8) & 0xff;
		int d = 0xff+(sqrt((float)((i-50)*(i-50)+(j-80)*(j-80))));
		int v = inc * f + d * (1 - f);
		if (v > 0xff) v = 0xff;

		fb[c] = v << 8;
	}
}

void fade_from_white_linear(int * fb, float f)
{
	if (f < 0) f = 0;
	if (f > 1) return;
	f = SMOOTHSTEP(f);
	f = SMOOTHSTEP(f);
	int i, j, c;
	for (i = 0, c = 0; i < 100; i++)
		for (j = 0; j < 160; j++, c++)
	{
		int inc = (fb[c] >> 8) & 0xff;
		int d = 0xff;
		int v = inc * f + d * (1 - f);
		if (v > 0xff) v = 0xff;

		fb[c] = v << 8;
	}
}

void fade_to_black(int * fb, float f)
{
	if (f < 0) return;
	if (f > 1) f = 1;
	f = SMOOTHSTEP(f);
	int i, j, c;
	for (i = 0, c = 0; i < 100; i++)
		for (j = 0; j < 160; j++, c++)
	{
		int inc = (fb[c] >> 8) & 0xff;
		int d = -(95-sqrt((float)((i-50)*(i-50)+(j-80)*(j-80))));
		int v = inc * (1-f) + d * f;
		if (v < 0) v = 0;

		fb[c] = v << 8;
	}
}

void fade_from_black(int * fb, float f)
{
	if (f < 0) f = 0;
	if (f > 1) return;
	f = SMOOTHSTEP(f);
	int i;
	for (i = 0; i < 160*100; i++)
	{
		fb[i] = (int)(((fb[i] >> 8) & 0xff) * f) << 8;
	}
}


int blend(int a, int b, float v)
{
	return (int)((((a >> 8)&0xff)*v) +
		         (((b >> 8)&0xff)*(1-v))) *0x010101;
}

void ball_translate(float t, int tick, int e)
{
	//tick = 150000;
	float q = 0;
	if (e == 9)
	{
		q = (1 - t);
		q = SMOOTHSTEP(q);
		q *= 2.6;
	}

	float k = 1;
	if (e == 11)
		k = (1-t);
	if (e > 11)
		k = 0;
	k = SMOOTHSTEP(k);

	glTranslatef( k*(float)(sin(tick * 0.00018)*0.5), 
		          k*(float)(sin(tick * 0.00019)*0.5)+q, 
				  k*-3.5+(sin(tick * 0.00012)+sin(tick * 0.00016)+sin(tick * 0.00022)+sin(tick * 0.00026)+q)*0.5);
	k = 1-k;
	glTranslatef( k*(float)(sin(tick * 0.00018)*0.05), 
		          k*(float)(sin(tick * 0.00019)*0.05), 
				  k*-20);

}

void ball_rotate(float t, int tick, int e)
{
//	tick = 150000;

	float k = 1;
	if (e == 11)
		k = 1-t;
	if (e > 11)
		k = 0;
	k = SMOOTHSTEP(k);

#define ROTATION 0.2,1,0.7
#define ROTATION2 0.5,0.3,1
#define ROTM -0.9
	if (e < 11)
	{
		glRotatef(ROTM*sin(tick * 0.0001)*360,ROTATION);
		glRotatef(ROTM*sin(tick * 0.00007)*360,ROTATION2);
	}
	else
	if (e == 11)
	{
		glRotatef(ROTM*sin((120000 + 26100 + pow(t,0.7f) * 4000) * 0.0001)*360,ROTATION);
		glRotatef(ROTM*sin((120000 + 26100 + pow(t,0.7f) * 4000) * 0.00007)*360,ROTATION2);
	}
	else
	{
		glRotatef(ROTM*sin((120000 + 26100 + 4000 + t*1000) * 0.0001)*360,ROTATION);
		glRotatef(ROTM*sin((120000 + 26100 + 4000 + t*1000) * 0.00007)*360,ROTATION2);
	}
}

void crap1(int * ptr, int w, int h, int tick)
{
	int i, j, c;

	for (i = 0, c = 0; i < h; i++)
		for (j = 0; j < w; j++, c++)
		{
			int x = j-w/2;
			int y = i-w/2;
			ptr[c] = (y*x + tick*10) << 4;
		}
}

void crap2(int * ptr, int w, int h, int tick)
{
	int i, j, c;

	for (i = 0, c = 0; i < h; i++)
		for (j = 0; j < w; j++, c++)
		{
			ptr[c] = fb[j*160/256+(i*100/256)*160];
		}
}

void draw_cube(float t, int tick, int e)
{
	if (tick < 127500) return;
	glEnable(GL_TEXTURE_2D);
	crap1((int*)glGetTexdataPtr(GL_TEXTURE_2D, texture[1], 0), 256,256,tick);
	crap2((int*)glGetTexdataPtr(GL_TEXTURE_2D, texture[2], 0), 256,256,tick);
	
	float k = 1;
	if (e == 11) k = 1-t;
	if (e > 11) return;

	k = k*k*k;

	int i;
	int j;
	for (j = 0; j < 8; j++)
	{
	glLoadIdentity();
	ball_translate(t,tick,e);
	ball_rotate(t,tick,e);
	
	for (i = 0; i < 3; i++)
	{
	
	glRotatef((tick) * (0.06 + j * 0.01)* 0.5, 0.1, 0.7, 0.3);
	switch(j)
	{
	case 0:
		glTranslatef(1.5*k,0,0);
		break;
	case 1:
		glTranslatef(0,0,6*k);
		break;
	case 2:
		glTranslatef(-12*k,0,0);
		break;
	case 3:
		glTranslatef(0,0,-24*k);
		break;
	case 4:
		glTranslatef(0,0,36*k);
		break;
	case 5:
		glTranslatef(0,0,-48*k);
		break;
	case 6:
		glTranslatef(0,0,64*k);
		break;
	case 7:
		glTranslatef(0,0,-78*k);
		break;
	}
	glScalef(0.4+j*0.05,
		     0.4+j*0.05,
		     0.4+j*0.05);
	glRotatef(tick * -0.05+70, 1, 0.7, 0.4);
	glBegin(GL_QUADS);
    glBindTexture( GL_TEXTURE_2D, texture[1] );
	  /* Front Face */
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( (float)(0), (float)(1) ); glVertex3f( (float)(-1),(float)(-1), (float)(1) );
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( (float)(1), (float)(1) ); glVertex3f( (float)(1), (float)(-1), (float)(1) );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( (float)(1), (float)(0) ); glVertex3f( (float)(1), (float)(1), (float)(1) );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( (float)(0), (float)(0) ); glVertex3f( (float)(-1), (float)(1), (float)(1) );

      /* Back Face */
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( (float)(0), (float)(0) ); glVertex3f( (float)(-1), (float)(-1), (float)(-1) );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( (float)(0), (float)(1) ); glVertex3f( (float)(-1), (float)(1), (float)(-1) );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( (float)(1), (float)(1) ); glVertex3f( (float)(1), (float)(1), (float)(-1) );
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( (float)(1), (float)(0) ); glVertex3f( (float)(1), (float)(-1), (float)(-1) );
    glBindTexture( GL_TEXTURE_2D, texture[2] );

      /* Top Face */
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( (float)(1), (float)(1) ); glVertex3f( (float)(-1),  (float)(1), (float)(-1) );
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( (float)(1), (float)(0) ); glVertex3f( (float)(-1),  (float)(1),  (float)(1) );
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( (float)(0), (float)(0) ); glVertex3f(  (float)(1),  (float)(1),  (float)(1) );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( (float)(0), (float)(1) ); glVertex3f(  (float)(1),  (float)(1), (float)(-1) );

      /* Bottom Face */
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( (float)(0), (float)(1) ); glVertex3f( (float)(-1), (float)(-1), (float)(-1) );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( (float)(1), (float)(1) ); glVertex3f(  (float)(1), (float)(-1), (float)(-1) );
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( (float)(1), (float)(0) ); glVertex3f(  (float)(1), (float)(-1),  (float)(1) );
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( (float)(0), (float)(0) ); glVertex3f( (float)(-1), (float)(-1),  (float)(1) );

      /* Right face */
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( (float)(0), (float)(0) ); glVertex3f( (float)(1), (float)(-1), (float)(-1) );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( (float)(0), (float)(1) ); glVertex3f( (float)(1),  (float)(1), (float)(-1) );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( (float)(1), (float)(1) ); glVertex3f( (float)(1),  (float)(1),  (float)(1) );
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( (float)(1), (float)(0) ); glVertex3f( (float)(1), (float)(-1),  (float)(1) );

      /* Left Face */
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( (float)(1), (float)(0) ); glVertex3f( (float)(-1), (float)(-1), (float)(-1) );
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( (float)(0), (float)(0) ); glVertex3f( (float)(-1), (float)(-1),  (float)(1) );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( (float)(0), (float)(1) ); glVertex3f( (float)(-1),  (float)(1),  (float)(1) );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( (float)(1), (float)(1) ); glVertex3f( (float)(-1),  (float)(1), (float)(-1) );
    glEnd( );
	}
	}
	glDisable(GL_TEXTURE_2D);
}


void draw_ball(float t, int tick, int e)
{
	int i;
	glLoadIdentity( );
	glClear(GL_DEPTH_BITS);
	glDisable(GL_TEXTURE_2D);

	float q = 1;
	float s = 0.8;
	if (e == 9)
	{
		q = 0;
		s = 1.2;
	}
	if (e == 10)
	{
		q = t;
		s = 0.8+(1-t)*0.4;
	}
	if (e == 11)
		q = 1+t*t*14;

	if (e == 12)
		q = 15+t*45;

	if (e == 11)
	{
		float ht = t * 16 - 7;
		if (ht < 0) ht = 0;
		if (ht > 1) ht = 1;
		ht = sqrt(ht);
		ht = SMOOTHSTEP(ht);
		ht = SMOOTHSTEP(ht);
		q += sin(ht * 3.1415926535897932384626433832795)*15;
	}

	for (i = 0; i < gSphere->mVertices; i++)
	{
		gSphere->mVertex[i*3+0] = gSphere->mNormal[i*3+0] * (sin((gSphere->mNormal[i*3+0]+tick*0.0001)*10)*0.2*q+s);
		gSphere->mVertex[i*3+1] = gSphere->mNormal[i*3+1] * (sin((gSphere->mNormal[i*3+1]+tick*0.0001)*15)*0.2*q+s);
		gSphere->mVertex[i*3+2] = gSphere->mNormal[i*3+2] * (sin((gSphere->mNormal[i*3+2]+tick*0.0001)*20)*0.2*q+s);
	}

	
	ball_translate(t,tick,e);
	ball_rotate(t,tick,e);

	glColor3f(1,1,1);
	glBegin(GL_TRIANGLES);
	for (i = 0; i < gSphere->mIndices/3; i++)
	{
		float c = (i % 32)/32.0;
		glColor3f(c,c,c);
		glVertex3f(gSphere->mVertex[gSphere->mIndex[i*3+0]*3+0], gSphere->mVertex[gSphere->mIndex[i*3+0]*3+1], gSphere->mVertex[gSphere->mIndex[i*3+0]*3+2]);
		glVertex3f(gSphere->mVertex[gSphere->mIndex[i*3+1]*3+0], gSphere->mVertex[gSphere->mIndex[i*3+1]*3+1], gSphere->mVertex[gSphere->mIndex[i*3+1]*3+2]);
		glVertex3f(gSphere->mVertex[gSphere->mIndex[i*3+2]*3+0], gSphere->mVertex[gSphere->mIndex[i*3+2]*3+1], gSphere->mVertex[gSphere->mIndex[i*3+2]*3+2]);
	}
	glEnd();
}

void draw_end(int *fb)
{
	initwell(7);
	int i, j;
	for (i = 0; i < 100; i++)
		for (j = 0; j < 160; j++)
			fb[i*160+j] = (((j+i/2)*2)/3) << 8;
	                 //1234567890
	draw_string(0, 0, "Litterae  "
		              "Finis     ", fb);
	draw_string(0,54, "    Trauma"
					  "      2012", fb);

}

void draw_suckfield(float aTick, float t)
{
	initwell(0x7aa7);

	int i;

	glDisable(GL_TEXTURE_2D);
	glColor4f(1,1,1,1);
//    glLoadIdentity();
//	glTranslatef( (float)(0), (float)(0), -4);


	glBegin(GL_POINTS);
	for (i = 0; i < 256*t; i++)
	{
		float x = ((well512() & 0xffff) / (float)0x10000)-0.5; 		
		float y = ((well512() & 0xffff) / (float)0x10000)-0.5; 
		float z = ((well512() & 0xffff) / (float)0x10000)-0.5; 
		float d = (well512() & 0xffff) / (float)0x10000;			
		d += (aTick * (aTick * 0.000000001));
		d = 1-fmod(d,1);
		d *= 20;
		
		glVertex3f(x*d,y*d,z*d);
	}
	glEnd();
}

void draw_suckfield2(float aTick, float t)
{
#define S1 (139200-800)
#define S2 (154700-800)
#define S3 (170200-800)
#define SLEN 4000
	initwell(0x7aa7);
	float p = 0;
	float s = 1;
	float k = 1;
	if (aTick < S1 || aTick > S3+SLEN) return;

	if (aTick > S1 && aTick < S1+SLEN)
	{
		p = (aTick - S1) / (float)SLEN;
		k = 2;
	}
	if (aTick > S2 && aTick < S2+SLEN)
	{
		p = (aTick - S2) / (float)SLEN;
		s++;
	}
	if (aTick > S3 && aTick < S3+SLEN)
	{
		p = (aTick - S3) / (float)SLEN;
		s+=4;
	}
	if (p == 0)
		return;
/*
	139200
	154700
	170200
	173200
	*/

	//p = sin(p*3.14);
	//p = p*p;
	p = SMOOTHSTEP(p);
	p = sqrt(p);

	int i;

	glDisable(GL_TEXTURE_2D);
	glColor4f(1,1,1,1);
//    glLoadIdentity();
//	glTranslatef( (float)(0), (float)(0), -4);

	glBegin(GL_POINTS);
	for (i = 0; i < 4096*(1-p); i++)
	{
		float r = ((well512() & 0xffff) / (float)0x10000); 		
		float y = ((well512() & 0xffff) / (float)0x10000)-0.5; 
		float z = ((well512() & 0xffff) / (float)0x10000)-0.5; 
		
		r=r*r*r*r*r;
		r = 1 - r;

		float l = sqrt(y*y+z*z);
		if (l)
		{
			//x = (x/l)*(p*s)*0;
			y = (y/l)*(p*s)*(2.5)*r*k;
			z = (z/l)*(p*s)*(2.5)*r*k;
		}

		glVertex3f(0,y,z);
	}
	glEnd();
}

void draw_suckgeom(float aTick, float t)
{
	initwell(0x7aa7);

	int i;

	glDisable(GL_TEXTURE_2D);
	glColor4f(1,1,1,1);
//    glLoadIdentity();
//	glTranslatef( (float)(0), (float)(0), -4);

	glBegin(GL_TRIANGLES);
	for (i = 0; i < 256; i++)
	{
		float d = (1-t) * 32;
		d -= i/8.0;
		if (d < 0) d = 0;		
		d *= 10;
		

		float c = (well512() % 100)/100.0f;
		glColor3f(c,c,c);
		float x = ((well512() & 0xffff) / (float)0x10000)-0.5; 		
		float y = ((well512() & 0xffff) / (float)0x10000)-0.5; 
		float z = ((well512() & 0xffff) / (float)0x10000)-0.5; 
		float l = sqrt(x*x+y*y+z*z);
		if (l != 0)
		{
			x /= l;
			y /= l;
			z /= l;
		}

		float x0 = ((well512() & 0xffff) / (float)0x10000)-0.5; 		
		float y0 = ((well512() & 0xffff) / (float)0x10000)-0.5; 
		float z0 = ((well512() & 0xffff) / (float)0x10000)-0.5; 
		float l0 = sqrt(x0*x0+y0*y0+z0*z0);

		if (l0 != 0)
		{
			x0 /= l;
			y0 /= l;
			z0 /= l;
		}

		x0 *= 0.1;
		y0 *= 0.1;
		z0 *= 0.1;

		
		glVertex3f((0+x)*d,(0+y)*d,(0+z)*d);
		glVertex3f((x0+x)*d,(y0+y)*d,(z0+z)*d);
		glVertex3f((0+x)*d,(-y0+y)*d,(z0+z)*d);
	}
	glEnd();
}


void demomain()
{
//	getch();
#ifdef SOUND
    FSOUND_Init(44100, 1, 0);
	stream = FSOUND_Stream_Open("data/!cube - starchild.mp3",FSOUND_MPEGACCURATE,0,0);
    FSOUND_Stream_SetMode(stream,FSOUND_LOOP_OFF);
#endif
	TFX_AsciiArt caa;

    TFX_SetTitle("Wait, precalculating..");

	boxframes = new int[160*100*180];
    
	int i;
	for (i = 0; i < 128; i++)
		chartomap(i, chartex + i * 12*8);

	caa.BuildLUT();

    tc = &caa;

	fb = new int[320*200];

	tfx_swgl_Context *ctx = tfx_swgl_CreateContext();
	tfx_swgl_MakeCurrent(fb,ctx);

	reshape(160,100);

	gSphere = generate_sphere(1, 6);

	init();

	for (i = 0; i < 180; i++)
	{
		box(i/180.0f);
		memcpy(boxframes + 160*100*i,fb,160*100*4);
	}


    char temp[256];
    sprintf(temp,"Litterae Finis - Trauma");
    TFX_SetTitle(temp);

#ifdef SOUND
	FSOUND_Stream_Play(0, stream);
	FSOUND_SetVolume(FSOUND_ALL, 200);
#define TICK (FSOUND_Stream_GetTime(stream))
#ifdef STARTTICK
	FSOUND_Stream_SetTime(stream, STARTTICK);
#endif

#else
#ifndef STARTTICK
#define STARTTICK 0
#endif
#define TICK (GetTickCount()-starttick + STARTTICK)

#endif

    unsigned int starttick = 0;
    unsigned int tick;
    starttick = TICK;
    tick = 0;

//	wlogf("-----\n");
                
    while(!_kbhit() && tick < 203500)
    {
        while (tick>=TICK) {Sleep(1);};
        int totick = TICK;
        
        while (tick < totick)
        {
            tick+=(1000/60);
        }

		int e;
		float t;
		ticskulator(tick,e,t);

		//sprintf(temp,"%d  %d  %d   %3.3f", tick, tick / 240, e, t);
		//TFX_SetTitle(temp);
		
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		switch (e)
		{
		case 1:
			draw_attractor1(t, tick);
			draw_starfield(tick);
			break;
		case 2:
			draw_attractor1(t+1, tick);
			draw_starfield(tick);
			break;
		case 3:
			draw_attractor2(t, tick);
			draw_starfield(tick);
			writer(" Code: Sol",t);
			break;
		case 4:
			draw_attractor3(t, tick);
			draw_star(t,tick);
			draw_starfield(tick);
			writer("Music: !Cube",t);
			break;
		case 5:
			draw_attractor4(t, tick);
			draw_star(t+1,tick);
			draw_starfield(tick);
			break;
		case 6:
			draw_attractor4(t+1, tick);
			draw_star(t+2,tick);
			draw_logo(t);
			draw_starfield(tick);
			break;
		case 7:
			draw_zoomer(t, fb);
			break;
		case 8:
		    glLoadIdentity();
			//glRotatef(tick * 0.0001 * 60, 0.01,0.007,1);
			rotate_tube(t, tick);
			draw_attractor(t*10, tick,1);
			draw_tube(t, tick);
			draw_starfield2(t, tick);
			break;
		case 9:
		case 10:
		case 11:
		case 12:
		    glLoadIdentity();
			ball_translate(t,tick,e);
			ball_rotate(t, tick, e);

			if (e == 11)
			{
				draw_suckfield(tick, ((1-t)*2));
			}

			if (e == 11)
				draw_suckgeom(tick, t);

			if (e < 11)
			draw_suckfield(tick, t+e-9);

		    glLoadIdentity();
			if (e == 12)
				glTranslatef(0,0,-20*t*t);
			rotate_tube(1, 120000 + 3000);
			ball_rotate(t, tick, e);

			if (e == 12)
			{
				draw_attractor(1, tick, (1-t*t*t)*10);
			}
			else
			if (e < 12)
			{
				draw_attractor(1, tick, 10);
			}

			draw_ball(t, tick, e);
			draw_suckfield2(tick, t);
			draw_cube(t, tick, e);
			
		
			break;
		}


		switch (e)
		{
		case 1:
		    tfx_swgl_SwapBuffers();
			tile_reveal(fb, tick);
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			tfx_swgl_SwapBuffers();
			break;
		case 6:
		    tfx_swgl_SwapBuffers();
			fade_to_white(fb, t*6-5);
			break;
		case 7:
			fade_from_white(fb, t*20);

			fade_to_black(fb, t*30-29);
			break;
		case 8:
			tfx_swgl_SwapBuffers();
			draw_greets(t, fb);
			//fade_from_black(fb, t*50);
			break;
		case 9:
		case 10:
		case 11:
			tfx_swgl_SwapBuffers();
			break;
		case 12:
			tfx_swgl_SwapBuffers();
			fade_to_white(fb, t*7-6);
			break;
		case 13:
			draw_end(fb);
			fade_from_white_linear(fb, t*2);
			break;

		}

        tc->Dump2x(fb,TFXQuad(0,0,160,100),160,0,0);

        TFX_Present();

	}
}
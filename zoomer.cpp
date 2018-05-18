#include <windows.h>
#include <stdio.h>
#include "textfx.h"
#include "conio.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <gl/tfxswgl.h>
#include "stb_image.h"

#define SMOOTHSTEP(x) ((x)*(x)*(3-2*(x)))
#pragma STFU(4305) // double-float
#pragma STFU(4018) // signed/unsigned mismatch

extern int * fb;
extern TFX_TCConverter * tc;
extern int chartex[12*8*128];
extern int *boxframes;
extern GLuint texture[3];

int blend(int a, int b, float v);

void box(float t)
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity( );
	glDisable(GL_TEXTURE_2D);
    glTranslatef( (float)(0), (float)(0), -4);
	
    glRotatef( t*180, (float)(1), (float)(0), (float)(0)); 

	float x = 1.7;
	float y = 0.1;
	float z = x*(1/2.5);

	glBegin(GL_QUADS);

    glColor3f(1,1,1);			glVertex3f( (float)(-x),(float)(-y), (float)(z) );
	glColor3f(1,1,1);		    glVertex3f( (float)(x), (float)(-y), (float)(z) );
	glColor3f(0.5,0.5,0.5);     glVertex3f( (float)(x), (float)(y), (float)(z) );
	glColor3f(0.5,0.5,0.5);     glVertex3f( (float)(-x), (float)(y), (float)(z) );

	glColor3f(1,1,1);			glVertex3f( (float)(-x), (float)(-y), (float)(-z) );
    glColor3f(1,1,1);		    glVertex3f( (float)(-x), (float)(y), (float)(-z) );
    glColor3f(0.5,0.5,0.5);     glVertex3f( (float)(x), (float)(y), (float)(-z) );
    glColor3f(0.5,0.5,0.5);     glVertex3f( (float)(x), (float)(-y), (float)(-z) );


    glColor3f(1,1,1);			glVertex3f( (float)(-x),  (float)(y), (float)(-z) );
    glColor3f(1,1,1);		    glVertex3f( (float)(-x),  (float)(y),  (float)(z) );
    glColor3f(0.5,0.5,0.5);     glVertex3f(  (float)(x),  (float)(y),  (float)(z) );
    glColor3f(0.5,0.5,0.5);     glVertex3f(  (float)(x),  (float)(y), (float)(-z) );

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glColor3f(1,1,1);
glTexCoord2f( (float)(0), (float)(1) ); glVertex3f( (float)(-x), (float)(-y), (float)(-z) );
glTexCoord2f( (float)(1), (float)(1) );	glVertex3f(  (float)(x), (float)(-y), (float)(-z) );
glTexCoord2f( (float)(1), (float)(0) );	glVertex3f(  (float)(x), (float)(-y),  (float)(z) );
glTexCoord2f( (float)(0), (float)(0) );	glVertex3f( (float)(-x), (float)(-y),  (float)(z) );
glDisable(GL_TEXTURE_2D);

/*
    glColor3f(1,1,1);			glVertex3f( (float)(-x), (float)(-y), (float)(-z) );
    glColor3f(1,1,1);		    glVertex3f(  (float)(x), (float)(-y), (float)(-z) );
    glColor3f(0.5,0.5,0.5);     glVertex3f(  (float)(x), (float)(-y),  (float)(z) );
    glColor3f(0.5,0.5,0.5);     glVertex3f( (float)(-x), (float)(-y),  (float)(z) );
	*/
    glColor3f(1,1,1);			glVertex3f( (float)(x), (float)(-y), (float)(-z) );
    glColor3f(1,1,1);		    glVertex3f( (float)(x),  (float)(y), (float)(-z) );
    glColor3f(0.5,0.5,0.5);     glVertex3f( (float)(x),  (float)(y),  (float)(z) );
    glColor3f(0.5,0.5,0.5);     glVertex3f( (float)(x), (float)(-y),  (float)(z) );

    glColor3f(1,1,1);			glVertex3f( (float)(-x), (float)(-y), (float)(-z) );
    glColor3f(1,1,1);		    glVertex3f( (float)(-x), (float)(-y),  (float)(z) );
    glColor3f(0.5,0.5,0.5);     glVertex3f( (float)(-x),  (float)(y),  (float)(z) );
    glColor3f(0.5,0.5,0.5);     glVertex3f( (float)(-x),  (float)(y), (float)(-z) );

    glEnd( );
	tfx_swgl_SwapBuffers();

}


void draw_zoomer(float t, int *fb)
{
	int i, j, c;
	int bb[160*100];
	unsigned short tm[80*50];
	for (i = 0, c = 0; i < 100; i++)
	{
		for (j = 0; j < 160; j++, c++)
		{
			int v = (((sin((i+100)*0.0352+10*t) +
					   sin((j+100)*0.0445+20*t) -
					   sin((-j+100)*0.0335+10*t) -
					   cos((-i+100)*0.0443+10*t)) + 4) / 8) * 0xfff;
			v = abs(0xff - ((v & 0x1ff)));
			if (v > 0xff) v = 0xff;
			bb[c] = v << 8;				  					   
		}
	}
	
	tc->Dump2x(bb,TFXQuad(0,0,160,100),160,0,0,(short *)tm);
	
	float zoom = sin(t*16+4.8)*1.75+2;//0.5+t*4;
	
	float xofs = 0;//(sin(0.43 * t) + 1) * 60;
	float yofs = 0;//(cos(0.41 * t) + 1) * 40;
	float mix = zoom - 1;
	if (mix < 0) mix = 0;
	if (mix > 1) mix = 1;
	mix = 1-mix;

	for (i = 0, c = 0; i < 100; i++)
	{
		for (j = 0; j < 160; j++, c++)
		{
			int chidx = (int)(xofs + j * zoom / 8) +
				        (int)(yofs + i * zoom / 12) * 80;
			int ch = (tm[chidx] & 0x7f) * 12 * 8;
			int chc = chartex[ch + ((int)(i*zoom)%12)*8+(int)(j*zoom)%8] << 7;
			int pxc = bb[(int)(xofs + j * zoom/4)+(int)(yofs + i * zoom/6)*160];
			
			fb[c] = blend(chc,pxc,mix);

		}
	}

	t = t * 20 - 1;
	if (t < 0) return;
	t /= 19;

	float q = t * 6 - 1;
	if (q < 0) q = 0;
	q /= 5;
	sqrt(q);
	int edge[160];
	for (i = 0; i < 160; i++)
		edge[i] = ((unsigned int)(sin((i + t*400) * 0.005)*(2500*q)+60+t*sin(t*16)*800) % 180) * 160*100;

	int dypp[160];
	float q1 = t * 6 - 5;
	if (q1 < 0) q1 = 0;
	q1 = q1*q1;
	for (i = 0; i < 160; i++)
	{
		float d = i/160.0 + (q1 * 4) - 2;
		d *= d * d;
		dypp[i] = d*100;
		if (dypp[i] < 0) dypp[i] = 0;
	}

	q = (1-t*6);
	if (q < 0) q = 0;
	q *= q * q * q * q;
	int ypos = abs(cos(t*(120+t*500)))*q*-100;
	int sypos = abs(cos(t*(120+t*500)))*q*12+6;
	// shadow
	for (i = 0, c = 0; i < 100; i++)
	{
		{
			for (j = 0; j < 160; j++, c++)
			{
				if (dypp[j]+i+sypos > 0 && dypp[j]+i+sypos < 100)
				{
					int frameofs = edge[j];
					if (boxframes[c+frameofs])
					{
						fb[c+(sypos+dypp[j])*160] = (fb[c+(sypos+dypp[j])*160] >> 1) & 0x7f7f7f;
					}
				}
			}
		}
	}

	// box
	for (i = 0, c = 0; i < 100; i++)
	{
		{
			for (j = 0; j < 160; j++, c++)
			{
				if (dypp[j]+i+ypos > 0 && dypp[j]+i+ypos < 100)
				{
					int frameofs = edge[j];
					if (boxframes[c+frameofs])
					{
						fb[c+(ypos+dypp[j])*160] = boxframes[c+frameofs];
					}
				}
			}
		}
	}	
}

void chartomap(int chars, int *blurred)
{
	int charoffset = chars * 12;
    int i, j, c;
    int scratch[8 * 12];
    unsigned char * pchar = (unsigned char *)TFX_AsciiFontdata;
    for (i = 0, c = 0; i < 12; i++)
    {
        for (j = 0; j < 8; j++, c++)
        {
            int cc = IS_BIT(*(pchar + i + charoffset), j) ? 255 : 0;
            scratch[i * 8 + j] = cc;
        }
    }
	
    for (i = 0, c = 0; i < 12; i++)
    {
        for (j = 0; j < 8; j++, c++)
        {
            blurred[c] = scratch[c];
        }
    }
	
}

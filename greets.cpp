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

extern int chartex[12*8*128];
void initwell(int v);
unsigned long well512(void);

void rotate_tube(float t, int tick)
{
	glRotatef(sin(tick*0.000234)*90,0,0,1);
	glRotatef(sin(tick*0.000134)*20,0,1,0);
	glRotatef(sin(tick*0.000034)*20,1,0,0);
}

void draw_tube(float t, int tick)
{
	initwell(31);
	glClear(GL_DEPTH_BITS);
	glDisable(GL_TEXTURE_2D);
	glColor4f(1,1,1,1);
    glLoadIdentity();
	glRotatef(sin(tick*0.000234)*270,0,0,1);
	rotate_tube(t, tick);
	glTranslatef( (float)(0), (float)(sin(t*400*0.0843)*0.3), t*400);

	glBegin(GL_TRIANGLES);
	int i;
	for (i = 0; i < 990; i++)
	{
		if (i < 400 && well512() % 200 > i) continue;
		if (i > 700 && well512() % 400 < (i-700)) continue;
		float p1 = well512() * 0.00234;
		float p = (((well512() % 100) / 100.0f) - 50.0f) * (0.01+(t*2-1)*0.003);
		float p2, p3;

		p2 = p1 + p;
		p3 = p1 - p;

		float c = (well512() % 100)/100.0f;
		glColor3f(c,c,c);
		glVertex3f(sin(p1),cos(p1)+sin(i*0.0943)*0.3,(p3-p1)*3-i*0.4);
		glVertex3f(sin(p2),cos(p2)+sin(i*0.0943)*0.3,-i*0.4);
		glVertex3f(sin(p3),cos(p3)+sin(i*0.0943)*0.3,-i*0.4);
	}
	glEnd();
}

void draw_char(int x, int y, int c, int *fb)
{
	int i, j;
	for (i = 0; i < 12*2; i++)
	{
		for (j = 0; j < 8*2; j++)
		{
			if (chartex[c*12*8+(i/2)*8+j/2])
			{
				fb[x+j+(i+y)*160] = 0xffffff;
				fb[x+j+(i+y)*160+161] = (fb[x+j+(i+y)*160+161]>>1)&0x7f7f7f;
				fb[x+j+(i+y)*160+322] = (fb[x+j+(i+y)*160+322]>>1)&0x7f7f7f;
			}
		}
	}
}

void draw_string(int x, int y, char *s, int *fb)
{
	int chs = 0;
	while (*s)
	{
		draw_char(x,y,*s,fb);
		s++;
		x += 16;
		chs++;
		if (chs == 10)
		{
			x = 0;
			y += 24;
			chs = 0;
		}
	}

}

void draw_greets(float t, int *fb)
{
char *greets[50] = {
//2345678901234567890
" ",
" ",
"Howdy to  textmode  freaks:",
"Howdy to  textmode  freaks:",
" ",
" ",
"Alpha     Design",
"Anthill   Unlimited",
"bawlz",
"Calodox",
"Creative  Mind",
"Crimson   Shine",
"Fallow",
"Four Byte Aliens",
"Grin",
"Hedelmae",
"Kaleido",
"Northern  Dragons",
"Null Ok",
"paulius",
"Portal    Project",
"Psikorp",
"Recreation",
"Static",
"tAAt",
"Traction",
"Trailer   Park Demo",
"xplsv",
" ",
" ",
" ",
" "};


	char *s = greets[(int)(t*31.5)];
	int l = strlen(s);
	int x = 0;
	int y = 100-24*((l-1)/10+1)-2;
	draw_string(x, y, s, fb);
}

void draw_starfield2(float t, float aTick)
{
	initwell(0x7aa7);

	int i;

	glDisable(GL_TEXTURE_2D);
	glColor4f(1,1,1,1);
    glLoadIdentity();
	glTranslatef( (float)(0), (float)(0), -4);


	glBegin(GL_POINTS);
	for (i = 0; i < 1024 * (1-t); i++)
	{
		float x = ((well512() & 0xffff) / (float)0x10000); 		
		float y = ((well512() & 0xffff) / (float)0x10000); 
		x = (x  - 0.5) * 4;
		y = (y  - 0.5) * 4;
		float z = (well512() & 0xffff) / (float)0x10000; 
		
		z += (aTick * 0.001);
		z = fmod(z,1);
		//z = 1 - z;
		//glColor4f(z,z,z,1);
		z *= 8;
		glVertex3f(x,y,z);
	}
	glEnd();
}

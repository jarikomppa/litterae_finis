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

#include "include\scripts.h"
#include "hanska.h"



void draw_logo(float aTime)
{
	glDisable(GL_TEXTURE_2D);
	glClear( GL_DEPTH_BUFFER_BIT );
	glColor4f(1,1,1,1);
    glLoadIdentity();
    glTranslatef( (float)(0), (float)(0), -5);
	glBegin(GL_LINES);
	float scale = 0.04f;
	int i;

	char l[]="Litterae";
	char *sp = l;
	int xofs = 0;
	float z = aTime * 400;
	float zscale = 0.02f;
	float xscale = 1 - (aTime * 0.6); // 08
	int letters = aTime * 400;
	while (*sp)
	{
		int c = *sp - ' ';
		for (i = 0; letters > 0 && i < scripts_size[c]/2; i++)
		{
			glVertex3f(-1.5 + scale * (xofs + scripts[c][i*2+0]) * xscale, 1.1 + scale*-scripts[c][i*2+1], zscale * z);
			if (i & 1)
			{
				letters--;
				z--;
			}
		}
		xofs += scripts_width[c];
		sp++;
	}

	char f[]="Finis";
	sp = f;
	xofs = 0;
	while (*sp)
	{
		int c = *sp - ' ';
		for (i = 0; letters > 0 && i < scripts_size[c]/2; i++)
		{
			glVertex3f(-0.5 + scale * (xofs + scripts[c][i*2+0]) * xscale, 0 + scale*-scripts[c][i*2+1], zscale * z);
			if (i & 1)
			{
				letters--;
				z--;
			}
		}
		xofs += scripts_width[c];
		sp++;
	}
	glEnd();
	glEnable(GL_TEXTURE_2D);
}
// 0.374 1.919 1.824 1.436 1.289
void attractor_gen(float &x, float &y, float &z, float a0, float a1, float a2, float a3, float a4)
{
	float xn,yn,zn;
	xn = sin(a0 * y) - z * cos(a1 * x);
	yn = z * sin(a2 * x) - cos(a3 * y);
	zn = a4 * sin(x);
	x = xn;
	y = yn;
	z = zn;
}

int is_phase(float x, float y, float z, int i, int phase)
{
	int v = 0;
	if (x > 0) v++;
	if (y > 0) v+=2;
	if (z > 0) v+=4;
	if (i & 1) v= v^7 + 8;
	return v == (phase & 15);
}


void draw_attractor1(float aTime, float tick)
{
	glDisable(GL_TEXTURE_2D);
	glColor4f(1,1,1,1);
    glLoadIdentity();
    glTranslatef( (float)(0), (float)(0), -0);
	glRotatef(tick * 0.0002 * 60, 0.1,1,0.3);
	float x = 0, y = 0, z = 0;

	//glColor4f(0.5,0.5,0.5,1.0);
	glColor4f(1,1,1,1);
	glBegin(GL_POINTS);
	int i;
	for (i = 0; i < hanskavertcount/3; i++)
	{
		attractor_gen(x,y,z,1.374,1.919,1.824,1.436,1.289);
		float dc = (1-((int)tick % 240)/240.0) * 0.8 + 0.2;
		if (is_phase(x,y,z,i,tick / 240))
			glColor4f(dc, dc, dc, 1); 
		else
			glColor4f(0.2,0.2,0.2,1);
		glVertex3f(x,y,z);
	}
	glEnd();

	glEnable(GL_TEXTURE_2D);
}

void draw_attractor2(float aTime, float tick)
{
	glDisable(GL_TEXTURE_2D);
	glColor4f(1,1,1,1);
    glLoadIdentity();
	float t = aTime;
	t = t*t;//SMOOTHSTEP(t);
	t = SMOOTHSTEP(t);
	t *= 7;
    glTranslatef( (float)(0), (float)(0), -t);
	glRotatef(tick * 0.0002 * (60 + t * 10), 0.1,1,0.3);
	float x = 0, y = 0, z = 0;

	glColor4f(1,1,1,1);
	glBegin(GL_POINTS);
	int i;
	for (i = 0; i < hanskavertcount/3; i++)
	{
		attractor_gen(x,y,z,1.374,1.919,1.824,1.436,1.289);
		float dc = (1-((int)tick % 240)/240.0) * 0.8 + 0.2;
		if (is_phase(x,y,z,i,tick / 240))
			glColor4f(dc, dc, dc, 1); 
		else
			glColor4f(0.2,0.2,0.2,1);
		glVertex3f(x,y,z);
	}
	glEnd();

	glEnable(GL_TEXTURE_2D);
}

void draw_attractor3(float aTime, float tick)
{
	glDisable(GL_TEXTURE_2D);
	glColor4f(1,1,1,1);
    glLoadIdentity();
	float t = aTime;
	//t = SMOOTHSTEP(t);
	t *= 5;
	if (t > 1) t = 1;
    glTranslatef( (float)(0), (float)(0), -7);
	glRotatef(tick * 0.0002 * 130, 0.1,1,0.3);
	float x = 0, y = 0, z = 0;

	//glColor4f(0.5,0.5,0.5,1.0);
	glColor4f(1,1,1,1);
	glBegin(GL_POINTS);
	int i;
	for (i = 0; i < hanskavertcount/3; i++)
	{
		attractor_gen(x,y,z,1.374,1.919,1.824,1.436,1.289);
		
		float xx, yy, zz;
		xx=x;
		yy=y;
		zz=z;
		if (x < (aTime * 10)-5)
		{
			xx = hanskavert[i*3+0];
			yy = hanskavert[i*3+1]-0.3;
			zz = hanskavert[i*3+2];
		}

		float dc = (1-((int)tick % 240)/240.0) * 0.8 + 0.2;
		if (is_phase(xx,yy,zz,i,tick / 240))
			glColor4f(dc, dc, dc, 1); 
		else
			glColor4f(0.2,0.2,0.2,1);
		glVertex3f(xx,yy,zz);
	}
	glEnd();

	glEnable(GL_TEXTURE_2D);
}

void draw_attractor4(float aTime, float tick)
{
	glDisable(GL_TEXTURE_2D);
	glColor4f(1,1,1,1);
    glLoadIdentity();
	float t = aTime;
	if (t > 1) t -= 1;
	t = sqrt(t);
	//t = SMOOTHSTEP(t);
	if (aTime > 1)
		t += 1;
    glTranslatef( (float)(0), (float)(0), -7+t*3.4);
	glRotatef(tick * 0.0002 * 130, 0.1,1,0.3);
	float x = 0, y = 0, z = 0;

	glBegin(GL_POINTS);
	int i;
	for (i = 0; i < hanskavertcount/3; i++)
	{
		float xx = hanskavert[i*3+0];
		float yy = hanskavert[i*3+1] - 0.3;
		float zz = hanskavert[i*3+2];

		float dc = (1-((int)tick % 240)/240.0) * 0.8 + 0.2;

		if (is_phase(xx,yy,zz,i,tick / 240))
			glColor4f(dc, dc, dc, 1); 
		else
			glColor4f(0.2,0.2,0.2,1);
		glVertex3f(xx,yy,zz);
	}
	glEnd();

	glEnable(GL_TEXTURE_2D);
}

/* WELL512 algorithm implementation
   by Chris Lomont, from Game Programming Gems 7, page 120-121
   public domain 
 */
 
/* initialize state to random bits */
static unsigned long state[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
/* init should also reset this to 0 */
static unsigned int index = 0;
void initwell(int v)
{
	int i;
	for (i = 0; i < 16; i++)
		state[i] = v + i;
	index = 0;
}
/* return 32 bit random number */
unsigned long well512(void)
{
  unsigned long a, b, c, d;
  a = state[index];
  c = state[(index + 13) & 15];
  b = a ^ c ^ (a << 16) ^ (c << 15);
  c = state[(index + 9) & 15];
  c ^= (c >> 11);
  a = state[index] = b ^ c;
  d = a ^ ((a << 5) & 0xDA442D20UL);
  index = (index + 15) & 15;
  a = state[index];
  state[index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
  return state[index];
}


void draw_starfield(float aTick)
{
	initwell(0x7aa7);

	int i;

	glDisable(GL_TEXTURE_2D);
	glColor4f(1,1,1,1);
    glLoadIdentity();
	glTranslatef( (float)(0), (float)(0), -4);


	glBegin(GL_POINTS);
	for (i = 0; i < 1024; i++)
	{
		float x = ((well512() & 0xffff) / (float)0x10000); 		
		float y = ((well512() & 0xffff) / (float)0x10000); 
		x = (x  - 0.5) * 4;
		y = (y  - 0.5) * 4;
		float z = (well512() & 0xffff) / (float)0x10000; 
		
		z += (aTick * 0.0004);
		z = fmod(z,1);
		z = 1 - z;
		//glColor4f(z,z,z,1);
		z *= 8;
		glVertex3f(x,y,z);
	}
	glEnd();
}

void draw_star(float intensity, int tick)
{
	glColor4f(1,1,1,1);
	int i;
	
	glBegin(GL_POINTS);
	for (i = 0; i < 1024; i++)
	{
		float x = ((well512() & 0xffff) / (float)0x10000); 		
		float y = ((well512() & 0xffff) / (float)0x10000); 
		float z = (well512() & 0xffff) / (float)0x10000; 
		x -= 0.5;
		y -= 0.5;
		z -= 0.5;

		float p = (i / 1024.0);

		float l = sqrt(x*x+y*y+z*z) * sqrt(p);//((well512()%1000)/1000.0);
		if (l != 0)
		{
			x /= l;
			y /= l;
			z /= l;
		}

		x *= intensity * 0.005;
		y *= intensity * 0.005;
		z *= intensity * 0.005;
		if (i < 1024*intensity)
		{
			glVertex3f(x,y,z);
		}	
	}
	glEnd();
	
	glBegin(GL_LINES);
	int d = (tick / 240) - 90;
	for (i = 0; i < 9*d; i++)
		well512();

	for (i = 0; i < 20; i++)
	{
		float x = ((well512() & 0xffff) / (float)0x10000); 		
		float y = ((well512() & 0xffff) / (float)0x10000); 
		float z = (well512() & 0xffff) / (float)0x10000; 
		x -= 0.5;
		y -= 0.5;
		z -= 0.5;

		float l = sqrt(x*x+y*y+z*z);
		if (l != 0)
		{
			x /= l;
			y /= l;
			z /= l;
		}

		x *= intensity * 0.02;
		y *= intensity * 0.02;
		z *= intensity * 0.02;

		glColor4f(0.5,0.5,0.5,1);
		glVertex3f(x,y,z);		
		glVertex3f(x*(20-i+1),y*(20-i+1),z*(20-i+1));
	}
	glEnd();
}

void tile_reveal(int *fb, int tick)
{
	int blip = (tick / 242);
	float d = (tick % 242) / 242.0;
	int color1 = (int)(0x7f*(1-d))*0x010101;
	int color2 = (int)(0x80+0x7f*(1-d))*0x010101;

	if (blip > 25)
		color2 = (int)((0x7f - blip*3)*(1-d))*0x010101;

	int i, j, k, l;

	for (i = 0; i < 5; i++)
		for (j = 0; j < 5; j++)
		{
			int tile = ((j * 5 + i) * 11) % (5*5);

			for (k = 0; k < 160/5; k++)
				for (l = 0; l < 100/5; l++)
				{
					if (tile > blip)
					{
						fb[j*(160/5)+k+(i*(100/5)+l)*160] = 0;
					}
					if (tile == blip-1)
					{
						int c = fb[j*(160/5)+k+(i*(100/5)+l)*160];
						if ((c & 0xff00) < (color1 & 0xff00))
						 fb[j*(160/5)+k+(i*(100/5)+l)*160] = color1;
					}
					if (tile == blip % 25)
					{
						int c = fb[j*(160/5)+k+(i*(100/5)+l)*160];
						if ((c & 0xff00) < (color2 & 0xff00))
						 fb[j*(160/5)+k+(i*(100/5)+l)*160] = color2;
					}
				}
		}
}

void writer(char *aText, float aTime)
{
	glColor4f(1,1,1,1);
    glLoadIdentity();
	glTranslatef( (float)(0), (float)(0), -5);
	glBegin(GL_LINES);
	float scale = 0.024f;
	int i;

	char *sp = aText;
	int xofs = 0;
	int letters = aTime * 400;
	while (*sp)
	{
		int c = *sp - ' ';
		for (i = 0; letters > 0 && i < scripts_size[c]/2; i++)
		{
			float z;
			z = (i - (letters - 100)) * 0.1;
			z *= aTime;
			if (z > 0) z = 0;
			
			glVertex3f(-2.1 + scale * (xofs + scripts[c][i*2+0]), -1.2 + scale*-scripts[c][i*2+1], z);
			if (i & 1)
			{
				letters--;
			}
		}
		xofs += scripts_width[c];
		sp++;
	}

	glEnd();
}

void draw_attractor(float aTime, float tick, float scale)
{
//    glLoadIdentity();
//    glTranslatef( (float)(0), (float)(0), 0);
//	glRotatef(tick * 0.0001 * 60, 0.01,0.007,1);
	float x = 0, y = 0, z = 0;

	if (aTime < 0) aTime = 0;
	if (aTime > 1) aTime = 1;

	glBegin(GL_POINTS);
	int i;
	for (i = 0; i < aTime * 4000; i++)
	{
		attractor_gen(x,y,z,1.374,1.919,1.824,1.436,1.289);
		float dc = (sin((1-((int)tick % 960)/960.0) * 3.14 * 2)+1)*0.4+0.2;
		glColor4f(dc, dc, dc, 1); 
		glVertex3f(x*scale,y*scale,z*scale);
	}
	glEnd();
}

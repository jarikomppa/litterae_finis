#include <GL/gl.h>
#include <GL/glu.h>
#include <gl/tfxswgl.h>
#include "stb_image.h"
#include "vertexbuffer.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

static void tesselate_center(VertexBuffer * aVB)
{
	// centerpoint tesselate (not good, but easy to implement)
	//      .
	//     /|\
	//    / . \
	//   /_/_\_\
	//
	
	// one new vertex per triangle
	GLfloat * vtx  = new GLfloat[(aVB->mVertices + aVB->mIndices / 3) * 3];
	GLfloat * norm = new GLfloat[(aVB->mVertices + aVB->mIndices / 3) * 3];
	// three times the indices
	GLuint  * idx = new GLuint[aVB->mIndices * 3]; 

	int i;
	for (i = 0; i < aVB->mVertices * 3; i++)
	{
		vtx[i] = aVB->mVertex[i];
		norm[i] = aVB->mNormal[i];
	}

	for (i = 0; i < aVB->mIndices / 3; i++)
	{
		norm[(aVB->mVertices + i) * 3 + 0] = vtx[(aVB->mVertices + i) * 3 + 0] = (vtx[aVB->mIndex[i * 3 + 0] * 3 + 0] + vtx[aVB->mIndex[i * 3 + 1] * 3 + 0] + vtx[aVB->mIndex[i * 3 + 2] * 3 + 0]) / 3;
		norm[(aVB->mVertices + i) * 3 + 1] = vtx[(aVB->mVertices + i) * 3 + 1] = (vtx[aVB->mIndex[i * 3 + 0] * 3 + 1] + vtx[aVB->mIndex[i * 3 + 1] * 3 + 1] + vtx[aVB->mIndex[i * 3 + 2] * 3 + 1]) / 3;
		norm[(aVB->mVertices + i) * 3 + 2] = vtx[(aVB->mVertices + i) * 3 + 2] = (vtx[aVB->mIndex[i * 3 + 0] * 3 + 2] + vtx[aVB->mIndex[i * 3 + 1] * 3 + 2] + vtx[aVB->mIndex[i * 3 + 2] * 3 + 2]) / 3;
	}

	for (i = 0; i < aVB->mIndices / 3; i++)
	{
		idx[i * 9 + 0] = aVB->mIndex[i * 3 + 0];
		idx[i * 9 + 1] = aVB->mIndex[i * 3 + 1];
		idx[i * 9 + 2] = aVB->mVertices + i;

		idx[i * 9 + 3] = aVB->mIndex[i * 3 + 1];
		idx[i * 9 + 4] = aVB->mIndex[i * 3 + 2];
		idx[i * 9 + 5] = aVB->mVertices + i;

		idx[i * 9 + 6] = aVB->mIndex[i * 3 + 2];
		idx[i * 9 + 7] = aVB->mIndex[i * 3 + 0];
		idx[i * 9 + 8] = aVB->mVertices + i;
	}	

	delete[] aVB->mIndex;
	delete[] aVB->mNormal;
	delete[] aVB->mVertex;
	aVB->mIndex = idx;
	aVB->mNormal = norm;
	aVB->mVertex = vtx;	

	aVB->mVertices += aVB->mIndices / 3;
	aVB->mIndices *= 3;
}

static void tesselate_edge(VertexBuffer * aVB)
{
	// edge-center tesselate
	//
	//      .
	//     / \
	//    /___\
	//   / \ / \
	//  /___V___\
	
	// new vertex per two indices
	GLfloat * vtx  = new GLfloat[(aVB->mVertices + aVB->mIndices / 2) * 3];
	GLfloat * norm = new GLfloat[(aVB->mVertices + aVB->mIndices / 2) * 3];
	// four times the indices
	GLuint  * idx = new GLuint[aVB->mIndices * 4];
	// lookup for edges: an edge per two indices, two ints per edge
	int * lookup = new int[aVB->mIndices];
	// clear lookup to zeros to begin with
	memset(lookup, 0, sizeof(int) * aVB->mIndices);

	int i;
	// copy originals
	for (i = 0; i < aVB->mVertices * 3; i++)
	{
		vtx[i] = aVB->mVertex[i];
		norm[i] = aVB->mNormal[i];
	}

	// create new ones (one per edge, discarding duplicates)
	int newpos = 0;
	for (i = 0; i < aVB->mIndices / 3; i++)
	{
		int j;
		for (j = 0; j < 3; j++)
		{
			int pa = aVB->mIndex[i * 3 + ((j + 0) % 3)];
			int pb = aVB->mIndex[i * 3 + ((j + 1) % 3)];
			
			if (pa > pb)
			{
				int temp = pa;
				pa = pb;
				pb = temp;
			}

			int found = 0;
			int k;
			for (k = 0; !found && k < newpos; k++)
			{
				if (lookup[k*2+0] == pa &&
					lookup[k*2+1] == pb)
				{
					found = 1;
				}
			}
			if (!found)
			{
				float nx = (aVB->mVertex[pa * 3 + 0] + aVB->mVertex[pb * 3 + 0]) / 2;
				float ny = (aVB->mVertex[pa * 3 + 1] + aVB->mVertex[pb * 3 + 1]) / 2;
				float nz = (aVB->mVertex[pa * 3 + 2] + aVB->mVertex[pb * 3 + 2]) / 2;
				norm[(newpos+aVB->mVertices)*3+0] = vtx[(newpos+aVB->mVertices)*3+0] = nx;
				norm[(newpos+aVB->mVertices)*3+1] = vtx[(newpos+aVB->mVertices)*3+1] = ny;
				norm[(newpos+aVB->mVertices)*3+2] = vtx[(newpos+aVB->mVertices)*3+2] = nz;
				lookup[newpos*2+0] = pa;
				lookup[newpos*2+1] = pb;
				newpos++;
			}
		}		
	}

	// create new indices
	int idxpos = 0;
	for (i = 0; i < aVB->mIndices / 3; i++)
	{
		// first, find the mid-edge verts for all triangles
		int edgevtxs[3];
		int j;
		for (j = 0; j < 3; j++)
		{
			int pa = aVB->mIndex[i * 3 + ((j + 0) % 3)];
			int pb = aVB->mIndex[i * 3 + ((j + 1) % 3)];
			
			if (pa > pb)
			{
				int temp = pa;
				pa = pb;
				pb = temp;
			}

			int found = 0;
			int edgevtx = -1;
			int k;
			for (k = 0; !found && k < newpos; k++)
			{
				if (lookup[k*2+0] == pa &&
					lookup[k*2+1] == pb)
				{
					found = 1;
					edgevtx = k + aVB->mVertices;
				}
			}
			edgevtxs[j] = edgevtx;
		}
		// next, create triangles, careful with the vertex 
		// order to preserve backface culling
		idx[idxpos++] = aVB->mIndex[i * 3 + 0];
		idx[idxpos++] = edgevtxs[0];
		idx[idxpos++] = edgevtxs[2];

		idx[idxpos++] = edgevtxs[1];
		idx[idxpos++] = aVB->mIndex[i * 3 + 2];
		idx[idxpos++] = edgevtxs[2];

		idx[idxpos++] = edgevtxs[1];
		idx[idxpos++] = edgevtxs[0];
		idx[idxpos++] = aVB->mIndex[i * 3 + 1];

		// the new center triangle
		idx[idxpos++] = edgevtxs[0];
		idx[idxpos++] = edgevtxs[1];
		idx[idxpos++] = edgevtxs[2];
	}

	// free the old junk
	delete[] lookup;
	delete[] aVB->mIndex;
	delete[] aVB->mNormal;
	delete[] aVB->mVertex;
	
	// replace pointers
	aVB->mIndex = idx;
	aVB->mNormal = norm;
	aVB->mVertex = vtx;	

	// set the counts
	aVB->mVertices += newpos;
	aVB->mIndices = idxpos;

	// done!
}

static void normalize(VertexBuffer *aVB, float aScale)
{
	int i;
	for (i = 0; i < aVB->mVertices; i++)
	{
		float l = sqrt(aVB->mVertex[i * 3 + 0] * aVB->mVertex[i * 3 + 0] +
					   aVB->mVertex[i * 3 + 1] * aVB->mVertex[i * 3 + 1] +
				 	   aVB->mVertex[i * 3 + 2] * aVB->mVertex[i * 3 + 2]);

		aVB->mNormal[i * 3 + 0] = aVB->mVertex[i * 3 + 0] / l;
		aVB->mNormal[i * 3 + 1] = aVB->mVertex[i * 3 + 1] / l;
		aVB->mNormal[i * 3 + 2] = aVB->mVertex[i * 3 + 2] / l;
	}

	for (i = 0; i < aVB->mVertices; i++)
	{
		aVB->mVertex[i * 3 + 0] = aVB->mNormal[i * 3 + 0] * aScale;
		aVB->mVertex[i * 3 + 1] = aVB->mNormal[i * 3 + 1] * aScale;
		aVB->mVertex[i * 3 + 2] = aVB->mNormal[i * 3 + 2] * aScale;
	}

}

static void fix_uv(VertexBuffer *aVB)
{
	// We have a slight problem: the UV coordinates wrap around when they 
	// get around the sphere. Soo.. we need to find the problematic vertices 
	// and duplicate them.

	// we don't have any idea just how much space we'll need, so let's
	// make the buffers huge enough to make sure.
	GLfloat * vtx = new GLfloat[aVB->mVertices * 8 * 3];
	GLfloat * norm = new GLfloat[aVB->mVertices * 8 * 3];
	GLfloat * uv = new GLfloat[aVB->mVertices * 8 * 2];

	memcpy(vtx, aVB->mVertex, sizeof(GLfloat) * 3 * aVB->mVertices);
	memcpy(norm, aVB->mNormal, sizeof(GLfloat) * 3 * aVB->mVertices);
	memcpy(uv, aVB->mTexcoord[0], sizeof(GLfloat) * 2 * aVB->mVertices);

	// find cap vertices
	int i;
	float cap0v = 1.0f;
	float cap1v = 0.0f;
	for (i = 0; i < aVB->mVertices; i++)
	{
		if (uv[i*2+1] < cap0v)
		{
			cap0v = uv[i*2+1];
		}
		if (uv[i*2+1] > cap1v)
		{
			cap1v = uv[i*2+1];
		}
	}

	int newidx = aVB->mVertices;
	
	for (i = 0; i < aVB->mIndices / 3; i++)
	{
		int j;
		for (j = 0; j < 3; j++)
		{
			if ((uv[aVB->mIndex[i*3+j]*2+0]-uv[aVB->mIndex[i*3+((j+1)%3)]*2+0]) > 0.25f ||
				(uv[aVB->mIndex[i*3+j]*2+0]-uv[aVB->mIndex[i*3+((j+2)%3)]*2+0]) > 0.25f)
			{
				// duplicate this vertex
				vtx[newidx * 3 + 0] = vtx[aVB->mIndex[i*3+j]*3+0];
				vtx[newidx * 3 + 1] = vtx[aVB->mIndex[i*3+j]*3+1];
				vtx[newidx * 3 + 2] = vtx[aVB->mIndex[i*3+j]*3+2];
				norm[newidx * 3 + 0] = norm[aVB->mIndex[i*3+j]*3+0];
				norm[newidx * 3 + 1] = norm[aVB->mIndex[i*3+j]*3+1];
				norm[newidx * 3 + 2] = norm[aVB->mIndex[i*3+j]*3+2];
				uv[newidx * 2 + 0] = uv[aVB->mIndex[i*3+j]*2+0];
				uv[newidx * 2 + 1] = uv[aVB->mIndex[i*3+j]*2+1];

				uv[newidx * 2 + 0] -= 1.0f;
				
				aVB->mIndex[i*3+j] = newidx;
				newidx++;
			}

			if (uv[aVB->mIndex[i*3+j]*2+1] == cap0v ||
				uv[aVB->mIndex[i*3+j]*2+1] == cap1v)
			{
				// cap vertex - need to duplicate and recalculate UV
				// (this will leak two verts, but who cares)
				// duplicate this vertex

				vtx[newidx * 3 + 0] = vtx[aVB->mIndex[i*3+j]*3+0];
				vtx[newidx * 3 + 1] = vtx[aVB->mIndex[i*3+j]*3+1];
				vtx[newidx * 3 + 2] = vtx[aVB->mIndex[i*3+j]*3+2];
				norm[newidx * 3 + 0] = norm[aVB->mIndex[i*3+j]*3+0];
				norm[newidx * 3 + 1] = norm[aVB->mIndex[i*3+j]*3+1];
				norm[newidx * 3 + 2] = norm[aVB->mIndex[i*3+j]*3+2];
				uv[newidx * 2 + 0] = (uv[aVB->mIndex[i*3+((j+1)%3)]*2+0] + uv[aVB->mIndex[i*3+((j+2)%3)]*2+0]) / 2;
				uv[newidx * 2 + 1] = uv[aVB->mIndex[i*3+j]*2+1];

				// a small hack to fix a bug that occurs in some iterations
				if ((uv[newidx*2+0]-uv[aVB->mIndex[i*3+((j+1)%3)]*2+0]) > 0.25f ||
					(uv[newidx*2+0]-uv[aVB->mIndex[i*3+((j+2)%3)]*2+0]) > 0.25f)
					uv[newidx*2+0] -= 0.5f;

				aVB->mIndex[i*3+j] = newidx;
				newidx++;
			}
			
		}
	}



	delete[] aVB->mTexcoord[0];
	delete[] aVB->mNormal;
	delete[] aVB->mVertex;
	
	// replace pointers
	aVB->mTexcoord[0] = new GLfloat[newidx * 2];
	aVB->mNormal = new GLfloat[newidx * 3];
	aVB->mVertex = new GLfloat[newidx * 3];	

	memcpy(aVB->mVertex, vtx, sizeof(GLfloat) * 3 * newidx);
	memcpy(aVB->mNormal, norm, sizeof(GLfloat) * 3 * newidx);
	memcpy(aVB->mTexcoord[0], uv, sizeof(GLfloat) * 2 * newidx);

	delete[] vtx;
	delete[] norm;
	delete[] uv;

	// set the counts
	aVB->mVertices = newidx;
}


VertexBuffer * generate_sphere(float aScale, int aIters)
{
	int i;

	static GLfloat vtx[] =
	{
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 1.0f, 0.0f
	};

	static GLuint idx[] =
	{
		0,    1,    2,
		0,    2,    3,
		0,    3,    4,
		0,    4,    1,
		1,    5,    2,
		2,    5,    3,
		3,    5,    4,
		4,    5,    1
	};

	VertexBuffer * vb = new VertexBuffer;

	vb->mPrimitiveType = GL_TRIANGLES;
	vb->mIndices = 8 * 3;
	vb->mVertices = 6;

	vb->mIndex = new GLuint[vb->mIndices];
	for (i = 0; i < vb->mIndices; i++)
		vb->mIndex[i] = idx[i];

	vb->mNormal = new GLfloat[vb->mVertices * 3];
	for (i = 0; i < vb->mVertices; i++)
	{
		float l = sqrt(vtx[i * 3 + 0] * vtx[i * 3 + 0] +
			vtx[i * 3 + 1] * vtx[i * 3 + 1] +
			vtx[i * 3 + 2] * vtx[i * 3 + 2]);

		vb->mNormal[i * 3 + 0] = vtx[i * 3 + 0] / l;
		vb->mNormal[i * 3 + 1] = vtx[i * 3 + 1] / l;
		vb->mNormal[i * 3 + 2] = vtx[i * 3 + 2] / l;
	}

	vb->mVertex = new GLfloat[vb->mVertices * 3];
	
	for (i = 0; i < vb->mVertices; i++)
	{
		vb->mVertex[i * 3 + 0] = vb->mNormal[i * 3 + 0] * aScale;
		vb->mVertex[i * 3 + 1] = vb->mNormal[i * 3 + 1] * aScale;
		vb->mVertex[i * 3 + 2] = vb->mNormal[i * 3 + 2] * aScale;
	}

	for (i = 1; i < aIters; i++)
	{
		tesselate_edge(vb);
	}

	normalize(vb, aScale);

	vb->mTexcoord[0] = new GLfloat[vb->mVertices*2];

	for (i = 0; i < vb->mVertices; i++)
	{
		float l = sqrt(vb->mVertex[i*3+0]*vb->mVertex[i*3+0]+
					   vb->mVertex[i*3+1]*vb->mVertex[i*3+1]+
					   vb->mVertex[i*3+2]*vb->mVertex[i*3+2]);
		vb->mTexcoord[0][i * 2 + 1] = acos(vb->mVertex[i * 3 + 1] / l) / M_PI;
		vb->mTexcoord[0][i * 2 + 0] = (atan2(vb->mVertex[i * 3 + 2], -vb->mVertex[i * 3 + 0]) + M_PI) / (M_PI * 2);
	}

	fix_uv(vb);

	vb->mTexcoord[1] = vb->mTexcoord[2] = vb->mTexcoord[3] = vb->mTexcoord[0];

	return vb;   
}
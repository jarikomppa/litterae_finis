#include <GL/gl.h>
#include <GL/glu.h>
#include <gl/tfxswgl.h>
#include "stb_image.h"

#include "vertexbuffer.h"

void VertexBuffer::optimize()
{
}

void VertexBuffer::render_normals(float aScale)
{
}


void VertexBuffer::activate()
{
}

void VertexBuffer::deactivate()
{
}

void VertexBuffer::render()
{
}

VertexBuffer::VertexBuffer()
{
    mVertex = NULL;
    mNormal = NULL;
    mIndex = NULL;
    mIndices = 0;
    mPrimitiveType = 0;
    mTexcoord[0] = mTexcoord[1] = mTexcoord[2] = mTexcoord[3] = 0;
}

VertexBuffer::~VertexBuffer()
{
    int i,j;

	delete[] mVertex;
    mVertex = NULL;

    delete[] mNormal;
    mNormal = NULL;

    delete[] mIndex;
    mIndex = NULL;

    mIndices = 0;
    mPrimitiveType = 0;

    // clear tex coord arrays, guarding against double deletion
    for (i = 0; i < 4; i++)
    {
        delete[] mTexcoord[i];
        for (j = i + 1; j < 4; j++)
        {
            if (mTexcoord[j] == mTexcoord[i])
                mTexcoord[j] = NULL;
        }
        mTexcoord[i] = NULL;
    }   
}

#pragma once

#include "../precompiled.h"

namespace GLUtils
{
	inline void DeleteBuffer(GLuint& id)
	{
		if (id != 0)
		{
			glDeleteBuffers(1, &id);
			id = 0;
		}
	}

	inline void DeleteTexture(GLuint& id)
	{
		if (id != 0)
		{
			glDeleteTextures(1, &id);
			id = 0;
		}
	}

	inline void DeleteVAO(GLuint& id)
	{
		if (id != 0)
		{
			glDeleteVertexArrays(1, &id);
			id = 0;
		}
	}

	inline void DeleteFramebuffer(GLuint& id)
	{
		if (id != 0)
		{
			glDeleteFramebuffers(1, &id);
			id = 0;
		}
	}

	inline void DeleteRenderbuffer(GLuint& id)
	{
		if (id != 0)
		{
			glDeleteRenderbuffers(1, &id);
			id = 0;
		}
	}

	inline void DeleteTextures(GLuint* ids, int count)
	{
		bool anyNonZero = false;
		for (int i = 0; i < count; ++i)
		{
			if (ids[i] != 0) { anyNonZero = true; break; }
		}
		if (anyNonZero)
		{
			glDeleteTextures(count, ids);
			for (int i = 0; i < count; ++i) ids[i] = 0;
		}
	}

	inline void DeleteFramebuffers(GLuint* ids, int count)
	{
		bool anyNonZero = false;
		for (int i = 0; i < count; ++i)
		{
			if (ids[i] != 0) { anyNonZero = true; break; }
		}
		if (anyNonZero)
		{
			glDeleteFramebuffers(count, ids);
			for (int i = 0; i < count; ++i) ids[i] = 0;
		}
	}
}

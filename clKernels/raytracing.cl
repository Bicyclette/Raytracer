#define CL_HPP_TARGET_OPENCL_VERSION 200

// ---------- TYPE DEFINITION ----------
// -------------------------------------

typedef struct __attribute__ ((packed)) Camera_t
{
	float3 position;
	float3 target;
	float3 right;
	float3 up;
	float fov;
} Camera_t;

typedef struct __attribute__ ((packed)) PixelShade
{
	float3 color;
	float diffuse;
	float specular;
} PixelShade;

typedef struct __attribute__ ((packed)) PointLight_t
{
	float brightness;
	float3 position;
	int spCount;
} PointLight_t;

// ---------- PROTOTYPES ----------
// --------------------------------

bool rayTriangleIntersect(const float3 origin, const float3 dir, const float3 v0, const float3 v1, const float3 v2, float* t, float* u, float* v);

bool intersect(
		global const float* vertices,
		int vertexOffset,
		global const int* indices,
		int nbIndices,
		int indexOffset,
		float3 rayOrigin,
		float3 rayDir,
		float3* pHit, float3* nHit, float2* texCoords);

struct PixelShade computePixelShade(
		int meshIndex,
		global const float* colorShininess,
		global const unsigned char* texData,
		int nbTextures,
		int searchTexDataOffset,
		int2 w,
		int2 h,
		int2 channels,
		const float3 fragPos,
		const float3 fragNormal,
		const float2 fragTexCoords,
		const float3 lPos,
		const float3 primRay);

float computeSpecular(const float3 pHit, const float3 nHit, const float3 lightPosition, const float3 primRay, const float shininess);

float computeDiffuse(const float3 pHit, const float3 nHit, const float3 lightPosition);

float computeShadowFactor(
		const float3 fragPos,
		const float3 fragNormal,
		global const struct PointLight_t* light,
		global const float3* samplePoints,
		const int meshCount,
		global const float* vertices,
		global const int* verticesCount,
		const int vertexOffset,
		global const int* indices,
		const int nbIndices,
		global const int* indicesCount,
		const int indexOffset);

void computePrimaryRay(const int x, const int y, const int scrX, const int scrY, global const Camera_t* cam, float3* primRay);

// ---------- RAYTRACE FUNCTIONS ----------
// ----------------------------------------

kernel void raytrace(
		const int n,
		global unsigned char* img,
		const int width,
		const int height,
		global const Camera_t* cam,
		global const PointLight_t* pl,
		global const float3* samplePoints,
		const int meshCount,
		global const float* vertices, // vertices of all meshes
		const int vertexStride, // float3 + float3 + float2 (pos + norm + texCoords) => 8
		global const int* verticesCount, // vertex count for each mesh
		global const int* indices, // indices of all meshes
		global const int* indicesCount, // indices count per mesh
		global const float* colorShininess, // float4 color + float shininess for each mesh (stride = 5)
		global const int* texturesCount, //  texture count for each mesh
		global const unsigned char* texData, // all textures
		global const int* texAttributes, // width, height, channels
		const int texAttributesStride // width, height, channels => 3
		)
{
	size_t id = get_global_id(0);

	float3 primRay;

	float3 pHit;
	float3 nHit;
	float2 texCoords;

	float3 fragPos;
	float3 fragNormal;
	float2 fragTexCoords;

	struct PixelShade pixShade;
	float shadow;
	int rgb[3];

	float fragDist = FLT_MAX;
	float prevFragDist = fragDist;
	int meshIndex;
	int vertOffset = 0;
	int indOffset = 0;
	int numIndices = 0;
	int nbTexs = 0;
	int texDataOffset = 0;
	int2 w;
	int2 h;
	int2 channels;
	bool inter = false;

	int c = id % width;
	int r = (id / width) % height;
	computePrimaryRay(c, r, width, height, cam, &primRay);

	for(int i = 0; i < meshCount; ++i)
	{
		int nbVertices = verticesCount[i];
		int vertexOffset = 0;
		int nbIndices = indicesCount[i];
		int indexOffset = 0;	
		int nbTextures = texturesCount[i];
		for(int j = 0; j < i; ++j)
		{
			vertexOffset += verticesCount[j];
			indexOffset += indicesCount[j];
		}

		int searchTexDataOffset = 0;
		int searchTexAttributesOffset = 0;

		if(nbTextures > 0)
		{
			for(int j = 0; j < i; ++j)
			{
				int wTmp = wTmp + texAttributes[searchTexAttributesOffset];
				int hTmp = hTmp + texAttributes[searchTexAttributesOffset + 1];
				int channelsTmp = channelsTmp + texAttributes[searchTexAttributesOffset + 2];
				searchTexDataOffset = searchTexDataOffset + wTmp * hTmp * channelsTmp;
				searchTexAttributesOffset = searchTexAttributesOffset + texAttributesStride * texturesCount[j];
			}
			w.x = texAttributes[searchTexAttributesOffset];
			h.x = texAttributes[searchTexAttributesOffset + 1];
			channels.x = texAttributes[searchTexAttributesOffset + 2];
		}

		if(intersect(vertices, vertexOffset, indices, nbIndices, indexOffset, cam->position, primRay, &pHit, &nHit, &texCoords))
		{
			fragDist = distance(cam->position, pHit);
			if(fragDist < prevFragDist)
			{
				prevFragDist = fragDist;
				meshIndex = i;
				vertOffset = vertexOffset;
				numIndices = nbIndices;
				indOffset = indexOffset;
				inter = true;

				nbTexs = nbTextures;
				texDataOffset = searchTexDataOffset;

				fragPos = pHit;
				fragNormal = nHit;
				fragTexCoords = texCoords;
			}
		}
	}

	if(inter)
	{
		// get pixel shade data
		pixShade = computePixelShade(
				meshIndex,
				colorShininess,
				texData,
				nbTexs,
				texDataOffset,
				w, h, channels,
				fragPos,
				fragNormal,
				fragTexCoords,
				pl->position,
				primRay);

		// compute shadow factor
		shadow = computeShadowFactor(fragPos, fragNormal, pl, samplePoints, meshCount, vertices, verticesCount, vertOffset, indices, numIndices, indicesCount, indOffset);
		
		// shade
		rgb[0] = min((int)(pixShade.color.x * (1.0f - shadow) * (pixShade.diffuse + pixShade.specular) * pl->brightness * 255.0f), 255);
		rgb[1] = min((int)(pixShade.color.y * (1.0f - shadow) * (pixShade.diffuse + pixShade.specular) * pl->brightness * 255.0f), 255);
		rgb[2] = min((int)(pixShade.color.z * (1.0f - shadow) * (pixShade.diffuse + pixShade.specular) * pl->brightness * 255.0f), 255);

		// draw color
		int pixel;
		if(r >= 1)
			pixel = 3 * ((r - 1) * width + c);
		else
			pixel = 3 * c;

		img[pixel] = rgb[0];
		img[pixel+1] = rgb[1];
		img[pixel+2] = rgb[2];
	}
	else
	{
		// draw color
		int pixel;
		if(r >= 1)
			pixel = 3 * ((r - 1) * width + c);
		else
			pixel = 3 * c;

		img[pixel] = 0;
		img[pixel+1] = 0;
		img[pixel+2] = 0;

	}
}

void computePrimaryRay(const int x, const int y, const int scrX, const int scrY, global const Camera_t* cam, float3* primRay)
{
	// aspect ratio
	float aspectRatio = (float)(scrX) / (float)(scrY);

	// map x coordinate to range [-1, 1]
	float ndcX = (((float)(x) / (float)(scrX)) - 0.5f) * 2.0f;

	// map y coordinate to range [-1, 1]
	float ndcY = (((float)(y) / (float)(scrY)) - 0.5f) * 2.0f;

	// compute factor of contribution constant (max value)
	float f = sin(cam->fov * 2.0f);

	// amount of right and up vector to get
	float rightAmount = ndcX * f * aspectRatio;
	float upAmount = ndcY * f;

	// result
	*primRay = normalize(cam->target - cam->position) + rightAmount * cam->right + upAmount * cam->up;
}

float computeShadowFactor(
		const float3 fragPos,
		const float3 fragNormal,
		global const struct PointLight_t* light,
		global const float3* samplePoints,
		const int meshCount,
		global const float* vertices,
		global const int* verticesCount,
		const int vertexOffset,
		global const int* indices,
		const int nbIndices,
		global const int* indicesCount,
		const int indexOffset)
{
	float3 rayDirections[512];
	for(int i = 0; i < light->spCount; ++i)
	{
		float3 sample = samplePoints[i];
		rayDirections[i] = normalize(sample - fragPos);
	}
	float3 offset = 0.001f * normalize(fragNormal);
	float3 rayOrigin = fragPos + offset;

	// begin useless data
	float3 pHit;
	float3 nHit;
	float2 tCoords;
	// end useless data
	int vertOffset = 0;
	int indOffset = 0;	

	float shadow = 0.0f;

	for(int j = 0; j < light->spCount; ++j)
	{
		vertOffset = 0;
		indOffset = 0;	
		for(int i = 0; i < meshCount; ++i)
		{
			if(intersect(vertices, vertOffset, indices, indicesCount[i], indOffset, rayOrigin, rayDirections[j], &pHit, &nHit, &tCoords))
			{
				shadow += 1.0f;
				break;
			}
			vertOffset += verticesCount[i];
			indOffset += indicesCount[i];
		}
	}

	return shadow / (float)(light->spCount);
}

float computeDiffuse(const float3 pHit, const float3 nHit, const float3 lightPosition)
{
	float3 lightDir = normalize(lightPosition - pHit);
	return max(dot(lightDir, normalize(nHit)), 0.0f);
}

float computeSpecular(const float3 pHit, const float3 nHit, const float3 lightPosition, const float3 primRay, const float shininess)
{
	float3 lightDir = normalize(lightPosition - pHit);
	float3 halfwayDir = normalize(-primRay + lightDir);
	float spec = max(dot(nHit, halfwayDir), 0.0f);
	spec = pow(spec, shininess);
	return spec;
}

struct PixelShade computePixelShade(
		int meshIndex,
		global const float* colorShininess,
		global const unsigned char* texData,
		int nbTextures,
		int searchTexDataOffset,
		int2 w, int2 h, int2 channels,
		const float3 fragPos,
		const float3 fragNormal,
		const float2 fragTexCoords,
		const float3 lPos,
		const float3 primRay)
{
	// get mesh material
	int offset = meshIndex * 5;
	float3 color = (float3)(colorShininess[offset], colorShininess[offset + 1], colorShininess[offset + 2]);
	float shininess = colorShininess[offset + 4];
	float specFactor = 1.0f;

	int width; int height; int comp;
	int sampleX;
	int sampleY;
	int sampleIndex;
	int sample[3];

	for(int i = 0; i < nbTextures && i < 2; ++i)
	{
		if(i == 0)
		{
			width = w.x; height = h.x; comp = channels.x;
		}
		else
		{
			width = w.y; height = h.y; comp = channels.y;	
		}
		sampleX = (int)(fragTexCoords.x * (width - 1));
		sampleY = (int)(fragTexCoords.y * (height - 1));
		sampleIndex = comp * ((sampleY - 1) * width + sampleX);
		sample[0] = texData[searchTexDataOffset + sampleIndex];
		sample[1] = texData[searchTexDataOffset + sampleIndex+1];
		sample[2] = texData[searchTexDataOffset + sampleIndex+2];

		if(i == 0)
			color = (float3)(sample[0]/255.0f, sample[1]/255.0f, sample[2]/255.0f);
		else if(i == 1)
			specFactor = sample[0] / 255.0f;

		searchTexDataOffset += width * height * comp;
	}

	// get diffuse component
	float diffuse = computeDiffuse(fragPos, fragNormal, lPos);
	
	// get specular component
	float specular = computeSpecular(fragPos, fragNormal, lPos, primRay, shininess);
	specular *= specFactor;
	
	// PixelShade
	struct PixelShade result;
	result.color = color;
	result.diffuse = diffuse;
	result.specular = specular;
	return result;
}

bool intersect(
		global const float* vertices,
		int vertexOffset,
		global const int* indices,
		int nbIndices,
		int indexOffset,
		float3 rayOrigin,
		float3 rayDir,
		float3* pHit, float3* nHit, float2* texCoords)
{
	// intersection distance from camera's origin
	float minDistance = FLT_MAX;
	float prevMinDistance = minDistance;

	// barycentric coordinates (u,v,w)
	float u = 0.0f;
	float v = 0.0f;
	float w = 0.0f;

	// intersection ?
	bool res = false;

	// base vertex offset
	vertexOffset = vertexOffset * 8;

	for(int i = 0; i < nbIndices; i += 3)
	{
		int index1 = indexOffset + i; // index of v0
		int index2 = indexOffset + i + 1; // index of v1
		int index3 = indexOffset + i + 2; // index of v2

		index1 = indices[index1] * 8;
		index2 = indices[index2] * 8;
		index3 = indices[index3] * 8;

		float3 v0 = (float3)(vertices[vertexOffset + index1], vertices[vertexOffset + index1 + 1], vertices[vertexOffset + index1 + 2]);
		float3 normV0 = (float3)(vertices[vertexOffset + index1 + 3], vertices[vertexOffset + index1 + 4], vertices[vertexOffset + index1 + 5]);
		float2 texV0 = (float2)(vertices[vertexOffset + index1 + 6], vertices[vertexOffset + index1 + 7]);
		
		float3 v1 = (float3)(vertices[vertexOffset + index2], vertices[vertexOffset + index2 + 1], vertices[vertexOffset + index2 + 2]);
		float3 normV1 = (float3)(vertices[vertexOffset + index2 + 3], vertices[vertexOffset + index2 + 4], vertices[vertexOffset + index2 + 5]);
		float2 texV1 = (float2)(vertices[vertexOffset + index2 + 6], vertices[vertexOffset + index2 + 7]);
		
		float3 v2 = (float3)(vertices[vertexOffset + index3], vertices[vertexOffset + index3 + 1], vertices[vertexOffset + index3 + 2]);
		float3 normV2 = (float3)(vertices[vertexOffset + index3 + 3], vertices[vertexOffset + index3 + 4], vertices[vertexOffset + index3 + 5]);
		float2 texV2 = (float2)(vertices[vertexOffset + index3 + 6], vertices[vertexOffset + index3 + 7]);

		if(rayTriangleIntersect(rayOrigin, rayDir, v0, v1, v2, &minDistance, &u, &v))
		{
			if(minDistance < prevMinDistance && minDistance > 0.0f)
			{
				prevMinDistance = minDistance;
				res = true;
				w = 1.0f - u - v;
				*pHit = rayOrigin + minDistance * rayDir;
				*nHit = u * normV1 + v * normV2 + w * normV0;
				*texCoords = u * texV1 + v * texV2 + w * texV0;
			}
		}
	}

	return res;
}

bool rayTriangleIntersect(
								 const float3 origin,
								 const float3 dir,
								 const float3 v0,
								 const float3 v1,
								 const float3 v2,
								 float* t,
								 float* u,
								 float* v)
{
	const float EPSILON = 0.0000001f;
	float3 v0v1 = v1 - v0;
	float3 v0v2 = v2 - v0;
	float3 pvec = cross(dir, v0v2);
	float det = dot(v0v1, pvec);

	// if ray is back facing, or is parallel to the triangle's plane, then return false
	if(det < EPSILON || fabs(det) < EPSILON)
		return false;

	float invDet = 1.0f / det;

	float3 tvec = origin - v0;
	*u = dot(tvec, pvec) * invDet;
	if(*u < 0.0f || *u > 1.0f)
		return false;

	float3 qvec = cross(tvec, v0v1);
	*v = dot(dir, qvec) * invDet;
	if(*v < 0.0f || *u + *v > 1.0f)
		return false;

	*t = dot(v0v2, qvec) * invDet;

	return true;
}

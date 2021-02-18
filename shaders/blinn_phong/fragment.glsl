#version 460 core

struct Light
{
	int type; // 0 => point, 1 => directional, 2 => spot
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;
	float kc;
	float kl;
	float kq;
	vec3 ambientStrength;
	vec3 diffuseStrength;
	vec3 specularStrength;
	mat4 lightSpaceMatrix;
};

struct Material
{
	vec4 color;
	float shininess;
	sampler2D diffuse;
	int hasDiffuse;
	sampler2D specular;
	int hasSpecular;
	sampler2D normal;
	int hasNormal;
	int nbTextures;
};

struct Camera
{
	vec3 viewPos;
};

in VS_OUT
{
	vec2 texCoords;
	vec3 normal;
	vec3 fragPos;
} fs_in;

uniform Camera cam;

uniform int shadowOn;
uniform int lightCount;
uniform Light light[10];
uniform sampler2D depthMap[10];
uniform samplerCube omniDepthMap[10];
uniform int pointLightCount;

uniform Material material;

out vec4 color;

float linearizeDepth(float depth)
{
	float near = 0.1f;
	float far = 100.0f;
	float ndc = depth * 2.0f - 1.0f;
	float z = (2.0f * near * far) / (far + near - ndc * (far - near));
	return z / far;
}

vec4 calculateDiffuse(vec3 lightDir, vec3 diffuseStrength, vec4 objColor)
{
	vec3 norm = normalize(fs_in.normal);
	float diff = max(dot(norm, -lightDir), 0.0f);
	return vec4(diffuseStrength * diff, 1.0f) * objColor;
}

vec4 calculateSpecular(vec3 viewPos, vec3 lightDir, vec3 specularStrength, vec4 objColor)
{
	vec3 viewDir = normalize(viewPos - fs_in.fragPos);
	vec3 halfwayDir = normalize(-lightDir + viewDir);
	vec3 reflectDir = reflect(lightDir, fs_in.normal);
	float spec = pow(max(dot(fs_in.normal, halfwayDir), 0.0f), material.shininess);
	return vec4(specularStrength * spec, 1.0f) * objColor;
}

float calculateShadow(vec4 fragPosLightSpace, vec3 lightDir, int l)
{
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(depthMap[l - pointLightCount], 0);
	float bias = max(0.005 * (1.0 - dot(fs_in.normal, -lightDir)), 0.0005);

	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
	// transform to [0,1] range
	projCoords = (projCoords * 0.5) + 0.5;
	if(projCoords.z > 1.0)
	{
		shadow = 0.0;
		return shadow;
	}
		
	// get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	
	// get closest depth value from light's perspective
	float closestDepth = texture(depthMap[l - pointLightCount], projCoords.xy).r;

	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(depthMap[l - pointLightCount], projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
		}
	}
	
	return shadow / 9.0;
}

float calculateOmniShadow(vec3 fragPos, vec3 lightPos, int l)
{
	float shadow = 0.0;
	float bias = 0.005;
	
	vec3 distFragLight = fragPos - lightPos;
	float closestDepth = texture(omniDepthMap[l], distFragLight).r;
	closestDepth *= 100.0;
	float currentDepth = length(distFragLight);

	shadow = (currentDepth - bias) > closestDepth ? 1.0 : 0.0;

	return 0.0;
}

void main()
{
	for(int l = 0; l < lightCount; ++l)
	{
		vec3 lightDir;
		float distance = length(light[l].position - fs_in.fragPos);
		float attenuation;
		float theta;
		float epsilon;
		float intensity;

		if(light[l].type == 0)
		{
			lightDir = normalize(fs_in.fragPos - light[l].position);
			attenuation = 1.0f / (light[l].kc + light[l].kl * distance + light[l].kq * (distance * distance));
		}
		else if(light[l].type == 1)
		{
			lightDir = light[l].direction;
		}
		else if(light[l].type == 2)
		{
			lightDir = normalize(fs_in.fragPos - light[l].position);
			theta = dot(lightDir, normalize(light[l].direction));
			epsilon = light[l].cutOff - light[l].outerCutOff;
			intensity = clamp((theta - light[l].outerCutOff) / epsilon, 0.0f, 1.0f);
		}

		// calculate shadow
		float shadow;
		if(shadowOn == 1 && light[l].type == 1)
			shadow = calculateShadow(light[l].lightSpaceMatrix * vec4(fs_in.fragPos, 1.0f), lightDir, l);
		else if(shadowOn == 1 && light[l].type == 0)
			shadow = calculateOmniShadow(fs_in.fragPos, light[l].position, l);
		else if(shadowOn == 0)
			shadow = 0.0f;

		if(material.nbTextures > 0)
		{
			// ambiant
			vec4 ambient = vec4(light[l].ambientStrength, 1.0f) * texture(material.diffuse, fs_in.texCoords);

			if(light[l].type == 2 && theta > light[l].outerCutOff)
			{
				// diffuse
				vec4 diffuse = calculateDiffuse(lightDir, light[l].diffuseStrength, texture(material.diffuse, fs_in.texCoords));

				// specular
				vec4 specular;
				if(material.hasSpecular == 1)
					specular = calculateSpecular(cam.viewPos, lightDir, light[l].specularStrength, texture(material.specular, fs_in.texCoords));
				else
					specular = calculateSpecular(cam.viewPos, lightDir, light[l].specularStrength, vec4(0.0));
				
				diffuse = vec4(vec3(diffuse) * intensity, diffuse.a) * (1.0 - shadow);
				specular = vec4(vec3(specular) * intensity, specular.a) * (1.0 - shadow);
			
				// putting it all together
				float alpha = ambient.a + diffuse.a + specular.a;
				color += vec4(vec3(ambient + diffuse + specular), alpha);
			}
			else if(light[l].type == 2)
			{
				color += ambient;
			}
			else
			{
				// diffuse
				vec4 diffuse = calculateDiffuse(lightDir, light[l].diffuseStrength, texture(material.diffuse, fs_in.texCoords)) * (1.0 - shadow);

				// specular
				vec4 specular;
				if(material.hasSpecular == 1)
					specular = calculateSpecular(cam.viewPos, lightDir, light[l].specularStrength, texture(material.specular, fs_in.texCoords)) * (1.0 - shadow);
				else
					specular = calculateSpecular(cam.viewPos, lightDir, light[l].specularStrength, vec4(0.0)) * (1.0 - shadow);

				// putting it all together
				float alpha = ambient.a + diffuse.a + specular.a;
				if(light[l].type == 0)
					color += vec4(vec3(ambient + diffuse + specular) * attenuation, alpha);
				else
					color += (ambient + diffuse + specular);
			}
		}

		else
		{
			// ambiant
			vec4 ambient = vec4(light[l].ambientStrength, 1.0f) * material.color;

			if(light[l].type == 2 && theta > light[l].outerCutOff)
			{
				// diffuse
				vec4 diffuse = calculateDiffuse(lightDir, light[l].diffuseStrength, material.color) * (1.0 - shadow);

				// specular
				vec4 specular = calculateSpecular(cam.viewPos, lightDir, light[l].specularStrength, material.color) * (1.0 - shadow);
				
				diffuse = vec4(vec3(diffuse) * intensity, diffuse.a);
				specular = vec4(vec3(specular) * intensity, specular.a);
			
				// putting it all together
				float alpha = ambient.a + diffuse.a + specular.a;
				color += vec4(vec3(ambient + diffuse + specular), alpha);
			}
			else if(light[l].type == 2)
			{
				color += ambient;
			}
			else
			{
				// diffuse
				vec4 diffuse = calculateDiffuse(lightDir, light[l].diffuseStrength, material.color) * (1.0 - shadow);

				// specular
				vec4 specular = calculateSpecular(cam.viewPos, lightDir, light[l].specularStrength, material.color) * (1.0 - shadow);

				// putting it all together
				float alpha = ambient.a + diffuse.a + specular.a;
				if(light[l].type == 0)
					color += vec4(vec3(ambient + diffuse + specular) * attenuation, alpha);
				else
					color += (ambient + diffuse + specular);
			}
		}
	}
}

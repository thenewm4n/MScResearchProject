// #version 450 core

#define MAX_SPOT_LIGHTS 5

struct DirectionalLight
{
    vec3 direction;
    vec3 colour;
    float intensity;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    vec3 colour;
    float intensity;
    float cutoffAngle;
};

// Inputs from vertex shader
in vec2 textureCoords;
in vec3 normal;
in vec3 fragPositionWorldSpace;


// Uniforms
    // Colours and Textures
uniform sampler2D diffuseTexture;
uniform bool useTexture;
uniform sampler2D specularMap;
uniform vec3 objectColour;              // Fallback for objects not using textures i.e. streetlights, cables

    // Lighting
uniform vec3 cameraPosition;
uniform int numPosLights;
uniform int numSpotLights;
uniform DirectionalLight moonlight;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

    // Distance fog
uniform vec3 skyColour;


// Pixel colour
out vec4 fragColour;


vec3 calculateDirectionalIllumination(DirectionalLight light, vec3 nNormal, vec3 nViewDirection, vec3 baseColour, float specularStrength)
{
	vec3 nToLight = normalize(-light.direction);
	float diffuse = max(0.0, dot(nToLight, nNormal));

	vec3 nReflectedLight = reflect(light.direction, nNormal);
	float specularAngle = max(0.0, dot(nReflectedLight, nViewDirection));
	float specular = pow(specularAngle, 32.0);

    vec3 illumination = light.intensity * light.colour  * ((diffuse * baseColour) + (specular * specularStrength));

	return illumination;
}

vec3 calculateSpotIllumination(SpotLight light, vec3 nNormal, vec3 nViewDirection, vec3 baseColour, float specularStrength)
{
	vec3 nToLight = normalize(light.position - fragPositionWorldSpace);
	float diffuse = max(0.0, dot(nToLight, nNormal));

	vec3 nReflectedLight = reflect(-nToLight, nNormal);
	float specularAngle = max(0.0, dot(nReflectedLight, nViewDirection));
	float specular = pow(specularAngle, 128.0);

	float distance = length(light.position - fragPositionWorldSpace);
	float attenuation = 1.0 / (1.0 + (0.14 * distance) + (0.07 * distance * distance));

	vec3 nSpotlightDirection = normalize(light.direction);
	float theta = dot(-nToLight, nSpotlightDirection);

	vec3 illumination;
	if (theta > light.cutoffAngle)
	{
		illumination = attenuation * light.intensity * light.colour * ((diffuse * baseColour) + (specular * specularStrength));
	}
	else
	{
		illumination = vec3(0.0);
	}

	return illumination;
}

void main()
{
    vec3 baseColour;
    float specularStrength = 0.0;

    if (useTexture)
    {
        baseColour = texture(diffuseTexture, textureCoords).rgb;
        specularStrength = texture(specularMap, textureCoords).r;
    }
    else
    {
        baseColour = objectColour;
        specularStrength = 0.5;
    }

	vec3 nNormal = normalize(normal);
    vec3 nViewDirection = normalize(cameraPosition - fragPositionWorldSpace);

    // Add ambient colour
    float ambientStrength = 0.1;
    vec3 finalColour = ambientStrength * baseColour;

    // Directional light
    finalColour += calculateDirectionalIllumination(moonlight, nNormal, nViewDirection, baseColour, specularStrength);

    // Spot lights
    for (int i = 0; i < numSpotLights; i++)
    {
        finalColour += calculateSpotIllumination(spotLights[i], nNormal, nViewDirection, baseColour, specularStrength);
    }

    fragColour = vec4(finalColour, 1.0);
}
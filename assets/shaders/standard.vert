// #version 450 core 

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTextureCoords; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int numParticlesLeftFrontalis;

out vec2 textureCoords;
out vec3 normal;
out vec3 fragPositionWorldSpace;


struct Particle
{
    vec4 initialPosition;       // w = fixed flag
    vec4 currentPosition;       // w = density
    vec4 velocity;              // w = pressure
    ivec4 clusterIDsA;
    ivec4 clusterIDsB;
};

struct ParticleMeshBinding
{
    ivec4 particleIDs;
    vec4 particleWeights;
};

layout(std430, binding = 0) buffer LeftParticleBuffer
{
    Particle leftFrontalisParticles[];
};

layout(std430, binding = 3) buffer RightParticleBuffer
{
    Particle rightFrontalisParticles[];
};

layout(std430, binding = 2) buffer BindingBuffer
{
    ParticleMeshBinding bindings[];
};


void main()
{
    ParticleMeshBinding vertexBinding = bindings[gl_VertexID];

    vec3 netDisplacement = vec3(0.0);
    float sumOfWeights = 0.0f;

    for (int i = 0; i < 4; i++)
    {
        int particleID = vertexBinding.particleIDs[i];
        if (particleID != -1)
        {
            vec3 currentParticleDisplacement;
            if (particleID >= numParticlesLeftFrontalis)
            {
                int rightParticleID = particleID - numParticlesLeftFrontalis;
                currentParticleDisplacement = rightFrontalisParticles[rightParticleID].currentPosition.xyz - rightFrontalisParticles[rightParticleID].initialPosition.xyz;
            }
            else
            {
                currentParticleDisplacement = leftFrontalisParticles[particleID].currentPosition.xyz - leftFrontalisParticles[particleID].initialPosition.xyz;
            }

            netDisplacement += vertexBinding.particleWeights[i] * currentParticleDisplacement;
            sumOfWeights += vertexBinding.particleWeights[i];
        }
    }

    vec3 newPosition = vertexPosition;
    if (sumOfWeights > 0.0)
    {
        newPosition += netDisplacement;
    }

    vec4 worldPosition = model * vec4(newPosition, 1.0);
    fragPositionWorldSpace = worldPosition.xyz;

    textureCoords = vertexTextureCoords;
    normal = mat3(transpose(inverse(model))) * vertexNormal;

    gl_Position = projection * view * worldPosition;
}
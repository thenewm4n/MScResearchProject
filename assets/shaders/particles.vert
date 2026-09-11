// #version 450 core 

struct Particle
{
    vec4 initialPosition;
    vec4 currentPosition;
    vec4 velocity;
    ivec4 clusterIDsA;
    ivec4 clusterIDsB;
};

layout(std430, binding = 0) buffer ParticleBuffer
{
    Particle particles[];
};

uniform mat4 view;
uniform mat4 projection;

out float fixedFlag;


void main()
{
    Particle p = particles[gl_VertexID];
    
    fixedFlag = p.initialPosition.w;

    vec4 viewSpacePos = view * vec4(p.currentPosition.xyz, 1.0);

    gl_Position = projection * viewSpacePos;
    gl_PointSize = 7.0f / -viewSpacePos.z;
}
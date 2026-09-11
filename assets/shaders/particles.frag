// #version 330 core

in float fixedFlag;
out vec4 fragColour;


void main()
{
    if (fixedFlag > 0.5)                            // Not fixedFlag == 1.0 because floating point inaccuracy on GPU
    {
        fragColour = vec4(0.8, 0.1, 0.1, 1.0);      // red
    }
    else
    {
        fragColour = vec4(0.2, 0.6, 1.0, 1.0);      // blue
    }
}
#version 330 core

layout (location = 0) in vec2 aPoint;

uniform vec2 uPos;
uniform vec2 uSize;
uniform float uRotation;
uniform float uAspect;

void main()
{
	vec2 p = aPoint * uSize;
	p.x *= uAspect;
	float c = cos(uRotation);
	float s = sin(uRotation);
	p = vec2(c * p.x - s * p.y, s * p.x + c * p.y);
	p.x /= uAspect;
	gl_Position = vec4(p + uPos, 0.0, 1.0);
}

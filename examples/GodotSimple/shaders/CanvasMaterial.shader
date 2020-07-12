shader_type canvas_item;

uniform float uMultiplier;

void vertex()
{
	VERTEX.x += cos(TIME) * uMultiplier;
	VERTEX.y += sin(TIME) * uMultiplier;
}
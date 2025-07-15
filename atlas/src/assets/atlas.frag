// Uniform variables (in order of importance):
uniform sampler2D Atlas;
uniform vec2 Resolution;
uniform float Scale;
uniform vec2 Position;
//uniform vec2 Mouse;
//uniform bvec2 MouseClick;
//uniform float Time;

layout (location = 0) out vec4 FragColor;

float N = 2048.0;

vec3 getAtlasColor(vec2 uv, float zoom) {
	return textureLod(Atlas, uv, log2(N*zoom)).xyz;
}

void main() {
	// View the entire Atlas centering the canvas at Scale = 1.0.
	float zoom = max(1.0/Resolution.x, 1.0/Resolution.y) / Scale;
	vec2 uv = (gl_FragCoord.xy - 0.5*Resolution) * zoom + Position;
	// Color everything else black.
	bool onAtlas = max(abs(uv.x - 0.5), abs(uv.y - 0.5)) <= 0.5;
	vec3 color = onAtlas? getAtlasColor(uv, zoom): vec3(0.06, 0.06, 0.06);
	// Output.
	FragColor = vec4(color, 1.0);
}

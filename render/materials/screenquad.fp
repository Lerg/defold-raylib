varying mediump vec4 position;
varying mediump vec2 var_texcoord0;

uniform lowp sampler2D original;

void main() {
	gl_FragColor = texture2D(original, var_texcoord0.xy);
}
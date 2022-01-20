#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0; // albedo
uniform sampler2D texture1; // metallness
uniform sampler2D texture2; // normal
uniform vec4 colDiffuse;

uniform int useMapNormal; // normal
uniform int useMapMetalness; // metallness

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

#define     MAX_LIGHTS              6
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct MaterialProperty {
    vec3 color;
    int useSampler;
    sampler2D sampler;
};

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);
	float metalnessValue = 1.0;

	if (useMapNormal == 1) {
		// obtain normal from normal map in range [0,1]
		normal = texture(texture2, fragTexCoord).rgb;
		// transform normal vector to range [-1,1]
		normal = normalize(normal * 2.0 - 1.0);
		normal.z = 0.6;
		normal = normalize(normal);
	}
	if (useMapMetalness == 1) {
		metalnessValue = texture(texture1, fragTexCoord).r;
	}

    // NOTE: Implement here your fragment shader code

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light = vec3(0.0);

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                light = -normalize(lights[i].target - lights[i].position);
            }

			float attenuation_inv = 1.0;
            if (lights[i].type == LIGHT_POINT)
            {
                light = normalize(lights[i].position - fragPosition);
				attenuation_inv = 1.0 - min(1.0, pow(length(lights[i].position - fragPosition) / 24.0, 4.0));
            }

            float NdotL = max(dot(normal, light), 0.0);
            lightDot += attenuation_inv * lights[i].color.rgb*NdotL;

            float specCo = 0.0;
            if (NdotL > 0.0) specCo = attenuation_inv * pow(max(0.0, dot(viewD, reflect(-(light), normal))), 16.0 * (0.5 + metalnessValue / 2.0)); // 16 refers to shine
            specular += specCo;
        }
    }

    finalColor = (texelColor*((colDiffuse + vec4(specular, 1.0))*vec4(lightDot, 1.0)));
    finalColor += texelColor*(ambient/10.0)*colDiffuse;

	// Gamma correction
    finalColor = pow(finalColor, vec4(1.0/2.2));

	// Fog calculation
    float dist = length(viewPos - fragPosition);

    // these could be parameters...
    const vec4 fogColor = vec4(0.1, 0.2, 0.5, 1.0);
    const float fogDensity = 0.16;

    // Linear fog (less nice)
    const float fogStart = 10.0;
    const float fogEnd = 100.0;
    float fogFactor = (fogEnd - dist)/(fogEnd - fogStart);

    fogFactor = clamp(fogFactor, 0.0, 1.0);

    finalColor = mix(fogColor, finalColor, fogFactor);
}

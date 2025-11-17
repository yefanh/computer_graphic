#version 330

const int MAX_LIGHTS = 10;

struct LightInfo {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec4 position;
    vec4 spotDirection;
    float spotCutoff;
};

struct MaterialInfo {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 emission;
    float shininess;
};

in vec3 fNormal;
in vec3 fPosition;
out vec4 fColor;

uniform int numLights;
uniform LightInfo lights[MAX_LIGHTS];
uniform MaterialInfo material;

// 0 = regular Phong, 1 = toon
uniform int toonMode;

vec3 applyLight(LightInfo light, vec3 normal) {
    vec3 color = vec3(0.0);
    vec3 n = normalize(normal);
    vec3 lightDir;
    if (light.position.w == 0.0) {
        lightDir = normalize(-light.position.xyz);
    } else {
        lightDir = normalize(light.position.xyz - fPosition);
    }

    float spotFactor = 1.0;
    if (light.spotCutoff > 0.0 && length(light.spotDirection.xyz) > 0.0 && light.position.w != 0.0) {
        vec3 lightToFragment = normalize(fPosition - light.position.xyz);
        vec3 spotDir = normalize(light.spotDirection.xyz);
        float cutoff = cos(radians(light.spotCutoff));
        float alignment = dot(-lightToFragment, spotDir);
        if (alignment < cutoff) {
            return vec3(0.0);
        }
        spotFactor = alignment;
    }

    float diff = max(dot(n, lightDir), 0.0);
    vec3 viewDir = normalize(-fPosition);
    vec3 reflectDir = reflect(-lightDir, n);
    float spec = 0.0;
    if (diff > 0.0 && material.shininess > 0.0) {
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }

    color += light.ambient * material.ambient;
    color += spotFactor * (light.diffuse * material.diffuse * diff + light.specular * material.specular * spec);
    return color;
}

float lambertForLight(LightInfo light, vec3 normal) {
    vec3 n = normalize(normal);
    vec3 lightDir;
    if (light.position.w == 0.0) {
        lightDir = normalize(-light.position.xyz);
    } else {
        lightDir = normalize(light.position.xyz - fPosition);
    }
    return max(dot(n, lightDir), 0.0);
}

vec3 phongShade(vec3 normal) {
    vec3 color = material.emission;
    int count = min(numLights, MAX_LIGHTS);
    for (int i=0; i<count; i++) {
        color += applyLight(lights[i], normal);
    }
    return color;
}

vec3 toonShade(vec3 normal) {
    int count = min(numLights, MAX_LIGHTS);
    float maxLambert = 0.0;
    for (int i=0; i<count; i++) {
        maxLambert = max(maxLambert, lambertForLight(lights[i], normal));
    }

    float level;
    if (maxLambert > 0.8) {
        level = 1.0;
    } else if (maxLambert > 0.4) {
        level = 0.6;
    } else if (maxLambert > 0.1) {
        level = 0.3;
    } else {
        level = 0.15;
    }

    vec3 base = material.diffuse;
    vec3 color = material.ambient + level * base;
    // keep emission as-is so self-emissive objects still glow
    color += material.emission;
    return color;
}

void main()
{
    vec3 normal = normalize(fNormal);
    vec3 color = (toonMode == 0) ? phongShade(normal) : toonShade(normal);
    fColor = vec4(color,1.0);
}

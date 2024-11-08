#include "common.hlsl"

struct Targets
{
    float4 color: SV_Target0;

    //float4 normal: SV_Target1;
};

float4 GROUND_FRAG(
    float4 pos            : SV_Position,
    float2 tex            : TEXCOORD0,
	float3 t              : TEXCOORD1,
	float3 b              : TEXCOORD2,
	float3 n              : TEXCOORD3,     
    float3 color          : TEXCOORD4
):SV_TARGET
{ 
   float3 _t = normalize(t);
   float3 _b = normalize(b);
   float3 _n = normalize(n);
   
   float3x3 tbn = { 
    	_t.x, _t.y, _t.z,
	    _b.x, _b.y, _b.z,
	    _n.x, _n.y, _n.z,
   }; 
   float3 lightDiffuseColor = float3(1.0, 1.0, 1.0);
   float3 lightDir = -float3(0.5, -0.75, 0.25);
   float colorTextureBrightness = 2.0;      

   float3 diffuceColor = diffuceMap.Sample(linearSampler, tex).xyz;
   diffuceColor = saturate(color * diffuceColor * colorTextureBrightness);

   float3 normal = decodeNormal(heightMap.Sample(linearSampler, tex));
   normal = normalize(mul(normal, tbn));

   float lightIntensity = saturate(dot(normal, lightDir));

   return float4(diffuceColor * saturate(lightDiffuseColor * lightIntensity), 1.0);
};
//////////////////////////////////////////////////////////////////
Targets SKYDOME_FRAG(
    float3 normal         : TEXCOORD1,
    float2 tex            : TEXCOORD2
):SV_TARGET
{ 
   Targets output;

   float height = normal.y;  //normal is unmodified position through to the pixel shader.

   // The value ranges from -1.0f to +1.0f so change it to only positive values.
   if(height < 0.0)
	  height = 0.0f;

   // Determine the gradient color by interpolating between the apex and center based on the height of the pixel in the sky dome.
   output.color = lerp(float4(0.02f, 0.365f, 0.886f, 1.0f), float4(0.0f, 0.145f, 0.667f, 1.0f), height);

   return output;
};
//////////////////////////////////////////////////////////////////
Targets SKYPLANE_FRAG(
    float3 normal         : TEXCOORD1,
    float2 tex            : TEXCOORD2
):SV_TARGET
{ 
   Targets output;
   float scale = 0.3;
   float brightness = 0.5;
   float translation = normal.x;

   tex.x = tex.x + translation;
   float4 perturbValue = heightMap.Sample(linearSampler, tex); // as perturb map
   perturbValue = perturbValue * scale;
   perturbValue.xy = tex.xy + perturbValue.xy + float2(translation, translation);
   
   float4 color = diffuceMap.Sample(linearSampler, perturbValue.xy);

   output.color = brightness * color;

   return output;
};

//////////////////////////////////////////////////////////////////
Targets WATER_FRAG(
    float3 waterParams    : TEXCOORD1,
    float2 tex            : TEXCOORD2,
    float4 tex1           : TEXCOORD3,
    float4 tex2           : TEXCOORD4,
    float3 viewDirection  : TEXCOORD5
):SV_TARGET
{ 
   Targets output;

   //////
   float padding1 = 0.0f;
   float2 normalMapTiling = float2(0.01f, 0.02f);
   float2 padding2 = float2(0.0f, 0.0f);
   float waterTranslation = waterParams.y;
   float reflectRefractScale = 0.03f;
   float4 refractionTint =  float4(0.0f, 0.8f, 1.0f, 1.0f);
   float3 lightDirection = float3(0.5f, -0.75f, 0.25f);
   float specularShininess = 200.0f;
   float2 padding = float2(0.0f, 0.0f);   
   float2 tex_1 = tex / normalMapTiling.x;
   float2 tex_2 = tex / normalMapTiling.y;
   //////

   //////water
   // Move the position the water normal is sampled from to simulate moving water.	
   tex_1.y += waterTranslation;
   tex_2.y += waterTranslation;
   
   // Sample the normal from the normal map texture using the two different tiled and translated coordinates.
   float4 normalMap1 = waterNormalMap.Sample(linearSampler, tex_1);
   float4 normalMap2 = waterNormalMap.Sample(linearSampler, tex_2);
   
   // Expand the range of the normal from (0,1) to (-1,+1).
   float3 normal1 = (normalMap1.rgb * 2.0f) - 1.0f;
   float3 normal2 = (normalMap2.rgb * 2.0f) - 1.0f;

   // Combine the normals to add the normal maps together.
   float3 normal = normalize(normal1 + normal2);
   //////water 
   
   float4 refrProj = tex1 / tex1.w;

   float2 refrUV = float2(refrProj.x * 0.5 + 0.5, refrProj.y * -0.5 + 0.5);
   //
   refrUV = refrUV + (normal.xy * reflectRefractScale);
   //
   float4 refrColor = saturate(heightMap.Sample(linearSampler, refrUV) * refractionTint);


   float4 reflProj = tex2 / tex2.w;

   float2 reflUV = float2(reflProj.x * 0.5 + 0.5, reflProj.y * -0.5 + 0.5);
   //
   reflUV = reflUV + (normal.xy * reflectRefractScale);
   //
   float4 reflColor = diffuceMap.Sample(linearSampler, reflUV);


   // Get a modified viewing direction of the camera that only takes into account height.
   float3 heightView;
   heightView.x = viewDirection.y;
   heightView.y = viewDirection.y;
   heightView.z = viewDirection.y;

   // Now calculate the fresnel term based solely on height.
   float r = (1.2f - 1.0f) / (1.2f + 1.0f);
   float fresnelFactor = max(0.0f, min(1.0f, r + (1.0f - r) * pow(1.0f - dot(normal, heightView), 2)));

   float4 resultColor =  lerp(reflColor, refrColor, fresnelFactor);

   //specular
   float3 reflection = -reflect(normalize(lightDirection), normal);
   float specular = dot(normalize(reflection), normalize(viewDirection));
   if(specular > 0.0f)
   {
     //specular = pow(specular, specularShininess);
     //resultColor = saturate(resultColor + specular);
   }
   //specular

   output.color = resultColor;////reflColor;//float4(1,1,1,1);
   
   return output;
};
/////////////////////////////////////////////////////////////////////

void compute_ground_normal_stage1(in float4 frag: SV_POSITION)
{ 
    int i = frag.x;
    int j = frag.y;
    float size = terrainSize;
    if(i<terrainSize && j<terrainSize){
        float3 v1 = float3(    i/size, 255.0*heightMap.Load( int3(i,   j, 0) ).y,       j/size);
        float3 v2 = float3(    i/size, 255.0*heightMap.Load( int3(i,   j+1, 0) ).y, (j+1)/size);
        float3 v3 = float3((i+1)/size, 255.0*heightMap.Load( int3(i+1, j, 0) ).y,       j/size);

        faceNormalBuffer[int2(frag.xy)] = cross(v2-v1, v3-v2);
    }
}

/////////////////////////////////////////////////////////////////////

void compute_ground_normal_stage2(in float4 frag: SV_POSITION)
{ 
    int normalCount = 0;
    float3 avgNormal = float3(0,0,0);

    for(int i = int(frag.x)-1; i<int(frag.x)+1; i++){
        for(int j = int(frag.y)-1; j<int(frag.y)+1; j++){
            if(0<=i&&i<terrainSize&&0<=j&&j<terrainSize){  
                normalCount++;  
                avgNormal += faceNormalMap.Load(int3(i, j, 0));
            }
        }
    }

    avgNormal /= normalCount;

    vertexNormalBuffer[int2(frag.xy)] = normalize(avgNormal);
}

//////////////////////////////////////////////////////////////////////

Targets RENDER_TBN_PS(
    float4 color          : TEXCOORD1
):SV_TARGET
{ 
   Targets output;

   output.color = color;

   return output;
};
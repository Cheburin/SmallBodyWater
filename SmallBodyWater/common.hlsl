SamplerState linearSampler : register( s0 );

Texture2D<float4> heightMap            : register( t0 );

Texture2D<float4> diffuceMap           : register( t1 );

Texture2D<float3> faceNormalMap        : register( t2 );

Texture2D<float3> vertexNormalMap      : register( t3 );

Texture2D<float4> waterNormalMap       : register( t4 );

RWTexture2D<float3> faceNormalBuffer   : register( u1 );

RWTexture2D<float3> vertexNormalBuffer : register( u2 );

#define terrainSize 511

float4 encodeNormal(float3 normal){
   float3 _half = float3(0.5,0.5,0.5); 
   return float4(_half*normal+_half,1);
}

float3 decodeNormal(float4 normal){
   float3 _half = float3(0.5,0.5,0.5); 
   return (normal.xyz - _half)/_half;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

//float3 getGridNodePosition(int x, int y){
//  return float3(1.0/terrainSize, 255.0, 1.0/terrainSize) * float3(x, heightMap.Load( int3(x, y, 0) ).y, y);
//}

//float3 getGridNodeNormal(int x, int y){
//  return decodeNormal(normalMap.Load( int3(x, y, 0) ));
//}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
float3 getGridNodeColor2(float x, float y){
  return heightMap.Load( int3(x*terrainSize, y*terrainSize, 0) ).xyz;
}

float3 getGridNodePosition2(float x, float y){
  return float3(1.0, 255.0, 1.0) * float3(x, heightMap.Load( int3(x*terrainSize, y*terrainSize, 0) ).y, y);
}

float3 getGridNodeNormal2(float x, float y){
  return vertexNormalMap.Load( int3(x*terrainSize, y*terrainSize, 0) );  
}

//float3 getGridNodeTangent2(float x, float y){
//  return tangentMap.Load( int3(x*terrainSize, y*terrainSize, 0) );
//}

//float3 getGridNodeBinormal2(float x, float y){
//  return binormalMap.Load( int3(x*terrainSize, y*terrainSize, 0) );
//}

void CalculateTangentBinormal(in float3 vertex1, in float3 vertex2, in float3  vertex3,
                         in float2 tex1, in float2 tex2, in float2 tex3, 
                         out float3  tangent, out float3 binormal){
    float3 v1 = vertex2 - vertex1;                            
    float3 v2 = vertex3 - vertex1;
    float2 t1 = tex2 - tex1;                            
    float2 t2 = tex3 - tex1;
	float2x3 v1_2 = { v1.x, v1.y, v1.z, v2.x, v2.y, v2.z };

	float1x2 tn_arg = { t2.y, -t1.y };
	float3 tn = mul(tn_arg, v1_2);
    //tn.x = dot(float2(t2.y, -t1.y), float2(v1.x, v2.x));
    //tn.y = dot(float2(t2.y, -t1.y), float2(v1.y, v2.y));
    //tn.z = dot(float2(t2.y, -t1.y), float2(v1.z, v2.z));
	float1x2 bn_arg = { -t2.x, t1.x };
	float3 bn = mul(bn_arg, v1_2);
    //bn.x = dot(float2(-t2.x, t1.x), float2(v1.x, v2.x));
    //bn.y = dot(float2(-t2.x, t1.x), float2(v1.y, v2.y));
    //bn.z = dot(float2(-t2.x, t1.x), float2(v1.z, v2.z));

    float den = 1.0/((t1.x*t2.y)-(t2.x*t1.y));

    tangent = normalize(tn*den);
    binormal = normalize(bn*den);
}
cbuffer cbMain : register( b0 )
{
	matrix    g_mWorld;                         // World matrix
	matrix    g_mView;                          // View matrix
	matrix    g_mProjection;                    // Projection matrix
	matrix    g_mWorldViewProjection;           // WVP matrix
	matrix    g_mWorldView;                     // WV matrix
	matrix    g_mInvView;                       // Inverse of view matrix

	matrix    g_mObject1;                // VP matrix
	matrix    g_mObject1WorldView;                       // Inverse of view matrix
	matrix    g_mObject1WorldViewProjection;                       // Inverse of view matrix

	matrix    g_mObject2;                // VP matrix
	matrix    g_mObject2WorldView;                       // Inverse of view matrix
	matrix    g_mObject2WorldViewProjection;                       // Inverse of view matrix

	float4    g_vFrustumNearFar;              // Screen resolution
	float4    g_vFrustumParams;              // Screen resolution
	float4    g_viewLightPos;                   //
};

cbuffer cbWaterDef : register( b1 )
{
    float4 g_water_plane;
}

struct PosNormalTex2d
{
    float3 pos : SV_Position;
    float3 normal   : NORMAL;
    float2 tex      : TEXCOORD0;
};

struct ClipPosTBNTex2d
{
    float4 clip_pos       : SV_Position; // Output position
	float4 color          : COLOR0;
    float2 tex            : TEXCOORD0;
    float3 t        	  : TANGENT;   // Normal vector in world space
    float3 b         	  : BINORMAL;   // Normal vector in world space
    float3 n         	  : NORMAL;   // Normal vector in world space
};

struct ClipPosNormalTex2d
{
    float3 normal         : TEXCOORD1;   // Normal vector in world space
    float2 tex            : TEXCOORD2;
    float4 clip_pos       : SV_POSITION; // Output position
    float clip            : SV_ClipDistance0;
};

//////
struct ClipPosNormalTex2d2xTex4dCamPosWorldPos
{
    float3 waterParams    : TEXCOORD1;   // Normal vector in world space
    float2 tex            : TEXCOORD2;
    float4 tex1           : TEXCOORD3;
    float4 tex2           : TEXCOORD4;
    float3 viewDirection  : TEXCOORD5;
    float4 clip_pos       : SV_POSITION; // Output position
    float clip            : SV_ClipDistance0;
};
//////

struct OutPutClipPosTBNTex2d
{
    float4 clip_pos       : SV_Position; // Output position
    float2 tex            : TEXCOORD0;
	float3 t              : TEXCOORD1;
	float3 b              : TEXCOORD2;
	float3 n              : TEXCOORD3;  
	float3 color          : TEXCOORD4;     
    float clip            : SV_ClipDistance0;
};

struct ClipPosColor
{
    float4 color          : TEXCOORD1;
    float4 clip_pos       : SV_POSITION; // Output position
};

struct ExpandPos
{
    float4 pos      : SV_POSITION;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "common.hlsl"
///////////////////////////////////////////////////////////////////////////////////////////////////
OutPutClipPosTBNTex2d GROUND_VERTEX( in ClipPosTBNTex2d i )
{
    OutPutClipPosTBNTex2d output;

	output.tex = i.tex;
	output.color = i.color.xyz;
    output.clip_pos = mul( i.clip_pos, g_mWorldViewProjection );
	output.t = normalize(mul(i.t, (float3x3)g_mWorld));
	output.b = normalize(mul(i.b, (float3x3)g_mWorld));
	output.n = normalize(mul(i.n, (float3x3)g_mObject1));

    output.clip = dot( mul(i.clip_pos, g_mWorld ), g_water_plane );

    return output;
}; 
///////////////////////////////////////////////////////////////////////////////////////////////////
ClipPosNormalTex2d SKYDOME_VERTEX( in PosNormalTex2d i )
{
    ClipPosNormalTex2d output;

    output.clip_pos = mul( float4( i.pos, 1.0 ), g_mWorldViewProjection );
   
    output.normal = i.pos;

    output.clip = dot( mul(float4( i.pos, 1.0 ), g_mWorld ), g_water_plane );

    return output;
}; 
///////////////////////////////////////////////////////////////////////////////////////////////////
ClipPosNormalTex2d SKYPLANE_VERTEX( in PosNormalTex2d i )
{
    ClipPosNormalTex2d output;

    float3 pos = i.pos; 

    pos.xz -= float2(0.5, 0.5);

    output.tex = float2(i.pos.x*2, i.pos.z*2);
     
    output.normal = float3(g_viewLightPos.xxx);

    output.clip_pos = mul( float4( pos, 1.0 ), g_mWorldViewProjection );

    output.clip = dot( mul(float4( pos, 1.0 ), g_mWorld ), g_water_plane );

    return output;
}; 
///////////////////////////////////////////////////////////////////////////////////////////////////
ClipPosNormalTex2d2xTex4dCamPosWorldPos WATER_VERTEX( in PosNormalTex2d i )
{
    ClipPosNormalTex2d2xTex4dCamPosWorldPos output;

    float3 pos = i.pos; 

    pos.xz -= float2(0.5, 0.5);

    output.waterParams = float3(g_viewLightPos.yyy);

    output.viewDirection = normalize(g_mInvView._m30_m31_m32 - mul(float4( pos, 1.0 ), g_mWorld ).xyz);

    output.tex = i.pos.xz;

    output.clip_pos = mul( float4( pos, 1.0 ), g_mWorldViewProjection );

    output.tex1 = mul( float4( pos, 1.0 ), g_mWorldViewProjection );

    output.tex2 = mul( float4( pos, 1.0 ), g_mObject1WorldViewProjection );

    output.clip = dot( mul(float4( pos, 1.0 ), g_mWorld ), g_water_plane );

    return output;
}; 
/////////////////////////////////////////////////////////////////////////////////////////////////////

ExpandPos _p(float x, float y){
  ExpandPos p;
  p.pos = float4(x, y, 0.5, 1); 
  return p;
}

ExpandPos SCREEN_SPACE_QUAD_VS(float3 pos : SV_Position) 
{
  ExpandPos output;
  output.pos = float4(pos.xyz, 1);
  return output;
} 

[maxvertexcount(4)]
void SCREEN_SPACE_QUAD_GS(point ExpandPos pnt[1], uint primID : SV_PrimitiveID,  inout TriangleStream<ExpandPos> triStream )
{
	triStream.Append(_p(-1, -1));
	triStream.Append(_p(-1,  1));
	triStream.Append(_p( 1, -1));
	triStream.Append(_p( 1,  1));
    triStream.RestartStrip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClipPosTBNTex2d MAKE_GROUND_VS( in PosNormalTex2d i )
{
  ClipPosTBNTex2d output;

  float3 pos = getGridNodePosition2(i.pos.x, i.pos.z) - float3(0.5, 0, 0.5); 

  float3 normal = getGridNodeNormal2(i.pos.x, i.pos.z);

  float3 color = getGridNodeColor2(i.pos.x, i.pos.z);

  output.clip_pos = float4(pos,1);

  output.color = float4(color,1);

  output.tex = float2(i.pos.x*terrainSize, i.pos.z*terrainSize);

  output.t = float3(0,0,0);
  
  output.b = float3(0,0,0);

  output.n = normal;

  return output;
}   

[maxvertexcount(3)]
void MAKE_GROUND_GS(triangle ClipPosTBNTex2d input[3], uint primID : SV_PrimitiveID, inout TriangleStream<ClipPosTBNTex2d> SpriteStream){
    float3 tangent = float3(0,0,0); 
    float3 binormal = float3(0,0,0);	
	
    float3 v1 = input[0].clip_pos.xyz; float2 tex1 = input[0].tex;
    float3 v2 = input[1].clip_pos.xyz; float2 tex2 = input[1].tex;
    float3 v3 = input[2].clip_pos.xyz; float2 tex3 = input[2].tex;

	CalculateTangentBinormal(v1, v2, v3, tex1, tex2, tex3, tangent, binormal);

	ClipPosTBNTex2d output = input[0];
	output.t = tangent;
	output.b = binormal;
	SpriteStream.Append( output );

	output = input[1];
	output.t = tangent;
	output.b = binormal;
	SpriteStream.Append( output );

	output = input[2];
	output.t = tangent;
	output.b = binormal;
	SpriteStream.Append( output );

	SpriteStream.RestartStrip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClipPosTBNTex2d RENDER_TBN_VS( in ClipPosTBNTex2d i )
{
    ClipPosTBNTex2d output;

	output = i;

	return output;
}

void axis(in float4 color, in float4 position, in float4 direction, inout LineStream<ClipPosColor> SpriteStream){
    ClipPosColor output;

    output.color = color;
    output.clip_pos = mul(mul( position, g_mView ), g_mProjection );
    SpriteStream.Append( output );

    output.color = color;
    output.clip_pos = mul(mul(position + 1.0 * direction, g_mView ), g_mProjection );
    SpriteStream.Append( output );

    SpriteStream.RestartStrip();
}

[maxvertexcount(6)]
void RENDER_TBN_GS(point ClipPosTBNTex2d input[1], uint primID : SV_PrimitiveID, inout LineStream<ClipPosColor> SpriteStream){
  axis(float4(1,0,0,1), mul(input[0].clip_pos, g_mWorld), float4(normalize(mul(input[0].t, (float3x3)g_mWorld)), 0), SpriteStream);
  axis(float4(0,1,0,1), mul(input[0].clip_pos, g_mWorld), float4(normalize(mul(input[0].b, (float3x3)g_mWorld)), 0), SpriteStream);
  axis(float4(0,0,1,1), mul(input[0].clip_pos, g_mWorld), float4(normalize(mul(input[0].n, (float3x3)g_mObject1)), 0), SpriteStream);

  //axis(float4(0,0,1,1), mul(input[0].clip_pos, g_mWorld), float4(input[0].n, 0), SpriteStream);
}
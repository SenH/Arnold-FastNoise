// MIT License

// Copyright (c) 2017 Sen Haerens

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <ai.h>
#include "FastNoise.h"

#define SHADER_INFO "FastNoise was originally developed by Jordan Peck as an open-source noise library. Source code ported to Arnold by Sen Haerens."
AI_SHADER_NODE_EXPORT_METHODS(fastNoiseMethods)

enum noiseSpace
{ 
	NS_WORLD, 
	NS_OBJECT, 
	NS_PREF, 
	NS_UV,
};

static const char* noiseSpaceNames[] =
{
	"world", 
	"object", 
	"Pref", 
	"UV", 
	NULL
};

static const char* noiseTypeNames[] = 
{
	"Value", 
	"Value Fractal", 
	"Perlin", 
	"Perlin Fractal", 
	"Simplex", 
	"Simplex Fractal", 
	"Cellular", 
	"Cellular Fractal",
	"White Noise",
	"Cubic", 
	"Cubic Fractal", 
	NULL
};
	
static const char* fractalTypeNames[] = 
{
	"FBM", 
	"Billow", 
	"Ridge Multi", 
	NULL
};

static const char* cellularDistanceFunctionNames[] = 
{
	"Euclidean", 
	"Manhattan", 
	"Natural",
	NULL
};

static const char* cellularReturnTypeNames[] = 
{
	"CellValue", 
	"NoiseLookup", 
	"Distance", 
	"Distance2", 
	"Distance2Add", 
	"Distance2Sub", 
	"Distance2Mul", 
	"Distance2Div",
	NULL
};

enum positionWarp
{
	PW_OFF,
	PW_ON,
	PW_FRACTAL,
};

static const char* positionWarpNames[] = 
{
	"Off", 
	"On", 
	"Fractal", 
	NULL
};

enum fastNoiseParams
{
	p_space,
	p_offset,
	p_scale,
	p_rotate,
	p_P,
	
	p_noise_type,
	p_seed,
	p_frequency,

	p_fractal_type,
	p_fractal_octaves,
	p_fractal_lacunarity,
	p_fractal_gain,

	p_cellular_distance_function,
	p_cellular_return_type,
	p_cellular_jitter,

	p_position_warp,
	p_position_warp_amplitude,
	p_info
};

node_parameters
{
	AiParameterEnum("space", NS_OBJECT, noiseSpaceNames);
	AiParameterVec("offset", 0.0f, 0.0f, 0.0f);
	AiParameterVec("scale", 1.0, 1.0, 1.0);
	AiParameterVec("rotate", 0.0f, 0.0f, 0.0f);
	AiParameterVec("P", 0.0f, 0.0f, 0.0f);
	
	AiParameterEnum("noise_type", FastNoise::NoiseType::SimplexFractal, noiseTypeNames);
	AiParameterInt("seed", 1337);
	AiParameterFlt("frequency", 2.0f);

	AiParameterEnum("fractal_type", FastNoise::FractalType::FBM, fractalTypeNames);
	AiParameterInt("fractal_octaves", 3);
	AiParameterFlt("fractal_lacunarity", 2.0f);
	AiParameterFlt("fractal_gain", 0.5f);

	AiParameterEnum("cellular_distance_function", 
					FastNoise::CellularDistanceFunction::Euclidean, cellularDistanceFunctionNames);
	AiParameterEnum("cellular_return_type", 
					FastNoise::CellularReturnType::CellValue, cellularReturnTypeNames);
	AiParameterFlt("cellular_jitter", 0.45f);

	AiParameterEnum("position_warp", PW_OFF, positionWarpNames);
	AiParameterFlt("position_warp_amplitude", 0.25f);
	AiParameterStr("info", SHADER_INFO);
}

struct ShaderData
{
	int space;
	FastNoise fn;
	FastNoise cn;
};

node_initialize
{
	ShaderData* data = new ShaderData;
	AiNodeSetLocalData(node, data);
}

node_update
{
	ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);
	
	data->space = AiNodeGetInt(node, "space");

	data->fn.SetNoiseType(static_cast<FastNoise::NoiseType>(AiNodeGetInt(node, "noise_type")));
	data->fn.SetSeed(AiNodeGetInt(node, "seed"));
	data->fn.SetFrequency(AiNodeGetFlt(node, "frequency"));
	
	data->fn.SetFractalType(static_cast<FastNoise::FractalType>(AiNodeGetInt(node, "fractal_type")));
	data->fn.SetFractalOctaves(AiNodeGetInt(node, "fractal_octaves"));
	data->fn.SetFractalLacunarity(AiNodeGetFlt(node, "fractal_lacunarity"));
	data->fn.SetFractalGain(AiNodeGetFlt(node, "fractal_gain"));
	
	data->fn.SetCellularDistanceFunction(
			static_cast<FastNoise::CellularDistanceFunction>(AiNodeGetInt(node, "cellular_distance_function")));
	data->fn.SetCellularReturnType(
			static_cast<FastNoise::CellularReturnType>(AiNodeGetInt(node, "cellular_return_type")));
	data->fn.SetCellularJitter(AiNodeGetFlt(node, "cellular_jitter"));

	if (data->fn.GetCellularReturnType() == FastNoise::CellularReturnType::NoiseLookup)
	{
		data->cn.SetNoiseType(FastNoise::NoiseType::Perlin);
		data->cn.SetSeed(data->fn.GetSeed());
		data->cn.SetFrequency(data->fn.GetFrequency());

		data->fn.SetCellularNoiseLookup(&data->cn);
	}
	
	data->fn.SetGradientPerturbAmp(AiNodeGetFlt(node, "position_warp_amplitude"));
}

node_finish
{
	ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);
	delete data;
}

shader_evaluate
{
	AtVector P, rot;
	AtVector Pin = AiShaderEvalParamVec(p_P);
	ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);
	
	// Evaluate coordinates
	static AtString str_Pref("Pref");
	if (AiNodeIsLinked(node, "P"))
	{
		P = Pin;
	}
	else
	{
		switch (data->space)
		{
		case NS_OBJECT:
			P = sg->Po;
			break;
		case NS_UV:
			P.x = sg->u;
			P.y = sg->v;
			P.z = 0.0f;
			break;
		case NS_PREF:
			if (!AiUDataGetVec(str_Pref, P))
				P = sg->Po;
			break;
		default:
			P = sg->P;
			break;
		}
	}
	
	P += AiShaderEvalParamVec(p_offset);
	P *= AiShaderEvalParamVec(p_scale);
	rot = AiShaderEvalParamVec(p_rotate);
	if (rot != AI_P3_ZERO) 
		P = AiM4VectorByMatrixMult(AiM4Mult(AiM4Mult(AiM4RotationX(rot.x), AiM4RotationY(rot.y)), AiM4RotationZ(rot.z)), P);

	// Evaluate noise
	switch (AiShaderEvalParamInt(p_position_warp))
	{
		case PW_ON:
			data->fn.GradientPerturb(P.x, P.y, P.z);
			break;
		case PW_FRACTAL:
			data->fn.GradientPerturbFractal(P.x, P.y, P.z);
			break;
		default:
			break;
	}

	sg->out.FLT() = (data->fn.GetNoise(P.x, P.y, P.z) + 1.0) / 2.0;
}

node_loader
{
	if (i > 0)
		return false;

	node->methods	  = fastNoiseMethods;
	node->output_type = AI_TYPE_FLOAT;
	node->name		  = "FastNoise";
	node->node_type	  = AI_NODE_SHADER;
	strcpy(node->version, AI_VERSION);
	return true;
}
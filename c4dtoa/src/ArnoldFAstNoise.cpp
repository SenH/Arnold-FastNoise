#include "ArnoldFAstNoise.h"

#include "ainode_FastNoise.h"

void ArnoldFAstNoise::InitValues(GeListNode* node)
{
   return ArnoldShaderGvOperatorData::InitValues(node);
}

String ArnoldFAstNoise::GetAiNodeEntryName(BaseList2D* node)
{
   return String("FastNoise");
}

Bool ArnoldFAstNoise::Message(GeListNode* node, Int32 type, void* data)
{
   return ArnoldShaderGvOperatorData::Message(node, type, data);
}


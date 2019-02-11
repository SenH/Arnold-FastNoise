#include "ArnoldFAstNoiseTranslator.h"

#include "ainode_FastNoise.h"

ArnoldFAstNoiseTranslator::ArnoldFAstNoiseTranslator(BaseList2D* node, RenderContext* context) : AbstractGvShaderTranslator(FASTNOISE_TRANSLATOR, node, context)
{
}

char* ArnoldFAstNoiseTranslator::GetAiNodeEntryName()
{
   return "FastNoise";
}

void ArnoldFAstNoiseTranslator::InitSteps(int nsteps)
{
   // init all node array parameters
   AbstractGvShaderTranslator::InitSteps(nsteps);

   BaseList2D* node = (BaseList2D*)GetC4DNode();
   if (!m_aiNode || !node) return;
}

void ArnoldFAstNoiseTranslator::Export(int step)
{
   // exports all node parameters
   AbstractGvShaderTranslator::Export(step);

   BaseList2D* node = (BaseList2D*)GetC4DNode();
   if (!m_aiNode || !node) return;

   // first motion step
   if (step == 0)
   {
   }
}


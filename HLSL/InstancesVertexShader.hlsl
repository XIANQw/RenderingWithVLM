#include "RenderSponza.hlsli"


InstancesVertexPosHWNormal InstancesVS(InstancesPosNormal vIn)
{
    InstancesVertexPosHWNormal vOut;
    matrix viewProj = mul(g_View, g_Proj);
    float4 posW = mul(float4(vIn.PosL, 1.0f), vIn.World);

    if (g_SHMode == 0 && g_UseSH)
    {
        // Compute indirection UVs from world position
        float3 BrickUV = ComputeVolumetricLightmapBrickTextureUVs((float3)posW);

        SHCoefs2BandRGB IrradianceSH = GetVolumetricLightmapSH2(BrickUV);
        vOut.VertexIndirectSH[0] = IrradianceSH.R.coefs1_4;
        vOut.VertexIndirectSH[1] = IrradianceSH.G.coefs1_4;
        vOut.VertexIndirectSH[2] = IrradianceSH.B.coefs1_4;
    }

    //����ǰ�ڻ�����Ӱ���Ƚ���ͶӰ����
    [flatten]
    if (g_IsShadow)
    {
        posW = mul(posW, g_Shadow);
    }

    vOut.PosH = mul(posW, viewProj);
    vOut.PosW = posW.xyz;
    vOut.NormalW = mul(vIn.NormalL, (float3x3) vIn.WorldInvTranspose);
    return vOut;
}
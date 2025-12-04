#ifndef __SHADER_LIST_H__
#define __SHADER_LIST_H__

#include "Shader.h"
#include "../Model.h"


class ShaderList
{
	// 定義
public:
	enum VSKind
	{
		VS_WORLD, // SetWVP
		VS_ANIME, // SetWVP, SetBones
		VS_KIND_MAX
	};
	enum PSKind
	{
		PS_UNLIT,			// SetMaterial
		PS_LAMBERT,			// SetMaterial, SetLight
		PS_SPECULAR,		// SetMaterial, SetLight, SetCameraPos
		PS_CUSTOM_LAMBERT,	// SetMaterial, SetLight
		PS_CUSTOM_SPECULAR,	// SetMaterial, SetLight, SetCameraPos
		PS_TOON,			// SetMaterial, SetLight
		PS_FOG,				// SetMaterial, SetLight, SetFog
		PS_KIND_MAX
	};


public:
	ShaderList();
	~ShaderList();

	static void Init();
	static void Uninit();

	// シェーダー設定
	static VertexShader* GetVS(VSKind vs);
	static PixelShader* GetPS(PSKind ps);

	// 定数バッファへの設定
	static void SetWVP(DirectX::XMFLOAT4X4* wvp);
	static void SetBones(DirectX::XMFLOAT4X4* bones200);
	static void SetMaterial(const Model::Material& material);
	static void SetLight(DirectX::XMFLOAT4 color, DirectX::XMFLOAT3 dir);
	static void SetCameraPos(const DirectX::XMFLOAT3 pos);
	static void SetFog(DirectX::XMFLOAT4 color, float start, float range);
	
	/**
	* @brief 3D カメラ行列からライト方向とカメラ位置を設定するヘルパ
	* @param viewT 転置済みビュー行列 (FollowCameraSystem::GetView() をそのまま渡す想定)
	* @param projT 転置済みプロジェクション行列 (今は未使用だが将来用に残す)
	*/
	static void SetL(DirectX::XMFLOAT4X4 viewT, DirectX::XMFLOAT4X4 projT);

private:
	static void MakeWorldVS();
	static void MakeAnimeVS();
	static void MakeUnlitPS();
	static void MakeLambertPS();
	static void MakeSpecularPS();
	static void MakeCustomLambertPS();
	static void MakeCustomSpecularPS();
	static void MakeToonPS();
	static void MakeFogPS();

private:
	static VertexShader* m_pVS[VS_KIND_MAX];
	static PixelShader* m_pPS[PS_KIND_MAX];
	
};

#endif // __SHADER_LIST_H__
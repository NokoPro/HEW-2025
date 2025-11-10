/*****************************************************************//**
 * @file   CameraMath.h
 * @brief  3Dカメラ用のビュー行列・射影行列を生成するユーティリティ
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include <DirectXMath.h>
#include "ECS/Components/Core/Camera3DComponent.h"

 /**
  * @brief Camera3DComponent からビュー行列を生成します
  * @param cam カメラコンポーネント
  * @return ビュー行列
  */
DirectX::XMMATRIX MakeViewMatrix(const Camera3DComponent& cam);

/**
 * @brief Camera3DComponent から透視射影行列を生成します
 * @param cam カメラコンポーネント
 * @return 射影行列
 */
DirectX::XMMATRIX MakeProjMatrix(const Camera3DComponent& cam);

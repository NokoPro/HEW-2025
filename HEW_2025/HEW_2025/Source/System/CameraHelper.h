/*****************************************************************//**
 * @file   CameraHelper.h
 * @brief  Camera3DComponent を画面サイズから補完するユーティリティ
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include "ECS/Components/Core/Camera3DComponent.h"

 /**
  * @brief 画面サイズからカメラのアスペクト比を設定します
  * @param cam   更新するカメラ
  * @param width 画面横幅
  * @param height画面縦幅
  */
inline void Camera_SetAspectFromSize(Camera3DComponent& cam, float width, float height)
{
    if (height <= 0.0f)
    {
        return;
    }
    cam.aspect = width / height;
}

/*****************************************************************//**
 * @file   BackGroundScrollComponent.h
 * @brief  背景スクロールのための情報を保持するコンポーネント
 *         背景の上が無くなったら下の画像を上の画像
 *
 * @author 土本蒼翔
 * @date   2025/12/01
 *********************************************************************/
#pragma once

struct BackGroundScrollComponent
{ 
    bool UpWarp = false;   // 背景を上にワープ
    bool DownWarp = false; // 背景を下にワープ

};

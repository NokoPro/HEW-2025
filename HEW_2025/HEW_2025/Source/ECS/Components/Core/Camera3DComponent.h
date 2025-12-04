/*****************************************************************//**
 * @file   Camera3DComponent.h
 * @brief  3Dシーンを描画するためのカメラ情報を保持するコンポーネント
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include <DirectXMath.h>
#include <cstdint>
#include "ECS/ECS.h"  // EntityId

 /**
  * @brief サイドビュー縦スクロール＋一般的な3Dカメラの設定をまとめたコンポーネント
  * 
  * * このコンポーネントは以下のモードをサポートします:
  * - Orbit: ターゲットの周囲を回るオービットカメラ
  * - SideScroll: 縦スクロールのサイドビュー（主に2Dライクなフォロー挙動）
  * - Fixed: 固定サイドビュー（並行投影）
  */
struct Camera3DComponent
{
    // =========================
    // 純粋なカメラパラメータ
    // =========================

    /** @brief カメラ位置 (eye) の X 座標 */
    float eyeX = 0.0f;
    /** @brief カメラ位置 (eye) の Y 座標 */
    float eyeY = 5.0f;
    /** @brief カメラ位置 (eye) の Z 座標 */
    float eyeZ = -10.0f;

    /** @brief 注視点 (target) の X 座標 */
    float targetX = 0.0f;
    /** @brief 注視点 (target) の Y 座標 */
    float targetY = 0.0f;
    /** @brief 注視点 (target) の Z 座標 */
    float targetZ = 0.0f;

    /** @brief カメラの上方向ベクトルの X 成分 */
    float upX = 0.0f;
    /** @brief カメラの上方向ベクトルの Y 成分 */
    float upY = 1.0f;
    /** @brief カメラの上方向ベクトルの Z 成分 */
    float upZ = 0.0f;

    /** @brief 垂直視野角 (degrees) */
    float fovY = 60.0f;
    /** @brief アスペクト比 (width / height) */
    float aspect = 16.0f / 9.0f;
    /** @brief ニアクリップ距離 */
    float nearZ = 0.1f;
    /** @brief ファークリップ距離 */
    float farZ = 1000.0f;

    // =========================
    // ここから制御(リグ)用
    // =========================
    /** @brief カメラ制御モード */
    enum class Mode
    {
        /** @brief ターゲットを中心に回り込むオービットカメラ */
        Orbit,
        /** @brief 縦スクロールのサイドビュー（Y方向に自動追従など） */
        SideScroll,
        /** @brief 完全固定のサイドビュー（並行投影）。追従なし。UIや固定画面用 */
        Fixed
    };

    /** @brief 使用するモード */
    Mode mode = Mode::Orbit;

    /** @brief 追従対象のエンティティID (0 = 無効) */
    EntityId target = 0;

    // Orbit 用パラメータ
    /** @brief オービットのヨー角 (度) */
    float orbitYawDeg = 90.0f;
    /** @brief オービットのピッチ角 (度) */
    float orbitPitchDeg = 0.0f;
    /** @brief ターゲットからの距離 */
    float orbitDistance = 8.0f;
    /** @brief 注視点に対するオフセット（ワールド座標） */
    DirectX::XMFLOAT3 lookAtOffset{ 0.0f, 1.5f, 0.0f };

    // SideScroll 用パラメータ
    /** @brief カメラが自動で上に進む速度（ワールド単位/秒） */
    float scrollSpeed = 1.0f;
    /** @brief 追従時に注視するYオフセット（プレイヤーの頭上など） */
    float followOffsetY = 1.5f;
    /** @brief この距離以上離れていたら追従に切り替える閾値（ワールド単位） */
    float followMarginY = 0.5f;

    /** @brief 追従のスムース時間（大きいほど遅れて追従） */
    float followSmoothTimeY = 0.12f;
    /** @brief 追従のデッドゾーン（目標との差がこれ以内なら移動しない） */
    float followDeadzoneY = 0.02f;
    /** @brief 自動スクロールのトグル最短間隔（秒） */
    float autoToggleInterval = 0.25f;

    /** @brief 自動スクロールが有効か */
    bool  autoScroll = false;
    /** @brief 前回トグルからの経過時間（秒） */
    float autoToggleTimer = 0.0f;

    /** @brief サイドビュー時に X を固定する値 */
    float sideFixedX = 0.0f;
    /** @brief サイドビュー時の固定 Z（奥行き） */
    float sideFixedZ = -10.0f;
    /** @brief サイドビュー時に常に向く X 方向（注視点の X） */
    float sideLookAtX = 0.0f;

    /** @brief 正射影時の表示高さ（ワールド単位）。値が小さいほどズームイン */
    float orthoHeight = 10.0f;

    /** @brief 深度(Z)を sideFixedZ に近づける補間速度（秒率）。0で即時反映 */
    float depthLerpSpeed = 10.0f;

    /** @brief 自動スクロールの基準となる Y（初期カメラY等） */
    float baseScrollY = 0.0f;

    /** @brief 実行時フラグ: 現在ターゲットのYにロックして追従中か */
    bool followingTarget = false;

    /** @brief 自動スクロール再開判定で、最高ターゲットが baseScrollY よりこの値だけ下にある必要がある */
    float returnToAutoMargin = 0.5f;
};
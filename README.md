# HEW 2025 プロジェクト コーディング規約

このドキュメントは、プロジェクトにおけるC++コードの命名規則とコメント規約を定義します。

---

## 1. 命名規則

コードの一貫性を保つため、以下の命名規則を遵守してください。

| 対象 | 命名規則 | 例 |
| :--- | :--- | :--- |
| ファイル名 | `PascalCase` | `TestScene.h`, `FollowCameraSystem.cpp` |
| クラス | `PascalCase` | `TestScene`, `FollowCameraSystem` |
| Struct | `PascalCase` | `TransformComponent`, `SpawnParams` |
| メソッド / 関数 | `PascalCase` | `Update()`, `GetView()` |
| メンバ変数 (クラス/Struct) | `m_` + `camelCase` | `m_world`, `m_viewT` |
| ローカル変数 | `camelCase` | `dt`, `world`, `sp` |
| 引数 | `camelCase` | `world`, `dt` |
| 定数 (const / constexpr) | `UPPER_SNAKE` または `kPascalCase` | `FLT_MAX`, `kInvalidEntity` |
| enum | `PascalCase` | `Mode` |
| enum値 | `PascalCase` | `SideScroll` |
| インターフェース (抽象基底) | `I` + `PascalCase` | `IUpdateSystem` |
| Cスタイルグローバル関数 | `PascalCase_PascalCase` | `Game_Init` |
| グローバル変数 (非推奨) | `g_` + `PascalCase` | `g_SceneManager` |

---

## 2. Doxygenコメント規約

ソースコードの可読性とメンテナンス性向上のため、Doxygen形式でコメントを記述します。

### 2-1. ファイルヘッダ

すべての `.h` および `.cpp` ファイルの先頭に配置します。

```cpp
/*****************************************************************//**
 * @file   FileName.h
 * @brief  ファイル（モジュール）の簡潔な説明。
 *
 * @author {作成者名}
 * @date   YYYY/MM/DD
 * @details
 * 必要に応じて、ファイルに関する詳細な説明や前提条件を記述します。
 * (例:)
 *********************************************************************/
```
### 2-2. クラス / Struct
class または struct の定義直前に配置します。

```cpp
/**
 * @brief クラス（Struct）の簡潔な説明。
 * @details
 * 必要に応じて、クラスの役割や設計意図などの詳細を記述します。
 */
class MyClassName
{
    // ...
};
```

### 2-3. メソッド / 関数
関数の宣言（.h ファイル内）または定義（.cpp ファイル内）の直前に配置します。

```cpp

/**
 * @brief 関数の目的や動作の簡潔な説明。
 * @param arg1 引数の説明。
 * @param arg2 引数の説明。
 * @return 戻り値の説明。
 */
bool MyClass::MyFunction(int arg1, float arg2)
{
    // ...
}
```

### 2-4. メンバ変数
メンバ変数の同一行の直後に ///< 形式で簡潔な説明を記述します。

```cpp

class MyClass
{
private:
    World m_world;          ///< ECSワールドのインスタンス
    int   m_playerEntity;   ///< プレイヤーエンティティのID
    DirectX::XMFLOAT4X4 m_viewT{};  ///< 転置済みビュー
};
```

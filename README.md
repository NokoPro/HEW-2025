# HEW 2025 ECS 利用ガイド（現状コードの解説と実践）

## 目次
- [1. 基本概念とコア API](#sec-1)
- [2. コンポーネントの作り方・使い方](#sec-2)
- [3. System の作り方（Update/Render）と登録](#sec-3)
- [4. Prefab の仕組みと新しい作り方・登録方法](#sec-4)
- [5. TestScene の流れ（現状コードの全体像）](#sec-5)
- [6. 新しく追加する手順（実践レシピ）](#sec-6)
- [7. よくある落とし穴と Tips](#sec-7)
- [8. 主要コンポーネント・システム・タグ（抜粋）](#sec-8)
- [9. コーディング規約（概要と必須ルール）](#sec-9)
- [10. コーディング規約 詳細（命名/Doxygen）](#sec-10)
- [11. やったらダメなこと（禁止事項）](#sec-11)
- [12. Q&A よくある質問](#sec-12)
- [13. GitHub の使い方とチーム開発のポイント](#sec-13)

このドキュメントは、現状のコードベース（C++17）をもとに、ECS（Entity-Component-System）の実際の使い方をわかりやすく解説します。
- Entity の作成方法
- Component の設計と利用方法
- System（Update/Render）の作り方と登録方法
- Prefab の仕組みと新しい作り方・登録方法
- TestScene を例にした一連の流れ
- よくある落とし穴と Tips

参考となるソースファイル（主要どころ）
- ECS コア: `ECS/ECS.h`, `ECS/World.h`
- システム基底とレジストリ: `ECS/Systems/IUpdateSystem.h`, `ECS/Systems/IRenderSystem.h`, `ECS/Systems/SystemRegistry.h`
- 代表的な Update システム:
  `ECS/Systems/Update/Input/PlayerInputSystem.cpp`,
  `ECS/Systems/Update/Physics/MovementApplySystem.cpp`,
  `ECS/Systems/Update/Physics/PhysicsStepSystem.cpp`,
  `ECS/Systems/Update/Physics/Collision2DSystem.cpp`,
  `ECS/Systems/Update/Game/GoalSystem.cpp`
- 代表的な Render システム: `ECS/Systems/Render/ModelRenderSystem.h`, `ECS/Systems/Render/SpriteRenderSystem.h`, `ECS/Systems/Render/CollisionDebugRenderSystem.h`
- コンポーネント例:
  `ECS/Components/Physics/TransformComponent.h`, `Rigidbody2DComponent.h`, `Collider2DComponent.h`
  `ECS/Components/Render/ModelComponent.h`
  `ECS/Components/Input/PlayerInputComponent.h`, `MovementIntentComponent.h`
- Prefab: `ECS/Prefabs/PrefabRegistry.h`, `PrefabPlayer.cpp`, `PrefabFloor.cpp`, `PrefabWall.cpp`, `PrefabStaticBlock.cpp`, `PrefabGoal.cpp`, `PrefabDeathZone.cpp`
- サンプルシーン: `Scene/TestScene.cpp`

---

<a id="sec-1"></a>
## 1. 基本概念とコア API 🔧

- `EntityId` は `uint32_t` の ID。`0` は無効（`kInvalidEntity`）
- `World` は全コンポーネントストレージの管理者で、型ごとに自動でストレージを生成（`std::type_index` をキーに動的管理）
- 代表メソッド（`ECS/World.h`）
  - `EntityId Create()` 新規エンティティ作成
  - `Add<T>(EntityId, args...)` コンポーネント追加（存在すれば上書き）
  - `Has<T>(EntityId)` 保持確認
  - `Get<T>(EntityId)` 参照取得（なければ assert）
  - `TryGet<T>(EntityId)` 取得 or `nullptr`
  - `Remove<T>(EntityId)` コンポーネント削除
  - `View<A, Rest...>(Fn)` A を主体に「A と Rest... をすべて持つ」エンティティ群を反復し、`fn(e, A&, Rest&...)` を呼ぶ（const/非const 両方あり）

最小例:
```
// Entity 作成と基本コンポーネントの追加
EntityId e = world.Create();
auto& tr = world.Add<TransformComponent>(e, pos, rotDeg, scale);
auto& rb = world.Add<Rigidbody2DComponent>(e);
if (world.Has<Rigidbody2DComponent>(e))
{
    rb.velocity.x = 1.0f;
}

// 条件に合うエンティティの走査
world.View<TransformComponent, Rigidbody2DComponent>([&](EntityId e, TransformComponent& tr, Rigidbody2DComponent& rb) {
    tr.position.x += rb.velocity.x * dt;
});
```

> [!TIP]
> `View` の先頭型は走査の起点になります。密な型を先頭にすると高速になりやすいです。

---

<a id="sec-2"></a>
## 2. コンポーネントの作り方・使い方 🧩

コンポーネントは Plain な `struct` として定義します。World 側は型ごとに自動ストレージを持つため、集中登録は不要です。

例（抜粋）:
```cpp
// Components/Physics/TransformComponent.h
struct TransformComponent
{
    DirectX::XMFLOAT3 position{ 0,0,0 };
    DirectX::XMFLOAT3 rotationDeg{ 0,0,0 };
    DirectX::XMFLOAT3 scale{ 1,1,1 };
};

// Components/Physics/Rigidbody2DComponent.h
struct Rigidbody2DComponent
{
    DirectX::XMFLOAT2 velocity{ 0,0 };
    DirectX::XMFLOAT2 accumulatedForce{ 0,0 };
    float mass = 1.0f;
    bool  useGravity = true;
    bool  onGround = false;
};

// Components/Physics/Collider2DComponent.h
struct Collider2DComponent
 {
    ColliderShapeType shape = ColliderShapeType::AABB2D;
    Physics::LayerMask layer = Physics::LAYER_DEFAULT;
    Physics::LayerMask hitMask = 0xFFFFFFFF;
    PhysicsMaterial material;
    Aabb2D    aabb;
    Circle2D  circle;
    Capsule2D capsule;
    bool isTrigger = false;
    bool isStatic = false;
    DirectX::XMFLOAT2 offset{ 0,0 };
};

// Components/Render/ModelComponent.h
struct ModelRendererComponent
{
    AssetHandle<Model> model;
    bool visible = true;
    DirectX::XMFLOAT3 localOffset{ 0,0,0 };
    DirectX::XMFLOAT3 localRotationDeg{ 0,0,0 };
    DirectX::XMFLOAT3 localScale{ 1,1,1 };
};
```

使い方:
```cpp
EntityId e = world.Create();
auto& tr = world.Add<TransformComponent>(e, pos, rot, scl);
auto& mr = world.Add<ModelRendererComponent>(e);
mr.model = AssetManager::GetModel("mdl_ground");
mr.visible = true;
```

> [!IMPORTANT]
> レイヤーマスクの計算はビット OR `|` を使います（論理 OR `||` は不可）。

---

<a id="sec-3"></a>
## 3. System の作り方（Update/Render）と登録 🚀

- Update 系は `IUpdateSystem` を継承し、`void Update(World& world, float dt)` を実装
- Render 系は `IRenderSystem` を継承し、`void Render(const World& world)` を実装
- 実行順・生存期間は `SystemRegistry` が管理（`ECS/Systems/SystemRegistry.h`）

Update 実装例（移動適用、`Update/Physics/MovementApplySystem.cpp` 抜粋）:
```cpp
void MovementApplySystem::Update(World& world, float dt)
{
    world.View<MovementIntentComponent, Rigidbody2DComponent>(
        [&](EntityId e, MovementIntentComponent& intent, Rigidbody2DComponent& rb)
        {
            const bool onGround = rb.onGround;
            float friction = 1.0f;
            if (auto* col = world.TryGet<Collider2DComponent>(e))
            {
                friction = std::clamp(col->material.friction, 0.0f, 1.0f);
            }

            const float targetVelX = intent.moveX * m_maxSpeedX;
            const float accel = onGround ? m_groundAccel : m_airAccel;

            float vx = rb.velocity.x;
            float toTarget = targetVelX - vx;
            float add = accel * dt * friction;
            if (std::abs(toTarget) <= add) vx = targetVelX;
            else vx += (toTarget > 0.f) ? add : -add;

            // 重力、ジャンプ、ブリンクなどの適用...
            rb.velocity.x = vx;
        }
    );
}
```

Render 実装例（3D モデル描画、`Render/ModelRenderSystem.h`）:
```
class ModelRenderSystem final : public IRenderSystem
 {
public:
    void SetViewProj(const DirectX::XMFLOAT4X4& V, const DirectX::XMFLOAT4X4& P) { m_V = V; m_P = P; }
    void Render(const World& world) override; // World.View で Transform + ModelRenderer を走査して描画
};
```

System の登録と実行（`Scene/TestScene.cpp`）:
```cpp
// 2. System 登録
m_sys.AddUpdate<PlayerInputSystem>();
m_sys.AddUpdate<MovementApplySystem>();
m_sys.AddUpdate<PhysicsStepSystem>(&m_colBuf);

auto* colSys = &m_sys.AddUpdate<Collision2DSystem>(&m_colBuf);
m_sys.AddUpdate<GoalSystem>(colSys);
m_sys.AddUpdate<DeathZoneSystem>(colSys);

m_followCamera = &m_sys.AddUpdate<FollowCameraSystem>(); // ビュー/プロジェクション提供
m_drawModel    = &m_sys.AddRender<ModelRenderSystem>();  // モデル描画
m_debugCollision = &m_sys.AddRender<CollisionDebugRenderSystem>();

// 毎フレームの呼び出し
m_sys.Tick(m_world, dt);     // Update 系を順次実行
m_sys.Render(m_world);       // Render 系を順次実行
```

> [!NOTE]
> Update と Render は別レジストリに保持され、`Tick`/`Render` で順序通りに実行されます。

---

<a id="sec-4"></a>
## 4. Prefab の仕組みと新しい作り方・登録方法 🧱

Prefab は「エンティティ生成関数」を名前（文字列）に紐付けておき、任意に Spawn できる仕組みです（`Prefabs/PrefabRegistry.h`）。

- 型: `using SpawnFunc = std::function<EntityId(World&, const SpawnParams&)>;`
- 登録: `registry.Register("Name", SpawnFunc);`
- 生成: `EntityId e = registry.Spawn("Name", world, params);`
- パラメータ: `SpawnParams`（位置・回転・スケール・パッドインデックス・モデルエイリアス）

代表例（プレイヤー、`Prefabs/PrefabPlayer.cpp`）:
```cpp
void RegisterPlayerPrefab(PrefabRegistry& registry)
{
    registry.Register("Player",
        [](World& w, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            EntityId e = w.Create();
            if (e == kInvalidEntity) return kInvalidEntity;

            auto& tr  = w.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);
            auto& inp = w.Add<PlayerInputComponent>(e);
            inp.playerIndex = (sp.padIndex >= 0) ? sp.padIndex : 0;

            w.Add<MovementIntentComponent>(e);

            auto& rb = w.Add<Rigidbody2DComponent>(e);
            rb.useGravity = true;

            auto& mr = w.Add<ModelRendererComponent>(e);
            // モデル選択: sp.modelAlias > playerIndex デフォルト > フォールバック
            // ... AssetManager 経由で設定し、ShaderList でシェーダ設定

            auto& col = w.Add<Collider2DComponent>(e);
            col.shape = ColliderShapeType::AABB2D;
            col.aabb.halfX = tr.scale.x;
            col.aabb.halfY = tr.scale.y;
            col.layer = Physics::LAYER_PLAYER;
            col.hitMask = Physics::LAYER_GROUND | Physics::LAYER_GOAL; // ←ビット OR
            col.isStatic = false;
            col.offset = { 0.0f, 1.0f };

            return e;
        }
    );
}
```

Prefab の登録と生成（`Scene/TestScene.cpp`）：
```cpp
// 登録（Scene のコンストラクタ）
RegisterPlayerPrefab(m_prefabs);
RegisterFloorPrefab(m_prefabs);
RegisterWallPrefab(m_prefabs);
RegisterStaticBlockPrefab(m_prefabs);
RegisterGoalPrefab(m_prefabs);
RegisterDeathZonePrefab(m_prefabs);

// 生成
PrefabRegistry::SpawnParams sp;
sp.position = { -10.0f, -10.0f, 0.0f };
sp.padIndex = 0;
sp.modelAlias = "mdl_slime";
EntityId player = m_prefabs.Spawn("Player", m_world, sp);
```

> [!TIP]
> `SpawnParams` の `modelAlias` を使うと、レベルデザイン側からモデル差し替えが容易になります。

---

<a id="sec-5"></a>
## 5. TestScene の流れ（現状コードの全体像） 🗺️

- アセットの取得とシェーダ設定（`AssetManager`, `ShaderList`）
- System 登録（入力 → 移動適用 → 物理ステップ → 衝突判定/解決 → ゲームロジック → カメラ → レンダリング）
- ステージ構築（Prefab を使って床・壁・ブロック・ゴール・デスゾーン・プレイヤーを配置）
- カメラエンティティの生成（`ActiveCameraTag`, `Camera3DComponent`, `TransformComponent` を付与）
- Update で `m_sys.Tick`、Draw で `m_sys.Render` 実行

カメラエンティティの生成（抜粋、`Scene/TestScene.cpp`）：
```cpp
EntityId camEnt = m_world.Create();
m_world.Add<ActiveCameraTag>(camEnt);
auto& tr = m_world.Add<TransformComponent>(camEnt, {0,0,-16}, {0,0,0}, {1,1,1});
auto& cam = m_world.Add<Camera3DComponent>(camEnt);
cam.mode = Camera3DComponent::Mode::SideScroll;
cam.target = m_playerEntity; // 追従対象
// 画角・クリップ・サイドスクロール用パラメータなどを設定
```

ゴール判定（`Update/Game/GoalSystem.cpp`）：
- `Collision2DSystem` が発行する `CollisionEventBuffer` を監視
- 両プレイヤーがゴール領域（`LAYER_GOAL`）の `isTrigger` に触れたらクリア

---

<a id="sec-6"></a>
## 6. 新しく追加する手順（実践レシピ） 🆕

- 新しい Component を作る
  1) `ECS/Components/...` に `struct` を定義
  2) 使う箇所で `#include`
  3) 付与時は `world.Add<YourComponent>(e, ctorArgs...)`、参照は `Get` or `TryGet`

- 新しい Update System を作る
  1) `class YourSystem : public IUpdateSystem { void Update(World&, float dt) override; };`
  2) `world.View<...>` で対象エンティティを走査し、ロジックを実装
  3) `SystemRegistry` に `m_sys.AddUpdate<YourSystem>(ctorArgs...)` で登録

- 新しい Render System を作る
  1) `class YourRender : public IRenderSystem { void Render(const World&) override; };`
  2) `SystemRegistry` に `m_sys.AddRender<YourRender>()` で登録
  3) 必要ならカメラ由来の `View/Proj` を受け取り描画

- 新しい Prefab を作る
  1) `void RegisterXxxPrefab(PrefabRegistry&)` を用意
  2) `registry.Register("Xxx", [](World& w, const SpawnParams& sp){ EntityId e = w.Create(); ... return e; });`
  3) Scene の初期化で `RegisterXxxPrefab(m_prefabs)` を呼ぶ
  4) 生成は `m_prefabs.Spawn("Xxx", m_world, sp)`

---

<a id="sec-7"></a>
## 7. よくある落とし穴と Tips ⚠️

- ヒットレイヤの OR はビット OR `|` を使う
  - 例: `col.hitMask = Physics::LAYER_GROUND | Physics::LAYER_GOAL;`
  - `||`（論理 OR）では意図通りにならない
- `World.View` は最初の型のストレージを起点に反復。疎な型を先頭にすると軽い場合がある
- `Get<T>` は存在しないと `assert`。任意の場合は `TryGet<T>` を使って null チェック
- Trigger（`isTrigger = true`）同士の交差は解決（押し戻し）は行わず、`CollisionEventBuffer` へのイベント発行のみ。ゲームロジックで監視
- コライダの `offset` は 2D では X/Y のみ有効。見た目と当たり判定の調整に使う
- デバッグ描画（`CollisionDebugRenderSystem`）で AABB を可視化可能（内部フラグあり）

---

<a id="sec-8"></a>
## 8. 主要コンポーネント・システム・タグ（抜粋） 📚

- コンポーネント
  - `TransformComponent`（位置/回転/スケール）
  - `Rigidbody2DComponent`（速度/重力/接地）
  - `Collider2DComponent`（AABB/レイヤ/トリガ/静的/オフセット）
  - `ModelRendererComponent`（モデル表示）
  - `PlayerInputComponent` / `MovementIntentComponent`（入力 → 意図）

- システム（Update）
  - `PlayerInputSystem`: 入力から `MovementIntent` を更新
  - `MovementApplySystem`: `MovementIntent` を `Rigidbody2D` に適用（加減速/重力/ジャンプ/ブリンク）
  - `PhysicsStepSystem`: 速度を位置に反映・簡易衝突解決・イベント発行
  - `Collision2DSystem`: AABB 交差の検出と解決・イベント発行
  - `GoalSystem`: ゴール到達判定（両プレイヤー）
  - `DeathZoneSystem`: デスゾーン到達処理

- システム（Render）
  - `ModelRenderSystem`: Transform + Model を描画
  - `SpriteRenderSystem`: Transform + Sprite2D を描画
  - `CollisionDebugRenderSystem`: AABB の可視化

- タグ（`ECS/Tag/Tag.h`）
  - `TagPlayer`, `TagDeathZone`, `TagGoalArea` など

---

<a id="sec-9"></a>
## 9. コーディング規約（概要と必須ルール） 📐

> [!IMPORTANT]
> 本プロジェクトでは、以下を強く推奨/必須とします。
>
> - 中括弧 `{}` は「改行して置く（Allman スタイル）」
>   - クラス/構造体/名前空間/関数/制御構文すべてで適用
>   - 例：
>     ```cpp
>     class Foo
>     {
>     public:
>         void Bar()
>         {
>             if (cond)
>             {
>                 Do();
>             }
>         }
>     };
>     ```
> - 生成 AI のコードをそのままコピー＆ペーストすることを禁止
>   - 必ず内容を理解し、命名規約/設計方針/安全性に合わせて修正
>   - 導入時はレビュー/ビルド/実行確認/スタイル統一を徹底

---

<a id="sec-10"></a>
## 10. コーディング規約 詳細（命名/Doxygen）

### 命名規則

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

### Doxygenコメント規約

すべての `.h` および `.cpp` ファイルの先頭に配置：

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

クラス / Struct の直前：

```cpp
/**
 * @brief クラス（Struct）の簡潔な説明。
 * @details 役割や設計意図など。
 */
class MyClassName
{
    // ...
};
```

メソッド / 関数の直前：

```cpp
/**
 * @brief 目的や動作の説明。
 * @param arg1 引数の説明。
 * @param arg2 引数の説明。
 * @return 戻り値の説明。
 */
bool MyClass::MyFunction(int arg1, float arg2)
{
    // ...
}
```

メンバ変数の後ろ：

```cpp
class MyClass
{
private:
    World m_world;          ///< ECSワールドのインスタンス
    int   m_playerEntity;   ///< プレイヤーエンティティのID
    DirectX::XMFLOAT4X4 m_viewT{};  ///< 転置済みビュー
};
```

---

<a id="sec-11"></a>
## 11. やったらダメなこと（禁止事項） 🛑

- 生成 AI のコードをそのままコピペしてコミットしない（理解・整形・規約準拠・レビュー必須）
- レイヤーマスクで `||` を使わない（必ず `|`）
- `Get<T>` を乱用しない（`TryGet<T>` と null チェックを優先）
- `View` で走査中の先頭型ストレージを破壊的に変更しない（安全でない再ハッシュや大量削除は避ける）
- マジックナンバー直書き（定数化・設定化）
- 命名規約違反（PascalCase/camelCase/接頭辞の徹底）
- 中括弧 `{}` を同一行に置かない（必ず改行して配置）
- `extern` やグローバル変数の濫用は禁止（スコープを最小化し依存を注入する）
- クラスの `public` に状態（可変データ）を直置きしない（カプセル化し `private` + アクセサ/メソッドで提供）
- hファイルに関数を理由もなくべた書きしない！！最小限のものにとどめましょう！

---

<a id="sec-12"></a>
## 12. Q&A よくある質問 💬

- Q: Component はどう作る？
  - A: `ECS/Components/...` に `struct` を定義し、使う場所で `#include`。付与は `world.Add<YourComp>(e, ...)`、参照は `Get`/`TryGet`。

- Q: System の登録方法は？
  - A: Update 系は `m_sys.AddUpdate<YourSystem>(...)`、Render 系は `m_sys.AddRender<YourSystem>(...)`。実装は `IUpdateSystem` / `IRenderSystem` を継承。

- Q: Prefab はどう作る/使う？
  - A: `PrefabRegistry` に `Register("Name", SpawnFunc)`。生成は `registry.Spawn("Name", world, sp)`。`SpawnParams` で位置やモデル差し替え可。

- Q: Entity/Component の削除は？
  - A: `world.Destroy(e)` でエンティティ配下の各コンポーネントを一括削除。個別は `world.Remove<T>(e)`。

- Q: 複数コンポーネントをまとめて処理？
  - A: `world.View<A, B, C>([](EntityId, A&, B&, C&){ ... });` を使用。

- Q: 衝突イベントはどう受け取る？
  - A: `Collision2DSystem`（または `PhysicsStepSystem`）が書き込む `CollisionEventBuffer` を介して取得。Goal/Death 判定例は `GoalSystem.cpp` 参照。

- Q: 当たり判定サイズが合わないときは？
  - A: `Collider2DComponent.aabb.halfX/halfY` と `offset` を調整。見た目と原点がズレている場合は `ModelRendererComponent.localOffset` も活用。

- Q: デバッグ描画で当たりを見たい
  - A: `CollisionDebugRenderSystem` を Render レジストリに追加し、カメラの `View/Proj` を渡す。

---

<a id="sec-13"></a>
## 13. GitHub の使い方とチーム開発のポイント 🌿

### 基礎用語（最重要）
- リポジトリ（repo）: プロジェクトそのもの
- クローン（clone）: リモートをローカルへ複製
- フェッチ（fetch）: 変更の取得（マージはしない）
- プル（pull）: 取得＋マージ（`fetch + merge`）
- プッシュ（push）: ローカルの変更をリモートへ反映
- ブランチ（branch）: 作業ラインの分岐（例: `feature/xxx`, `fix/yyy`）
- プルリク（PR）: 変更のレビューとマージの窓口
- コンフリクト（conflict）: 同じ箇所に異なる変更がぶつかった状態

> [!TIP]
> `.gitignore` に生成物（`/Build`, `/bin`, `/obj` など）を含め、実行ファイルや中間ファイルはコミットしない。

### 競合の解決と対策
- 解決手順：
  1) `git pull` して競合ファイルを確認
  2) 手動で差分を統合（どちらを残すかを判断）
  3) ビルド・動作確認
  4) `git add` → `git commit` → `git push`
- 予防策：
  - こまめに `git pull`／小さな粒度でコミット
  - 同じファイルを長時間ロックしない設計（責務分割）
  - フォーマッタ統一（改行コード・インデント）で無駄な差分を削減
  - 変更の意図をコミットメッセージで明確化（英語/日本語どちらでも可）

### 共有とレビューの重要性 🤝
- README/設計ドキュメントを更新して共有
- PR ではスクショ/動画/ログなどを添えて背景を説明
- レビューでは設計・安全性・パフォーマンス・可読性を確認

### 毎日の習慣 ⏰
- 作業開始前に `git pull`（最新版を取得）
- 作業中も適宜 `pull` してズレを早期発見
- 作業終了時に「ビルド通過 → コミット → プッシュ」
- 大きい変更はブランチを切って PR でマージ

### 困ったらどうする？ 🆘
- 焦らずにリードプログラマー/メンターに相談
- 相談時は「再現手順」「ログ」「該当ファイル/行」「期待と実際」を簡潔に共有
- 破壊的変更は一人で決めない（合意形成）

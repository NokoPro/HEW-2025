/*********************************************************************/
/* @file   _geometory.cpp
 * @brief 単純ジオメトリ（Box/Cylinder/Sphere）の頂点・インデックス生成。
 * @details
 * - 座標系: +Y が上、+Z が前、原点中心スケール 1（半径/半辺 0.5）
 * - UV: 各形状に合わせた素朴な展開（箱=面単位、円柱=周方向U/高さV、球=経度U/緯度V）
 * - 重要: DirectX での「表向き」は既定で **CCW（反時計回り）**。
 *         インデックス順序が CW だと裏面カリングされるので注意。
 * @note RasterizerState.FrontCounterClockwise を変えれば表裏の基準を切替可能。
 *
 * @author 浅野勇生
 * @date   2025/10/28
 *********************************************************************/
#include "../Geometory.h"
#include <vector>
#include <cmath>

 /**
  * @brief 位置(x,y,z)とUV(u,v)を持つ1頂点を構築するユーティリティ。
  * @param x,y,z  頂点位置
  * @param u,v    テクスチャ座標
  * @return Geometory::Vertex
  * @details pos[]/uv[] への格納のみ。法線や接線は別途生成対象であれば追加する。
  */
static inline Geometory::Vertex MakeV(float x, float y, float z, float u, float v)
{
    Geometory::Vertex out{};
    out.pos[0] = x; out.pos[1] = y; out.pos[2] = z;
    out.uv[0] = u; out.uv[1] = v;
    return out;
}

/**
 * @brief 原点中心の単位ボックス（各辺長1）を生成する。
 * @details
 * - 各面ごとに独立UVを張るため、頂点数は 6面×4=24。
 * - 各面を2三角形で構成（合計12三角形）。
 * - インデックスは CCW を維持（+Z面: 左下→右下→左上 など）。
 * @note 法線/接線が必要なら面単位で一定ベクトルを割当てやすい（スムーズ不要）。
 */
void Geometory::MakeBox()
{
    // 単位ボックス：中心原点、各辺長 1（±0.5）
    constexpr float s = 0.5f;

    // 面ごとにUVを貼るため 24頂点（6面×4）
    Vertex vtx[] =
    {
        // +Z (front)
        MakeV(-s,-s, s, 0,1), MakeV(s,-s, s, 1,1),
        MakeV(-s, s, s, 0,0), MakeV(s, s, s, 1,0),
        // -Z (back)
        MakeV(s,-s,-s, 0,1), MakeV(-s,-s,-s, 1,1),
        MakeV(s, s,-s, 0,0), MakeV(-s, s,-s, 1,0),
        // +X (right)
        MakeV(s,-s, s, 0,1), MakeV(s,-s,-s, 1,1),
        MakeV(s, s, s, 0,0), MakeV(s, s,-s, 1,0),
        // -X (left)
        MakeV(-s,-s,-s, 0,1), MakeV(-s,-s, s, 1,1),
        MakeV(-s, s,-s, 0,0), MakeV(-s, s, s, 1,0),
        // +Y (top)
        MakeV(-s, s, s, 0,1), MakeV(s, s, s, 1,1),
        MakeV(-s, s,-s, 0,0), MakeV(s, s,-s, 1,0),
        // -Y (bottom)
        MakeV(-s,-s,-s, 0,1), MakeV(s,-s,-s, 1,1),
        MakeV(-s,-s, s, 0,0), MakeV(s,-s, s, 1,0),
    };

    // 各面 2三角形
    uint32_t idx[] =
    {
        0,1,2, 2,1,3,      4,5,6, 6,5,7,
        8,9,10, 10,9,11,   12,13,14, 14,13,15,
        16,17,18, 18,17,19, 20,21,22, 22,21,23
    };

    MeshBuffer::Description desc{};
    desc.pVtx = vtx;
    desc.vtxSize = sizeof(Vertex);
    desc.vtxCount = (UINT)_countof(vtx);
    desc.pIdx = idx;
    desc.idxSize = sizeof(uint32_t);
    desc.idxCount = (UINT)_countof(idx);
    desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    m_pBox = new MeshBuffer();
    m_pBox->Create(desc);
}

/**
 * @brief 半径0.5・高さ1 の円柱を生成（天面/底面/側面）。
 * @details
 * - 分割数: CIRCLE_DETAIL = N。周方向 N、側面ストリップは N+1 でシームを閉じる。
 * - 天面/底面: 原点投影UV（中心=0.5,0.5）。極周辺の歪みは最小限。
 * - 側面UV: U=0..1 を周方向、V=0..1 を高さ。
 * - 重要（バグ対策）: シーム頂点は **位置角度は %N で再利用**、**UVは i/N で 0..1 に伸ばす**。
 *   これにより最後の縦ストリップで U が 1→0 にジャンプせず密集が解消される。
 * - 法線: 本実装では未出力。必要に応じて外向き/±Y を別途計算する。
 * @note テクスチャアトラス使用時はサンプラCLAMP + 端ピクセルの余白（0.5px）推奨。
 */
void Geometory::MakeCylinder()
{
    // 半径0.5, 高さ1.0（y: -0.5..+0.5）
    const int N = CIRCLE_DETAIL;
    const float r = 0.5f;
    const float yTop = 0.5f;
    const float yBot = -0.5f;

    std::vector<Vertex> vtx;
    std::vector<uint32_t> idx;

    // 天面中心・底面中心
    uint32_t topCenter = (uint32_t)vtx.size();
    vtx.push_back(MakeV(0, yTop, 0, 0.5f, 0.5f));
    uint32_t botCenter = (uint32_t)vtx.size();
    vtx.push_back(MakeV(0, yBot, 0, 0.5f, 0.5f));

    // 天/底リング頂点（UVは簡易で円を[0..1]へ投影）
    std::vector<uint32_t> topRing, botRing;
    topRing.reserve(N);
    botRing.reserve(N);

    for (int i = 0; i < N; ++i)
    {
        float t = (float)i / N;
        float ang = t * 2.0f * 3.1415926535f;
        float cx = std::cos(ang), sz = std::sin(ang);

        // 天面
        topRing.push_back((uint32_t)vtx.size());
        vtx.push_back(MakeV(cx * r, yTop, sz * r, 0.5f + cx * 0.5f, 0.5f - sz * 0.5f));
        // 底面
        botRing.push_back((uint32_t)vtx.size());
        vtx.push_back(MakeV(cx * r, yBot, sz * r, 0.5f + cx * 0.5f, 0.5f + sz * 0.5f)); // 裏面になるのでVを反転気味に
    }

    // 側面リング（u: 周方向, v: 高さ）
    // 側面リング（u: 周方向, v: 高さ）
    std::vector<uint32_t> sideTop, sideBot;
    sideTop.reserve(N + 1);
    sideBot.reserve(N + 1);

    for (int i = 0; i <= N; ++i) {             // N+1 本。最後はシームを閉じるための重複
        int   k = i % N;                  // 位置用のインデックス（角度を繰り返す）
        float tpos = (float)k / N;           // 位置計算用（0..1 だが角度は %N で循環）
        float tu = (float)i / N;           // UV 用（0..1 を素直に伸ばす／i=N で 1.0）
        float ang = tpos * 2.0f * 3.1415926535f;
        float cx = std::cos(ang), sz = std::sin(ang);

        sideTop.push_back((uint32_t)vtx.size());
        vtx.push_back(MakeV(cx * r, yTop, sz * r, tu, 0.0f));

        sideBot.push_back((uint32_t)vtx.size());
        vtx.push_back(MakeV(cx * r, yBot, sz * r, tu, 1.0f));
    }


    // --- インデックス
    // --- 天面（+Y 法線）。上から見て CW になるよう center, next, curr
    for (int i = 0; i < N; ++i)
    {
        uint32_t a = topCenter;
        uint32_t b = topRing[(i + 1) % N];
        uint32_t c = topRing[i];
        idx.insert(idx.end(), { a,b,c });
    }

    // --- 底面（-Y 法線）。下から見て CW（= 上から見れば CCW）になるよう center, curr, next
    for (int i = 0; i < N; ++i)
    {
        uint32_t a = botCenter;
        uint32_t b = botRing[i];
        uint32_t c = botRing[(i + 1) % N];
        idx.insert(idx.end(), { a,b,c });
    }

    // --- 側面（外向き）。縦ストリップを 2 三角形： (t0, t1, b0), (t1, b1, b0)
    for (int i = 0; i < N; ++i)
    {
        uint32_t t0 = sideTop[i];
        uint32_t b0 = sideBot[i];
        uint32_t t1 = sideTop[i + 1];
        uint32_t b1 = sideBot[i + 1];
        idx.insert(idx.end(), { t0, t1, b0,   t1, b1, b0 });
    }

    MeshBuffer::Description desc{};
    desc.pVtx = vtx.data();
    desc.vtxSize = sizeof(Vertex);
    desc.vtxCount = (UINT)vtx.size();
    desc.pIdx = idx.data();
    desc.idxSize = sizeof(uint32_t);
    desc.idxCount = (UINT)idx.size();
    desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    m_pCylinder = new MeshBuffer();
    m_pCylinder->Create(desc);
}

/**
 * @brief 経度×緯度のUV球（半径0.5）を生成する。
 * @details
 * - stacks = 緯度（北極0→南極stacks）, slices = 経度（0..2π）
 * - 頂点: (stacks+1)×(slices+1)、UV: u=x/slices, v=1 - y/stacks
 * - 重要（表裏）: インデックスは **CCW** で貼る（i0,i1,i2 / i1,i3,i2）。
 *   CW だと裏面扱い→外側が消えて中身のみ見える“反転症状”になる。
 * @note 球の極では UV 歪みは避けられない。模様に敏感ならキューブマップ等を検討。
 */
void Geometory::MakeSphere()
{
    // 経度(slices)×緯度(stacks) でUV球
    const int stacks = CIRCLE_DETAIL;        // 緯度（南北）
    const int slices = CIRCLE_DETAIL * 2;    // 経度（東西）
    const float r = 0.5f;

    std::vector<Vertex> vtx;
    std::vector<uint32_t> idx;
    vtx.reserve((stacks + 1) * (slices + 1));

    for (int y = 0; y <= stacks; ++y)
    {
        float v = (float)y / stacks;
        float phi = v * 3.1415926535f;            // 0..π
        float cp = std::cos(phi), sp = std::sin(phi);
        for (int x = 0; x <= slices; ++x) {
            float u = (float)x / slices;
            float theta = u * 2.0f * 3.1415926535f; // 0..2π
            float ct = std::cos(theta), st = std::sin(theta);
            float px = r * sp * ct;
            float py = r * cp;
            float pz = r * sp * st;
            vtx.push_back(MakeV(px, py, pz, u, 1.0f - v));
        }
    }

    // 2三角形/クアッド
    auto Id = [slices](int yy, int xx) { return (uint32_t)(yy * (slices + 1) + xx); };
    for (int y = 0; y < stacks; ++y)
    {
        for (int x = 0; x < slices; ++x)
        {
            uint32_t i0 = Id(y, x);
            uint32_t i1 = Id(y, x + 1);
            uint32_t i2 = Id(y + 1, x);
            uint32_t i3 = Id(y + 1, x + 1);
            idx.insert(idx.end(), { i0,i1,i2,  i1,i3,i2 });
        }
    }

    MeshBuffer::Description desc{};
    desc.pVtx = vtx.data();
    desc.vtxSize = sizeof(Vertex);
    desc.vtxCount = (UINT)vtx.size();
    desc.pIdx = idx.data();
    desc.idxSize = sizeof(uint32_t);
    desc.idxCount = (UINT)idx.size();
    desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    m_pSphere = new MeshBuffer();
    m_pSphere->Create(desc);
}
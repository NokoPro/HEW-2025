#include "../Model.h"
#include "../DirectXTex/DirectXTex.h"
#include "../DirectXTex/TextureLoad.h" 
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cstring> 


void Model::MakeMesh(const void* ptr, float scale, Flip flip)
{
	// 事前準備
	aiVector3D zero3(0.0f, 0.0f, 0.0f);
	aiColor4D one4(1.0f, 1.0f, 1.0f, 1.0f);
	const aiScene* pScene = reinterpret_cast<const aiScene*>(ptr);
	float xFlip = flip == Flip::XFlip ? -1.0f : 1.0f;
	float zFlip = (flip == Flip::ZFlip || flip == Flip::ZFlipUseAnime) ? -1.0f : 1.0f;
	int idx1 = (flip == Flip::XFlip || flip == Flip::ZFlip) ? 2 : 1;
	int idx2 = (flip == Flip::XFlip || flip == Flip::ZFlip) ? 1 : 2;

	// メッシュの作成
	m_meshes.resize(pScene->mNumMeshes);
	for (unsigned int i = 0; i < m_meshes.size(); ++i)
	{
		// 頂点書き込み先の領域を用意
		m_meshes[i].vertices.resize(pScene->mMeshes[i]->mNumVertices);

		// 頂点データの書き込み
		for (unsigned int j = 0; j < m_meshes[i].vertices.size(); ++j) {
			// ☆モデルデータから値の取得
			aiVector3D pos = pScene->mMeshes[i]->mVertices[j];
			aiVector3D normal = pScene->mMeshes[i]->HasNormals() ?
				pScene->mMeshes[i]->mNormals[j] : zero3;
			aiVector3D uv = pScene->mMeshes[i]->HasTextureCoords(0) ?
				pScene->mMeshes[i]->mTextureCoords[0][j] : zero3;
			aiColor4D color = pScene->mMeshes[i]->HasVertexColors(0) ?
				pScene->mMeshes[i]->mColors[0][j] : one4;

			// ☆値を設定
			m_meshes[i].vertices[j] = {
				DirectX::XMFLOAT3(pos.x * scale * xFlip, pos.y * scale, pos.z * scale * zFlip),
				DirectX::XMFLOAT3(normal.x, normal.y, normal.z),
				DirectX::XMFLOAT2(uv.x, uv.y),
				DirectX::XMFLOAT4(color.r, color.g, color.b, color.a)
			};
		}

		// ボーン生成
		MakeWeight(pScene, i);

		// インデックスの書き込み先の用意
		// mNumFacesはポリゴンの数を表す(１ポリゴンで3インデックス
		m_meshes[i].indices.resize(pScene->mMeshes[i]->mNumFaces * 3);

		// インデックスの書き込み
		for (unsigned int j = 0; j < pScene->mMeshes[i]->mNumFaces; ++j) {
			// ☆モデルデータから値の取得
			aiFace face = pScene->mMeshes[i]->mFaces[j];
			// ☆値の設定
			int idx = j * 3;
			m_meshes[i].indices[idx + 0] = face.mIndices[0];
			m_meshes[i].indices[idx + 1] = face.mIndices[idx1];
			m_meshes[i].indices[idx + 2] = face.mIndices[idx2];
		}

		// マテリアルの割り当て
		m_meshes[i].materialID = pScene->mMeshes[i]->mMaterialIndex;

		// ☆頂点バッファに必要なデータを設定
		MeshBuffer::Description desc = {};
		desc.pVtx = m_meshes[i].vertices.data();
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = m_meshes[i].vertices.size();
		desc.pIdx = m_meshes[i].indices.data();
		desc.idxSize = sizeof(unsigned long);
		desc.idxCount = m_meshes[i].indices.size();
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				
		// ☆頂点バッファ作成
		m_meshes[i].pMesh = new MeshBuffer();
		m_meshes[i].pMesh->Create(desc);
	}
}

void Model::MakeMaterial(const void* ptr, std::string directory)
{
    const aiScene* pScene = reinterpret_cast<const aiScene*>(ptr);
    if (!pScene) { return; }

    // 既存マテリアルのテクスチャ解放
    for (auto& mat : m_materials)
    {
        if (mat.pTexture)
        {
            delete mat.pTexture;
            mat.pTexture = nullptr;
        }
    }
    m_materials.clear();

    m_materials.resize(pScene->mNumMaterials);

    for (unsigned int i = 0; i < pScene->mNumMaterials; ++i)
    {
        Material material{};
        material.diffuse = DirectX::XMFLOAT4(1, 1, 1, 1);
        material.ambient = DirectX::XMFLOAT4(0, 0, 0, 1);
        material.specular = DirectX::XMFLOAT4(0, 0, 0, 1);
        material.pTexture = nullptr;

        aiMaterial* aiMat = pScene->mMaterials[i];

#ifdef _DEBUG
        {
            aiString name;
            aiMat->Get(AI_MATKEY_NAME, name);
            int texCount = aiMat->GetTextureCount(aiTextureType_DIFFUSE);

            std::string msg = "[MakeMaterial] material[" + std::to_string(i) + "] name=" +
                name.C_Str() + " diffuseTexCount=" + std::to_string(texCount) + "\n";
            OutputDebugStringA(msg.c_str());

            for (int t = 0; t < texCount; ++t)
            {
                aiString texPath;
                if (AI_SUCCESS == aiMat->GetTexture(aiTextureType_DIFFUSE, t, &texPath))
                {
                    std::string p = texPath.C_Str();
                    std::string kind = (!p.empty() && p[0] == '*') ? " (embedded)" : " (external)";
                    std::string msg2 = "  tex[" + std::to_string(t) + "] = " + p + kind + "\n";
                    OutputDebugStringA(msg2.c_str());
                }
            }
        }
#endif

        // ---- 色 ----
        aiColor4D col;
        if (AI_SUCCESS == aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_DIFFUSE, &col))
        {
            material.diffuse = DirectX::XMFLOAT4(col.r, col.g, col.b, col.a);
        }
        if (AI_SUCCESS == aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_AMBIENT, &col))
        {
            material.ambient = DirectX::XMFLOAT4(col.r, col.g, col.b, col.a);
        }
        if (AI_SUCCESS == aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_SPECULAR, &col))
        {
            material.specular = DirectX::XMFLOAT4(col.r, col.g, col.b, col.a);
        }

        // ---- ディフューズテクスチャ ----
        if (aiMat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString texPath;
            if (AI_SUCCESS == aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath))
            {
                std::string texFile = texPath.C_Str();

                if (!texFile.empty() && texFile[0] == '*')
                {
                    // 埋め込みテクスチャ
                    int texIndex = std::atoi(texFile.c_str() + 1);
                    if (texIndex >= 0 && texIndex < static_cast<int>(pScene->mNumTextures))
                    {
                        const aiTexture* aiTex = pScene->mTextures[texIndex];

                        Texture* tex = new Texture();

                        if (aiTex->mHeight != 0)
                        {
                            // 非圧縮 (mWidth x mHeight / aiTexel)
                            const unsigned int width = aiTex->mWidth;
                            const unsigned int height = aiTex->mHeight;

                            std::vector<uint32_t> pixels(width * height);
                            for (unsigned int y = 0; y < height; ++y)
                            {
                                for (unsigned int x = 0; x < width; ++x)
                                {
                                    const aiTexel& t = aiTex->pcData[y * width + x];
                                    // aiTexel は BGRA っぽい並びなので RGBA に詰め替える
                                    uint8_t r = t.r;
                                    uint8_t g = t.g;
                                    uint8_t b = t.b;
                                    uint8_t a = t.a;
                                    pixels[y * width + x] =
                                        (uint32_t(a) << 24) |
                                        (uint32_t(r) << 16) |
                                        (uint32_t(g) << 8) |
                                        (uint32_t(b));
                                }
                            }

                            if (SUCCEEDED(tex->Create(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, pixels.data())))
                            {
                                material.pTexture = tex;
                            }
                            else
                            {
                                delete tex;
                            }
                        }
                        else
                        {
                            // 圧縮テクスチャ（PNG/JPG など）
                            // aiTex->pcData: 圧縮データ
                            // aiTex->mWidth: データサイズ(bytes)、mHeight==0
                            using namespace DirectX;

                            const size_t dataSize = static_cast<size_t>(aiTex->mWidth);

                            TexMetadata metadata{};
                            ScratchImage scratch;

                            // WIC 経由でメモリからデコード
                            HRESULT hr = LoadFromWICMemory(
                                aiTex->pcData,
                                dataSize,
                                WIC_FLAGS_IGNORE_SRGB,
                                &metadata,
                                scratch);

                            if (FAILED(hr))
                            {
                                delete tex;
                                tex = nullptr;
                            }
                            else
                            {
                                const Image* img = scratch.GetImage(0, 0, 0);
                                if (!img)
                                {
                                    delete tex;
                                    tex = nullptr;
                                }
                                else
                                {
                                    // 必ず R8G8B8A8 に揃えて、Texture::Create で扱いやすくする
                                    ScratchImage converted;
                                    const Image* src = img;

                                    if (img->format != DXGI_FORMAT_R8G8B8A8_UNORM)
                                    {
                                        hr = Convert(
                                            *img,
                                            DXGI_FORMAT_R8G8B8A8_UNORM,
                                            TEX_FILTER_DEFAULT,
                                            TEX_THRESHOLD_DEFAULT,
                                            converted);
                                        if (FAILED(hr))
                                        {
                                            delete tex;
                                            tex = nullptr;
                                        }
                                        else
                                        {
                                            src = converted.GetImage(0, 0, 0);
                                        }
                                    }

                                    if (src)
                                    {
                                        const uint32_t width = static_cast<uint32_t>(src->width);
                                        const uint32_t height = static_cast<uint32_t>(src->height);

                                        // rowPitch が width * 4 と違う可能性を考慮してコピー
                                        std::vector<uint8_t> pixels(width * height * 4);

                                        const uint8_t* srcPixels = src->pixels;
                                        for (uint32_t y = 0; y < height; ++y)
                                        {
                                            std::memcpy(
                                                pixels.data() + y * width * 4,
                                                srcPixels + y * src->rowPitch,
                                                width * 4);
                                        }

                                        if (SUCCEEDED(tex->Create(
                                            DXGI_FORMAT_R8G8B8A8_UNORM,
                                            width,
                                            height,
                                            pixels.data())))
                                        {
                                            material.pTexture = tex;
                                        }
                                        else
                                        {
                                            delete tex;
                                            tex = nullptr;
                                        }
                                    }
                                    else
                                    {
                                        delete tex;
                                        tex = nullptr;
                                    }
                                }
                            }
                        }

                    }
                }
                if (!texFile.empty() && texFile[0] != '*')
                {
                    // スラッシュを統一
                    for (auto& ch : texFile)
                    {
                        if (ch == '/')
                        {
                            ch = '\\';
                        }
                    }

                    // ディレクトリ部分を切り落としてファイル名だけにする
                    std::string fileName;
                    {
                        size_t pos = texFile.find_last_of("\\/");
                        if (pos != std::string::npos)
                        {
                            fileName = texFile.substr(pos + 1);
                        }
                        else
                        {
                            fileName = texFile;
                        }
                    }

                    // Model::Load で作った directory は「末尾に \ が付いてる」想定
                    std::string fullPath = directory + fileName;

                    Texture* tex = new Texture();
                    if (SUCCEEDED(tex->Create(fullPath.c_str())))
                    {
                        material.pTexture = tex;
                    }
                    else
                    {
#ifdef _DEBUG
                        OutputDebugStringA(("Model::MakeMaterial : failed to load texture : " + fullPath + "\n").c_str());
#endif
                        delete tex;

                        // どうしても読みたい場合、元の texFile でも一応トライ
                        // （プロジェクト側で texFile をそのままの相対パスで配置したとき用）
                        tex = new Texture();
                        if (SUCCEEDED(tex->Create(texFile.c_str())))
                        {
                            material.pTexture = tex;
                        }
                        else
                        {
                            delete tex;
                        }
                    }
                }
            }
        }

        m_materials[i] = material;
    }
}
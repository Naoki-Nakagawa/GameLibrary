﻿class Mesh
{
public:
    Float3 position;
    Float3 angles;
    Float3 scale;
    std::vector<Vertex> vertices;
    std::vector<UINT> indices;

    Mesh()
    {
        InitializeApplication();

        position = Float3(0.0f, 0.0f, 0.0f);
        angles = Float3(0.0f, 0.0f, 0.0f);
        scale = Float3(1.0f, 1.0f, 1.0f);

        material.Create(
            "cbuffer Camera : register(b0)"
            "{"
            "    matrix view;"
            "    matrix projection;"
            "};"
            "cbuffer Object : register(b1)"
            "{"
            "    matrix world;"
            "};"
            "Texture2D texture0 : register(t0);"
            "SamplerState sampler0 : register(s0);"
            "struct Vertex"
            "{"
            "    float4 position : POSITION;"
            "    float3 normal : NORMAL;"
            "    float2 uv : TEXCOORD;"
            "};"
            "struct Pixel"
            "{"
            "    float4 position : SV_POSITION;"
            "    float3 normal : NORMAL;"
            "    float2 uv : TEXCOORD;"
            "};"
            "Pixel VS(Vertex vertex)"
            "{"
            "    Pixel output;"
            "    output.position = mul(vertex.position, world);"
            "    output.position = mul(output.position, view);"
            "    output.position = mul(output.position, projection);"
            "    output.normal = mul(vertex.normal, (float3x3)world);"
            "    output.uv = vertex.uv;"
            "    return output;"
            "}"
            "float4 PS(Pixel pixel) : SV_TARGET"
            "{"
            "    float3 normal = normalize(pixel.normal);"
            "    float3 lightDirection = normalize(float3(0.25, -1.0, 0.5));"
            "    float3 lightColor = float3(1.0, 1.0, 1.0);"
            "    float4 diffuseColor = texture0.Sample(sampler0, pixel.uv);"
            "    float3 diffuseIntensity = dot(-lightDirection, normal) * lightColor;"
            "    float3 ambientIntensity = lightColor * 0.2;"
            "    return diffuseColor * float4(diffuseIntensity + ambientIntensity, 1);"
            "}"
        );

        SetCullingMode(D3D11_CULL_BACK);

        CreateCube();
    }
    ~Mesh()
    {
    }
    void CreatePlane(Float2 size, Float3 offset = Float3(0.0f, 0.0f, 0.0f), bool shouldClear = true, Float3 leftDirection = Float3(1.0f, 0.0f, 0.0f), Float3 upDirection = Float3(0.0f, 1.0f, 0.0f), Float3 forwardDirection = Float3(0.0f, 0.0f, 1.0f))
    {
        if (shouldClear)
        {
            vertices.clear();
            indices.clear();
        }

        leftDirection = DirectX::XMVector3Normalize(leftDirection);
        upDirection = DirectX::XMVector3Normalize(upDirection);
        forwardDirection = DirectX::XMVector3Normalize(forwardDirection);

        vertices.push_back(Vertex(leftDirection * -size.x + upDirection * size.y + offset, -forwardDirection, Float2(0.0f, 0.0f)));
        vertices.push_back(Vertex(leftDirection * size.x + upDirection * size.y + offset, -forwardDirection, Float2(1.0f, 0.0f)));
        vertices.push_back(Vertex(leftDirection * -size.x + upDirection * -size.y + offset, -forwardDirection, Float2(0.0f, 1.0f)));
        vertices.push_back(Vertex(leftDirection * size.x + upDirection * -size.y + offset, -forwardDirection, Float2(1.0f, 1.0f)));

        int indexOffset = (int)vertices.size() - 4;
        indices.push_back(indexOffset + 0);
        indices.push_back(indexOffset + 1);
        indices.push_back(indexOffset + 2);
        indices.push_back(indexOffset + 3);
        indices.push_back(indexOffset + 2);
        indices.push_back(indexOffset + 1);

        if (shouldClear)
            Apply();
    }
    void CreateCube(Float3 size = Float3(0.5f, 0.5f, 0.5f), bool shouldClear = true)
    {
        if (shouldClear)
        {
            vertices.clear();
            indices.clear();
        }

        CreatePlane(Float2(size.x, size.y), Float3(0.0f, 0.0f, -size.z), false, Float3(1.0f, 0.0f, 0.0f), Float3(0.0f, 1.0f, 0.0f), Float3(0.0f, 0.0f, 1.0f));	// front
        CreatePlane(Float2(size.x, size.y), Float3(0.0f, 0.0f, size.z), false, Float3(-1.0f, 0.0f, 0.0f), Float3(0.0f, 1.0f, 0.0f), Float3(0.0f, 0.0f, -1.0f));	// back
        CreatePlane(Float2(size.z, size.y), Float3(size.x, 0.0f, 0.0f), false, Float3(0.0f, 0.0f, 1.0f), Float3(0.0f, 1.0f, 0.0f), Float3(-1.0f, 0.0f, 0.0f));	// left
        CreatePlane(Float2(size.z, size.y), Float3(-size.x, 0.0f, 0.0f), false, Float3(0.0f, 0.0f, -1.0f), Float3(0.0f, 1.0f, 0.0f), Float3(1.0f, 0.0f, 0.0f));	// right
        CreatePlane(Float2(size.x, size.z), Float3(0.0f, size.y, 0.0f), false, Float3(1.0f, 0.0f, 0.0f), Float3(0.0f, 0.0f, 1.0f), Float3(0.0f, -1.0f, 0.0f));	// up
        CreatePlane(Float2(size.x, size.z), Float3(0.0f, -size.y, 0.0f), false, Float3(1.0f, 0.0f, 0.0f), Float3(0.0f, 0.0f, -1.0f), Float3(0.0f, 1.0f, 0.0f));	// down

        if (shouldClear)
            Apply();
    }
    Material& GetMaterial()
    {
        return material;
    }
    void SetCullingMode(D3D11_CULL_MODE cullingMode)
    {
        D3D11_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = cullingMode;
        rasterizerState.Reset();
        Graphics::GetDevice3D().CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());
    }
    void Apply()
    {
        vertexBuffer.Reset();
        if (vertices.size() > 0)
        {
            D3D11_BUFFER_DESC vertexBufferDesc = {};
            vertexBufferDesc.ByteWidth = sizeof(Vertex) * (UINT)vertices.size();
            vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
            vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
            vertexSubresourceData.pSysMem = vertices.data();
            Graphics::GetDevice3D().CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, vertexBuffer.GetAddressOf());
        }

        indexBuffer.Reset();
        if (indices.size() > 0)
        {
            D3D11_BUFFER_DESC indexBufferDesc = {};
            indexBufferDesc.ByteWidth = sizeof(int) * (UINT)indices.size();
            indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
            indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            D3D11_SUBRESOURCE_DATA indexSubresourceData = {};
            indexSubresourceData.pSysMem = indices.data();
            Graphics::GetDevice3D().CreateBuffer(&indexBufferDesc, &indexSubresourceData, indexBuffer.GetAddressOf());
        }

        material.SetBuffer(1, &constant, sizeof(Constant));
    }
    void Draw()
    {
        if (vertexBuffer == nullptr)
            return;

        constant.world = DirectX::XMMatrixTranspose(
            DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
            DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(angles.x)) *
            DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(angles.y)) *
            DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(angles.z)) *
            DirectX::XMMatrixTranslation(position.x, position.y, position.z)
        );

        material.Attach();

        Graphics::GetContext3D().RSSetState(rasterizerState.Get());

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        Graphics::GetContext3D().IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

        if (indexBuffer == nullptr)
        {
            Graphics::GetContext3D().Draw((UINT)vertices.size(), 0);
        }
        else
        {
            Graphics::GetContext3D().IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            Graphics::GetContext3D().DrawIndexed((UINT)indices.size(), 0, 0);
        }
    }

private:
    struct Constant
    {
        DirectX::XMMATRIX world;
    };

    Material material;
    Constant constant;
    ComPtr<ID3D11Buffer> vertexBuffer = nullptr;
    ComPtr<ID3D11Buffer> indexBuffer = nullptr;
    ComPtr<ID3D11RasterizerState> rasterizerState = nullptr;
};
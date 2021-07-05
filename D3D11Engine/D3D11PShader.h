#pragma once

class D3D11ConstantBuffer;

class D3D11PShader {
public:

    D3D11PShader();
    ~D3D11PShader();

    /** Loads shader */
    XRESULT LoadShader( const char* pixelShader, const std::vector<D3D_SHADER_MACRO>& makros = std::vector<D3D_SHADER_MACRO>() );

    /** Applys the shader */
    XRESULT Apply();

    /** Returns a reference to the constantBuffer vector*/
    std::vector<D3D11ConstantBuffer*>& GetConstantBuffer();

    /** Returns the shader */
    Microsoft::WRL::ComPtr<ID3D11PixelShader> GetShader() { return PixelShader.Get(); }

    /** Returns this textures ID */
    UINT16 GetID() { return ID; };

private:

    /** Compiles the shader from file and outputs error messages if needed */
    HRESULT CompileShaderFromFile( const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, const std::vector<D3D_SHADER_MACRO>& makros );

    Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
    std::vector<D3D11ConstantBuffer*> ConstantBuffers;

    std::string File;

    /** ID of this shader */
    UINT16 ID;
};


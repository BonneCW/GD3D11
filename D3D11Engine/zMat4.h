#pragma once
#include "pch.h"
#include "HookedFunctions.h"

class zMat4
{
    public:
        /** Hooks the functions of this Class */
        static void Hook()
        {
            PatchJMP( 0x00515080, reinterpret_cast<DWORD>(&zMat4Transpose) );
            PatchJMP( 0x00515500, reinterpret_cast<DWORD>(&zMat4Inverse) );
        }

        static XMFLOAT4X4& __fastcall zMat4Transpose(XMFLOAT4X4& _THIS, int _EDX, XMFLOAT4X4& output)
        {
            XMMATRIX input = XMLoadFloat4x4(&_THIS);
            XMMATRIX transpose = XMMatrixTranspose(input);
            XMStoreFloat4x4(&output, transpose);
			return output;
        }

        static XMFLOAT4X4& __fastcall zMat4Inverse(XMFLOAT4X4& _THIS, int _EDX, XMFLOAT4X4& output)
        {
            XMMATRIX input = XMLoadFloat4x4(&_THIS);
            XMMATRIX inverse = XMMatrixInverse(nullptr, input);
            XMStoreFloat4x4(&output, inverse);
            return output;
        }
};

#pragma once
#include "zTypes.h"
#include "HookedFunctions.h"

class zQuat
{
    public:
        /** Hooks the functions of this Class */
        static void Hook()
        {
            PatchJMP( 0x00518EF0, reinterpret_cast<DWORD>(&zQuatLerp) );
            PatchJMP( 0x00518D10, reinterpret_cast<DWORD>(&zQuatSlerp) );
            PatchJMP( 0x00518560, reinterpret_cast<DWORD>(&zQuatMat4ToQuat) );
            PatchJMP( 0x00518360, reinterpret_cast<DWORD>(&zQuatQuatToMat4) );
        }

		static void __fastcall zQuatLerp(float4& output, int _EDX, float t, float4& q0, float4& q1)
		{
			const __m128 Zero = _mm_setzero_ps();
			const __m128 One = _mm_set_ps1( 1.0f );
			const __m128 NegativeOne = _mm_set_ps1( -1.0f );

			__m128 S1 = _mm_set_ps1( t );
			__m128 S0 = _mm_sub_ps( One, S1 );
			__m128 Q0 = _mm_loadu_ps( reinterpret_cast<const float*>( &q0.x ) );
			__m128 Q1 = _mm_loadu_ps( reinterpret_cast<const float*>( &q1.x ) );

			__m128 CosOmega = XMVector4Dot( Q0, Q1 );
			__m128 Control = _mm_cmplt_ps( CosOmega, Zero );
			__m128 Sign = _mm_or_ps( _mm_andnot_ps( Control, One ), _mm_and_ps( NegativeOne, Control ) );

			S0 = _mm_mul_ps( S0, Q0 );
			S1 = _mm_mul_ps( S1, Q1 );
			_mm_storeu_ps( reinterpret_cast<float*>( &output.x ), _mm_add_ps( S0, _mm_mul_ps( S1, Sign ) ) );
		}

        static void __fastcall zQuatSlerp(float4& output, int _EDX, float t, float4& q0, float4& q1)
        {
			const __m128 Zero = _mm_setzero_ps();
			const __m128 One = _mm_set_ps1( 1.0f );
			const __m128 NegativeOne = _mm_set_ps1( -1.0f );

			__m128 T = _mm_set_ps1( t );
			__m128 Q0 = _mm_loadu_ps( reinterpret_cast<const float*>( &q0.x ) );
			__m128 Q1 = _mm_loadu_ps( reinterpret_cast<const float*>( &q1.x ) );

			__m128 CosOmega = XMVector4Dot( Q0, Q1 );
			__m128 Control = _mm_cmplt_ps( CosOmega, Zero );
			__m128 Sign = _mm_or_ps( _mm_andnot_ps( Control, One ), _mm_and_ps( NegativeOne, Control ) );

			CosOmega = _mm_mul_ps( CosOmega, Sign );
			CosOmega = _mm_min_ps( CosOmega, One );

			__m128 Theta = XMVectorACos( CosOmega );
			__m128 SinOmega = XMVectorSin( Theta );
			if(_mm_cvtss_f32( SinOmega ) < 0.001f)
			{
				// Skip some calculations here since we don't need them
				output.x = q0.x;
				output.y = q0.y;
				output.z = q0.z;
				output.w = q0.w;
				return;
			}

			SinOmega = _mm_div_ps( One, SinOmega );
			__m128 S0 = _mm_mul_ps( XMVectorSin( _mm_mul_ps( _mm_sub_ps( One, T ), Theta ) ), SinOmega );
			__m128 S1 = _mm_mul_ps( XMVectorSin( _mm_mul_ps( T, Theta ) ), SinOmega );
			S0 = _mm_mul_ps( S0, Q0 );
			S1 = _mm_mul_ps( S1, Q1 );
			_mm_storeu_ps( reinterpret_cast<float*>( &output.x ), _mm_add_ps( S0, _mm_mul_ps( S1, Sign ) ) );
        }

		static void __fastcall zQuatMat4ToQuat(float4& output, int _EDX, XMFLOAT4X4& M)
		{
            const __m128 XMPMMP = _mm_setr_ps( +1.0f, -1.0f, -1.0f, +1.0f );
            const __m128 XMMPMP = _mm_setr_ps( -1.0f, +1.0f, -1.0f, +1.0f );
            const __m128 XMMMPP = _mm_setr_ps( -1.0f, -1.0f, +1.0f, +1.0f );

            __m128 r0 = _mm_loadu_ps( reinterpret_cast<const float*>( &M.m[0] ) );
            __m128 r1 = _mm_loadu_ps( reinterpret_cast<const float*>( &M.m[1] ) );
            __m128 r2 = _mm_loadu_ps( reinterpret_cast<const float*>( &M.m[2] ) );

            __m128 r00 = _mm_shuffle_ps( r0, r0, _MM_SHUFFLE( 0, 0, 0, 0 ) );
            __m128 r11 = _mm_shuffle_ps( r1, r1, _MM_SHUFFLE( 1, 1, 1, 1 ) );
            __m128 r22 = _mm_shuffle_ps( r2, r2, _MM_SHUFFLE( 2, 2, 2, 2 ) );

            __m128 r11mr00 = _mm_sub_ps( r11, r00 );
            __m128 x2gey2 = _mm_cmple_ps( r11mr00, _mm_setzero_ps() );

            __m128 r11pr00 = _mm_add_ps( r11, r00 );
            __m128 z2gew2 = _mm_cmple_ps( r11pr00, _mm_setzero_ps() );

            __m128 x2py2gez2pw2 = _mm_cmple_ps( r22, _mm_setzero_ps() );

            __m128 t0 = XM_FMADD_PS( XMPMMP, r00, _mm_set_ps1( 1.0f ) );
            __m128 t1 = _mm_mul_ps( XMMPMP, r11 );
            __m128 t2 = XM_FMADD_PS( XMMMPP, r22, t0 );
            __m128 x2y2z2w2 = _mm_add_ps( t1, t2 );

            t0 = _mm_shuffle_ps( r0, r1, _MM_SHUFFLE( 1, 2, 2, 1 ) );
            t1 = _mm_shuffle_ps( r1, r2, _MM_SHUFFLE( 1, 0, 0, 0 ) );
            t1 = _mm_shuffle_ps( t1, t1, _MM_SHUFFLE( 1, 3, 2, 0 ) );

            __m128 xyxzyz = _mm_add_ps( t0, t1 );

            t0 = _mm_shuffle_ps( r2, r1, _MM_SHUFFLE( 0, 0, 0, 1 ) );
            t1 = _mm_shuffle_ps( r1, r0, _MM_SHUFFLE( 1, 2, 2, 2 ) );
            t1 = _mm_shuffle_ps( t1, t1, _MM_SHUFFLE( 1, 3, 2, 0 ) );

            __m128 xwywzw = _mm_sub_ps( t0, t1 );
            xwywzw = _mm_mul_ps( XMMPMP, xwywzw );

            t0 = _mm_shuffle_ps( x2y2z2w2, xyxzyz, _MM_SHUFFLE( 0, 0, 1, 0 ) );
            t1 = _mm_shuffle_ps( x2y2z2w2, xwywzw, _MM_SHUFFLE( 0, 2, 3, 2 ) );
            t2 = _mm_shuffle_ps( xyxzyz, xwywzw, _MM_SHUFFLE( 1, 0, 2, 1 ) );

            __m128 tensor0 = _mm_shuffle_ps( t0, t2, _MM_SHUFFLE( 2, 0, 2, 0 ) );
            __m128 tensor1 = _mm_shuffle_ps( t0, t2, _MM_SHUFFLE( 3, 1, 1, 2 ) );
            __m128 tensor2 = _mm_shuffle_ps( t2, t1, _MM_SHUFFLE( 2, 0, 1, 0 ) );
            __m128 tensor3 = _mm_shuffle_ps( t2, t1, _MM_SHUFFLE( 1, 2, 3, 2 ) );

            t0 = _mm_and_ps( x2gey2, tensor0 );
            t1 = _mm_andnot_ps( x2gey2, tensor1 );
            t0 = _mm_or_ps( t0, t1 );
            t1 = _mm_and_ps( z2gew2, tensor2 );
            t2 = _mm_andnot_ps( z2gew2, tensor3 );
            t1 = _mm_or_ps( t1, t2 );
            t0 = _mm_and_ps( x2py2gez2pw2, t0 );
            t1 = _mm_andnot_ps( x2py2gez2pw2, t1 );
            t2 = _mm_or_ps( t0, t1 );

            t0 = XMVector4Length( t2 );
			_mm_storeu_ps( reinterpret_cast<float*>( &output.x ), _mm_div_ps( t2, t0 ) );
		}

		static void __fastcall zQuatQuatToMat4(float4& quat, int _EDX, XMFLOAT4X4& output)
		{
            const __m128 XMPPPZ = _mm_setr_ps( 1.0f, 1.0f, 1.0f, 0.0f );
            const __m128 XMMMMZ = _mm_castsi128_ps( _mm_setr_epi32( -1, -1, -1, 0 ) );

            __m128 quaternion = _mm_loadu_ps( reinterpret_cast<const float*>(&quat.x) );
            __m128 Q0 = _mm_add_ps( quaternion, quaternion );
            __m128 Q1 = _mm_mul_ps( quaternion, Q0 );

            __m128 V0 = _mm_shuffle_ps( Q1, Q1, _MM_SHUFFLE( 3, 0, 0, 1 ) );
            V0 = _mm_and_ps( V0, XMMMMZ );
            __m128 V1 = _mm_shuffle_ps( Q1, Q1, _MM_SHUFFLE( 3, 1, 2, 2 ) );
            V1 = _mm_and_ps( V1, XMMMMZ );
            __m128 R0 = _mm_sub_ps( XMPPPZ, V0 );
            R0 = _mm_sub_ps( R0, V1 );

            V0 = _mm_shuffle_ps( quaternion, quaternion, _MM_SHUFFLE( 3, 1, 0, 0 ) );
            V1 = _mm_shuffle_ps( Q0, Q0, _MM_SHUFFLE( 3, 2, 1, 2 ) );
            V0 = _mm_mul_ps( V0, V1 );

            V1 = _mm_shuffle_ps( quaternion, quaternion, _MM_SHUFFLE( 3, 3, 3, 3 ) );
            __m128 V2 = _mm_shuffle_ps( Q0, Q0, _MM_SHUFFLE( 3, 0, 2, 1 ) );
            V1 = _mm_mul_ps( V1, V2 );

            __m128 R1 = _mm_add_ps( V0, V1 );
            __m128 R2 = _mm_sub_ps( V0, V1 );

            V0 = _mm_shuffle_ps( R1, R2, _MM_SHUFFLE( 1, 0, 2, 1 ) );
            V0 = _mm_shuffle_ps( V0, V0, _MM_SHUFFLE( 1, 3, 2, 0 ) );
            V1 = _mm_shuffle_ps( R1, R2, _MM_SHUFFLE( 2, 2, 0, 0 ) );
            V1 = _mm_shuffle_ps( V1, V1, _MM_SHUFFLE( 2, 0, 2, 0 ) );

            Q1 = _mm_shuffle_ps( R0, V0, _MM_SHUFFLE( 1, 0, 3, 0 ) );
            Q1 = _mm_shuffle_ps( Q1, Q1, _MM_SHUFFLE( 1, 3, 2, 0 ) );

            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>(&output.m[0]), Q1 );
            Q1 = _mm_shuffle_ps( R0, V0, _MM_SHUFFLE( 3, 2, 3, 1 ) );
            Q1 = _mm_shuffle_ps( Q1, Q1, _MM_SHUFFLE( 1, 3, 0, 2 ) );
            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>(&output.m[1]), Q1 );
            Q1 = _mm_shuffle_ps( V1, R0, _MM_SHUFFLE( 3, 2, 1, 0 ) );
            XMStoreFloat3( reinterpret_cast<XMFLOAT3*>(&output.m[2]), Q1 );
		}
};

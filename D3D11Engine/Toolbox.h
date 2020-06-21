#pragma once

#include <map>
#include <string>
#include <unordered_map>

#include <Windows.h>

#include "Types.h"

using namespace DirectX;

/** Misc. tools */
enum zTCam_ClipType;
struct zTBBox3D;
struct zTPlane;

namespace Toolbox {

	/** Checks if one of a series of strings is found within the input-string */
	bool StringContainsOneOf( const std::string& string, const std::string* checkStrings, int numStrings );

	/** Erases an element by value from a vector */
	template<typename T>
	void EraseByElement( std::vector<T>& vector, T value ) {
		auto it = std::find( vector.begin(), vector.end(), value );
		if ( it != vector.end() ) {
			vector.erase( it );
		}
	}

	/** Erases an element by value from a vector */
	template<typename T, typename S>
	void EraseByElement( std::map<T, S>& map, S value ) {
		for ( auto it = map.begin(); it != map.end();) {
			if ( it->second == value ) {
				it = map.erase( it );
			} else {
				++it;
			}
		}
	}

	/** Deletes all elements of the given std::vector */
	template<typename T>
	void DeleteElements( std::vector<T>& vector ) {
		for ( auto it = vector.begin(); it != vector.end(); ++it ) {
			delete* it;
		}
		vector.clear();
	}

	/** Deletes all elements of the given std::vector */
	template<typename T>
	void DeleteElements( std::list<T>& list ) {
		for ( auto it = list.begin(); it != list.end(); ++it ) {
			delete* it;
		}
		list.clear();
	}

	/** Deletes all (second) elements of the given std::map */
	template<typename T, typename S>
	void DeleteElements( std::map<T, S>& map ) {
		for ( auto it = map.begin(); it != map.end(); ++it ) {
			delete it->second;
		}
		map.clear();
	}

	/** Deletes all (second) elements of the given std::unordered_map */
	template<typename T, typename S>
	void DeleteElements( std::unordered_map<T, S>& map ) {
		for ( auto it = map.begin(); it != map.end(); ++it ) {
			delete it->second;
		}
		map.clear();
	}

	/** Sorts the vector and removes all doubles */
	template<typename T>
	void RemoveDoubles( std::vector<T>& vector ) {
		std::sort( vector.begin(), vector.end() );
		std::unique( vector.begin(), vector.end() );
	}

	FORCEINLINE
		float XMVector3LengthFloat( DirectX::FXMVECTOR vector ) {
		float len; DirectX::XMStoreFloat( &len, DirectX::XMVector3Length( vector ) );
		return len;
	}
	FORCEINLINE
		float XMVector3LengthSqFloat( DirectX::FXMVECTOR vector ) {
		float len; DirectX::XMStoreFloat( &len, DirectX::XMVector3LengthSq( vector ) );
		return len;
	}
	FORCEINLINE
		float XMVector2LengthFloat( DirectX::FXMVECTOR vector ) {
		float len; DirectX::XMStoreFloat( &len, DirectX::XMVector2Length( vector ) );
		return len;
	}
	FORCEINLINE
		float XMVector2LengthSqFloat( DirectX::FXMVECTOR vector ) {
		float len; DirectX::XMStoreFloat( &len, DirectX::XMVector2LengthSq( vector ) );
		return len;
	}
	/** Checks whether a given boundingbox is inside the given frustum. The index in "cache" is tested first, if it isn't set to -1 */
	zTCam_ClipType BBox3DInFrustumCached( const zTBBox3D& bbox3D, zTPlane* frustumPlanes, uint8_t* signbits, int& cache );

	/** Checks if a folder exists */
	bool FolderExists( const std::string& dirName_in );

	/** Hashes the given float value */
	void hash_combine( std::size_t& seed, float value );

	/** Hashes the given DWORD value */
	void hash_combine( std::size_t& seed, DWORD value );

	/** Returns true if the given position is inside the box */
	bool PositionInsideBox( const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max );

	/** Converts an errorcode into a string */
	std::string MakeErrorString( XRESULT code );

	/** Returns the number of bits inside a bitmask */
	WORD GetNumberOfBits( DWORD dwMask );

	/** Returns the size of a DDS-Image in bytes */
	unsigned int GetDDSStorageRequirements( unsigned int width, unsigned int height, bool dxt1 );

	/** Returns the RowPitch-Size of a DDS-Image */
	unsigned int GetDDSRowPitchSize( unsigned int width, bool dxt1 );

	/** Returns a random number between 0 and 1 */
	float frand();

	/** Linear interpolation */
	float lerp( float a, float b, float w );

	/** Converts a multi-byte-string to wide-char */
	std::wstring ToWideChar( const std::string& str );

	/** Converts a wide-char-string to  multi-byte*/
	std::string ToMultiByte( const std::wstring& str );

	/** Returns whether two AABBs are intersecting or not */
	bool AABBsOverlapping( const DirectX::XMFLOAT3& minA, const DirectX::XMFLOAT3& maxA, const DirectX::XMFLOAT3& minB, const DirectX::XMFLOAT3& maxB );

	/** Does a ray vs aabb test */
	bool IntersectBox( const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max, const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float& t );

	/** Does a ray vs aabb test */
	bool IntersectTri( const DirectX::XMFLOAT3& v0, const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2, const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float& u, float& v, float& t );

	/** Computes the normal of a triangle */
	DirectX::XMFLOAT3 ComputeNormal( const DirectX::XMFLOAT3& v0, const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2 );

	/** Computes the distance of a point to an AABB */
	float ComputePointAABBDistance( const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max );

	/** Returns whether the given file exists */
	bool FileExists( const std::string& file );

	/** Saves a std::string to a FILE* */
	void SaveStringToFILE( FILE* f, const std::string& str );

	/** sse2 memcpy implementation by William Chan and Google */
	void X_aligned_memcpy_sse2( void* dest, const void* src, const unsigned long size_t );

	/** Loads a std::string from a FILE* */
	std::string LoadStringFromFILE( FILE* f );

	/** Returns the elapsed milliseconds since first execution of this method */
	DWORD timeSinceStartMs();
}

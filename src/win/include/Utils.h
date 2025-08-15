#pragma once

template<typename T>
inline void SafeRelease(T * pointer)
{ if(pointer) pointer->Release(); pointer = nullptr; }

template<typename T>
inline void SafeDelete(T * pointer)
{ if(pointer) delete pointer; }

template<typename T, const size_t N>
inline constexpr unsigned int Countof(T (&array)[N])
{ return static_cast<unsigned int>(N); }
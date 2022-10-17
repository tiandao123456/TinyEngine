#pragma once
enum DataFormat
{
	R8G8B8A8,
	D32,
	R24G8,
	D24S8
};

enum BufferUsageFormat
{
	Output,
    UnorderedAccess  
};

enum SwapEffect
{
	Discard,
	Sequential,
	FlipSequential,
	FlipDiscard
};

enum HeapType
{
	RTV,
	DSV,
	CBVSRVUAV
};

enum ViewDimension
{
	Texture2D
};

enum ResDimension
{
	ResTexture2D
};

enum Flag
{
	None,
	DSVReadOnlyDepth,
	DSVReadOnlyStencil
};

enum TexLayout
{
	LayoutUnknown
};

enum ResFlag
{
	ResNone,
	FlagAllowDepthStencil,
	FlagAllowUnorderedAccess,
	FlagAllowRenderTarget
};

enum HeapFlag
{
	HeapNone,
	HeapVisible
};

enum SrvDimension
{
	SRVTexture2D
};
#include <Engine/StdAfx.h>
#include <Engine/Font.h>
#include <Engine/ITexture.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>
#include <Engine/Renderer.h>
#include <Engine/ICamera.h>
#include <Engine/RendererEnums.h>
#include <Engine/IVertexBuffer.h>
#include <Engine/IShader.h>
#include <Engine/IInputLayout.h>
#include <UI/IUIManager.h>
#include <CommonLib/Math/fbMath.h>
#include <CommonLib/Math/Vec2.h>
#include <CommonLib/Profiler.h>
#include <CommonLib/Unicode.h>

namespace fastbird
{
	//----------------------------------------------------------------------------
	// FontLoader
	//----------------------------------------------------------------------------
	class FontLoader
	{
	public:
		FontLoader(FILE *f, Font *font, const char *fontFile);

		virtual int Load() = 0; // Must be implemented by derived class

	protected:
		void LoadPage(int id, const char *pageFile, const std::string& fontFile);
		void SetFontInfo(int outlineThickness);
		void SetCommonInfo(int fontHeight, int base, int scaleW, int scaleH, int pages, bool isPacked);
		void AddChar(int id, int x, int y, int w, int h, int xoffset, int yoffset, int xadvance, int page, int chnl);
		void AddKerningPair(int first, int second, int amount);

		FILE *f;
		Font *font;
		std::string fontFile;

		int outlineThickness;
	};

	class FontLoaderTextFormat : public FontLoader
	{
	public:
		FontLoaderTextFormat(FILE *f, Font *font, const char *fontFile);

		int Load();

		int SkipWhiteSpace(const std::string &str, int start);
		int FindEndOfToken(const std::string &str, int start);

		void InterpretInfo(const std::string &str, int start);
		void InterpretCommon(const std::string &str, int start);
		void InterpretChar(const std::string &str, int start);
		void InterpretSpacing(const std::string &str, int start);
		void InterpretKerning(const std::string &str, int start);
		// intentionally not reference
		void InterpretPage(const std::string str, int start, const std::string fontFile);
	};

	class FontLoaderBinaryFormat : public FontLoader
	{
	public:
		FontLoaderBinaryFormat(FILE *f, Font *font, const char *fontFile);

		int Load();

		void ReadInfoBlock(int size);
		void ReadCommonBlock(int size);
		void ReadPagesBlock(int size);
		void ReadCharsBlock(int size);
		void ReadKerningPairsBlock(int size);
	};

	const unsigned int Font::MAX_BATCH = 4*2000;
	//----------------------------------------------------------------------------
}

using namespace fastbird;

//----------------------------------------------------------------------------
Font::Font()
{
	mFontHeight = 0;
	mScaledFontSize = 0;
	mBase = 0;
	mScaleW = 0;
	mScaleH = 0;
	mScale = 1.0f;
	mVertexLocation = 0;
	mHasOutline = false;
	mEncoding = NONE;
	mColor = 0xFFFFFFFF;
	mInitialized = false;
}

//----------------------------------------------------------------------------
Font::~Font()
{
	std::map<int, SCharDescr*>::iterator it = mChars.begin();
	while( it != mChars.end() )
	{
		FB_SAFE_DEL(it->second);
		it++;
	}
}

//----------------------------------------------------------------------------
// IFont
//----------------------------------------------------------------------------
int Font::Init(const char *fontFile)
{

	if (mInitialized)
		return 0;
	Profiler profiler("'Font Init'");
	// Load the font
	FILE *f = 0;
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	fopen_s(&f, fontFile, "rb");
#else
	f = fopen(fontFile, "rb");
#endif
	if( f == 0 )
	{
		gFBEnv->pEngine->Log("Failed to open font file '%s'!", fontFile);
		return -1;
	}

	// Determine format by reading the first bytes of the file
	char str[4] = {0};
	fread(str, 3, 1, f);
	fseek(f, 0, SEEK_SET);
	int r = 0;
	{
		Profiler profiler("'Font loding'");
		FontLoader *loader = 0;
		if( strcmp(str, "BMF") == 0 )
			loader = FB_NEW(FontLoaderBinaryFormat)(f, this, fontFile);
		else
			loader = FB_NEW(FontLoaderTextFormat)(f, this, fontFile);

		r = loader->Load();
		FB_SAFE_DEL(loader);
	}

	mVertexBuffer = gFBEnv->pEngine->GetRenderer()->CreateVertexBuffer(0, sizeof(FontVertex), MAX_BATCH, 
		BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE); 
	assert(mVertexBuffer);

	// init shader
	mShader = gFBEnv->pEngine->GetRenderer()->CreateShader(
		"es/shaders/font.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS,
		IMaterial::SHADER_DEFINES());
	mInputLayout = gFBEnv->pEngine->GetRenderer()->GetInputLayout(
		DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD_BLENDINDICES, mShader);
	FB_SET_DEVICE_DEBUG_NAME(mInputLayout, "font input layout");
		
	mInitialized = r==0;

	mTextureMaterial = gFBEnv->pRenderer->GetMaterial(DEFAULT_MATERIALS::QUAD_TEXTURE)->Clone();

	BLEND_DESC desc;
	desc.RenderTarget[0].BlendEnable = true;
	desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_RED | COLOR_WRITE_MASK_GREEN | COLOR_WRITE_MASK_BLUE;
	mTextureMaterial->SetBlendState(desc);
	mTextureMaterial->CloneRenderStates();
	

	mRenderTargetSize = gFBEnv->_pInternalRenderer->GetMainRTSize();
	mObjectConstants.gWorldViewProj = MakeOrthogonalMatrix(0, 0, 
		(float)mRenderTargetSize.x,
		(float)mRenderTargetSize.y,
		0.f, 1.0f);
	mObjectConstants.gWorld.MakeIdentity();

	
	return r;
}

//----------------------------------------------------------------------------
void Font::SetTextEncoding(EFontTextEncoding encoding)
{
	mEncoding = encoding;
}

bool Font::ApplyTag(const char* text, int start, int end, float& x, float& y)
{
	static TextureAtlas* textureAtlas = gFBEnv->pRenderer->GetTextureAtlas("data/textures/gameui.xml");

	assert(end - start > 8);
	char buf[256];
	TextTags::Enum tagType = GetTagType(&text[start], end - start, buf);
	switch (tagType)
	{
	case TextTags::Img:
	{
						  if (textureAtlas)
						  {
							  auto region = textureAtlas->GetRegion(buf);
							  if (region)
							  {
								  auto& regionSize = region->GetSize();
								  float ratio = regionSize.x / (float)regionSize.y;
								  Vec2I imgSize = regionSize;
								  int yoffset = 0;
								  if (imgSize.y < mScaledFontSize){
									  yoffset = Round((mScaledFontSize - imgSize.y) * .5f);
								  }
								  mTextureMaterial->SetDiffuseColor(Vec4(1, 1, 1, ( (mColor & 0xff000000) >> 24)/255.f ));
								  gFBEnv->pRenderer->DrawQuadWithTextureUV(Vec2I((int)x, (int)y + yoffset), imgSize, region->mUVStart, region->mUVEnd,
									  Color::White, textureAtlas->mTexture, mTextureMaterial);
								  
								  x += imgSize.x;
								  return true;
							  }
						  }
						  break;
	}
	case TextTags::ColorStart:
	{
								 unsigned color = StringConverter::parseHexa(buf);

								 int prevAlpha = mColor & 0xff000000;
								 mColorBackup.push(mColor);
								 mColor = Color::FixColorByteOrder(color);
								 mColor = mColor & 0x00ffffff + prevAlpha;
								 break;
	}
	case TextTags::ColorEnd:
	{
		if (!mColorBackup.empty()){
			mColor = mColorBackup.top();
			mColorBackup.pop();
		}
		break;
	}
	}
	
	return false;
}

int Font::SkipTags(const char* text, TextTags::Enum* tag, int* imgLen)
{
	static TextureAtlas* textureAtlas = gFBEnv->pRenderer->GetTextureAtlas("data/textures/gameui.xml");
	if (tag)
	{
		*tag = TextTags::Num;
	}

	if (text[0] == 0)
	{
		return 0;
	}

	if (text[0] == '[' && text[2] == '$')
	{
		// found end
		int pos = 4;
		for (; text[pos] != 0; pos += 2)
		{
			if (text[pos] == '$' && text[pos + 2] == ']')
			{
				int endLen = pos + 4;
				char buf[256];
				TextTags::Enum tagType = GetTagType(text, endLen, buf);

				if (tag)
				{
					*tag = tagType;
				}
				if (imgLen && tagType == TextTags::Img && textureAtlas){
					auto region = textureAtlas->GetRegion(buf);
					if (region)
					{
						auto& regionSize = region->GetSize();
						*imgLen = regionSize.x;
					}
					
				}
				return endLen;
			}
		}
	}
	return 0;
}

TextTags::Enum Font::GetTagType(const char* tagStart, int length, char* buf) const
{
	if (length < 8)
		return TextTags::Num;

	if (tagStart[0] != '[' || tagStart[length - 2] != ']')
		return TextTags::Num;

	if (tagStart[4] == 'i' && tagStart[6] == 'm' && tagStart[8] == 'g')
	{
		if (buf)
		{
			int oneI = 0;
			for (int i = 10; tagStart[i] != '$'; i += 2)
			{
				buf[oneI++] = tagStart[i];
			}
			buf[oneI] = 0;
		}
		
		return TextTags::Img;
	}

	if (tagStart[4] == 'c' && tagStart[6] == 'r')
	{
		if (buf)
		{
			int oneI = 0;
			for (int i = 8; tagStart[i] != '$'; i += 2)
			{
				buf[oneI++] = tagStart[i];
			}
			buf[oneI] = 0;
		}
		return tagStart[8] == '$' ? TextTags::ColorEnd: TextTags::ColorStart;
	}
	
	return TextTags::Num;
}

//----------------------------------------------------------------------------
void Font::InternalWrite(float x, float y, float z, const char *text, int count, float spacing)
{
	static FontVertex vertices[MAX_BATCH];

	const float initialX = x;
	int page = -1;
	y -= mScaledFontSize;
	unsigned int batchingVertices = 0;
	for( int n = 0; n < count; )
	{
		bool reapplyRender = false;
		int skiplen = SkipTags(&text[n]);
		if (skiplen>0)
		{
			do
			{
				reapplyRender = ApplyTag(text, n, n + skiplen, x, y) || reapplyRender;
				n += skiplen;
				skiplen = SkipTags(&text[n]);
			} while (skiplen > 0);
		}		

		if (n >= count)
			break;

		if (reapplyRender)
		{
			PrepareRenderResources();
			if (page!=-1)
				mPages[page]->Bind();
			gFBEnv->pRenderer->UpdateObjectConstantsBuffer(&mObjectConstants);
		}

		int charId = GetTextChar(text, n, &n);
		
		if (charId == L'\n')
		{
			y += mScale * mBase;
			x = initialX;
			if (batchingVertices > 0)
			{
				Flush(page, vertices, batchingVertices);
				batchingVertices = 0;
				mVertexLocation += batchingVertices;
			}
			
			continue;
		}

		SCharDescr *ch = GetChar(charId);
		if( ch == 0 ) 
			ch = &mDefChar;

		// Map the center of the texel to the corners
		// in order to get pixel perfect mapping
		float u = (float(ch->srcX)) / (float)mScaleW;
		float v = (float(ch->srcY)) / (float)mScaleH;
		float u2 = u + float(ch->srcW) / (float)mScaleW;
		float v2 = v + float(ch->srcH) / (float)mScaleH;

		float a = mScale * float(ch->xAdv);
		float w = mScale * float(ch->srcW);
		float h = mScale * float(ch->srcH);
		float ox = mScale * float(ch->xOff);
		float oy = mScale * float(ch->yOff);

		if( ch->page != page)
		{
			if (batchingVertices)
				Flush(page, vertices, batchingVertices);

			mVertexLocation+=batchingVertices;
			batchingVertices = 0;
			page = ch->page;
			mPages[page]->Bind();
		}

		if ( mVertexLocation + batchingVertices + 4 >= MAX_BATCH )
		{
			Flush(page, vertices, batchingVertices);
			batchingVertices = 0;
			mVertexLocation = 0;
		}

		float left = x+ox;
		float top = y + oy;
		float right = left + w;
		float bottom = top + h;

		vertices[mVertexLocation + batchingVertices++] = FontVertex(
			Vec3(left, top, z), mColor, Vec2(u, v), ch->chnl);
		vertices[mVertexLocation + batchingVertices++] = FontVertex(
			Vec3(right, top, z), mColor, Vec2(u2, v), ch->chnl);
		vertices[mVertexLocation + batchingVertices++] = FontVertex(
			Vec3(left, bottom, z), mColor, Vec2(u, v2), ch->chnl);
		vertices[mVertexLocation + batchingVertices++] = FontVertex(
			Vec3(right, bottom, z), mColor, Vec2(u2, v2), ch->chnl);		

		x += a;
		if( charId == ' ' )
			x += spacing;

		if( n < count )
			x += AdjustForKerningPairs(charId, GetTextChar(text,n));
	}
	Flush(page, vertices, batchingVertices);
	mVertexLocation+=batchingVertices;
}

//----------------------------------------------------------------------------
void Font::Flush(int page, const FontVertex* pVertices, unsigned int vertexCount)
{
	if (page==-1 || vertexCount ==0)
		return;

	IRenderer* pRenderer = gFBEnv->pEngine->GetRenderer();

	MapData data = pRenderer->MapVertexBuffer(mVertexBuffer, 0, 
		MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
	FontVertex* pDest = (FontVertex*)data.pData;
	memcpy(pDest + mVertexLocation, pVertices + mVertexLocation, 
		vertexCount * sizeof(FontVertex));

	pRenderer->UnmapVertexBuffer(mVertexBuffer, 0);	
	mVertexBuffer->Bind();
	pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pRenderer->Draw(vertexCount, mVertexLocation);
}

//----------------------------------------------------------------------------
void Font::Write(float x, float y, float z, unsigned int color, 
	const char *text, int count, FONT_ALIGN mode)
{
	if (!mInitialized)
		return;

	//
	if( count < 0 )
		count = GetTextLength(text);

	if( mode == FONT_ALIGN_CENTER )
	{
		float w = GetTextWidth(text, count);
		x -= w/2;
	}
	else if( mode == FONT_ALIGN_RIGHT )
	{
		float w = GetTextWidth(text, count);
		x -= w;
	}

	mColor = color;

	gFBEnv->pRenderer->UpdateObjectConstantsBuffer(&mObjectConstants);

	InternalWrite(x, y, z, text, count);
}

//----------------------------------------------------------------------------
void Font::SetHeight(float h)
{
	mScale = h / float(mFontHeight);
	mScaledFontSize = short(Round(h));
}

void Font::SetBackToOrigHeight()
{
	mScale = 1.f;
}

//----------------------------------------------------------------------------
float Font::GetHeight() const
{
	return mScale * mFontHeight;
}

float Font::GetBaseHeight() const
{
	return mScale * mBase;
}

std::wstring Font::InsertLineFeed(const char *text, int count, unsigned wrapAt, float* outWidth, unsigned* outLines)
{
	if (count < 0)
		count = GetTextLength(text);

	unsigned lines = 1;
	std::vector<float> maxes;
	float dummyMax = 0;
	float curX = 0;
	std::wstring multilineString;
	unsigned inputBeforeSpace=0;
	float lengthAfterSpace = 0;
	for (int n = 0; n < count;)
	{
		TextTags::Enum tag = TextTags::Num;
		int imgLen = 0;
		int numSkip = SkipTags(&text[n], &tag, &imgLen);
		if (numSkip)
		{
			do
			{
				if (tag == TextTags::Img)
				{
					curX += imgLen;
					lengthAfterSpace += imgLen;
				}
				int startN = n;
				for (; n < startN + numSkip;)
				{
					int charId = GetTextChar(text, n, &n);
					++inputBeforeSpace;
					multilineString.push_back(charId);
				}
				numSkip = SkipTags(&text[n], &tag, &imgLen);

			} while (numSkip);
			if (n >= count)
				break;
		}

		int charId = GetTextChar(text, n, &n);
		if (charId == L'\n')
		{
			maxes.push_back(curX);
			curX = 0;
			lengthAfterSpace = 0;
			++lines;
		}
		else
		{
			SCharDescr *ch = GetChar(charId);
			if (ch == 0) ch = &mDefChar;
			float length = (mScale * (ch->xAdv));
			if (n < count)
				length += AdjustForKerningPairs(charId, GetTextChar(text, n));
			curX += length;
			lengthAfterSpace += length;
		}
		
		if (curX > wrapAt)
		{
			multilineString.insert(multilineString.end() - inputBeforeSpace, L'\n');
			maxes.push_back(curX - lengthAfterSpace);
			curX = lengthAfterSpace;
			multilineString.push_back(charId);
			++lines;
			inputBeforeSpace = 0;
			lengthAfterSpace = 0;
		}
		else
		{
			if (iswspace(charId))
			{
				inputBeforeSpace = 0;
				lengthAfterSpace = 0;
			}
			else
			{
				++inputBeforeSpace;
			}
			multilineString.push_back(charId);
		}
		dummyMax = std::max(dummyMax, curX);
	}
	maxes.push_back(curX);

	if (outWidth)
	{
		if (maxes.empty())
			*outWidth = dummyMax;
		else
		{
			float biggest = 0;
			for (auto val : maxes)
			{
				biggest = std::max(biggest, val);
				*outWidth = biggest;
			}
		}
	}

	if (outLines)
		*outLines = lines;

	return multilineString;
}
//----------------------------------------------------------------------------
float Font::GetTextWidth(const char *text, int count, float *outMinY/* = 0*/, float *outMaxY/* = 0*/)
{
	if( count < 0 )
		count = GetTextLength(text);

	float x = 0;
	float minY = 10000;
	float maxY = -10000;
	
	for( int n = 0; n < count; )
	{
		TextTags::Enum tag;
		int imgLen;
		int skiplen = SkipTags(&text[n], &tag, &imgLen);
		if (skiplen>0)
		{
			if (tag == TextTags::Img)
			{
				x += imgLen;
			}
			do
			{
				n += skiplen;
				skiplen = SkipTags(&text[n], &tag, &imgLen);
				if (tag == TextTags::Img)
				{
					x += imgLen;
				}
			} while (skiplen > 0);
		}
		if (n >= count)
			break;

		int charId = GetTextChar(text,n,&n);

		SCharDescr *ch = GetChar(charId);
		if( ch == 0 ) ch = &mDefChar;

		x += mScale * (ch->xAdv);
		float h = mScale * float(ch->srcH);
		float y = mScale * (float(mBase) - float(ch->yOff));
		if( minY > y-h )
			minY = y-h;
		if( maxY < y )
			maxY = y;

		if( n < count )
			x += AdjustForKerningPairs(charId, GetTextChar(text,n));
	}

	if( outMinY ) *outMinY = minY;
	if( outMaxY ) *outMaxY = maxY;

	return x;
}

//----------------------------------------------------------------------------
void Font::PrepareRenderResources()
{
	if (!mInitialized)
		return;
	gFBEnv->pRenderer->SetAlphaBlendState();
	mShader->Bind();
	mInputLayout->Bind();
}

void Font::SetRenderStates(bool depthEnable, bool scissorEnable)
{
	if (!mInitialized)
		return;

	if (scissorEnable)
	{
		RASTERIZER_DESC rd;
		rd.ScissorEnable = true;
		mTextureMaterial->SetRasterizerState(rd);
	}
	else
	{
		RASTERIZER_DESC rd;
		rd.ScissorEnable = false;
		mTextureMaterial->SetRasterizerState(rd);
	}
	if (depthEnable)
	{
		mTextureMaterial->SetDepthStencilState(DEPTH_STENCIL_DESC());
		
	}
	else
	{
		DEPTH_STENCIL_DESC desc;
		desc.DepthEnable = false;
		mTextureMaterial->SetDepthStencilState(desc);
	}
}

//----------------------------------------------------------------------------
int Font::GetTextLength(const char *text)
{
	if( mEncoding == UTF16 )
	{
		int textLen = 0;
		for(;;)
		{
			unsigned int len;
			int r = DecodeUTF16((const unsigned char *)&text[textLen], &len);
			if (r > 0)
			{
				textLen += len;
			}				
			else if (r < 0)
			{
				textLen++;
			}				
			else
				return textLen;
		}
	}

	// Both UTF8 and standard ASCII strings can use strlen
	return (int)strlen(text);
}

//----------------------------------------------------------------------------
// Own
//----------------------------------------------------------------------------
float Font::GetBottomOffset()
{
	return mScale * (mBase - mFontHeight);
}

float Font::GetTopOffset()
{
	return mScale * (mBase - 0);
}

//----------------------------------------------------------------------------
float Font::AdjustForKerningPairs(int first, int second)
{	
	SCharDescr *ch = GetChar(first);
	if( ch == 0 ) return 0;
	for( UINT n = 0; n < ch->kerningPairs.size(); n += 2 )
	{
		if( ch->kerningPairs[n] == second )
			return ch->kerningPairs[n+1] * mScale;
	}

	return 0;
}

//----------------------------------------------------------------------------
SCharDescr *Font::GetChar(int id)
{
	std::map<int, SCharDescr*>::iterator it = mChars.find(id);
	if( it == mChars.end() ) return 0;

	return it->second;
}

//----------------------------------------------------------------------------
int Font::GetTextChar(const char *text, int pos, int *nextPos)
{
	int ch;
	unsigned int len;
	if( mEncoding == UTF8 )
	{
		ch = DecodeUTF8((const unsigned char *)&text[pos], &len);
		if( ch == -1 ) len = 1;
	}
	else if( mEncoding == UTF16 )
	{
		ch = DecodeUTF16((const unsigned char *)&text[pos], &len);
		if( ch == -1 ) len = 2;
	}
	else
	{
		len = 1;
		ch = (unsigned char)text[pos];
	}

	if( nextPos ) *nextPos = pos + len;
	return ch;
}

void Font::SetRenderTargetSize(const Vec2I& rtSize)
{
	mRenderTargetSize = rtSize;
	mObjectConstants.gWorldViewProj = MakeOrthogonalMatrix(0, 0,
		(float)mRenderTargetSize.x,
		(float)mRenderTargetSize.y,
		0.f, 1.0f);
	mObjectConstants.gWorld.MakeIdentity();
}

void Font::RestoreRenderTargetSize()
{
	const auto& rtSize = gFBEnv->_pInternalRenderer->GetMainRTSize();
	mRenderTargetSize = rtSize;
	mObjectConstants.gWorldViewProj = MakeOrthogonalMatrix(0, 0,
		(float)mRenderTargetSize.x,
		(float)mRenderTargetSize.y,
		0.f, 1.0f);
	mObjectConstants.gWorld.MakeIdentity();
}

std::wstring Font::StripTags(const wchar_t* text)
{
	std::wstring newText;
	int len = wcslen(text);
	bool tagStarted = false;
	for (int i = 0; i < len; i++)
	{
		if (text[i] == L'[' && text[i+1] == '$')
		{
			for (int e = i + 2; e < len; ++e)
			{
				if (text[e] == '$' && text[e+1] == ']')
				{
					i = e + 2;
					if (i >= len)
					{
						break;
					}
				}
			}
		}
		newText.push_back(text[i]);
	}
	return newText;
}

//=============================================================================
// FontLoader
//
// This is the base class for all loader classes. This is the only class
// that has access to and knows how to set the Font members.
//=============================================================================

FontLoader::FontLoader(FILE *f, Font *font, const char *fontFile)
{
	this->f = f;
	this->font = font;
	this->fontFile = fontFile;

	outlineThickness = 0;
}

void FontLoader::LoadPage(int id, const char *pageFile, const std::string& fontFile)
{
	std::string str;

	// Load the texture from the same directory as the font descriptor file

	// Find the directory
	str = fontFile;
	for( size_t n = 0; (n = str.find('/', n)) != std::string::npos; ) str.replace(n, 1, "\\");
	size_t i = str.rfind('\\');
	if( i != std::string::npos )
		str = str.substr(0, i+1);
	else
		str = "";

	// Load the font textures
	str += pageFile;
	font->mPages[id] = gFBEnv->pEngine->GetRenderer()->CreateTexture(str.c_str());
	font->mPages[id]->SetShaderStage(BINDING_SHADER_PS);
	font->mPages[id]->SetSlot(0);
	//font->mPages[id]->SetSamplerDesc(SAMPLER_DESC());

	//font->mPages[id]->SetSamplerDesc(SAMPLER_DESC());
	
	if( font->mPages[id]==0 )
		gFBEnv->pEngine->Log("Failed to load font page '$s'!", str.c_str());
}

void FontLoader::SetFontInfo(int outlineThickness)
{
	this->outlineThickness = outlineThickness;
}

void FontLoader::SetCommonInfo(int fontHeight, int base, int scaleW, int scaleH, int pages, bool isPacked)
{
	font->mFontHeight = fontHeight;
	font->mScaledFontSize = font->mFontHeight;
	font->mBase = base;
	font->mScaleW = scaleW;
	font->mScaleH = scaleH;
	font->mPages.resize(pages);
	for( int n = 0; n < pages; n++ )
		font->mPages[n] = 0;

	if( isPacked && outlineThickness )
		font->mHasOutline = true;
}

void FontLoader::AddChar(int id, int x, int y, int w, int h, int xoffset, int yoffset, int xadvance, int page, int chnl)
{
	// Convert to a 4 element vector
	// TODO: Does this depend on hardware? It probably does
	if     ( chnl == 1 ) chnl = 0x00010000;  // Blue channel
	else if( chnl == 2 ) chnl = 0x00000100;  // Green channel
	else if( chnl == 4 ) chnl = 0x00000001;  // Red channel
	else if( chnl == 8 ) chnl = 0x01000000;  // Alpha channel
	else chnl = 0;

	if( id >= 0 )
	{
		SCharDescr *ch = FB_NEW(SCharDescr);
		ch->srcX = x;
		ch->srcY = y;
		ch->srcW = w;
		ch->srcH = h;
		ch->xOff = xoffset;
		ch->yOff = yoffset;
		ch->xAdv = xadvance;
		ch->page = page;
		ch->chnl = chnl;

		font->mChars.insert(std::map<int, SCharDescr*>::value_type(id, ch));
	}

	if( id == -1 )
	{
		font->mDefChar.srcX = x;
		font->mDefChar.srcY = y;
		font->mDefChar.srcW = w;
		font->mDefChar.srcH = h;
		font->mDefChar.xOff = xoffset;
		font->mDefChar.yOff = yoffset;
		font->mDefChar.xAdv = xadvance;
		font->mDefChar.page = page;
		font->mDefChar.chnl = chnl;
	}
}

void FontLoader::AddKerningPair(int first, int second, int amount)
{
	if( first >= 0 && first < 256 && font->mChars[first] )
	{
		font->mChars[first]->kerningPairs.push_back(second);
		font->mChars[first]->kerningPairs.push_back(amount);
	}
}

//=============================================================================
// FontLoaderTextFormat
//
// This class implements the logic for loading a BMFont file in text format
//=============================================================================

FontLoaderTextFormat::FontLoaderTextFormat(FILE *f, Font *font, const char *fontFile) 
	: FontLoader(f, font, fontFile)
{
}

int FontLoaderTextFormat::Load()
{
	std::string line;

	while( !feof(f) )
	{
		// Read until line feed (or EOF)
		line = "";
		line.reserve(256);
		while( !feof(f) )
		{
			char ch;
			if( fread(&ch, 1, 1, f) )
			{
				if( ch != '\n' ) 
					line += ch; 
				else
					break;
			}
		}

		// Skip white spaces
		int pos = SkipWhiteSpace(line, 0);
		if( pos == line.size() ) 
			break;
		// Read token
		int pos2 = FindEndOfToken(line, pos);
		std::string token = line.substr(pos, pos2-pos);

		// Interpret line
		if( token == "info" )
		{
			InterpretInfo(line, pos2);
		}
		else if( token == "common" )
		{
			InterpretCommon(line, pos2);
		}
		else if( token == "char" )
		{
			InterpretChar(line, pos2);
		}
		else if( token == "kerning" )
		{
			InterpretKerning(line, pos2);
		}
		else if( token == "page" )
		{
			InterpretPage(line, pos2, fontFile);
		}
	}

	fclose(f);

	// Success
	return 0;
}

int FontLoaderTextFormat::SkipWhiteSpace(const std::string &str, int start)
{
	UINT n = start;
	while( n < str.size() )
	{
		char ch = str[n];
		if( ch != ' ' && 
			ch != '\t' && 
			ch != '\r' && 
			ch != '\n' )
			break;

		++n;
	}

	return n;
}

int FontLoaderTextFormat::FindEndOfToken(const std::string &str, int start)
{
	UINT n = start;
	if( str[n] == '"' )
	{
		n++;
		while( n < str.size() )
		{
			char ch = str[n];
			if( ch == '"' )
			{
				// Include the last quote char in the token
				++n;
				break;
			}
			++n;
		}
	}
	else
	{
		while( n < str.size() )
		{
			char ch = str[n];
			if( ch == ' ' ||
				ch == '\t' ||
				ch == '\r' ||
				ch == '\n' ||
				ch == '=' )
				break;

			++n;
		}
	}

	return n;
}

void FontLoaderTextFormat::InterpretKerning(const std::string &str, int start)
{
	// Read the attributes
	int first = 0;
	int second = 0;
	int amount = 0;

	int pos, pos2 = start;
	while( true )
	{
		pos = SkipWhiteSpace(str, pos2);
		if( pos == str.size() ) 
			break;
		pos2 = FindEndOfToken(str, pos);

		std::string token = str.substr(pos, pos2-pos);

		pos = SkipWhiteSpace(str, pos2);
		if( pos == str.size() || str[pos] != '=' ) break;

		pos = SkipWhiteSpace(str, pos+1);
		pos2 = FindEndOfToken(str, pos);

		std::string value = str.substr(pos, pos2-pos);

		if( token == "first" )
			first = strtol(value.c_str(), 0, 10);
		else if( token == "second" )
			second = strtol(value.c_str(), 0, 10);
		else if( token == "amount" )
			amount = strtol(value.c_str(), 0, 10);

		if( pos == str.size() ) break;
	}

	// Store the attributes
	AddKerningPair(first, second, amount);
}

void FontLoaderTextFormat::InterpretChar(const std::string &str, int start)
{
	// Read all attributes
	int id = 0;
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	int xoffset = 0;
	int yoffset = 0;
	int xadvance = 0;
	int page = 0;
	int chnl = 0;

	int pos, pos2 = start;
	while( true )
	{
		pos = SkipWhiteSpace(str, pos2);
		if( pos == str.size() ) 
			break;
		pos2 = FindEndOfToken(str, pos);

		std::string token = str.substr(pos, pos2-pos);

		pos = SkipWhiteSpace(str, pos2);
		if( pos == str.size() || str[pos] != '=' ) break;

		pos = SkipWhiteSpace(str, pos+1);
		pos2 = FindEndOfToken(str, pos);

		std::string value = str.substr(pos, pos2-pos);

		if( token == "id" )
			id = strtol(value.c_str(), 0, 10);
		else if( token == "x" )
			x = strtol(value.c_str(), 0, 10);
		else if( token == "y" )
			y = strtol(value.c_str(), 0, 10);
		else if( token == "width" )
			width = strtol(value.c_str(), 0, 10);
		else if( token == "height" )
			height = strtol(value.c_str(), 0, 10);
		else if( token == "xoffset" )
			xoffset = strtol(value.c_str(), 0, 10);
		else if( token == "yoffset" )
			yoffset = strtol(value.c_str(), 0, 10);
		else if( token == "xadvance" )
			xadvance = strtol(value.c_str(), 0, 10);
		else if( token == "page" )
			page = strtol(value.c_str(), 0, 10);
		else if( token == "chnl" )
			chnl = strtol(value.c_str(), 0, 10);

		if( pos == str.size() ) break;
	}

	// Store the attributes
	AddChar(id, x, y, width, height, xoffset, yoffset, xadvance, page, chnl);
}

void FontLoaderTextFormat::InterpretCommon(const std::string &str, int start)
{
	int fontHeight;
	int base;
	int scaleW;
	int scaleH;
	int pages;
	int packed = 0;

	// Read all attributes
	int pos, pos2 = start;
	while( true )
	{
		pos = SkipWhiteSpace(str, pos2);
		if( pos == str.size() ) 
			break;
		pos2 = FindEndOfToken(str, pos);

		std::string token = str.substr(pos, pos2-pos);

		pos = SkipWhiteSpace(str, pos2);
		if( pos == str.size() || str[pos] != '=' ) break;

		pos = SkipWhiteSpace(str, pos+1);
		pos2 = FindEndOfToken(str, pos);

		std::string value = str.substr(pos, pos2-pos);

		if( token == "lineHeight" )
			fontHeight = (short)strtol(value.c_str(), 0, 10);
		else if( token == "base" )
			base = (short)strtol(value.c_str(), 0, 10);
		else if( token == "scaleW" )
			scaleW = (short)strtol(value.c_str(), 0, 10);
		else if( token == "scaleH" )
			scaleH = (short)strtol(value.c_str(), 0, 10);
		else if( token == "pages" )
			pages = strtol(value.c_str(), 0, 10);
		else if( token == "packed" )
			packed = strtol(value.c_str(), 0, 10);

		if( pos == str.size() ) break;
	}

	SetCommonInfo(fontHeight, base, scaleW, scaleH, pages, packed ? true : false);
}

void FontLoaderTextFormat::InterpretInfo(const std::string &str, int start)
{
	int outlineThickness = 0;

	// Read all attributes
	int pos, pos2 = start;
	while( true )
	{
		pos = SkipWhiteSpace(str, pos2);
		if( pos == str.size() ) 
			break;
		pos2 = FindEndOfToken(str, pos);

		std::string token = str.substr(pos, pos2-pos);

		pos = SkipWhiteSpace(str, pos2);
		if( pos == str.size() || str[pos] != '=' ) 
			break;

		pos = SkipWhiteSpace(str, pos+1);
		pos2 = FindEndOfToken(str, pos);

		std::string value = str.substr(pos, pos2-pos);

		if( token == "outline" )
			outlineThickness = (short)strtol(value.c_str(), 0, 10);

		if( pos == str.size() ) 
			break;
	}

	SetFontInfo(outlineThickness);
}

void FontLoaderTextFormat::InterpretPage(const std::string str, int start, 
	const std::string fontFile)
{
	int id = 0;
	std::string file;

	// Read all attributes
	int pos, pos2 = start;
	while( true )
	{
		pos = SkipWhiteSpace(str, pos2);
		if( pos == str.size() ) 
			break;
		pos2 = FindEndOfToken(str, pos);

		std::string token = str.substr(pos, pos2-pos);

		pos = SkipWhiteSpace(str, pos2);
		if( pos == str.size() || str[pos] != '=' ) break;

		pos = SkipWhiteSpace(str, pos+1);
		pos2 = FindEndOfToken(str, pos);

		std::string value = str.substr(pos, pos2-pos);

		if( token == "id" )
			id = strtol(value.c_str(), 0, 10);
		else if( token == "file" )
			file = value.substr(1, value.length()-2);

		if( pos == str.size() ) break;
	}

	LoadPage(id, file.c_str(), fontFile);
}

//=============================================================================
// FontLoaderBinaryFormat
//
// This class implements the logic for loading a BMFont file in binary format
//=============================================================================

FontLoaderBinaryFormat::FontLoaderBinaryFormat(FILE *f, Font *font, const char *fontFile) 
	: FontLoader(f, font, fontFile)
{
}

int FontLoaderBinaryFormat::Load()
{
	// Read and validate the tag. It should be 66, 77, 70, 2, 
	// or 'BMF' and 2 where the number is the file version.
	char magicString[4];
	fread(magicString, 4, 1, f);
	if( strncmp(magicString, "BMF\003", 4) != 0 )
	{
		Log("Unrecognized format for '$s'", fontFile.c_str());
		fclose(f);
		return -1;
	}

	// Read each block
	char blockType;
	int blockSize;
	while( fread(&blockType, 1, 1, f) )
	{
		// Read the blockSize
		fread(&blockSize, 4, 1, f);

		switch( blockType )
		{
		case 1: // info
			ReadInfoBlock(blockSize);
			break;
		case 2: // common
			ReadCommonBlock(blockSize);
			break;
		case 3: // pages
			ReadPagesBlock(blockSize);
			break;
		case 4: // chars
			ReadCharsBlock(blockSize);
			break;
		case 5: // kerning pairs
			ReadKerningPairsBlock(blockSize);
			break;
		default:
			gFBEnv->pEngine->Log("Unexpected block type (%d)", blockType);
			fclose(f);
			return -1;
		}
	}

	fclose(f);

	// Success
	return 0;
}

void FontLoaderBinaryFormat::ReadInfoBlock(int size)
{
#pragma pack(push)
#pragma pack(1)
struct infoBlock
{
    unsigned short fontSize;
    unsigned char  reserved:4;
    unsigned char  bold    :1;
    unsigned char  italic  :1;
    unsigned char  unicode :1;
    unsigned char  smooth  :1;
    unsigned char  charSet;
    unsigned short stretchH;
    unsigned char  aa;
    unsigned char  paddingUp;
    unsigned char  paddingRight;
    unsigned char  paddingDown;
    unsigned char  paddingLeft;
    unsigned char  spacingHoriz;
    unsigned char  spacingVert;
    unsigned char  outline;         // Added with version 2
    char fontName[1];
};
#pragma pack(pop)

	char *buffer = FB_ARRNEW(char, size);
	fread(buffer, size, 1, f);

	// We're only interested in the outline thickness
	infoBlock *blk = (infoBlock*)buffer;
	SetFontInfo(blk->outline);

	FB_ARRDELETE(buffer);
}

void FontLoaderBinaryFormat::ReadCommonBlock(int size)
{
#pragma pack(push)
#pragma pack(1)
struct commonBlock
{
    unsigned short lineHeight;
    unsigned short base;
    unsigned short scaleW;
    unsigned short scaleH;
    unsigned short pages;
    unsigned char  packed  :1;
    unsigned char  reserved:7;
	unsigned char  alphaChnl;
	unsigned char  redChnl;
	unsigned char  greenChnl;
	unsigned char  blueChnl;
}; 
#pragma pack(pop)

	char *buffer = FB_ARRNEW(char, size);
	fread(buffer, size, 1, f);

	commonBlock *blk = (commonBlock*)buffer;

	SetCommonInfo(blk->lineHeight, blk->base, blk->scaleW, blk->scaleH, blk->pages, blk->packed ? true : false);

	FB_ARRDELETE(buffer);
}

void FontLoaderBinaryFormat::ReadPagesBlock(int size)
{
#pragma pack(push)
#pragma pack(1)
struct pagesBlock
{
    char pageNames[1];
};
#pragma pack(pop)

	char *buffer = FB_ARRNEW(char, size);
	fread(buffer, size, 1, f);

	pagesBlock *blk = (pagesBlock*)buffer;

	for( int id = 0, pos = 0; pos < size; id++ )
	{
		LoadPage(id, &blk->pageNames[pos], fontFile);
		pos += 1 + (int)strlen(&blk->pageNames[pos]);
	}

	FB_ARRDELETE(buffer);
}

void FontLoaderBinaryFormat::ReadCharsBlock(int size)
{
#pragma pack(push)
#pragma pack(1)
struct charsBlock
{
    struct charInfo
    {
        DWORD id;
        unsigned short x;
        unsigned short y;
        unsigned short width;
        unsigned short height;
        short xoffset;
        short yoffset;
        short xadvance;
        unsigned char page;
        unsigned char chnl;
    } chars[1];
};
#pragma pack(pop)

	char *buffer = FB_ARRNEW(char, size);
	fread(buffer, size, 1, f);

	charsBlock *blk = (charsBlock*)buffer;

	for( int n = 0; int(n*sizeof(charsBlock::charInfo)) < size; n++ )
	{
		AddChar(blk->chars[n].id,
		        blk->chars[n].x,
				blk->chars[n].y,
				blk->chars[n].width,
				blk->chars[n].height,
				blk->chars[n].xoffset,
				blk->chars[n].yoffset,
				blk->chars[n].xadvance,
				blk->chars[n].page,
				blk->chars[n].chnl);
	}

	FB_ARRDELETE(buffer);
}

void FontLoaderBinaryFormat::ReadKerningPairsBlock(int size)
{
#pragma pack(push)
#pragma pack(1)
struct kerningPairsBlock
{
    struct kerningPair
    {
        DWORD first;
        DWORD second;
        short amount;
    } kerningPairs[1];
};
#pragma pack(pop)

	char *buffer = FB_ARRNEW(char, size);
	fread(buffer, size, 1, f);

	kerningPairsBlock *blk = (kerningPairsBlock*)buffer;

	for( int n = 0; int(n*sizeof(kerningPairsBlock::kerningPair)) < size; n++ )
	{
		AddKerningPair(blk->kerningPairs[n].first,
		               blk->kerningPairs[n].second,
					   blk->kerningPairs[n].amount);
	}

	FB_ARRDELETE(buffer);
}
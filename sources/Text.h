// (c) 2017 Naoki Nakagawa
#pragma once
#include <strsafe.h>

namespace GameLibrary {
	class Text : public Sprite {
		PUBLIC Text(const TCHAR* text, const TCHAR* fontFamily = _T("�l�r ����")) {
			LOGFONT logFont = {};
			logFont.lfHeight = 256;
			logFont.lfWidth = 0;
			logFont.lfEscapement = 0;
			logFont.lfOrientation = 0;
			logFont.lfWeight = 0;
			logFont.lfItalic = false;
			logFont.lfUnderline = false;
			logFont.lfStrikeOut = false;
			logFont.lfCharSet = SHIFTJIS_CHARSET;
			logFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;
			logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			logFont.lfQuality = PROOF_QUALITY;
			logFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
			StringCchCopy(logFont.lfFaceName, 32, fontFamily);
			HFONT font = CreateFontIndirect(&logFont);

			HDC dc = GetDC(nullptr);
			HFONT oldFont = (HFONT)SelectObject(dc, font);

			UINT code = 0;
#if _UNICODE
			code = (UINT)*text;
#else
			if (IsDBCSLeadByte(*text)) {
				code = (BYTE)text[0] << 8 | (BYTE)text[1];
			}
			else {
				code = text[0];
			}
#endif

			TEXTMETRIC textMetrics = {};
			GetTextMetrics(dc, &textMetrics);
			GLYPHMETRICS glyphMetrics;
			const MAT2 matrix = { { 0, 1 },{ 0, 0 },{ 0, 0 },{ 0, 1 } };
			DWORD size = GetGlyphOutline(dc, code, GGO_GRAY4_BITMAP, &glyphMetrics, 0, nullptr, &matrix);
			BYTE* ptr = new BYTE[size];
			GetGlyphOutline(dc, code, GGO_GRAY4_BITMAP, &glyphMetrics, size, ptr, &matrix);

			SelectObject(dc, oldFont);
			DeleteObject(font);
			ReleaseDC(nullptr, dc);

			width = glyphMetrics.gmCellIncX;
			height = textMetrics.tmHeight;

			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width = width;
			textureDesc.Height = height;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DYNAMIC;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			textureDesc.MiscFlags = 0;

			Game::GetDevice().CreateTexture2D(&textureDesc, nullptr, &texture);

			D3D11_MAPPED_SUBRESOURCE mapped;
			Game::GetDeviceContext().Map(texture, D3D11CalcSubresource(0, 0, 1), D3D11_MAP_WRITE_DISCARD, 0, &mapped);

			BYTE* bits = (BYTE*)mapped.pData;
			DirectX::XMINT2 origin;
			origin.x = glyphMetrics.gmptGlyphOrigin.x;
			origin.y = textMetrics.tmAscent - glyphMetrics.gmptGlyphOrigin.y;
			DirectX::XMINT2 bitmapSize;
			bitmapSize.x = glyphMetrics.gmBlackBoxX + (4 - (glyphMetrics.gmBlackBoxX % 4)) % 4;
			bitmapSize.y = glyphMetrics.gmBlackBoxY;
			const int LEVEL = 17;
			memset(bits, 0, mapped.RowPitch * textMetrics.tmHeight);

			for (int y = origin.y; y < origin.y + bitmapSize.y; y++) {
				for (int x = origin.x; x < origin.x + bitmapSize.x; x++) {
					DWORD alpha = (255 * ptr[x - origin.x + bitmapSize.x * (y - origin.y)]) / (LEVEL - 1);
					DWORD color = 0x00ffffff | (alpha << 24);
					memcpy((BYTE*)bits + mapped.RowPitch * y + 4 * x, &color, sizeof(DWORD));
				}
			}

			Game::GetDeviceContext().Unmap(texture, D3D11CalcSubresource(0, 0, 1));
			delete[] ptr;

			Initialize();

			Position() = DirectX::XMFLOAT2(0.0f, 0.0f);
			Angle() = 0.0f;
			Scale() = DirectX::XMFLOAT2(1.0f, 1.0f);
			Color() = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		}
	};
}

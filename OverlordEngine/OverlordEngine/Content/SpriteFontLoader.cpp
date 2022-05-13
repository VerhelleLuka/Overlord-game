#include "stdafx.h"
#include "SpriteFontLoader.h"

SpriteFont* SpriteFontLoader::LoadContent(const ContentLoadInfo& loadInfo)
{
	const auto pReader = new BinaryReader();
	pReader->Open(loadInfo.assetFullPath);

	if (!pReader->Exists())
	{
		Logger::LogError(L"Failed to read the assetFile!\nPath: \'{}\'", loadInfo.assetSubPath);
		return nullptr;
	}

	TODO_W5(L"Implement SpriteFontLoader >> Parse .fnt file")
		//See BMFont Documentation for Binary Layout

		//Parse the Identification bytes (B,M,F)
		if (pReader->Read<byte>() != 'B')
		{

			Logger::LogError(L"SpriteFontLoader::LoadContent > Not a valied.fnt font", loadInfo.assetSubPath);
			return nullptr;
		}
	if (pReader->Read<byte>() != 'M')
	{

		Logger::LogError(L"SpriteFontLoader::LoadContent > Not a valied.fnt font", loadInfo.assetSubPath);
		return nullptr;
	}
	if (pReader->Read<byte>() != 'F')
	{

		Logger::LogError(L"SpriteFontLoader::LoadContent > Not a valied.fnt font", loadInfo.assetSubPath);
		return nullptr;
	}
	//If Identification bytes doesn't match B|M|F,
	//Log Error (SpriteFontLoader::LoadContent > Not a valid .fnt font) &
	//return nullptr
	//...

	//Parse the version (version 3 required)

	if (pReader->Read<byte>() < 3) {
		Logger::LogError(L"SpriteFontLoader::LoadContent > Only .fnt version 3 is supported", loadInfo.assetSubPath);
		return nullptr;
	}
	//If version is < 3,
	//Log Error (SpriteFontLoader::LoadContent > Only .fnt version 3 is supported)
	//return nullptr
	//...

	//Valid .fnt file >> Start Parsing!
	//use this SpriteFontDesc to store all relevant information (used to initialize a SpriteFont object)
	SpriteFontDesc fontDesc{};

	//**********
	// BLOCK 0 *
	//**********
	//Retrieve the blockId and blockSize
	int blockId = pReader->Read<byte>();
	int size = pReader->Read<int>();
	//Retrieve the FontSize [fontDesc.fontSize]
	auto fontSize = pReader->Read<short>();
	//Move the binreader to the start of the FontName [BinaryReader::MoveBufferPosition(...) or you can set its position using BinaryReader::SetBufferPosition(...))
	pReader->MoveBufferPosition(12);
	//Retrieve the FontName [fontDesc.fontName]
	
	fontDesc.fontName = pReader->ReadNullString();

	fontDesc.fontSize = fontSize;
	//**********
	// BLOCK 1 *
	//**********
	//Retrieve the blockId and blockSize
	blockId = pReader->Read<byte>();
	size = pReader->Read<int>();

	//Retrieve Texture Width & Height [fontDesc.textureWidth/textureHeight]
	pReader->MoveBufferPosition(4);

	fontDesc.textureWidth = pReader->Read<UINT16>();
	fontDesc.textureHeight = pReader->Read<UINT16>();
	//Retrieve PageCount
	UINT16 pageCount = pReader->Read<UINT16>();
	//> if pagecount > 1
	if (pageCount > 1)
	{
		Logger::LogError(L"Only one texture per font is allowed!", loadInfo.assetSubPath);
		return nullptr;
	}
	//	> Log Error (Only one texture per font is allowed!)
	//Advance to Block2 (Move Reader)
	//...
	pReader->MoveBufferPosition(5);

	//**********
	// BLOCK 2 *
	//**********
	//Retrieve the blockId and blockSize
	blockId = pReader->Read<byte>();
	size = pReader->Read<int>();
	//Retrieve the PageName (BinaryReader::ReadNullString)
	std::wstring pageName = pReader->ReadNullString();
	//Construct the full path to the page texture file
	if (pageName.empty()) {
		Logger::LogError(L"SpriteFontLoader::LoadContent > SpriteFont (.fnt): Invlid Font, No page name found", loadInfo.assetSubPath);
		return nullptr;
	}
	std::wstring fullTextName = loadInfo.assetFullPath;
	auto textSize = fullTextName.rfind(L"Consolas_32.fnt");
	if (textSize != std::string::npos) {
		fullTextName = fullTextName.substr(0, textSize);
	}

	fullTextName.append(pageName);
	//	>> page texture should be stored next to the .fnt file, pageName contains the name of the texture file
	//	>> full texture path = asset parent_path of .fnt file (see loadInfo.assetFullPath > get parent_path) + pageName (filesystem::path::append)
	//	>> Load the texture (ContentManager::Load<TextureData>) & Store [fontDesc.pTexture]

	fontDesc.pTexture = ContentManager::Load<TextureData>(fullTextName);
	//**********
	// BLOCK 3 *
	//**********
	//Retrieve the blockId and blockSize
	blockId = pReader->Read<byte>();
	size = pReader->Read<int>();
	//Retrieve Character Count (see documentation)
	int charCount = size / 20;
	//Create loop for Character Count, and:
	//> Retrieve CharacterId (store Local) and cast to a 'wchar_t'
	for (int i{}; i < charCount; ++i)
	{
		UINT32 characterIdUINT = pReader->Read<UINT32>();
		wchar_t characterId =static_cast<wchar_t>(characterIdUINT);

		//> Create instance of FontMetric (struct)
		//	> Set Character (CharacterId) [FontMetric::character]
		FontMetric fontMetric;
		fontMetric.character = characterId;
		//	> Retrieve Xposition (store Local)
		//	> Retrieve Yposition (store Local)
		int x = (int)pReader->Read<short>();
		int y = (int)pReader->Read<short>();

		//	> Retrieve & Set Width [FontMetric::width]
		//	> Retrieve & Set Height [FontMetric::height]
		fontMetric.width = pReader->Read<unsigned short>();
		fontMetric.height = pReader->Read<unsigned short>();

		//	> Retrieve & Set OffsetX [FontMetric::offsetX]
		//	> Retrieve & Set OffsetY [FontMetric::offsetY]
		fontMetric.offsetX = pReader->Read<short>();
		fontMetric.offsetY = pReader->Read<short>();

		//	> Retrieve & Set AdvanceX [FontMetric::advanceX]
		//	> Retrieve & Set Page [FontMetric::page]
		fontMetric.advanceX = pReader->Read<short>();
		fontMetric.page = pReader->Read<unsigned char>();

		//	> Retrieve Channel (BITFIELD!!!) 
		//		> See documentation for BitField meaning [FontMetrix::channel]
		//	> Calculate Texture Coordinates using Xposition, Yposition, fontDesc.TextureWidth & fontDesc.TextureHeight [FontMetric::texCoord]
		
		//> Insert new FontMetric to the metrics [font.metrics] map
		//	> key = (wchar_t) charId
		//	> value = new FontMetric
		//(loop restarts till all metrics are parsed)
		BYTE bitField = pReader->Read<BYTE>();
		int rightMostPos = int(log2(bitField & -bitField) + 1);
		switch (rightMostPos)
		{
		case 1:
			fontMetric.channel = 2;
			break;
		case 2:
			fontMetric.channel = 1;
			break;
		case 3:
			fontMetric.channel = 0;
			break;
		case 4:
			fontMetric.channel = 3;
			break;
		}

		//> Calculate Texture Coordinates using Xposition, Yposition, TextureWidth & TextureHeight [FontMetric::TexCoord]
		//...
		fontMetric.texCoord = DirectX::XMFLOAT2(x / float(fontDesc.textureWidth), y / float(fontDesc.textureHeight));
		//
		//> Insert new FontMetric to the metrics [font.metrics] map
		//	> key = (wchar_t) charId
		//	> value = new FontMetric
		//(loop restarts till all metrics are parsed)

		fontDesc.metrics.insert(std::make_pair(fontMetric.character, fontMetric));
	}
	//Done!
	delete pReader;
	return new SpriteFont(fontDesc);
}

void SpriteFontLoader::Destroy(SpriteFont* objToDestroy)
{
	SafeDelete(objToDestroy);
}

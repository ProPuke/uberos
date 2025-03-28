#pragma once

#include <common/maths/Fixed.hpp>

#include "Buffer.hpp"

namespace graphics2d {
	struct FontCharacter {
		U32 code;

		maths::Fixed<I16,256> advance;

		maths::Fixed<I16,256> offsetX;
		maths::Fixed<I16,256> offsetY;

		U32 atlasX;
		U32 atlasY;
		U32 atlasWidth;
		U32 atlasHeight;
	};

	struct Font {
		Buffer atlas;

		U32 size;
		float lineHeight;
		float capHeight;
		float ascender;
		float descender;
		float underlineY;
		float underlineThickness;

		U32 characterCount;
		FontCharacter *characters; //must be stored in ascending code order

		FontCharacter* get_character(U32 code){
			//TODO:optimise: don't loop through EVERY character. Use a fixed size array or estimated starting position and intelligent search dir
			for(U32 i=0;i<characterCount;i++){
				auto &character = characters[i];
				if(character.code>code) break;
				if(character.code==code) return &character;
			}

			return nullptr;
		}
	};
}
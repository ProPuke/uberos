import * as fs from 'https://deno.land/std@0.135.0/fs/mod.ts';

for await(const dirEntry of Deno.readDir('.')){
	let matches:RegExpMatchArray|null;
	if(matches = dirEntry.name.match(/(.*)\.msdf.json$/)){
		const name = matches[1];

		if(fs.existsSync(`${name}.msdf.json`) && fs.existsSync(`${name}.msdf.txt`)){
			const msdfJson = JSON.parse(Deno.readTextFileSync(`${name}.msdf.json`));
			const msdfImageData = Deno.readTextFileSync(`${name}.msdf.txt`);
			let cppSource =
				`#include "../Font.hpp"\n`+
				`\n`+
				`namespace graphics2d {\n`+
				`	namespace font {\n`+
				`		namespace {\n`+
				`			U8 msdfData[${msdfJson.atlas.width*msdfJson.atlas.height*3}+1] =\n`
			;
			for(const line of msdfImageData.split('\n').reverse()){
				if(!line) continue;

				const values = line.split(/\s+/);
				
				cppSource +=
				`				"\\x${values.join('\\x')}"\n`
				;
			}
			cppSource +=
				`			;\n`+
				`			FontCharacter characters[${msdfJson.glyphs.length}] = {\n`
			;
			for(const glyph of msdfJson.glyphs){
				cppSource +=
				`				{${glyph.unicode}, FixedI16::fraction(${Math.round(glyph.advance*256)}), FixedI16::fraction(${glyph.planeBounds?Math.round(glyph.planeBounds.left*256):0}), FixedI16::fraction(${glyph.planeBounds?Math.round(glyph.planeBounds.top*256):0}), ${glyph.atlasBounds?Math.round(glyph.atlasBounds.left-0.5):0}, ${glyph.atlasBounds?Math.round(msdfJson.atlas.height-glyph.atlasBounds.top-0.5):0}, ${glyph.atlasBounds?Math.ceil(glyph.atlasBounds.right-glyph.atlasBounds.left):0}, ${glyph.atlasBounds?Math.ceil(glyph.atlasBounds.top-glyph.atlasBounds.bottom):0}},\n`
				;
			}
			cppSource +=
				`			};\n`+
				`		}\n`+
				`\n`+
				`		Font ${name[0].toLowerCase()+name.substr(1)} = {\n`+
				`			{msdfData, ${msdfJson.atlas.width*3}, ${msdfJson.atlas.width}, ${msdfJson.atlas.height}, graphics2d::BufferFormat::rgb8, graphics2d::BufferFormatOrder::argb},\n`+
				`			${msdfJson.atlas.size},\n`+
				`			${msdfJson.metrics.lineHeight},\n`+
				`			${msdfJson.metrics.ascender},\n`+
				`			${msdfJson.metrics.descender},\n`+
				`			${msdfJson.metrics.underlineY},\n`+
				`			${msdfJson.metrics.underlineThickness},\n`+
				`			sizeof(characters)/sizeof(characters[0]),\n`+
				`			characters\n`+
				`		};\n`+
				`	}\n`+
				`}\n`
			;
			Deno.writeTextFileSync(`${name}.cpp`, cppSource);
			Deno.removeSync(`${name}.msdf.txt`);
			Deno.removeSync(`${name}.msdf.json`);
			console.log(`Written ${name}.cpp`);
		}
	}
}

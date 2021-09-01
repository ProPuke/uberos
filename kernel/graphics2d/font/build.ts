import fs from 'fs';

(async() => {
	for(const name of fs.readdirSync('.')){
		let matches:RegExpMatchArray|null;
		if(matches = name.match(/(.*)\.json$/)){
			const name = matches[1];

			if(fs.existsSync(`${name}.json`) && fs.existsSync(`${name}.txt`)){
				const jsonData = JSON.parse(fs.readFileSync(`${name}.json`, 'utf-8'));
				const imageData = fs.readFileSync(`${name}.txt`, 'utf-8');
				let cppSource =
					`#include "../Font.hpp"\n`+
					`\n`+
					`namespace graphics2d {\n`+
					`	namespace font {\n`+
					`		namespace {\n`+
					`			U8 bufferData[${jsonData.atlas.width*jsonData.atlas.height*3}+1] =\n`
				;
				for(const line of imageData.split('\n').reverse()){
					if(!line) continue;
					cppSource +=
					`				"\\x${line.split(/\s+/).join('\\x')}"\n`
					;
				}
				cppSource +=
					`			;\n`+
					`			FontCharacter characters[${jsonData.glyphs.length}] = {\n`
				;
				for(const glyph of jsonData.glyphs){
					cppSource +=
					`				{${glyph.unicode}, ${Math.round(glyph.advance*256)}, ${glyph.planeBounds?Math.round(glyph.planeBounds.left*256):0}, ${glyph.planeBounds?Math.round(glyph.planeBounds.top*256):0}, ${glyph.atlasBounds?Math.round(glyph.atlasBounds.left-0.5):0}, ${glyph.atlasBounds?Math.round(jsonData.atlas.height-glyph.atlasBounds.top-0.5):0}, ${glyph.atlasBounds?Math.round(glyph.atlasBounds.right-glyph.atlasBounds.left):0}, ${glyph.atlasBounds?Math.round(glyph.atlasBounds.top-glyph.atlasBounds.bottom):0}},\n`
					;
				}
				cppSource +=
					`			};\n`+
					`		}\n`+
					`\n`+
					`		Font ${name[0].toLowerCase()+name.substr(1)} = {\n`+
					`			{bufferData, ${jsonData.atlas.width*jsonData.atlas.height*3}, ${jsonData.atlas.width}, ${jsonData.atlas.height}, FramebufferFormat::rgb8},\n`+
					`			${jsonData.atlas.size},\n`+
					`			${jsonData.metrics.lineHeight},\n`+
					`			${jsonData.metrics.ascender},\n`+
					`			${jsonData.metrics.descender},\n`+
					`			${jsonData.metrics.underlineY},\n`+
					`			${jsonData.metrics.underlineThickness},\n`+
					`			sizeof(characters)/sizeof(characters[0]),\n`+
					`			characters\n`+
					`		};\n`+
					`	}\n`+
					`}\n`
				;
				fs.writeFileSync(`${name}.cpp`, cppSource);
				fs.unlinkSync(`${name}.txt`);
				fs.unlinkSync(`${name}.json`);
			}
		}
	}
})();
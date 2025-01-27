import * as pngs from "https://deno.land/x/pngs@0.1.1/mod.ts";

const dir = Deno.args[0];
const relativeDir = dir.replace(/^\.\.\/common\/ui2d(\/|$)/, '');
// const dir = Deno.args[0].replace(/^\.\.\/common\/ui2d(\/|$)/, '');
const imageName = Deno.args[1];
const imagePath = `${dir}/${imageName}`;
const sourcePath = Deno.args[2];

// convert to pma rgba
function convert_image(image:pngs.DecodeResult):pngs.DecodeResult {
	for(let y=0;y<image.height;y++) {
		for(let x=0;x<image.width;x++) {
			let r = image.image[y*image.lineSize+x*4+0];
			let g = image.image[y*image.lineSize+x*4+1];
			let b = image.image[y*image.lineSize+x*4+2];
			let a = image.image[y*image.lineSize+x*4+3];

			image.image[y*image.lineSize+x*4+0] = r*a/255;
			image.image[y*image.lineSize+x*4+1] = g*a/255;
			image.image[y*image.lineSize+x*4+2] = b*a/255;
			image.image[y*image.lineSize+x*4+3] = a;
		}
	}

	return image;
}

const imageData = await Deno.readFileSync(imagePath);
// const image = pngs.decode(imageData);
const image = convert_image(pngs.decode(imageData));
const imageIdentifier = imageName.replace(/\.[^.]+$/, '').replace(/[^a-zA-Z0-9]/g, '_');

let cppSource =
`#include <common/graphics2d/Buffer.hpp>\n`+
`\n`+
`namespace ${'ui2d::image'+(relativeDir?'::'+relativeDir.replace('/', '::'):'')} {\n`+
`	namespace {\n`+
`		U8 data[${image.width*image.height*4}] = {${image.image.join(',')}};\n`+
`	}\n`+
`	graphics2d::Buffer ${imageIdentifier}\{\data, ${image.width*4}, ${image.width}, ${image.height}, graphics2d::BufferFormat::rgba8, graphics2d::BufferFormatOrder::argb};\n`+
`}\n`
;

Deno.writeTextFileSync(sourcePath, cppSource);

#pragma once

#include <kernel/keyboard.hpp>

namespace keyboard::layout::us {
	#define KEYBOARD_LAYOUT_US_ROW7 EMPTY
	#define KEYBOARD_LAYOUT_US_ROW6 EMPTY EMPTY EMPTY EMPTY EMPTY EMPTY                   KEY(f13      ,"F13") KEY(f14    ,"F14") KEY(f15    ,"F15") KEY(f16    ,"F16") EMPTY            KEY(f17    ,"F17") KEY(f18    ,"F18") KEY(f19    ,"F19") KEY(f20    ,"F20") EMPTY            KEY(f21      ,"F21"  ) KEY(f22        ,"F22") KEY(f23         ,"F23") KEY(f24         ,"F24" ) EMPTY EMPTY                     EMPTY               EMPTY                    EMPTY EMPTY              EMPTY                 EMPTY                   EMPTY
	#define KEYBOARD_LAYOUT_US_ROW5 EMPTY EMPTY EMPTY EMPTY EMPTY KEY(escape     ,"Esc" ) KEY(f1       ,"F1" ) KEY(f2     ,"F2" ) KEY(f3     ,"F3" ) KEY(f4     ,"F4" ) EMPTY            KEY(f5     ,"F5" ) KEY(f6     ,"F6" ) KEY(f7     ,"F7" ) KEY(f8     ,"F8" ) EMPTY            KEY(f9       ,"F9"   ) KEY(f10        ,"F10") KEY(f11         ,"F11") KEY(f12         ,"F12" ) EMPTY KEY(printScreen,"PrtScn") KEY(_pause,"Pause") KEY(scrollLock,"ScrLck") EMPTY EMPTY              EMPTY                 EMPTY                   EMPTY
	#define KEYBOARD_LAYOUT_US_ROW4 EMPTY EMPTY EMPTY EMPTY EMPTY KEY(backTick   ,"`"   ) KEY(number1  ,"1"  ) KEY(number2,"2"  ) KEY(number3,"3"  ) KEY(number4,"4"  ) KEY(number5,"5") KEY(number6,"6"  ) KEY(number7,"7"  ) KEY(number8,"8"  ) KEY(number9,"9"  ) KEY(number0,"0") KEY(hyphen   ,"-"    ) KEY(equals     ,"="  ) EXTEND_RIGHT            KEY(backspace          ) EMPTY KEY(insert     ,"Ins"   ) KEY(home  ,"Home" ) KEY(pageUp    ,"PgUp"  ) EMPTY KEY(numLock,"Num") KEY(numpadDivide,"/") KEY(numpadMultiply,"*") KEY(numpadSubtract,"-")
	#define KEYBOARD_LAYOUT_US_ROW3 EMPTY EMPTY EMPTY EMPTY EMPTY KEY(tab        ,"Tab" ) EXTEND_LEFT          KEY(letterQ,"Q"  ) KEY(letterW,"W"  ) KEY(letterE,"E"  ) KEY(letterR,"R") KEY(letterT,"T"  ) KEY(letterY,"Y"  ) KEY(letterU,"U"  ) KEY(letterI,"I"  ) KEY(letterO,"O") KEY(letterP  ,"P"    ) KEY(leftBracket,"["  ) KEY(rightBracket,"]"  ) KEY(backslash   ,"\\"  ) EMPTY KEY(_delete    ,"Del"   ) KEY(end   ,"End"  ) KEY(pageDown  ,"PgDn"  ) EMPTY KEY(numpad7,"7"  ) KEY(numpad8     ,"8") KEY(numpad9       ,"9") KEY(numpadPlus    ,"+")
	#define KEYBOARD_LAYOUT_US_ROW2 EMPTY EMPTY EMPTY EMPTY EMPTY KEY(capslock   ,"Caps") EXTEND_LEFT          KEY(letterA,"A"  ) KEY(letterS,"S"  ) KEY(letterD,"D"  ) KEY(letterF,"F") KEY(letterG,"G"  ) KEY(letterH,"H"  ) KEY(letterJ,"J"  ) KEY(letterK,"K"  ) KEY(letterL,"L") KEY(semicolon,";"    ) KEY(apostrophe ,"'"  ) EXTEND_RIGHT            KEY(enter              ) EMPTY EMPTY                     EMPTY               EMPTY                    EMPTY KEY(numpad4,"4"  ) KEY(numpad5     ,"5") KEY(numpad6       ,"6") EXTEND_UP
	#define KEYBOARD_LAYOUT_US_ROW1 EMPTY EMPTY EMPTY EMPTY EMPTY KEY(leftShift  ,"Shft") EXTEND_LEFT          KEY(letterZ,"Z"  ) KEY(letterX,"X"  ) KEY(letterC,"C"  ) KEY(letterV,"V") KEY(letterB,"B"  ) KEY(letterN,"N"  ) KEY(letterM,"M"  ) KEY(comma  ,","  ) KEY(dot    ,".") KEY(slash    ,"/"    ) EXTEND_RIGHT           EXTEND_RIGHT            KEY(rightShift  ,"Shft") EMPTY EMPTY                     KEY(upArrow       ) EMPTY                    EMPTY KEY(numpad1,"1"  ) KEY(numpad2     ,"2") KEY(numpad3       ,"3") KEY(numpadEnter       )
	#define KEYBOARD_LAYOUT_US_ROW0 EMPTY EMPTY EMPTY EMPTY EMPTY KEY(leftControl,"Ctrl") KEY(leftSuper      ) KEY(alt    ,"Alt") EXTEND_RIGHT       EXTEND_RIGHT       EXTEND_RIGHT     KEY(space        ) EXTEND_LEFT        EXTEND_LEFT        EXTEND_LEFT        EXTEND_LEFT      KEY(altGr    ,"AltGr") KEY(rightSuper       ) KEY(contextMenu )       KEY(rightControl,"Ctrl") EMPTY KEY(leftArrow           ) KEY(downArrow     ) KEY(rightArrow        )  EMPTY KEY(numpad0,"0"  ) EXTEND_LEFT           KEY(numpadDot     ,".") EXTEND_UP

	static inline Layout layout {
		"US",
		(C16[256]){
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,u'`',u'1' ,u'2',u'3',u'4',u'5',u'6',u'7',u'8',u'9',u'0',u'-',u'=' ,0   ,u'\b',0,0,0,0,0,0   ,u'/',u'*',u'-' ,0,0,0,
			0,0,0,0,0,0   ,0    ,u'q',u'w',u'e',u'r',u't',u'y',u'u',u'i',u'o',u'p',u'[' ,u']',u'\\',0,0,0,0,0,u'7',u'8',u'9',u'+' ,0,0,0,
			0,0,0,0,0,0   ,0    ,u'a',u's',u'd',u'f',u'g',u'h',u'j',u'k',u'l',u';',u'\'',0   ,u'\n',0,0,0,0,0,u'4',u'5',u'6',0    ,0,0,0,
			0,0,0,0,0,0   ,0    ,u'z',u'x',u'c',u'v',u'b',u'n',u'm',u',',u'.',u'/',0    ,0   ,0    ,0,0,0,0,0,u'1',u'2',u'3',u'\n',0,0,0,
			0,0,0,0,0,0   ,0    ,0   ,0   ,0   ,0   ,u' ',0   ,0   ,0   ,0   ,0   ,0    ,0   ,0    ,0,0,0,0,0,u'0',0   ,u'.',0    ,0,0,0
		},(C16[256]){
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,u'~',u'!' ,u'@',u'#',u'$',u'%',u'^',u'&',u'*',u'(',u')',u'_',u'+' ,0   ,u'\b',0,0,0,0,0,0   ,u'/',u'*',u'-' ,0,0,0,
			0,0,0,0,0,0   ,0    ,u'Q',u'W',u'E',u'R',u'T',u'Y',u'U',u'I',u'O',u'P',u'[' ,u']',u'|' ,0,0,0,0,0,u'7',u'8',u'9',u'+' ,0,0,0,
			0,0,0,0,0,0   ,0    ,u'A',u'S',u'D',u'F',u'G',u'H',u'J',u'K',u'L',u':',u'"' ,0   ,u'\n',0,0,0,0,0,u'4',u'5',u'6',0    ,0,0,0,
			0,0,0,0,0,0   ,0    ,u'Z',u'X',u'C',u'V',u'B',u'N',u'M',u',',u'.',u'/',0    ,0   ,0    ,0,0,0,0,0,u'1',u'2',u'3',u'\n',0,0,0,
			0,0,0,0,0,0   ,0    ,0   ,0   ,0   ,0   ,u' ',0   ,0   ,0   ,0   ,0   ,0    ,0   ,0    ,0,0,0,0,0,u'0',0   ,u'.',0    ,0,0,0
		},(C16[256]){
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,u'|',u'¡' ,u'²',u'³',u'¤',u'€',u'¼',u'½',u'¾',u'‘',u'’',u'¥',u'×' ,0   ,u'\b',0,0,0,0,0,0   ,u'/',u'*',u'-' ,0,0,0,
			0,0,0,0,0,0   ,0    ,u'ä',u'å',u'é',u'®',u'þ',u'ü',u'ú',u'í',u'ó',u'ö',u'»' ,u'þ',u'¬' ,0,0,0,0,0,u'7',u'8',u'9',u'+' ,0,0,0,
			0,0,0,0,0,0   ,0    ,u'á',u'ß',u'ð',u'ë',u'g',u'h',u'ï',u'œ',u'ø',u'¶',u'\'',0   ,u'\n',0,0,0,0,0,u'4',u'5',u'6',0    ,0,0,0,
			0,0,0,0,0,0   ,0    ,u'æ',u'·',u'©',u'v',u'b',u'ñ',u'µ',u'ç',u'˙',u'¿',0    ,0   ,0    ,0,0,0,0,0,u'1',u'2',u'3',u'\n',0,0,0,
			0,0,0,0,0,0   ,0    ,0   ,0   ,0   ,0   ,u' ',0   ,0   ,0   ,0   ,0   ,0    ,0   ,0    ,0,0,0,0,0,u'0',0  ,u'.' ,0    ,0,0,0
		},(C16[256]){
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,u'~',u'¹' ,u'˝',u'¯' ,u'£',u'¸',u'^',0   ,u'˛',u'˘',u'˚',u'.' ,u'÷',0   ,u'\b',0,0,0,0,0,0   ,u'/',u'*',u'-' ,0,0,0, //FIXME: shift+altgr+7 diacritic
			0,0,0,0,0,0   ,0    ,u'Ä',u'Å' ,u'É',u'™',u'Þ',u'Ü',u'Ü',u'Í',u'Ó',u'Ö' ,u'“',u'”',u'¦' ,0,0,0,0,0,u'7',u'8',u'9',u'+' ,0,0,0,
			0,0,0,0,0,0   ,0    ,u'Á',u'§' ,u'Ð',u'Ë',u'G',u'H',u'Ï',u'Œ',u'Ø',u'°' ,u'"',0   ,u'\n',0,0,0,0,0,u'4',u'5',u'6',0    ,0,0,0,
			0,0,0,0,0,0   ,0    ,u'Æ',0    ,u'¢',u'V',u'B',u'Ñ',u'±',u'Ç',u'>',u'ˀ' ,0   ,0   ,0    ,0,0,0,0,0,u'1',u'2',u'3',u'\n',0,0,0, //FIXME: shift+altgr+x diacritic
			0,0,0,0,0,0   ,0    ,0   ,0    ,0   ,0   ,u' ',0   ,0   ,0   ,0   ,0    ,0   ,0   ,0    ,0,0,0,0,0,u'0',0   ,u'.',0    ,0,0,0
		}
	};
}

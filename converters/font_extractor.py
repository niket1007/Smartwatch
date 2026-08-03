import re
from pathlib import Path

FONT_HEADER = """
#pragma once

typedef struct
{
    uint32_t bitmap_offset;
    uint16_t width;
    uint16_t height;
    int16_t x_offset;
    int16_t y_offset;
    uint16_t advance;
} glyph_t;

typedef struct
{
    const uint8_t *bitmap;
    const glyph_t *glyphs;
    uint32_t first_char;
    uint32_t last_char;
    uint16_t line_height;
} font_t;
"""

HEADER = """\
/****************************************************
 * AUTO GENERATED
 * DO NOT EDIT
 ****************************************************/

#pragma once

#include <stdint.h>
#include "font.h"

extern const font_t {font_name};
"""

def write_global_font_header(out_dir: Path, names: list[str]):
    string = ""
    for name in names:
        name = name.replace("-", "_")
        string += '#include "{}.h"\n'.format(name)
    (out_dir/'font_globals.h').write_text(string)


def extract_array(text: str, name: str) -> str:
    pos = text.find(name)
    if pos < 0:
        raise RuntimeError(f"{name} not found")

    pos = text.find("{", pos)
    if pos < 0:
        raise RuntimeError(f"{name}: opening brace not found")

    depth = 1
    i = pos + 1
    while depth and i < len(text):
        c = text[i]
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
        i += 1

    if depth:
        raise RuntimeError(f"{name}: unmatched braces")

    return text[pos + 1:i - 1]

def convert(name, txt, out_dir):
    bm = extract_array(txt, "glyph_bitmap[]")
    if len(bm.strip()) == 0:
        raise RuntimeError("Bitmap empty")
    
    gd = extract_array(txt, "glyph_dsc[]")

    

    lh = int(re.search(r"\.line_height\s*=\s*(\d+)", txt).group(1))

    cm = re.search(r"\.range_start\s*=\s*(\d+).*?\.range_length\s*=\s*(\d+)", txt, re.S)
    first, last = (32,126) if not cm else (
        int(cm.group(1)),
        int(cm.group(1))+int(cm.group(2))-1
    )

    if first > last:
        raise RuntimeError("Invalid unicode range")

    glyphs=[]
    for e in re.finditer(
        r"\.bitmap_index\s*=\s*(\d+).*?"
        r"\.adv_w\s*=\s*(\d+).*?"
        r"\.box_w\s*=\s*(\d+).*?"
        r"\.box_h\s*=\s*(\d+).*?"
        r"\.ofs_x\s*=\s*(-?\d+).*?"
        r"\.ofs_y\s*=\s*(-?\d+)",
        gd,re.S):
        glyphs.append((
            e.group(1),
            e.group(3),
            e.group(4),
            e.group(5),
            e.group(6),
            str(int(e.group(2))//16)
        ))

    if not glyphs:
        raise RuntimeError("No glyphs parsed")

    (out_dir/f"{name}.h").write_text(HEADER.format(font_name=name))

    lines=[
        """
/****************************************************
 * AUTO GENERATED
 * DO NOT EDIT
 ****************************************************/
        """,
        f'#include "{name}.h"',
        "",
        "static const uint8_t bitmap[]={",
        bm,
        "};",
        "",
        "static const glyph_t glyphs[]={"
    ]
    for g in glyphs:
        lines.append("    {%s,%s,%s,%s,%s,%s}," % g)

    lines += [
        "};","",
        f"const font_t {name}={{",
        "    .bitmap=bitmap,",
        "    .glyphs=glyphs,",
        f"    .first_char={first},",
        f"    .last_char={last},",
        f"    .line_height={lh}",
        "};"
    ]
    (out_dir/f"{name}.c").write_text("\n".join(lines))

def main():
    input_dir=Path("../main/Graphics/Fonts/ttf_and_c")
    output=Path("../main/Graphics/Fonts/generated")
    output.mkdir(parents=True,exist_ok=True)

    (output/f"font.h").write_text(FONT_HEADER)
    font_names = []
    for font in sorted(input_dir.glob("*.txt")):
        name = font.stem
        name = name.replace("-", "_")
        font_names.append(name)
        convert(font.stem,font.read_text(errors="ignore"),output)
    write_global_font_header(output, font_names)

if __name__=="__main__":
    main()

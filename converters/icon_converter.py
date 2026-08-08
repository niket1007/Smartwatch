from __future__ import annotations
import argparse
from io import BytesIO
from pathlib import Path
from PIL import Image, ImageOps
from reportlab.graphics import renderPM
from svglib.svglib import svg2rlg

AUTO_COMMENT = """
/****************************************************
 * AUTO GENERATED
 * DO NOT EDIT
 ****************************************************/
"""

ICON_HEADER = """#pragma once
/****************************************************
 * AUTO GENERATED
 * DO NOT EDIT
 ****************************************************/

#include <stdint.h>

typedef struct
{
    uint16_t width;
    uint16_t height;
    uint16_t stride;
    const uint8_t *data;
} icon_t;
"""


def ensure_icon_header(out_dir: Path):
    (out_dir / "icon.h").write_text(ICON_HEADER)


def write_global_icon_header(out_dir: Path, names: list[str]):
    string = ""
    for name in names:
        name = name.replace("-", "_")
        string += '#include "{}.h"\n'.format(name)
    (out_dir / "icon_globals.h").write_text(string)


def image_to_coverage(img: Image.Image, size: int) -> bytearray:
    img = img.convert("RGBA")
    img = ImageOps.contain(img, (size, size), Image.Resampling.LANCZOS)
    canvas = Image.new("RGBA", (size, size), (255, 255, 255, 0))
    x = (size - img.width) // 2
    y = (size - img.height) // 2
    canvas.paste(img, (x, y), img)
    cov = bytearray()
    for r, g, b, a in canvas.getdata():
        if a == 0:
            cov.append(0)
            continue
        cov.append(((255 - max(r, g, b)) * a + 127) // 255)
    return cov


def write_icon(name, cov, size, out_dir):
    (out_dir / f"{name}.h").write_text(
        AUTO_COMMENT
        + '\n#pragma once\n\n#include "icon.h"\n\nextern const icon_t icon_%s;\n' % name
    )
    lines = []
    for i in range(0, len(cov), 16):
        lines.append("    " + ",".join(f"{v:3d}" for v in cov[i : i + 16]) + ",")
    (out_dir / f"{name}.c").write_text(
        AUTO_COMMENT
        + f'\n#include "{name}.h"\n\nstatic const uint8_t {name}_data[]={{\n'
        + "\n".join(lines)
        + f"\n}};\n\nconst icon_t icon_{name}={{\n    .width={size},\n    .height={size},\n    .stride={size},\n    .data={name}_data\n}};\n"
    )

    # Uncomment if png preview is required
    # Image.frombytes('L',(size,size),bytes(cov)).save(out_dir/f'{name}_preview.png')


def convert(svg, out, size):
    name: str = svg.stem
    name = name.replace("-", "_")

    drawing = svg2rlg(str(svg))
    png = renderPM.drawToString(drawing, fmt="PNG")
    write_icon(name, image_to_coverage(Image.open(BytesIO(png)), size), size, out)


def main():
    input = Path("../main/Graphics/Icons/svg")
    output = Path("../main/Graphics/Icons/generated")
    size = 80
    output.mkdir(parents=True, exist_ok=True)
    ensure_icon_header(output)
    svg_names = []
    for svg in sorted(input.glob("*.svg")):
        svg_names.append(svg.stem)
        convert(svg, output, size)
    write_global_icon_header(output, svg_names)


if __name__ == "__main__":
    main()

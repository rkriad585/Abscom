#!/usr/bin/env python3
"""Generate the Screenshots/*.png images with Pillow.

Each image is a VS Code-style editor window showing a syntax-highlighted
C snippet from the Abscom examples/quick-start code. Run this after editing
the snippets or the repo layout:

    python tools/generate_screenshots.py [outdir]
"""

import os
import re
import sys
import textwrap

from PIL import Image, ImageDraw, ImageFont

WIDTH, HEIGHT = 1600, 900
TITLE_H = 40
TAB_H = 36
STATUS_H = 26
SIDEBAR_W = 240
GUTTER_W = 56
CODE_FONT = 19
LINE_H = 28
PAD_X = 20

BG = (30, 30, 30)
SIDEBAR_BG = (37, 37, 38)
TITLEBAR_BG = (50, 50, 51)
TAB_BG = (37, 37, 38)
TAB_ACTIVE_BG = (30, 30, 30)
EDITOR_BG = (30, 30, 30)
GUTTER_FG = (133, 133, 133)
ACTIVE_ROW = (55, 55, 61)
HOVER_ROW = (43, 43, 48)
STATUS_BG = (0, 122, 204)
STATUS_FG = (255, 255, 255)
CURSOR_BG = (174, 175, 173)
CURSOR_LINE = (42, 42, 42)
BORDER = (60, 60, 60)
TEXT_FG = (212, 212, 212)
DIM_FG = (187, 187, 187)
MUTED_FG = (128, 128, 128)

KW_FG = (86, 156, 214)
TYPE_FG = (78, 201, 176)
STR_FG = (206, 145, 120)
COM_FG = (106, 153, 85)
NUM_FG = (181, 206, 168)
FUN_FG = (220, 220, 170)
PRE_FG = (197, 134, 192)
CONST_FG = (86, 156, 214)

DOT_RED = (255, 95, 87)
DOT_YELLOW = (254, 188, 46)
DOT_GREEN = (40, 200, 72)

KEYWORDS = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if", "int",
    "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile",
    "while",
}
TYPES = {
    "bool", "var", "size_t", "AbsObj", "AbsFunc", "AbsMapFunc", "AbsFilterFunc",
    "ac_string_t", "ac_dynarray_t", "ac_hashmap_t",
}
CONSTANTS = {"true", "false", "NULL", "None"}
MACROS = {"foreach"}

RE_NUMBER = re.compile(r"(?:0[xX][0-9a-fA-F]+|\d+(?:\.\d+)?)$")
MASTER = re.compile(
    r"#\w+|//|/\*|"
    r'"(?:\\.|[^"\\])*"|'
    r"'(?:\\.|[^'\\])*'|"
    r"\b\d+(?:\.\d+)?\b|\b0[xX][0-9a-fA-F]+\b|"
    r"\b[A-Za-z_]\w*"
)


def find_font(size):
    candidates = [
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/cour.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
    ]
    for path in candidates:
        if os.path.exists(path):
            try:
                return ImageFont.truetype(path, size)
            except OSError:
                pass
    raise RuntimeError("no monospace font found; install Consolas or DejaVu Sans Mono")


def highlight_lines(code, font):
    out = []
    in_block = False
    for line in code.split("\n"):
        parts = []
        i = 0
        n = len(line)
        while i < n:
            if in_block:
                j = line.find("*/", i)
                if j == -1:
                    parts.append((line[i:], COM_FG))
                    break
                parts.append((line[i : j + 2], COM_FG))
                i = j + 2
                in_block = False
                continue
            m = MASTER.search(line, i)
            if not m:
                if i < n:
                    parts.append((line[i:], TEXT_FG))
                break
            start, end = m.span()
            if start > i:
                parts.append((line[i:start], TEXT_FG))
            tok = m.group(0)
            if tok == "/*":
                j = line.find("*/", end)
                if j == -1:
                    if end < n:
                        parts.append((line[end:], COM_FG))
                    in_block = True
                    break
                parts.append((line[end : j + 2], COM_FG))
                i = j + 2
            elif tok == "//":
                if end < n:
                    parts.append((line[end:], COM_FG))
                break
            elif tok.startswith("#"):
                parts.append((tok, PRE_FG))
                i = end
            elif tok[0] in "\"'":
                parts.append((tok, STR_FG))
                i = end
            elif RE_NUMBER.match(tok):
                parts.append((tok, NUM_FG))
                i = end
            elif tok in KEYWORDS:
                parts.append((tok, KW_FG))
                i = end
            elif tok in TYPES:
                parts.append((tok, TYPE_FG))
                i = end
            elif tok in CONSTANTS:
                parts.append((tok, CONST_FG))
                i = end
            elif tok in MACROS:
                parts.append((tok, PRE_FG))
                i = end
            elif end < n and line[end] == "(":
                parts.append((tok, FUN_FG))
                i = end
            else:
                parts.append((tok, TEXT_FG))
                i = end
        out.append(parts)
    return out


def draw_traffic_lights(draw, y):
    for i, color in enumerate((DOT_RED, DOT_YELLOW, DOT_GREEN)):
        cx = 22 + i * 18
        draw.ellipse((cx - 6, y + TITLE_H // 2 - 6, cx + 6, y + TITLE_H // 2 + 6), fill=color)


def draw_chevron(draw, x, y, down=False, color=(138, 138, 138)):
    if down:
        pts = [(x, y), (x + 7, y), (x + 3.5, y + 5)]
    else:
        pts = [(x + 2, y), (x + 2, y + 6), (x + 7, y + 3)]
    draw.polygon(pts, fill=color)


def draw_folder(draw, x, y, color=(214, 164, 108)):
    draw.rounded_rectangle((x, y, x + 13, y + 10), radius=2, fill=color)
    draw.rectangle((x + 1, y, x + 5, y + 2), fill=color)


def draw_file(draw, x, y, color=(200, 200, 200)):
    draw.rectangle((x + 2, y, x + 12, y + 13), fill=color)
    draw.polygon([(x + 2, y), (x + 6, y), (x + 6, y + 3), (x + 9, y + 3)], fill=(90, 90, 95))
    draw.polygon([(x + 2, y), (x + 2, y + 13), (x + 12, y + 13)], fill=color)


def draw_sidebar(draw, font, num_font, tree, active_name):
    draw.rectangle((0, TITLE_H, SIDEBAR_W, HEIGHT - STATUS_H), fill=SIDEBAR_BG)
    draw.line((SIDEBAR_W, TITLE_H, SIDEBAR_W, HEIGHT - STATUS_H), fill=BORDER)
    label = "EXPLORER"
    lx = 14
    for ch in label:
        draw.text((lx, TITLE_H + 10), ch, font=font, fill=DIM_FG)
        lx += font.getlength(ch) + 3
    y = TITLE_H + 38
    for indent, name, kind, active in tree:
        if active:
            draw.rectangle((0, y - 2, SIDEBAR_W, y + 16), fill=ACTIVE_ROW)
            name_color = (255, 255, 255)
        else:
            name_color = (204, 204, 204)
        x = 12 + indent * 16
        if kind == "folder":
            draw_chevron(draw, x, y + 2, down=True)
            draw_folder(draw, x + 12, y - 1)
            draw.text((x + 32, y - 2), name, font=font, fill=name_color)
        else:
            draw_chevron(draw, x, y + 2, down=False)
            draw_file(draw, x + 12, y - 2)
            draw.text((x + 32, y - 2), name, font=font, fill=name_color)
        y += 26


def draw_tabs(draw, font, filename):
    draw.rectangle((SIDEBAR_W, TITLE_H, WIDTH, TITLE_H + TAB_H), fill=TAB_BG)
    draw.rectangle((SIDEBAR_W + 1, TITLE_H + 2, SIDEBAR_W + 170, TITLE_H + TAB_H - 2), fill=TAB_ACTIVE_BG)
    draw.text((SIDEBAR_W + 14, TITLE_H + (TAB_H - font.size) // 2), filename, font=font, fill=(255, 255, 255))
    cx = SIDEBAR_W + 150
    draw.text((cx, TITLE_H + (TAB_H - font.size) // 2 - 1), "x", font=font, fill=DIM_FG)
    draw.text((WIDTH - 26, TITLE_H + (TAB_H - font.size) // 2 - 1), "+", font=font, fill=DIM_FG)


def draw_editor(draw, font, num_font, code, gutter_w, cursor):
    ex = SIDEBAR_W
    ey = TITLE_H + TAB_H
    ew = WIDTH - SIDEBAR_W
    eh = HEIGHT - ey - STATUS_H
    draw.rectangle((ex, ey, WIDTH, ey + eh), fill=EDITOR_BG)
    draw.line((ex + gutter_w, ey, ex + gutter_w, ey + eh), fill=BORDER)
    cursor_line, cursor_col = cursor
    if cursor_line >= 1:
        cy = ey + (cursor_line - 1) * LINE_H
        draw.rectangle((ex + gutter_w, cy, WIDTH, cy + LINE_H), fill=CURSOR_LINE)
    text_x = ex + gutter_w + PAD_X
    for idx, parts in enumerate(code):
        y = ey + idx * LINE_H
        num = idx + 1
        nw = num_font.getlength(str(num))
        draw.text((ex + gutter_w - nw - 12, y + (LINE_H - num_font.size) // 2 - 1), str(num), font=num_font, fill=GUTTER_FG)
        cx = text_x
        for text, color in parts:
            draw.text((cx, y + (LINE_H - font.size) // 2 - 1), text, font=font, fill=color)
            cx += font.getlength(text)
    if cursor_line >= 1 and cursor_col >= 1:
        if cursor_line - 1 < len(code):
            prefix = "".join(t for t, _ in code[cursor_line - 1])
            prefix = prefix[: max(0, cursor_col - 1)]
            cx = text_x + font.getlength(prefix)
            cy = ey + (cursor_line - 1) * LINE_H + (LINE_H - font.size) // 2 - 1
            draw.rectangle((cx, cy, cx + 2, cy + font.size), fill=CURSOR_BG)


def draw_status(draw, font, cursor, c_lang="C"):
    sy = HEIGHT - STATUS_H
    draw.rectangle((0, sy, WIDTH, HEIGHT), fill=STATUS_BG)
    y = sy + (STATUS_H - font.size) // 2
    x = SIDEBAR_W + 12
    draw.ellipse((x, y + 3, x + 8, y + 11), outline=STATUS_FG, width=2)
    x += 14
    draw.text((x, y), "main", font=font, fill=STATUS_FG)
    x += font.getlength("main") + 24
    draw.text((x, y), "0 errors", font=font, fill=STATUS_FG)
    x += font.getlength("0 errors") + 18
    draw.text((x, y), "0 warnings", font=font, fill=STATUS_FG)
    x += font.getlength("0 warnings") + 24
    draw.text((x, y), c_lang, font=font, fill=STATUS_FG)
    right_items = [f"Ln {cursor[0]}, Col {cursor[1]}", "Spaces: 4", "UTF-8", "LF"]
    rx = WIDTH - 12
    for item in reversed(right_items):
        iw = font.getlength(item)
        rx -= iw
        draw.text((rx, y), item, font=font, fill=STATUS_FG)
        rx -= 18


def repo_tree(active):
    root = [
        (0, "Abscom", "folder", False),
        (1, "include", "folder", False),
        (2, "abscom", "folder", False),
        (3, "ac.h", "file", False),
        (3, "ac_py.h", "file", False),
        (1, "src", "folder", False),
        (2, "ac_py.c", "file", False),
        (1, "examples", "folder", False),
        (2, "demo.c", "file", active == "demo.c"),
        (2, "py_demo.c", "file", active == "py_demo.c"),
        (2, "data_demo.c", "file", active == "data_demo.c"),
        (2, "v6_demo.c", "file", active == "v6_demo.c"),
        (0, "meson.build", "file", False),
        (0, "README.md", "file", False),
    ]
    return root


HELLO = """
    #include "abscom/ac_py.h"

    int main(void) {
        abs_init();

        var nums = List();
        append(nums, v(10));
        append(nums, v(20));
        append(nums, v(30));

        var user = Dict();
        dset(user, "name", v("Alice"));
        dset(user, "role", v("Admin"));

        print(v("Sum:"), sum_val(nums));
        print(v("Name:"), dget(user, "name"));

        abs_cleanup();
        return 0;
    }
"""

PY_DEMO = """
    #include "abscom/ac_py.h"

    int main(void) {
        abs_init();

        var data = json_parse("{\\"id\\": 101, \\"scores\\": [10, 20, 30]}");
        print(v("ID:"), dget(data, "id"));
        print(v("Scores:"), dget(data, "scores"));

        var s = Set();
        set_add(s, v(3));
        set_add(s, v(1));
        set_add(s, v(3));          /* deduped */
        print(v("Set:"), s);

        var item;
        foreach (item, range(0, 10)) {
            if (item->val.i % 2 == 0) print(item);
        }

        var Dog = Class("Dog");
        var rex = New(Dog);
        set_attr(rex, "name", v("Rex"));
        print(v("Dog:"), get_attr(rex, "name"));

        abs_cleanup();
        return 0;
    }
"""

CORE_DEMO = """
    #include "abscom/ac.h"

    int main(void) {
        ac_string_t buf;
        ac_string_init(&buf);
        ac_string_append_fmt(&buf, "Hello %s!", "world");

        ac_dynarray_t nums;
        ac_dynarray_init(&nums, sizeof(int));
        int x = 42;
        ac_dynarray_push(&nums, &x);
        printf("%d\\n", *(int *)ac_dynarray_at(&nums, 0));

        ac_hashmap_t *m = ac_hashmap_create(NULL);
        ac_hashmap_set(m, "key", &x);
        printf("exists: %d\\n", ac_hashmap_contains(m, "key"));

        printf("%s\\n", ac_string_c_str(&buf));
        printf("now: %.2f\\n", ac_time_wall());

        ac_fs_write_file("out.txt", ac_string_c_str(&buf), ac_string_len(&buf));

        ac_hashmap_destroy(m);
        ac_string_destroy(&buf);
        ac_dynarray_destroy(&nums);
        return 0;
    }
"""

V6_DEMO = """
    #include "abscom/ac_py.h"

    static var square_it(var x) { return abs_new_int(x->val.i * x->val.i); }
    static bool is_odd_b(var x) { return x->val.i % 2 != 0; }

    int main(void) {
        abs_init();

        var item;
        long total = 0;
        foreach (item, range(0, 10)) {
            total += item->val.i;
        }
        print(v("Sum 0..9:"), abs_new_int(total));

        var odds = list_comp(range(0, 10), square_it, is_odd_b);
        print(v("Squares of evens:"), odds);

        var html = http_get("http://example.com/");
        if (is_err(html)) {
            print(v("HTTP error"));
        } else {
            print(v("Body length:"), len(html));
        }

        abs_cleanup();
        return 0;
    }
"""


def main():
    outdir = sys.argv[1] if len(sys.argv) > 1 else None
    specs = [
        ("home.png", "hello.c", repo_tree("hello.c"), HELLO, (6, 9)),
        ("ac_py.png", "py_demo.c", repo_tree("py_demo.c"), PY_DEMO, (6, 9)),
        ("core.png", "core_demo.c", repo_tree("core_demo.c"), CORE_DEMO, (10, 9)),
        ("examples.png", "v6_demo.c", repo_tree("v6_demo.c"), V6_DEMO, (11, 16)),
    ]
    font = find_font(CODE_FONT)
    num_font = find_font(13)
    title_font = find_font(14)
    small_font = find_font(13)
    for fname, tab, tree, snippet, cursor in specs:
        code = textwrap.dedent(snippet).strip("\n")
        img = Image.new("RGB", (WIDTH, HEIGHT), BG)
        draw = ImageDraw.Draw(img)
        draw.rectangle((0, 0, WIDTH, TITLE_H), fill=TITLEBAR_BG)
        draw_traffic_lights(draw, 0)
        full = f"{tab} - Abscom"
        tw = title_font.getlength(full)
        draw.text((WIDTH / 2 - tw / 2, (TITLE_H - title_font.size) // 2), full, font=title_font, fill=DIM_FG)
        draw_sidebar(draw, small_font, num_font, tree, tab)
        draw_tabs(draw, small_font, tab)
        highlighted = highlight_lines(code, font)
        draw_editor(draw, font, num_font, highlighted, GUTTER_W, cursor)
        draw_status(draw, small_font, cursor)
        path = os.path.join(outdir or os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "Screenshots"), fname)
        img.save(path)
        print("wrote", path)


if __name__ == "__main__":
    main()

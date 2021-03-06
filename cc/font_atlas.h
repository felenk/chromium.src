// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CCFontAtlas_h
#define CCFontAtlas_h

#include <string>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/rect.h"

class SkCanvas;

namespace gfx {
class Point;
class Size;
}

namespace cc {

// This class provides basic ability to draw text onto the heads-up display.
class FontAtlas {
public:
    static scoped_ptr<FontAtlas> create(SkBitmap bitmap, gfx::Rect asciiToRectTable[128], int fontHeight)
    {
        return make_scoped_ptr(new FontAtlas(bitmap, asciiToRectTable, fontHeight));
    }
    ~FontAtlas();

    // Draws multiple lines of text where each line of text is separated by '\n'.
    // - Correct glyphs will be drawn for ASCII codes in the range 32-127; any characters
    //   outside that range will be displayed as a default rectangle glyph.
    // - gfx::Size clip is used to avoid wasting time drawing things that are outside the
    //   target canvas bounds.
    // - Should only be called only on the impl thread.
    void drawText(SkCanvas*, const SkPaint&, const std::string& text, const gfx::Point& destPosition, const gfx::Size& clip) const;

    // Draws the entire atlas at the specified position, just for debugging purposes.
    void drawDebugAtlas(SkCanvas*, const gfx::Point& destPosition) const;

private:
    FontAtlas(SkBitmap, gfx::Rect asciiToRectTable[128], int fontHeight);

    void drawOneLineOfTextInternal(SkCanvas*, const SkPaint&, const std::string&, const gfx::Point& destPosition) const;

    // The actual texture atlas containing all the pre-rendered glyphs.
    SkBitmap m_atlas;

    // The look-up tables mapping ascii characters to their gfx::Rect locations on the atlas.
    gfx::Rect m_asciiToRectTable[128];

    int m_fontHeight;

    DISALLOW_COPY_AND_ASSIGN(FontAtlas);
};

}  // namespace cc

#endif

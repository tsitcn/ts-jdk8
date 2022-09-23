/*
 * Copyright (c) 2003, 2005, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

package sun.font;

import java.util.Map;
import java.awt.Font;
import java.awt.font.TextAttribute;
import java.awt.font.FontRenderContext;
import java.awt.geom.AffineTransform;
import static sun.awt.SunHints.*;

/*
 * This class encapsulates every thing needed that distinguishes a strike.
 * It can be used as a key to locate a FontStrike in a Hashmap/cache.
 * It is not mutatable, but contains mutatable AffineTransform objects,
 * which for performance reasons it does not keep private copies of.
 * Therefore code constructing these must pass in transforms it guarantees
 * not to mutate.
 */
public class FontStrikeDesc {

    /* Values to use as a mask that is used for faster comparison of
     * two strikes using just an int equality test.
     * The ones we don't use are listed here but commented out.
     * ie style is already built and hint "OFF" values are zero.
     * Note that this is used as a strike key and the same strike is used
     * for HRGB and HBGR, so only the orientation needed (H or V) is needed
     * to construct and distinguish a FontStrikeDesc. The rgb ordering
     * needed for rendering is stored in the graphics state.
     */
//     static final int STYLE_PLAIN       = Font.PLAIN;            // 0x0000
//     static final int STYLE_BOLD        = Font.BOLD;             // 0x0001
//     static final int STYLE_ITALIC      = Font.ITALIC;           // 0x0002
//     static final int STYLE_BOLDITALIC  = Font.BOLD|Font.ITALIC; // 0x0003
//     static final int AA_OFF            = 0x0000;
    static final int AA_ON             = 0x0010;
    static final int AA_LCD_H          = 0x0020;
    static final int AA_LCD_V          = 0x0040;
//     static final int FRAC_METRICS_OFF  = 0x0000;
    static final int FRAC_METRICS_ON   = 0x0100;
    static final int FRAC_METRICS_SP   = 0x0200;


    /**
     * Must be same with freetype.h
     */
    private static final int FLAG_POSTURE_TO_BOTTOM             = (0x01 <<  0);
    
    private static final int FLAG_SOLID_SIZE_ENABLE             = (0x01 <<  1);
    private static final int FLAG_SOLID_SIZE_X_ENABLE           = (0x01 <<  2);
    private static final int FLAG_SOLID_SIZE_Y_ENABLE           = (0x01 <<  3);
    
    private static final int FLAG_BITMAP_BOLD_GRAY_ENABLE       = (0x01 <<  4);
    private static final int FLAG_BITMAP_BOLD_GRAY_NORTH_ENABLE = (0x01 <<  5);
    private static final int FLAG_BITMAP_BOLD_GRAY_EAST_ENABLE  = (0x01 <<  6);
    private static final int FLAG_BITMAP_BOLD_GRAY_SORTH_ENABLE = (0x01 <<  7);
    private static final int FLAG_BITMAP_BOLD_GRAY_WEST_ENABLE  = (0x01 <<  8);

    private static final int FLAG_FLIP_L2R                      = (0x01 <<  9);
    private static final int FLAG_FLIP_T2B                      = (0x01 << 10);

    /* devTx is to get an inverse transform to get user space values
     * for metrics. Its not used otherwise, as the glyphTx is the important
     * one. But it does mean that a strike representing a 6pt font and identity
     * graphics transform is not equal to one for a 12 pt font and 2x scaled
     * graphics transform. Its likely to be very rare that this causes
     * duplication.
     */
    AffineTransform devTx;
    AffineTransform glyphTx; // all of ptSize, Font tx and Graphics tx.
    int style;
    int aaHint;
    int fmHint;
    Map<TextAttribute, ?> fontAttrs;
    
    private int hashCode;
    private int valuemask;
    
    // {{{{{{{{{{ extends for taishan office
    
    private int     flags             = 0;

    private int     office_style      = 0;
    private boolean bitmap_first      = false;

    private float   posture           = Font.POSTURE_PLAIN;
    private float   posture_office    = Font.POSTURE_PLAIN;

    private float   weight            = Font.WEIGHT_PLAIN;
    private float   weight_x          = Font.WEIGHT_PLAIN;
    private float   weight_y          = Font.WEIGHT_PLAIN;
    
    
    
    public  boolean isBitmapFirst()
    {
        return bitmap_first;
    }

    // }}}}}}}}}} extends for taishan office
    
    public int hashCode() {
        /* Can cache hashcode since a strike(desc) is immutable.*/
        if (hashCode == 0) {
            hashCode = glyphTx.hashCode() + devTx.hashCode() + valuemask;
        }
        return hashCode;
    }

    public boolean equals(Object obj) {
        try {
            FontStrikeDesc desc = (FontStrikeDesc)obj;
            return (desc.valuemask         == this.valuemask         &&
            
                    desc.flags             == this.flags             &&
                    desc.office_style      == this.office_style      &&
                    desc.bitmap_first      == this.bitmap_first      &&

                    desc.posture           == this.posture           &&
                    desc.posture_office    == this.posture_office    &&

                    desc.weight            == this.weight            &&
                    desc.weight_x          == this.weight_x          &&
                    desc.weight_y          == this.weight_y          &&
                    
                    desc.glyphTx.equals(      this.glyphTx)          &&
                    desc.devTx.equals(        this.devTx));
        } catch (Exception e) {
            /* class cast or NP exceptions should not happen often, if ever,
             * and I am hoping that this is faster than an instanceof check.
             */
            return false;
        }
    }

    FontStrikeDesc() {
        // used with init
    }

    /* This maps a public text AA hint value into one of the subset of values
     * used to index strikes. For the purpose of the strike cache there are
     * only 4 values : OFF, ON, LCD_HRGB, LCD_VRGB.
     * Font and ptSize are needed to resolve the 'gasp' table. The ptSize
     * must therefore include device and font transforms.
     */
    public static int getAAHintIntVal(Object aa, Font2D font2D, int ptSize) {

        if (FontUtilities.isMacOSX14 &&
            (aa == VALUE_TEXT_ANTIALIAS_OFF ||
             aa == VALUE_TEXT_ANTIALIAS_DEFAULT ||
             aa == VALUE_TEXT_ANTIALIAS_ON ||
             aa == VALUE_TEXT_ANTIALIAS_GASP))
        {
             return INTVAL_TEXT_ANTIALIAS_ON;
        }

        if (aa == VALUE_TEXT_ANTIALIAS_OFF ||
            aa == VALUE_TEXT_ANTIALIAS_DEFAULT) {
            return INTVAL_TEXT_ANTIALIAS_OFF;
        } else if (aa == VALUE_TEXT_ANTIALIAS_ON) {
            return INTVAL_TEXT_ANTIALIAS_ON;
        } else if (aa == VALUE_TEXT_ANTIALIAS_GASP) {
            if (font2D.useAAForPtSize(ptSize)) {
                return INTVAL_TEXT_ANTIALIAS_ON;
            } else {
                return INTVAL_TEXT_ANTIALIAS_OFF;
            }
        } else if (aa == VALUE_TEXT_ANTIALIAS_LCD_HRGB ||
                   aa == VALUE_TEXT_ANTIALIAS_LCD_HBGR) {
            return INTVAL_TEXT_ANTIALIAS_LCD_HRGB;
        } else if (aa == VALUE_TEXT_ANTIALIAS_LCD_VRGB ||
                   aa == VALUE_TEXT_ANTIALIAS_LCD_VBGR) {
            return INTVAL_TEXT_ANTIALIAS_LCD_VRGB;
        } else {
            return INTVAL_TEXT_ANTIALIAS_OFF;
        }
    }

    /* This maps a public text AA hint value into one of the subset of values
     * used to index strikes. For the purpose of the strike cache there are
     * only 4 values : OFF, ON, LCD_HRGB, LCD_VRGB.
     * Font and FontRenderContext are needed to resolve the 'gasp' table.
     * This is similar to the method above, but used by callers which have not
     * already calculated the glyph device point size.
     */
    public static int getAAHintIntVal(Font2D font2D, Font font,
                                      FontRenderContext frc) {
        Object aa = frc.getAntiAliasingHint();

        if (FontUtilities.isMacOSX14 &&
            (aa == VALUE_TEXT_ANTIALIAS_OFF ||
             aa == VALUE_TEXT_ANTIALIAS_DEFAULT ||
             aa == VALUE_TEXT_ANTIALIAS_ON ||
             aa == VALUE_TEXT_ANTIALIAS_GASP))
        {
             return INTVAL_TEXT_ANTIALIAS_ON;
        }

        if (aa == VALUE_TEXT_ANTIALIAS_OFF ||
            aa == VALUE_TEXT_ANTIALIAS_DEFAULT) {
            return INTVAL_TEXT_ANTIALIAS_OFF;
        } else if (aa == VALUE_TEXT_ANTIALIAS_ON) {
            return INTVAL_TEXT_ANTIALIAS_ON;
        } else if (aa == VALUE_TEXT_ANTIALIAS_GASP) {
            /* FRC.isIdentity() would have been useful */
            int ptSize;
            AffineTransform tx = frc.getTransform();
            if (tx.isIdentity() && !font.isTransformed()) {
                ptSize = font.getSize();
            } else {
                /* one or both transforms is not identity */
                float size = font.getSize2D();
                if (tx.isIdentity()) {
                    tx = font.getTransform();
                    tx.scale(size, size);
                } else {
                    tx.scale(size, size);
                    if (font.isTransformed()) {
                        tx.concatenate(font.getTransform());
                    }
                }
                double shearx = tx.getShearX();
                double scaley = tx.getScaleY();
                if (shearx != 0) {
                    scaley = Math.sqrt(shearx * shearx + scaley * scaley);
                }
                ptSize = (int)(Math.abs(scaley)+0.5);
            }
            if (font2D.useAAForPtSize(ptSize)) {
                return INTVAL_TEXT_ANTIALIAS_ON;
            } else {
                return INTVAL_TEXT_ANTIALIAS_OFF;
            }
        } else if (aa == VALUE_TEXT_ANTIALIAS_LCD_HRGB ||
                   aa == VALUE_TEXT_ANTIALIAS_LCD_HBGR) {
            return INTVAL_TEXT_ANTIALIAS_LCD_HRGB;
        } else if (aa == VALUE_TEXT_ANTIALIAS_LCD_VRGB ||
                   aa == VALUE_TEXT_ANTIALIAS_LCD_VBGR) {
            return INTVAL_TEXT_ANTIALIAS_LCD_VRGB;
        } else {
            return INTVAL_TEXT_ANTIALIAS_OFF;
        }
    }

    public static int getFMHintIntVal(Object fm) {
        if (fm == VALUE_FRACTIONALMETRICS_OFF ||
            fm == VALUE_FRACTIONALMETRICS_DEFAULT) {
            return INTVAL_FRACTIONALMETRICS_OFF;
        } else {
            return INTVAL_FRACTIONALMETRICS_ON;
        }
    }

    public FontStrikeDesc(AffineTransform devAt, AffineTransform at,
                          int fStyle, int aa, int fm,
                          Map<TextAttribute,?> attrs) {
        this(devAt, at, fStyle, false, aa, fm, attrs);
    }

    public FontStrikeDesc(AffineTransform devAt, AffineTransform at,
                          int fStyle, boolean bitmap, int aa, int fm,
                          Map<TextAttribute,?> attrs) {
        devTx        = devAt;
        glyphTx      = at; // not cloning glyphTx. Callers trusted to not mutate it.
        style        = fStyle;
        bitmap_first = bitmap;
        aaHint       = aa;
        fmHint       = fm;
        valuemask    = fStyle;
        fontAttrs    = attrs;
        switch (aa) {
           case INTVAL_TEXT_ANTIALIAS_OFF :
                break;
           case INTVAL_TEXT_ANTIALIAS_ON  :
                valuemask |= AA_ON;
                break;
           case INTVAL_TEXT_ANTIALIAS_LCD_HRGB :
           case INTVAL_TEXT_ANTIALIAS_LCD_HBGR :
                valuemask |= AA_LCD_H;
                break;
           case INTVAL_TEXT_ANTIALIAS_LCD_VRGB :
           case INTVAL_TEXT_ANTIALIAS_LCD_VBGR :
                valuemask |= AA_LCD_V;
                break;
           default: break;
        }
        if (fm == INTVAL_FRACTIONALMETRICS_ON) {
           valuemask |= FRAC_METRICS_ON;
        }
        initAttrs();
    }

    FontStrikeDesc(FontStrikeDesc desc) {
        devTx     = desc.devTx;
        // Clone the TX in this case as this is called when its known
        // that "desc" is being re-used by its creator.
        glyphTx   = (AffineTransform)desc.glyphTx.clone();
        style     = desc.style;
        aaHint    = desc.aaHint;
        fmHint    = desc.fmHint;
        hashCode  = desc.hashCode;
        valuemask = desc.valuemask;
        fontAttrs = desc.fontAttrs;
        initAttrs();
    }

    private void initAttrs()
    {
        if (this.fontAttrs != null)
        {
            Object value = fontAttrs.get(TextAttribute.OFFICE_STYLE);
            if (value != null)
            {
                this.office_style   = (Integer)value;
            }
            
            value = fontAttrs.get(TextAttribute.POSTURE_OFFICE);
            if (value != null)
            {
                this.posture_office = (Float)value;
            }
            
            value = fontAttrs.get(TextAttribute.WEIGHT);
            if (value != null)
            {
                this.weight = (Float)value;
            }
            value = fontAttrs.get(TextAttribute.WEIGHT_X);
            if (value != null)
            {
                this.weight_x = (Float)value;
            }
            value = fontAttrs.get(TextAttribute.WEIGHT_Y);
            if (value != null)
            {
                this.weight_y = (Float)value;
            }
            
            flags = getFlags();
        }
    }
    

    public int getFlags()
    {
        int flags = 0;

        //control set.
        Object value = fontAttrs.get(TextAttribute.POSTURE_DIRECTION);
        if (value != null && (Boolean)value)
        {
            flags |= FLAG_POSTURE_TO_BOTTOM;
        }
        value = fontAttrs.get(TextAttribute.SOLID_SIZE);
        if (value != null && (Boolean)value)
        {
            flags |= FLAG_SOLID_SIZE_ENABLE;
        }
        value = fontAttrs.get(TextAttribute.SOLID_SIZE_X);
        if (value != null && (Boolean)value)
        {
            flags |= FLAG_SOLID_SIZE_X_ENABLE;
        }
        value = fontAttrs.get(TextAttribute.SOLID_SIZE_Y);
        if (value != null && (Boolean)value)
        {
            flags |= FLAG_SOLID_SIZE_Y_ENABLE;
        }

        value = fontAttrs.get(TextAttribute.BITMAP_BOLD_GRAY);
        if (value != null && (Boolean)value)
        {
            flags |= FLAG_BITMAP_BOLD_GRAY_ENABLE;
        }
        value = fontAttrs.get(TextAttribute.BITMAP_BOLD_GRAY_NORTH);
        if (value != null && (Boolean)value)
        {
            flags |= FLAG_BITMAP_BOLD_GRAY_NORTH_ENABLE;
        }
        value = fontAttrs.get(TextAttribute.BITMAP_BOLD_GRAY_EAST);
        if (value != null && (Boolean)value)
        {
            flags |= FLAG_BITMAP_BOLD_GRAY_EAST_ENABLE;
        }
        value = fontAttrs.get(TextAttribute.BITMAP_BOLD_GRAY_SORTH);
        if (value != null && (Boolean)value)
        {
            flags |= FLAG_BITMAP_BOLD_GRAY_SORTH_ENABLE;
        }
        value = fontAttrs.get(TextAttribute.BITMAP_BOLD_GRAY_WEST);
        if (value != null && (Boolean)value)
        {
            flags |= FLAG_BITMAP_BOLD_GRAY_WEST_ENABLE;
        }
        
        value = fontAttrs.get(TextAttribute.FLIP_L2R);
        if (value != null && (Boolean)value)
        {
            flags |= FLAG_FLIP_L2R;
        }
        value = fontAttrs.get(TextAttribute.FLIP_T2B);
        if (value != null && (Boolean)value)
        {
            flags |= FLAG_FLIP_T2B;
        }
        
        return flags;
    }

    public String toString() {
        return "FontStrikeDesc:"
            + " Style=" +style  + " AA="+aaHint+ " FM="+fmHint
            + " devTx=" +devTx  + " devTx.FontTx.ptSize="+glyphTx
            
            + " flags="+flags
            + " bitmap_first="+bitmap_first
            + " office_style="+office_style
            
            + " posture=" +posture
            + " posture_office=" +posture_office
            
            + " weight="  +weight
            + " weight_x="+weight_x
            + " weight_y="+weight_y;
            
    }
}

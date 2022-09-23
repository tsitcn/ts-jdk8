/*
 * Copyright (c) 2004, 2008, Oracle and/or its affiliates. All rights reserved.
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

/*
 *
 * (C) Copyright IBM Corp. 2005 - All Rights Reserved
 *
 * The original version of this source code and documentation is
 * copyrighted and owned by IBM. These materials are provided
 * under terms of a License Agreement between IBM and Sun.
 * This technology is protected by multiple US and International
 * patents. This notice and attribution to IBM may not be removed.
 */

package sun.font;

import static sun.font.EAttribute.*;
import static java.lang.Math.*;

import java.awt.Font;
import java.awt.Paint;
import java.awt.Toolkit;
import java.awt.font.GraphicAttribute;
import java.awt.font.NumericShaper;
import java.awt.font.TextAttribute;
import java.awt.font.TransformAttribute;
import java.awt.geom.AffineTransform;
import java.awt.geom.NoninvertibleTransformException;
import java.awt.geom.Point2D;
import java.awt.im.InputMethodHighlight;
import java.io.Serializable;
import java.text.Annotation;
import java.text.AttributedCharacterIterator.Attribute;
import java.util.Map;
import java.util.HashMap;
import java.util.Hashtable;

public final class AttributeValues implements Cloneable {
    public final static float WEIGHT_MIN_0PEN =  0;
    public final static float WEIGHT_MAX_0PEN = 10;
    
    public final static float POSTURE_MIN     = -1;
    public final static float POSTURE_MAX     =  1;
    
    private long defined;
    private long nondefault;

    private String  family  = "Default";
    private float   width   = 1f;
    
    // {{{{{{{{{{ extend for taishan office

    private int     office_style           = 0;

    private float   posture                = TextAttribute.POSTURE_REGULAR; // 0f
    private float   posture_office         = TextAttribute.POSTURE_REGULAR; // 0f
    /** top side: left to right; right sid: top to bottom */
    private boolean posture_direction      = TextAttribute.POSTURE_DIRECTION_RIGHT; // false
        
    private float   weight                 = TextAttribute.WEIGHT_REGULAR;  //1f
    private float   weight_x               = TextAttribute.WEIGHT_REGULAR;
    private float   weight_y               = TextAttribute.WEIGHT_REGULAR;
    
    private boolean solid_size             = TextAttribute.SOLID_SIZE_DISABLE;
    private boolean solid_size_x           = TextAttribute.SOLID_SIZE_X_DISABLE;
    private boolean solid_size_y           = TextAttribute.SOLID_SIZE_Y_DISABLE;

    private boolean bitmap_bold_gray       = TextAttribute.BITMAP_BOLD_GRAY_DISABLE;
    private boolean bitmap_bold_gray_north = TextAttribute.BITMAP_BOLD_GRAY_NORTH_DISABLE;
    private boolean bitmap_bold_gray_east  = TextAttribute.BITMAP_BOLD_GRAY_EAST_DISABLE;
    private boolean bitmap_bold_gray_sorth = TextAttribute.BITMAP_BOLD_GRAY_SORTH_DISABLE;
    private boolean bitmap_bold_gray_west  = TextAttribute.BITMAP_BOLD_GRAY_WEST_DISABLE;

    private boolean flip_l2r               = TextAttribute.FLIP_L2R_DISABLE;
    private boolean flip_t2b               = TextAttribute.FLIP_T2B_DISABLE;

    // }}}}}}}}}} extend for taishan office

    
    private float size = 12f;
    private float tracking; // 0f
    private NumericShaper numericShaping; // null
    private AffineTransform transform; // null == identity
    private GraphicAttribute charReplacement; // null
    private Paint foreground; // null
    private Paint background; // null
    private float justification = 1f;
    private Object imHighlight; // null
    // (can be either Attribute wrapping IMH, or IMH itself
    private Font font; // here for completeness, don't actually use
    private byte imUnderline = -1; // same default as underline
    private byte superscript; // 0
    private byte underline = -1; // arrgh, value for ON is 0
    private byte runDirection = -2; // BIDI.DIRECTION_DEFAULT_LEFT_TO_RIGHT
    private byte bidiEmbedding; // 0
    private byte kerning; // 0
    private byte ligatures; // 0
    private boolean strikethrough; // false
    private boolean swapColors; // false

    private AffineTransform baselineTransform; // derived from transform
    private AffineTransform charTransform; // derived from transform

    private static final AttributeValues DEFAULT = new AttributeValues();

    // type-specific API
    public String getFamily() { return family; }
    public void   setFamily(String f) { this.family = f; update(EFAMILY); }

    public float  getWidth() { return width; }
    public void   setWidth(float f) { this.width = f; update(EWIDTH); }

    // {{{{{{{{{{ extends for taishan office
    
    public int     getOfficeStyle() {        return this.office_style; }
    public void    setOfficeStyle(       int   i) { this.office_style      = i; update(EOFFICE_STYLE); }

    public float   getPosture() {            return this.posture; }
    public void    setPosture(           float f) { this.posture           = f; update(EPOSTURE); }
    
    public float   getPostureOffice() {      return this.posture_office; }
    public void    setPostureOffice(     float f) { this.posture_office    = f; update(EPOSTURE_OFFICE); }

    public boolean getPostureDirection() {   return this.posture_direction; }
    public void    setPostureDirection(boolean b) { this.posture_direction = b; update(EPOSTURE_DIRECTION); }

    public float   getWeight() {             return this.weight; }
    public void    setWeight(            float f) { this.weight            = f; update(EWEIGHT); }

    public float   getWeightX() {            return this.weight_x; }
    public void    setWeightX(           float f) { this.weight_x          = f; update(EWEIGHT_X); }

    public float   getWeightY() {            return this.weight_y; }
    public void    setWeightY(           float f) { this.weight_y          = f; update(EWEIGHT_Y); }

    public boolean getSolidSize() {          return this.solid_size; }
    public void    setSolidSize(       boolean b) { this.solid_size        = b; update(ESOLID_SIZE); }

    public boolean getSolidSizeX() {         return this.solid_size_x; }
    public void    setSolidSizeX(      boolean b) { this.solid_size_x      = b; update(ESOLID_SIZE_X); }

    public boolean getSolidSizeY() {         return this.solid_size_y; }
    public void    setSolidSizeY(      boolean b) { this.solid_size_y      = b; update(ESOLID_SIZE_Y); }

    public boolean getBitmapBoldGray() {     return this.bitmap_bold_gray; }
    public void    setBitmapBoldGray(  boolean b) { this.bitmap_bold_gray  = b; update(EBITMAP_BOLD_GRAY); }

    public boolean getBitmapBoldGrayNorth() { return this.bitmap_bold_gray_north; }
    public void    setBitmapBoldGrayNorth(boolean b) { this.bitmap_bold_gray_north = b; update(EBITMAP_BOLD_GRAY_NORTH); }

    public boolean getBitmapBoldGrayEast()  { return this.bitmap_bold_gray_east;  }
    public void    setBitmapBoldGrayEast( boolean b) { this.bitmap_bold_gray_east  = b; update(EBITMAP_BOLD_GRAY_EAST);  }

    public boolean getBitmapBoldGraySorth() { return this.bitmap_bold_gray_sorth; }
    public void    setBitmapBoldGraySorth(boolean b) { this.bitmap_bold_gray_sorth = b; update(EBITMAP_BOLD_GRAY_SORTH); }

    public boolean getBitmapBoldGrayWest()  { return this.bitmap_bold_gray_west;  }
    public void    setBitmapBoldGrayWest( boolean b) { this.bitmap_bold_gray_west  = b; update(EBITMAP_BOLD_GRAY_WEST);  }

    public boolean getFlipL2R() {            return this.flip_l2r; }
    public void    setFlipL2R(         boolean b) { this.flip_l2r          = b; update(EFLIP_L2R); }

    public boolean getFlipT2B() {            return this.flip_t2b; }
    public void    setFlipT2B(         boolean b) { this.flip_t2b          = b; update(EFLIP_T2B); }

    // }}}}}}}}}} extends for taishan office
    

    public float getSize() { return size; }
    public void setSize(float f) { this.size = f; update(ESIZE); }

    public AffineTransform getTransform() { return transform; }
    public void setTransform(AffineTransform f) {
        this.transform = (f == null || f.isIdentity())
            ? DEFAULT.transform
            : new AffineTransform(f);
        updateDerivedTransforms();
        update(ETRANSFORM);
    }
    public void setTransform(TransformAttribute f) {
        this.transform = (f == null || f.isIdentity())
            ? DEFAULT.transform
            : f.getTransform();
        updateDerivedTransforms();
        update(ETRANSFORM);
    }

    public int getSuperscript() { return superscript; }
    public void setSuperscript(int f) {
      this.superscript = (byte)f; update(ESUPERSCRIPT); }

    public Font getFont() { return font; }
    public void setFont(Font f) { this.font = f; update(EFONT); }

    public GraphicAttribute getCharReplacement() { return charReplacement; }
    public void setCharReplacement(GraphicAttribute f) {
      this.charReplacement = f; update(ECHAR_REPLACEMENT); }

    public Paint getForeground() { return foreground; }
    public void setForeground(Paint f) {
      this.foreground = f; update(EFOREGROUND); }

    public Paint getBackground() { return background; }
    public void setBackground(Paint f) {
      this.background = f; update(EBACKGROUND); }

    public int getUnderline() { return underline; }
    public void setUnderline(int f) {
      this.underline = (byte)f; update(EUNDERLINE); }

    public boolean getStrikethrough() { return strikethrough; }
    public void setStrikethrough(boolean f) {
      this.strikethrough = f; update(ESTRIKETHROUGH); }

    public int getRunDirection() { return runDirection; }
    public void setRunDirection(int f) {
      this.runDirection = (byte)f; update(ERUN_DIRECTION); }

    public int getBidiEmbedding() { return bidiEmbedding; }
    public void setBidiEmbedding(int f) {
      this.bidiEmbedding = (byte)f; update(EBIDI_EMBEDDING); }

    public float getJustification() { return justification; }
    public void setJustification(float f) {
      this.justification = f; update(EJUSTIFICATION); }

    public Object getInputMethodHighlight() { return imHighlight; }
    public void setInputMethodHighlight(Annotation f) {
      this.imHighlight = f; update(EINPUT_METHOD_HIGHLIGHT); }
    public void setInputMethodHighlight(InputMethodHighlight f) {
      this.imHighlight = f; update(EINPUT_METHOD_HIGHLIGHT); }

    public int getInputMethodUnderline() { return imUnderline; }
    public void setInputMethodUnderline(int f) {
      this.imUnderline = (byte)f; update(EINPUT_METHOD_UNDERLINE); }

    public boolean getSwapColors() { return swapColors; }
    public void setSwapColors(boolean f) {
      this.swapColors = f; update(ESWAP_COLORS); }

    public NumericShaper getNumericShaping() { return numericShaping; }
    public void setNumericShaping(NumericShaper f) {
      this.numericShaping = f; update(ENUMERIC_SHAPING); }

    public int getKerning() { return kerning; }
    public void setKerning(int f) {
      this.kerning = (byte)f; update(EKERNING); }

    public float getTracking() { return tracking; }
    public void setTracking(float f) {
      this.tracking = (byte)f; update(ETRACKING); }

    public int getLigatures() { return ligatures; }
    public void setLigatures(int f) {
      this.ligatures = (byte)f; update(ELIGATURES); }


    public AffineTransform getBaselineTransform() { return baselineTransform; }
    public AffineTransform getCharTransform() { return charTransform; }

    // mask api

    public static long getMask(EAttribute att) {
        return att.mask;
    }

    public static long getMask(EAttribute ... atts) {
        long mask = 0;
        for (EAttribute a: atts) {
            mask |= a.mask;
        }
        return mask;
    }

    public static final long MASK_ALL =
        getMask(EAttribute.class.getEnumConstants());

    public void unsetDefault() {
        defined &= nondefault;
    }

    public void defineAll(long mask) {
        defined |= mask;
        if ((defined & EBASELINE_TRANSFORM.mask) != 0) {
            throw new InternalError("can't define derived attribute");
        }
    }

    public boolean allDefined(long mask) {
        return (defined & mask) == mask;
    }

    public boolean anyDefined(long mask) {
        return (defined & mask) != 0;
    }

    public boolean anyNonDefault(long mask) {
        return (nondefault & mask) != 0;
    }

    // generic EAttribute API

    public boolean isDefined(EAttribute a) {
        return (defined & a.mask) != 0;
    }

    public boolean isNonDefault(EAttribute a) {
        return (nondefault & a.mask) != 0;
    }

    public void setDefault(EAttribute a) {
        if (a.att == null) {
            throw new InternalError("can't set default derived attribute: " + a);
        }
        i_set(a, DEFAULT);
        defined |= a.mask;
        nondefault &= ~a.mask;
    }

    public void unset(EAttribute a) {
        if (a.att == null) {
            throw new InternalError("can't unset derived attribute: " + a);
        }
        i_set(a, DEFAULT);
        defined &= ~a.mask;
        nondefault &= ~a.mask;
    }

    public void set(EAttribute a, AttributeValues src) {
        if (a.att == null) {
            throw new InternalError("can't set derived attribute: " + a);
        }
        if (src == null || src == DEFAULT) {
            setDefault(a);
        } else {
            if ((src.defined & a.mask) != 0) {
                i_set(a, src);
                update(a);
            }
        }
    }

    public void set(EAttribute a, Object o) {
        if (a.att == null) {
            throw new InternalError("can't set derived attribute: " + a);
        }
        if (o != null) {
            try {
                i_set(a, o);
                update(a);
                return;
            } catch (Exception e) {
            }
        }
        setDefault(a);
    }

    public Object get(EAttribute a) {
        if (a.att == null) {
            throw new InternalError("can't get derived attribute: " + a);
        }
        if ((nondefault & a.mask) != 0) {
            return i_get(a);
        }
        return null;
    }

    // merging

    public AttributeValues merge(Map<? extends Attribute, ?>map) {
        return merge(map, MASK_ALL);
    }

    public AttributeValues merge(Map<? extends Attribute, ?>map,
                                 long mask) {
        if (map instanceof AttributeMap &&
            ((AttributeMap) map).getValues() != null) {
            merge(((AttributeMap)map).getValues(), mask);
        } else if (map != null && !map.isEmpty()) {
            for (Map.Entry<? extends Attribute, ?> e: map.entrySet()) {
                try {
                    EAttribute ea = EAttribute.forAttribute(e.getKey());
                    if (ea!= null && (mask & ea.mask) != 0) {
                        set(ea, e.getValue());
                    }
                } catch (ClassCastException cce) {
                    // IGNORED
                }
            }
        }
        return this;
    }

    public AttributeValues merge(AttributeValues src) {
        return merge(src, MASK_ALL);
    }

    public AttributeValues merge(AttributeValues src, long mask) {
        long m = mask & src.defined;
        for (EAttribute ea: EAttribute.atts) {
            if (m == 0) {
                break;
            }
            if ((m & ea.mask) != 0) {
                m &= ~ea.mask;
                i_set(ea, src);
                update(ea);
            }
        }
        return this;
    }

    // creation API

    public static AttributeValues fromMap(Map<? extends Attribute, ?> map) {
        return fromMap(map, MASK_ALL);
    }

    public static AttributeValues fromMap(Map<? extends Attribute, ?> map,
                                          long mask) {
        return new AttributeValues().merge(map, mask);
    }

    public Map<TextAttribute, Object> toMap(Map<TextAttribute, Object> fill) {
        if (fill == null) {
            fill = new HashMap<TextAttribute, Object>();
        }

        long m = defined;
        for (int i = 0; m != 0; ++i) {
            EAttribute ea = EAttribute.atts[i];
            if ((m & ea.mask) != 0) {
                m &= ~ea.mask;
                fill.put(ea.att, get(ea));
            }
        }

        return fill;
    }

    // key must be serializable, so use String, not Object
    private static final String DEFINED_KEY =
        "sun.font.attributevalues.defined_key";

    public static boolean is16Hashtable(Hashtable<Object, Object> ht) {
        return ht.containsKey(DEFINED_KEY);
    }

    public static AttributeValues
    fromSerializableHashtable(Hashtable<Object, Object> ht)
    {
        AttributeValues result = new AttributeValues();
        if (ht != null && !ht.isEmpty()) {
            for (Map.Entry<Object, Object> e: ht.entrySet()) {
                Object key = e.getKey();
                Object val = e.getValue();
                if (key.equals(DEFINED_KEY)) {
                    result.defineAll(((Integer)val).intValue());
                } else {
                    try {
                        EAttribute ea =
                            EAttribute.forAttribute((Attribute)key);
                        if (ea != null) {
                            result.set(ea, val);
                        }
                    }
                    catch (ClassCastException ex) {
                    }
                }
            }
        }
        return result;
    }

    public Hashtable<Object, Object> toSerializableHashtable() {
        Hashtable ht = new Hashtable();
        long hashkey = defined;
        long m = defined;
        for (int i = 0; m != 0; ++i) {
            EAttribute ea = EAttribute.atts[i];
            if ((m & ea.mask) != 0) {
                m &= ~ea.mask;
                Object o = get(ea);
                if (o == null) {
                    // hashkey will handle it
                } else if (o instanceof Serializable) { // check all...
                    ht.put(ea.att, o);
                } else {
                    hashkey &= ~ea.mask;
                }
            }
        }
        ht.put(DEFINED_KEY, Long.valueOf(hashkey));

        return ht;
    }

    // boilerplate
    public int hashCode() {
        return (int)(defined << 8 ^ nondefault);
    }

    public boolean equals(Object rhs) {
        try {
            return equals((AttributeValues)rhs);
        }
        catch (ClassCastException e) {
        }
        return false;
    }

    public boolean equals(AttributeValues rhs) {
        // test in order of most likely to differ and easiest to compare
        // also assumes we're generally calling this only if family,
        // size, weight, posture are the same

        if (rhs == null) return false;
        if (rhs == this) return true;

        return defined       == rhs.defined
            && nondefault    == rhs.nondefault
            && size          == rhs.size
            && underline     == rhs.underline
            && imUnderline   == rhs.imUnderline
            && strikethrough == rhs.strikethrough
            && superscript   == rhs.superscript
            && width         == rhs.width
            && kerning       == rhs.kerning
            && tracking      == rhs.tracking
            && ligatures     == rhs.ligatures
            && runDirection  == rhs.runDirection
            && bidiEmbedding == rhs.bidiEmbedding
            && swapColors    == rhs.swapColors
            && equals(transform,         rhs.transform)
            && equals(foreground,        rhs.foreground)
            && equals(background,        rhs.background)
            && equals(numericShaping,    rhs.numericShaping)
            && equals(justification,     rhs.justification)
            && equals(charReplacement,   rhs.charReplacement)
            
            && office_style           == rhs.office_style

            && posture                == rhs.posture
            && posture_office         == rhs.posture_office
            && posture_direction      == rhs.posture_direction
            
            && weight                 == rhs.weight
            && weight_x               == rhs.weight_x
            && weight_y               == rhs.weight_y
            && solid_size             == rhs.solid_size
            && solid_size_x           == rhs.solid_size_x
            && solid_size_y           == rhs.solid_size_y

            && bitmap_bold_gray       == rhs.bitmap_bold_gray
            && bitmap_bold_gray_north == rhs.bitmap_bold_gray_north
            && bitmap_bold_gray_east  == rhs.bitmap_bold_gray_east
            && bitmap_bold_gray_sorth == rhs.bitmap_bold_gray_sorth
            && bitmap_bold_gray_west  == rhs.bitmap_bold_gray_west

            && flip_l2r               == rhs.flip_l2r
            && flip_t2b               == rhs.flip_t2b
            
            && equals(family,            rhs.family)
            && equals(font,              rhs.font)
            && equals(imHighlight,       rhs.imHighlight);
    }

    public AttributeValues clone() {
        try {
            AttributeValues result = (AttributeValues)super.clone();
            if (transform != null) { // AffineTransform is mutable
                result.transform = new AffineTransform(transform);
                result.updateDerivedTransforms();
            }
            // if transform is null, derived transforms are null
            // so there's nothing to do
            return result;
        }
        catch (CloneNotSupportedException e) {
            // never happens
            return null;
        }
    }

    public String toString() {
        StringBuilder b = new StringBuilder();
        b.append('{');
        long m = defined;
        for (int i = 0; m != 0; ++i) {
            EAttribute ea = EAttribute.atts[i];
            if ((m & ea.mask) != 0) {
                m &= ~ea.mask;
                if (b.length() > 1) {
                    b.append(", ");
                }
                b.append(ea);
                b.append('=');
                switch (ea) {
                case EFAMILY: b.append('"');
                  b.append(family);
                  b.append('"'); break;
                case EWIDTH: b.append(width); break;

                case EOFFICE_STYLE:      b.append(office_style);      break;

                case EPOSTURE:           b.append(posture);           break;
                case EPOSTURE_OFFICE:    b.append(posture_office);    break;
                case EPOSTURE_DIRECTION: b.append(posture_direction); break;
                
                case EWEIGHT:            b.append(weight);            break;
                case EWEIGHT_X:          b.append(weight_x);          break;
                case EWEIGHT_Y:          b.append(weight_y);          break;
                case ESOLID_SIZE:        b.append(solid_size);        break;
                case ESOLID_SIZE_X:      b.append(solid_size_x);      break;
                case ESOLID_SIZE_Y:      b.append(solid_size_y);      break;

                case EBITMAP_BOLD_GRAY:       b.append(bitmap_bold_gray);       break;
                case EBITMAP_BOLD_GRAY_NORTH: b.append(bitmap_bold_gray_north); break;
                case EBITMAP_BOLD_GRAY_EAST:  b.append(bitmap_bold_gray_east);  break;
                case EBITMAP_BOLD_GRAY_SORTH: b.append(bitmap_bold_gray_sorth); break;
                case EBITMAP_BOLD_GRAY_WEST:  b.append(bitmap_bold_gray_west);  break;

                case EFLIP_L2R:          b.append(flip_l2r);            break;
                case EFLIP_T2B:          b.append(flip_t2b);            break;
                
                case ESIZE: b.append(size); break;
                case ETRANSFORM: b.append(transform); break;
                case ESUPERSCRIPT: b.append(superscript); break;
                case EFONT: b.append(font); break;
                case ECHAR_REPLACEMENT: b.append(charReplacement); break;
                case EFOREGROUND: b.append(foreground); break;
                case EBACKGROUND: b.append(background); break;
                case EUNDERLINE: b.append(underline); break;
                case ESTRIKETHROUGH: b.append(strikethrough); break;
                case ERUN_DIRECTION: b.append(runDirection); break;
                case EBIDI_EMBEDDING: b.append(bidiEmbedding); break;
                case EJUSTIFICATION: b.append(justification); break;
                case EINPUT_METHOD_HIGHLIGHT: b.append(imHighlight); break;
                case EINPUT_METHOD_UNDERLINE: b.append(imUnderline); break;
                case ESWAP_COLORS: b.append(swapColors); break;
                case ENUMERIC_SHAPING: b.append(numericShaping); break;
                case EKERNING: b.append(kerning); break;
                case ELIGATURES: b.append(ligatures); break;
                case ETRACKING: b.append(tracking); break;
                default: throw new InternalError();
                }
                if ((nondefault & ea.mask) == 0) {
                    b.append('*');
                }
            }
        }
        b.append("[btx=" + baselineTransform + ", ctx=" + charTransform + "]");
        b.append('}');
        return b.toString();
    }

    // internal utilities

    private static boolean equals(Object lhs, Object rhs) {
        return lhs == null ? rhs == null : lhs.equals(rhs);
    }

    private void update(EAttribute a) {
        defined |= a.mask;
        if (i_validate(a)) {
            if (i_equals(a, DEFAULT)) {
                nondefault &= ~a.mask;
            } else {
                nondefault |= a.mask;
            }
        } else {
            setDefault(a);
        }
    }

    // dispatch

    private void i_set(EAttribute a, AttributeValues src) {
        switch (a) {
        case EFAMILY: family   = src.family;  break;
        case EWIDTH: width     = src.width;   break;

        case EOFFICE_STYLE:      office_style      = src.office_style;      break;

        case EPOSTURE:           posture           = src.posture;           break;
        case EPOSTURE_OFFICE:    posture_office    = src.posture_office;    break;
        case EPOSTURE_DIRECTION: posture_direction = src.posture_direction; break;
        
        case EWEIGHT:            weight            = src.weight;            break;
        case EWEIGHT_X:          weight_x          = src.weight_x;          break;
        case EWEIGHT_Y:          weight_y          = src.weight_y;          break;
        case ESOLID_SIZE:        solid_size        = src.solid_size;        break;
        case ESOLID_SIZE_X:      solid_size_x      = src.solid_size_x;      break;
        case ESOLID_SIZE_Y:      solid_size_y      = src.solid_size_y;      break;

        case EBITMAP_BOLD_GRAY:       bitmap_bold_gray       = src.bitmap_bold_gray;       break;
        case EBITMAP_BOLD_GRAY_NORTH: bitmap_bold_gray_north = src.bitmap_bold_gray_north; break;
        case EBITMAP_BOLD_GRAY_EAST:  bitmap_bold_gray_east  = src.bitmap_bold_gray_east;  break;
        case EBITMAP_BOLD_GRAY_SORTH: bitmap_bold_gray_sorth = src.bitmap_bold_gray_sorth; break;
        case EBITMAP_BOLD_GRAY_WEST:  bitmap_bold_gray_west  = src.bitmap_bold_gray_west;  break;

        case EFLIP_L2R:          flip_l2r          = src.flip_l2r;          break;
        case EFLIP_T2B:          flip_t2b          = src.flip_t2b;          break;
        
        case ESIZE: size = src.size; break;
        case ETRANSFORM: transform = src.transform; updateDerivedTransforms(); break;
        case ESUPERSCRIPT: superscript = src.superscript; break;
        case EFONT: font = src.font; break;
        case ECHAR_REPLACEMENT: charReplacement = src.charReplacement; break;
        case EFOREGROUND: foreground = src.foreground; break;
        case EBACKGROUND: background = src.background; break;
        case EUNDERLINE: underline = src.underline; break;
        case ESTRIKETHROUGH: strikethrough = src.strikethrough; break;
        case ERUN_DIRECTION: runDirection = src.runDirection; break;
        case EBIDI_EMBEDDING: bidiEmbedding = src.bidiEmbedding; break;
        case EJUSTIFICATION: justification = src.justification; break;
        case EINPUT_METHOD_HIGHLIGHT: imHighlight = src.imHighlight; break;
        case EINPUT_METHOD_UNDERLINE: imUnderline = src.imUnderline; break;
        case ESWAP_COLORS: swapColors = src.swapColors; break;
        case ENUMERIC_SHAPING: numericShaping = src.numericShaping; break;
        case EKERNING: kerning = src.kerning; break;
        case ELIGATURES: ligatures = src.ligatures; break;
        case ETRACKING: tracking = src.tracking; break;
        default: throw new InternalError();
        }
    }

    private boolean i_equals(EAttribute a, AttributeValues src) {
        switch (a) {
        case EFAMILY:  return equals(family, src.family);
        case EWIDTH:   return width   == src.width;
        
        case EOFFICE_STYLE:      return office_style      == src.office_style;

        case EPOSTURE:           return posture           == src.posture;
        case EPOSTURE_OFFICE:    return posture_office    == src.posture_office;
        case EPOSTURE_DIRECTION: return posture_direction == src.posture_direction;

        case EWEIGHT:            return weight            == src.weight;
        case EWEIGHT_X:          return weight_x          == src.weight_x;
        case EWEIGHT_Y:          return weight_y          == src.weight_y;
        case ESOLID_SIZE:        return solid_size        == src.solid_size;
        case ESOLID_SIZE_X:      return solid_size_x      == src.solid_size_x;
        case ESOLID_SIZE_Y:      return solid_size_y      == src.solid_size_y;

        case EBITMAP_BOLD_GRAY:       return bitmap_bold_gray       == src.bitmap_bold_gray;
        case EBITMAP_BOLD_GRAY_NORTH: return bitmap_bold_gray_north == src.bitmap_bold_gray_north;
        case EBITMAP_BOLD_GRAY_EAST:  return bitmap_bold_gray_east  == src.bitmap_bold_gray_east;
        case EBITMAP_BOLD_GRAY_SORTH: return bitmap_bold_gray_sorth == src.bitmap_bold_gray_sorth;
        case EBITMAP_BOLD_GRAY_WEST:  return bitmap_bold_gray_west  == src.bitmap_bold_gray_west;

        case EFLIP_L2R:          return flip_l2r          == src.flip_l2r;
        case EFLIP_T2B:          return flip_t2b          == src.flip_t2b;

        case ESIZE: return size == src.size;
        case ETRANSFORM: return equals(transform, src.transform);
        case ESUPERSCRIPT: return superscript == src.superscript;
        case EFONT: return equals(font, src.font);
        case ECHAR_REPLACEMENT: return equals(charReplacement, src.charReplacement);
        case EFOREGROUND: return equals(foreground, src.foreground);
        case EBACKGROUND: return equals(background, src.background);
        case EUNDERLINE: return underline == src.underline;
        case ESTRIKETHROUGH: return strikethrough == src.strikethrough;
        case ERUN_DIRECTION: return runDirection == src.runDirection;
        case EBIDI_EMBEDDING: return bidiEmbedding == src.bidiEmbedding;
        case EJUSTIFICATION: return justification == src.justification;
        case EINPUT_METHOD_HIGHLIGHT: return equals(imHighlight, src.imHighlight);
        case EINPUT_METHOD_UNDERLINE: return imUnderline == src.imUnderline;
        case ESWAP_COLORS: return swapColors == src.swapColors;
        case ENUMERIC_SHAPING: return equals(numericShaping, src.numericShaping);
        case EKERNING: return kerning == src.kerning;
        case ELIGATURES: return ligatures == src.ligatures;
        case ETRACKING: return tracking == src.tracking;
        default: throw new InternalError();
        }
    }

    private void i_set(EAttribute a, Object o) {
        switch (a) {
        case EFAMILY: family = ((String)o).trim(); break;
        case EWIDTH: width = ((Number)o).floatValue(); break;

        case EOFFICE_STYLE:      office_style      = ((Number )o).intValue();     break;

        case EPOSTURE:           posture           = ((Number )o).floatValue();   break;
        case EPOSTURE_OFFICE:    posture           = ((Number )o).floatValue();   break;
        case EPOSTURE_DIRECTION: posture_direction = ((Boolean)o).booleanValue(); break;

        case EWEIGHT:            weight            = ((Number )o).floatValue();   break;
        case EWEIGHT_X:          weight_x          = ((Number )o).floatValue();   break;
        case EWEIGHT_Y:          weight_y          = ((Number )o).floatValue();   break;
        case ESOLID_SIZE:        solid_size        = ((Boolean)o).booleanValue(); break;
        case ESOLID_SIZE_X:      solid_size_x      = ((Boolean)o).booleanValue(); break;
        case ESOLID_SIZE_Y:      solid_size_y      = ((Boolean)o).booleanValue(); break;

        case EBITMAP_BOLD_GRAY:       bitmap_bold_gray       = ((Boolean)o).booleanValue(); break;
        case EBITMAP_BOLD_GRAY_NORTH: bitmap_bold_gray_north = ((Boolean)o).booleanValue(); break;
        case EBITMAP_BOLD_GRAY_EAST:  bitmap_bold_gray_east  = ((Boolean)o).booleanValue(); break;
        case EBITMAP_BOLD_GRAY_SORTH: bitmap_bold_gray_sorth = ((Boolean)o).booleanValue(); break;
        case EBITMAP_BOLD_GRAY_WEST:  bitmap_bold_gray_west  = ((Boolean)o).booleanValue(); break;

        case EFLIP_L2R:               flip_l2r               = ((Boolean)o).booleanValue(); break;
        case EFLIP_T2B:               flip_t2b               = ((Boolean)o).booleanValue(); break;

        case ESIZE: size = ((Number)o).floatValue(); break;
        case ETRANSFORM: {
            if (o instanceof TransformAttribute) {
                TransformAttribute ta = (TransformAttribute)o;
                if (ta.isIdentity()) {
                    transform = null;
                } else {
                    transform = ta.getTransform();
                }
            } else {
                transform = new AffineTransform((AffineTransform)o);
            }
            updateDerivedTransforms();
        } break;
        case ESUPERSCRIPT: superscript = (byte)((Integer)o).intValue(); break;
        case EFONT: font = (Font)o; break;
        case ECHAR_REPLACEMENT: charReplacement = (GraphicAttribute)o; break;
        case EFOREGROUND: foreground = (Paint)o; break;
        case EBACKGROUND: background = (Paint)o; break;
        case EUNDERLINE: underline = (byte)((Integer)o).intValue(); break;
        case ESTRIKETHROUGH: strikethrough = ((Boolean)o).booleanValue(); break;
        case ERUN_DIRECTION: {
            if (o instanceof Boolean) {
                runDirection = (byte)(TextAttribute.RUN_DIRECTION_LTR.equals(o) ? 0 : 1);
            } else {
                runDirection = (byte)((Integer)o).intValue();
            }
        } break;
        case EBIDI_EMBEDDING: bidiEmbedding = (byte)((Integer)o).intValue(); break;
        case EJUSTIFICATION: justification = ((Number)o).floatValue(); break;
        case EINPUT_METHOD_HIGHLIGHT: {
            if (o instanceof Annotation) {
                Annotation at = (Annotation)o;
                imHighlight = (InputMethodHighlight)at.getValue();
            } else {
                imHighlight = (InputMethodHighlight)o;
            }
        } break;
        case EINPUT_METHOD_UNDERLINE: imUnderline = (byte)((Integer)o).intValue();
          break;
        case ESWAP_COLORS: swapColors = ((Boolean)o).booleanValue(); break;
        case ENUMERIC_SHAPING: numericShaping = (NumericShaper)o; break;
        case EKERNING: kerning = (byte)((Integer)o).intValue(); break;
        case ELIGATURES: ligatures = (byte)((Integer)o).intValue(); break;
        case ETRACKING: tracking = ((Number)o).floatValue(); break;
        default: throw new InternalError();
        }
    }

    private Object i_get(EAttribute a) {
        switch (a) {
        case EFAMILY:  return family;
        case EWIDTH:   return Float.valueOf(width);
        
        case EOFFICE_STYLE:      return Integer.valueOf(office_style);

        case EPOSTURE:           return   Float.valueOf(posture);
        case EPOSTURE_OFFICE:    return   Float.valueOf(posture_office);
        case EPOSTURE_DIRECTION: return Boolean.valueOf(posture_direction);

        case EWEIGHT:            return   Float.valueOf(weight);
        case EWEIGHT_X:          return   Float.valueOf(weight_x);
        case EWEIGHT_Y:          return   Float.valueOf(weight_y);
        case ESOLID_SIZE:        return Boolean.valueOf(solid_size);
        case ESOLID_SIZE_X:      return Boolean.valueOf(solid_size_x);
        case ESOLID_SIZE_Y:      return Boolean.valueOf(solid_size_y);

        case EBITMAP_BOLD_GRAY:       return Boolean.valueOf(bitmap_bold_gray);
        case EBITMAP_BOLD_GRAY_NORTH: return Boolean.valueOf(bitmap_bold_gray_north);
        case EBITMAP_BOLD_GRAY_EAST:  return Boolean.valueOf(bitmap_bold_gray_east);
        case EBITMAP_BOLD_GRAY_SORTH: return Boolean.valueOf(bitmap_bold_gray_sorth);
        case EBITMAP_BOLD_GRAY_WEST:  return Boolean.valueOf(bitmap_bold_gray_west);

        case EFLIP_L2R:               return Boolean.valueOf(flip_l2r);
        case EFLIP_T2B:               return Boolean.valueOf(flip_t2b);

        case ESIZE: return Float.valueOf(size);
        case ETRANSFORM:
            return transform == null
                ? TransformAttribute.IDENTITY
                : new TransformAttribute(transform);
        case ESUPERSCRIPT: return Integer.valueOf(superscript);
        case EFONT: return font;
        case ECHAR_REPLACEMENT: return charReplacement;
        case EFOREGROUND: return foreground;
        case EBACKGROUND: return background;
        case EUNDERLINE: return Integer.valueOf(underline);
        case ESTRIKETHROUGH: return Boolean.valueOf(strikethrough);
        case ERUN_DIRECTION: {
            switch (runDirection) {
                // todo: figure out a way to indicate this value
                // case -1: return Integer.valueOf(runDirection);
            case 0: return TextAttribute.RUN_DIRECTION_LTR;
            case 1: return TextAttribute.RUN_DIRECTION_RTL;
            default: return null;
            }
        } // not reachable
        case EBIDI_EMBEDDING: return Integer.valueOf(bidiEmbedding);
        case EJUSTIFICATION: return Float.valueOf(justification);
        case EINPUT_METHOD_HIGHLIGHT: return imHighlight;
        case EINPUT_METHOD_UNDERLINE: return Integer.valueOf(imUnderline);
        case ESWAP_COLORS: return Boolean.valueOf(swapColors);
        case ENUMERIC_SHAPING: return numericShaping;
        case EKERNING: return Integer.valueOf(kerning);
        case ELIGATURES: return Integer.valueOf(ligatures);
        case ETRACKING: return Float.valueOf(tracking);
        default: throw new InternalError();
        }
    }

    private boolean i_validate(EAttribute a) {
        switch (a) {
        case EFAMILY: if (family == null || family.length() == 0)
          family = DEFAULT.family; return true;
        case EWIDTH: return width >= .5f && width < 10;
        
        case EOFFICE_STYLE:      return true;

        case EPOSTURE:           return posture        >= POSTURE_MIN     && posture        <= POSTURE_MAX;
        case EPOSTURE_OFFICE:    return posture_office >= POSTURE_MIN     && posture_office <= POSTURE_MAX;
        case EPOSTURE_DIRECTION: return true;

        case EWEIGHT:            return weight  >  WEIGHT_MIN_0PEN && weight  <  WEIGHT_MAX_0PEN;
        case EWEIGHT_X:          return weight  >  WEIGHT_MIN_0PEN && weight  <  WEIGHT_MAX_0PEN;
        case EWEIGHT_Y:          return weight  >  WEIGHT_MIN_0PEN && weight  <  WEIGHT_MAX_0PEN;
        case ESOLID_SIZE:        return true;
        case ESOLID_SIZE_X:      return true;
        case ESOLID_SIZE_Y:      return true;

        case EBITMAP_BOLD_GRAY:       return true;
        case EBITMAP_BOLD_GRAY_NORTH: return true;
        case EBITMAP_BOLD_GRAY_EAST:  return true;
        case EBITMAP_BOLD_GRAY_SORTH: return true;
        case EBITMAP_BOLD_GRAY_WEST:  return true;

        case EFLIP_L2R:               return true;
        case EFLIP_T2B:               return true;

        case ESIZE: return size >= 0;
        case ETRANSFORM: if (transform != null && transform.isIdentity())
            transform = DEFAULT.transform; return true;
        case ESUPERSCRIPT: return superscript >= -7 && superscript <= 7;
        case EFONT: return true;
        case ECHAR_REPLACEMENT: return true;
        case EFOREGROUND: return true;
        case EBACKGROUND: return true;
        case EUNDERLINE: return underline >= -1 && underline < 6;
        case ESTRIKETHROUGH: return true;
        case ERUN_DIRECTION: return runDirection >= -2 && runDirection <= 1;
        case EBIDI_EMBEDDING: return bidiEmbedding >= -61 && bidiEmbedding < 62;
        case EJUSTIFICATION: justification = max(0, min (justification, 1));
            return true;
        case EINPUT_METHOD_HIGHLIGHT: return true;
        case EINPUT_METHOD_UNDERLINE: return imUnderline >= -1 && imUnderline < 6;
        case ESWAP_COLORS: return true;
        case ENUMERIC_SHAPING: return true;
        case EKERNING: return kerning >= 0 && kerning <= 1;
        case ELIGATURES: return ligatures >= 0 && ligatures <= 1;
        case ETRACKING: return tracking >= -1 && tracking <= 10;
        default: throw new InternalError("unknown attribute: " + a);
        }
    }

    // Until textlayout is fixed to use AttributeValues, we'll end up
    // creating a map from the values for it.  This is a compromise between
    // creating the whole map and just checking a particular value.
    // Plan to remove these.
    public static float getJustification(Map<?, ?> map) {
        if (map != null) {
            if (map instanceof AttributeMap &&
                ((AttributeMap) map).getValues() != null) {
                return ((AttributeMap)map).getValues().justification;
            }
            Object obj = map.get(TextAttribute.JUSTIFICATION);
            if (obj != null && obj instanceof Number) {
                return max(0, min(1, ((Number)obj).floatValue()));
            }
        }
        return DEFAULT.justification;
    }

    public static NumericShaper getNumericShaping(Map<?, ?> map) {
        if (map != null) {
            if (map instanceof AttributeMap &&
                ((AttributeMap) map).getValues() != null) {
                return ((AttributeMap)map).getValues().numericShaping;
            }
            Object obj = map.get(TextAttribute.NUMERIC_SHAPING);
            if (obj != null && obj instanceof NumericShaper) {
                return (NumericShaper)obj;
            }
        }
        return DEFAULT.numericShaping;
    }

    /**
     * If this has an imHighlight, create copy of this with those attributes
     * applied to it.  Otherwise return this unchanged.
     */
    public AttributeValues applyIMHighlight() {
        if (imHighlight != null) {
            InputMethodHighlight hl = null;
            if (imHighlight instanceof InputMethodHighlight) {
                hl = (InputMethodHighlight)imHighlight;
            } else {
                hl = (InputMethodHighlight)((Annotation)imHighlight).getValue();
            }

            Map imStyles = hl.getStyle();
            if (imStyles == null) {
                Toolkit tk = Toolkit.getDefaultToolkit();
                imStyles = tk.mapInputMethodHighlight(hl);
            }

            if (imStyles != null) {
                return clone().merge(imStyles);
            }
        }

        return this;
    }

    public static AffineTransform getBaselineTransform(Map<?, ?> map) {
        if (map != null) {
            AttributeValues av = null;
            if (map instanceof AttributeMap &&
                ((AttributeMap) map).getValues() != null) {
                av = ((AttributeMap)map).getValues();
            } else if (map.get(TextAttribute.TRANSFORM) != null) {
                av = AttributeValues.fromMap((Map<Attribute, ?>)map); // yuck
            }
            if (av != null) {
                return av.baselineTransform;
            }
        }
        return null;
    }

    public static AffineTransform getCharTransform(Map<?, ?> map) {
        if (map != null) {
            AttributeValues av = null;
            if (map instanceof AttributeMap &&
                ((AttributeMap) map).getValues() != null) {
                av = ((AttributeMap)map).getValues();
            } else if (map.get(TextAttribute.TRANSFORM) != null) {
                av = AttributeValues.fromMap((Map<Attribute, ?>)map); // yuck
            }
            if (av != null) {
                return av.charTransform;
            }
        }
        return null;
    }

    public void updateDerivedTransforms() {
        // this also updates the mask for the baseline transform
        if (transform == null) {
            baselineTransform = null;
            charTransform = null;
        } else {
            charTransform = new AffineTransform(transform);
            baselineTransform = extractXRotation(charTransform, true);

            if (charTransform.isIdentity()) {
              charTransform = null;
            }

            if (baselineTransform.isIdentity()) {
              baselineTransform = null;
            }
        }

        if (baselineTransform == null) {
            nondefault &= ~EBASELINE_TRANSFORM.mask;
        } else {
            nondefault |= EBASELINE_TRANSFORM.mask;
        }
    }

    public static AffineTransform extractXRotation(AffineTransform tx,
                                                   boolean andTranslation) {
        return extractRotation(new Point2D.Double(1, 0), tx, andTranslation);
    }

    public static AffineTransform extractYRotation(AffineTransform tx,
                                                   boolean andTranslation) {
        return extractRotation(new Point2D.Double(0, 1), tx, andTranslation);
    }

    private static AffineTransform extractRotation(Point2D.Double pt,
        AffineTransform tx, boolean andTranslation) {

        tx.deltaTransform(pt, pt);
        AffineTransform rtx = AffineTransform.getRotateInstance(pt.x, pt.y);

        try {
            AffineTransform rtxi = rtx.createInverse();
            double dx = tx.getTranslateX();
            double dy = tx.getTranslateY();
            tx.preConcatenate(rtxi);
            if (andTranslation) {
                if (dx != 0 || dy != 0) {
                    tx.setTransform(tx.getScaleX(), tx.getShearY(),
                                    tx.getShearX(), tx.getScaleY(), 0, 0);
                    rtx.setTransform(rtx.getScaleX(), rtx.getShearY(),
                                     rtx.getShearX(), rtx.getScaleY(), dx, dy);
                }
            }
        }
        catch (NoninvertibleTransformException e) {
            return null;
        }
        return rtx;
    }
}

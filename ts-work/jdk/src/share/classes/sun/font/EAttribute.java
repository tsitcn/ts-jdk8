/*
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
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

import java.awt.font.TextAttribute;
import java.text.AttributedCharacterIterator.Attribute;

import static java.awt.font.TextAttribute.*;

public enum EAttribute {
    EFAMILY(FAMILY),
    EWEIGHT(WEIGHT),
    EWIDTH(WIDTH),
    EPOSTURE(POSTURE),
    
    ESIZE(SIZE),
    ETRANSFORM(TRANSFORM),
    ESUPERSCRIPT(SUPERSCRIPT),
    EFONT(FONT),
    ECHAR_REPLACEMENT(CHAR_REPLACEMENT),
    EFOREGROUND(FOREGROUND),
    EBACKGROUND(BACKGROUND),
    EUNDERLINE(UNDERLINE),
    ESTRIKETHROUGH(STRIKETHROUGH),
    ERUN_DIRECTION(RUN_DIRECTION),
    EBIDI_EMBEDDING(BIDI_EMBEDDING),
    EJUSTIFICATION(JUSTIFICATION),
    EINPUT_METHOD_HIGHLIGHT(INPUT_METHOD_HIGHLIGHT),
    EINPUT_METHOD_UNDERLINE(INPUT_METHOD_UNDERLINE),
    ESWAP_COLORS(SWAP_COLORS),
    ENUMERIC_SHAPING(NUMERIC_SHAPING),
    EKERNING(KERNING),
    ELIGATURES(LIGATURES),
    ETRACKING(TRACKING),

    // {{{{{{{{{{ extend for taishan office
    
    EOFFICE_STYLE(OFFICE_STYLE),

    EPOSTURE_OFFICE(POSTURE_OFFICE),
    EPOSTURE_DIRECTION(POSTURE_DIRECTION),
    
    EWEIGHT_X(WEIGHT_X),
    EWEIGHT_Y(WEIGHT_Y),
    
    ESOLID_SIZE(  SOLID_SIZE),
    ESOLID_SIZE_X(SOLID_SIZE_X),
    ESOLID_SIZE_Y(SOLID_SIZE_Y),

    EBITMAP_BOLD_GRAY      (BITMAP_BOLD_GRAY),
    EBITMAP_BOLD_GRAY_NORTH(BITMAP_BOLD_GRAY_NORTH),
    EBITMAP_BOLD_GRAY_EAST (BITMAP_BOLD_GRAY_EAST ),
    EBITMAP_BOLD_GRAY_SORTH(BITMAP_BOLD_GRAY_SORTH),
    EBITMAP_BOLD_GRAY_WEST (BITMAP_BOLD_GRAY_WEST ),

    EFLIP_L2R(FLIP_L2R),
    EFLIP_T2B(FLIP_T2B),
    
    // }}}}}}}}}} extend for taishan office
    
    EBASELINE_TRANSFORM(null);

    /**
     if member amount > 31, will throw exception when new font.
     so I change it to long.
     */
    /* package */ final long mask;
    /* package */ final TextAttribute att;

    EAttribute(TextAttribute ta) {
        mask = 1L << ordinal();
        att = ta;
    }

    /* package */ static final EAttribute[] atts = EAttribute.class.getEnumConstants();

    public static EAttribute forAttribute(Attribute ta) {
        for (EAttribute ea: atts) {
            if (ea.att == ta) {
                return ea;
            }
        }
        return null;
    }

    public String toString() {
        return name().substring(1).toLowerCase();
    }
}

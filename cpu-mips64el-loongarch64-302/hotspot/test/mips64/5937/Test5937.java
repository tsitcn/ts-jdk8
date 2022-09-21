/*
 * Copyright (c) 2015, 2016, Loongson Technology. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
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
 *
 */

/**
 * @test
 * @summary Modify the implementation of bytecode frem in templateTable
 * 
 * @run main/othervm -Xint Test5937
 * @run main/othervm -Xcomp Test5937
 */
public class Test5937 {

  private static float frem(float a, float b) {
    return a % b;
  }

  private static boolean equalsAsFloat(float a, float b) {
    return Float.compare(a, b) == 0;
  }

  public static void main(String[] args) throws Exception {

    final float[] result = {0.0f, 0.0f, 0.0f, 0.0f, -0.0f, -4.856822E-11f, -1.0f, 4.856822E-11f, 1.0f, 1.0E-10f};
    final float[] data = {0f, -1f, 1f, 1e-10f, Float.POSITIVE_INFINITY};
    int len = data.length;
    int index = 0;

    for (int i = 0; i < len; i++) {
      for (int j = i + 1; j < len; j++) {
        if (!equalsAsFloat(result[index++], data[i] % data[j])) {
          throw new Exception("data[" + i + "] % " + "data[" + j + "] is wrong!!!");
        }
      }
    }
  }

}

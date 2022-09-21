/*
 * Copyright (c) 2018, Loongson Technology. All rights reserved.
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

/*
 * @test
 * @summary Modify the implementation of bytecode frem in templateTable
 *
 * @run main/othervm -Xint Test5394
 * @run main/othervm -Xcomp Test5394
 */

public class Test5394 {
    public static float data_f;
    public static void main(String[] args) throws Exception {
        final String[] data = {
             "-1099511627776.000000", "1099511627776.000000", "-1125899906842624.000000", "1125899906842624.000000",
             "-1152921504606846976.000000", "1152921504606846976.000000", "-137438953472.000000", "137438953472.000000",
             "-140737488355328.000000", "140737488355328.000000", "-144115188075855872.000000", "144115188075855872.000000",
             "-17179869184.000000", "17179869184.000000", "-17592186044416.000000", "17592186044416.000000",
             "-18014398509481984.000000", "18014398509481984.000000", "2147483648.000000", "-2199023255552.000000",
             "2199023255552.000000", "-2251799813685248.000000", "2251799813685248.000000", "-2305843009213693952.000000",
             "2305843009213693952.000000", "2305843009213693952.0", "-2305843284091600896.000000", "-2305843558969507840.000000",
             "-2305844108725321728.000000", "-2305845208236949504.000000", "-2305847407260205056.000000", "-2305851805306716160.000000",
             "-2305860601399738368.000000", "-2305878193585782784.000000", "-2305913377957871616.000000", "-2305983746702049280.000000",
             "-2306124484190404608.000000", "-2306405959167115264.000000", "-2306968909120536576.000000", "-2308094809027379200.000000",
             "-2310346608841064448.000000", "-2314850208468434944.000000", "-2323857407723175936.000000", "-2341871806232657920.000000",
             "-2377900603251621888.000000", "-2449958197289549824.000000", "-2594073385365405696.000000"
        };

        final float[] result = {
             -1.09951163E12f, 1.09951163E12f, -1.12589991E15f, 1.12589991E15f, -1.1529215E18f, 1.1529215E18f,
             -1.37438953E11f, 1.37438953E11f, -1.40737488E14f, 1.40737488E14f, -1.44115188E17f, 1.44115188E17f,
             -1.71798692E10f, 1.71798692E10f, -1.7592186E13f, 1.7592186E13f, -1.80143985E16f, 1.80143985E16f,
             2.14748365E9f, -2.19902326E12f, 2.19902326E12f, -2.25179981E15f, 2.25179981E15f, -2.30584301E18f,
             2.30584301E18f, 2.30584301E18f, -2.30584328E18f, -2.30584356E18f, -2.30584411E18f, -2.30584521E18f,
             -2.30584741E18f, -2.30585181E18f, -2.3058606E18f, -2.30587819E18f, -2.30591338E18f, -2.30598375E18f,
             -2.30612448E18f, -2.30640596E18f, -2.30696891E18f, -2.30809481E18f, -2.31034661E18f, -2.31485021E18f,
            -2.32385741E18f, -2.34187181E18f, -2.3779006E18f, -2.4499582E18f, -2.59407339E18f
        };

        for (int i = 0; i <= 10000; i++) {
            for (int j = 0; j < data.length; j++) {
                func(data[j]);
                if (Math.abs(result[j] - data_f) > 0) {
                    throw new Exception("data: " + data[j] + " error; Correct: " + result[j]);
                }
            }
        }
        System.out.println("PASSED");
    }

    public static int func (String data_str) {
        data_f = Float.parseFloat(data_str);
        return (int)data_f;
    }

}

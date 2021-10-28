/*****************************************************************************
 * Copyright (C) 2021 Christian Doppler Laboratory ATHENA
 *
 * Authors: Vignesh V Menon <vignesh.menon@aau.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *****************************************************************************/

#include "common.h"
#include "primitives.h"
#include "threadpool.h"
#include "encoder.h"

#if _MSC_VER
#pragma warning(disable: 4996) // POSIX functions are just fine, thanks
#pragma warning(disable: 4706) // assignment within conditional
#endif

#if _WIN32
#define strcasecmp _stricmp
#endif

static const double weights_dct8[64] = {
0.000000, 0.000000, 0.368689, 0.369319, 0.370132, 0.371127, 0.372307, 0.373673,
0.000000, 0.369319, 0.371127, 0.373673, 0.376971, 0.381043, 0.385911, 0.391606,
0.368689, 0.371127, 0.375227, 0.381043, 0.388653, 0.398161, 0.409698, 0.423427,
0.369319, 0.373673, 0.381043, 0.391606, 0.405618, 0.423427, 0.445484, 0.472367,
0.370132, 0.376971, 0.388653, 0.405618, 0.428522, 0.458281, 0.496125, 0.543691,
0.371127, 0.381043, 0.398161, 0.423427, 0.458281, 0.504800, 0.565901, 0.645649,
0.372307, 0.385911, 0.409698, 0.445484, 0.496125, 0.565901, 0.661121, 0.791065,
0.373673, 0.391606, 0.423427, 0.472367, 0.543691, 0.645649, 0.791065, 1.000000,
};

static const double weights_dct16[256] = {
0.000000, 0.000000, 0.367930, 0.367969, 0.368020, 0.368082, 0.368155, 0.368239, 0.368334, 0.368441, 0.368559, 0.368689, 0.368829, 0.368981, 0.369145, 0.369319,
0.000000, 0.367969, 0.368082, 0.368239, 0.368441, 0.368689, 0.368981, 0.369319, 0.369703, 0.370132, 0.370606, 0.371127, 0.371694, 0.372307, 0.372966, 0.373673,
0.367930, 0.368082, 0.368334, 0.368689, 0.369145, 0.369703, 0.370363, 0.371127, 0.371994, 0.372966, 0.374043, 0.375227, 0.376517, 0.377916, 0.379424, 0.381043,
0.367969, 0.368239, 0.368689, 0.369319, 0.370132, 0.371127, 0.372307, 0.373673, 0.375227, 0.376971, 0.378909, 0.381043, 0.383376, 0.385911, 0.388653, 0.391606,
0.368020, 0.368441, 0.369145, 0.370132, 0.371405, 0.372966, 0.374821, 0.376971, 0.379424, 0.382184, 0.385258, 0.388653, 0.392377, 0.396439, 0.400849, 0.405618,
0.368082, 0.368689, 0.369703, 0.371127, 0.372966, 0.375227, 0.377916, 0.381043, 0.384618, 0.388653, 0.393162, 0.398161, 0.403667, 0.409698, 0.416277, 0.423427,
0.368155, 0.368981, 0.370363, 0.372307, 0.374821, 0.377916, 0.381607, 0.385911, 0.390847, 0.396439, 0.402713, 0.409698, 0.417429, 0.425941, 0.435277, 0.445484,
0.368239, 0.369319, 0.371127, 0.373673, 0.376971, 0.381043, 0.385911, 0.391606, 0.398161, 0.405618, 0.414022, 0.423427, 0.433891, 0.445484, 0.458281, 0.472367,
0.368334, 0.369703, 0.371994, 0.375227, 0.379424, 0.384618, 0.390847, 0.398161, 0.406616, 0.416277, 0.427223, 0.439542, 0.453336, 0.468719, 0.485824, 0.504800,
0.368441, 0.370132, 0.372966, 0.376971, 0.382184, 0.388653, 0.396439, 0.405618, 0.416277, 0.428522, 0.442476, 0.458281, 0.476100, 0.496125, 0.518572, 0.543691,
0.368559, 0.370606, 0.374043, 0.378909, 0.385258, 0.393162, 0.402713, 0.414022, 0.427223, 0.442476, 0.459969, 0.479922, 0.502594, 0.528283, 0.557340, 0.590171,
0.368689, 0.371127, 0.375227, 0.381043, 0.388653, 0.398161, 0.409698, 0.423427, 0.439542, 0.458281, 0.479922, 0.504800, 0.533305, 0.565901, 0.603134, 0.645649,
0.368829, 0.371694, 0.376517, 0.383376, 0.392377, 0.403667, 0.417429, 0.433891, 0.453336, 0.476100, 0.502594, 0.533305, 0.568819, 0.609834, 0.657188, 0.711882,
0.368981, 0.372307, 0.377916, 0.385911, 0.396439, 0.409698, 0.425941, 0.445484, 0.468719, 0.496125, 0.528283, 0.565901, 0.609834, 0.661121, 0.721021, 0.791065,
0.369145, 0.372966, 0.379424, 0.388653, 0.400849, 0.416277, 0.435277, 0.458281, 0.485824, 0.518572, 0.557340, 0.603134, 0.657188, 0.721021, 0.796503, 0.885951,
0.369319, 0.373673, 0.381043, 0.391606, 0.405618, 0.423427, 0.445484, 0.472367, 0.504800, 0.543691, 0.590171, 0.645649, 0.711882, 0.791065, 0.885951, 1.000000,
};

static const double weights_dct32[1024] = {
0.000000, 0.000000, 0.367883, 0.367885, 0.367888, 0.367892, 0.367897, 0.367902, 0.367908, 0.367915, 0.367922, 0.367930, 0.367939, 0.367948, 0.367958, 0.367969, 0.367981, 0.367993, 0.368006, 0.368020, 0.368034, 0.368049, 0.368065, 0.368082, 0.368099, 0.368117, 0.368135, 0.368155, 0.368175, 0.368195, 0.368217, 0.368239,
0.000000, 0.367885, 0.367892, 0.367902, 0.367915, 0.367930, 0.367948, 0.367969, 0.367993, 0.368020, 0.368049, 0.368082, 0.368117, 0.368155, 0.368195, 0.368239, 0.368285, 0.368334, 0.368386, 0.368441, 0.368499, 0.368559, 0.368623, 0.368689, 0.368758, 0.368829, 0.368904, 0.368981, 0.369062, 0.369145, 0.369231, 0.369319,
0.367883, 0.367892, 0.367908, 0.367930, 0.367958, 0.367993, 0.368034, 0.368082, 0.368135, 0.368195, 0.368262, 0.368334, 0.368413, 0.368499, 0.368591, 0.368689, 0.368793, 0.368904, 0.369021, 0.369145, 0.369275, 0.369411, 0.369554, 0.369703, 0.369858, 0.370020, 0.370189, 0.370363, 0.370545, 0.370732, 0.370926, 0.371127,
0.367885, 0.367902, 0.367930, 0.367969, 0.368020, 0.368082, 0.368155, 0.368239, 0.368334, 0.368441, 0.368559, 0.368689, 0.368829, 0.368981, 0.369145, 0.369319, 0.369505, 0.369703, 0.369911, 0.370132, 0.370363, 0.370606, 0.370861, 0.371127, 0.371405, 0.371694, 0.371994, 0.372307, 0.372631, 0.372966, 0.373314, 0.373673,
0.367888, 0.367915, 0.367958, 0.368020, 0.368099, 0.368195, 0.368309, 0.368441, 0.368591, 0.368758, 0.368942, 0.369145, 0.369365, 0.369603, 0.369858, 0.370132, 0.370423, 0.370732, 0.371059, 0.371405, 0.371768, 0.372149, 0.372549, 0.372966, 0.373402, 0.373857, 0.374329, 0.374821, 0.375330, 0.375859, 0.376406, 0.376971,
0.367892, 0.367930, 0.367993, 0.368082, 0.368195, 0.368334, 0.368499, 0.368689, 0.368904, 0.369145, 0.369411, 0.369703, 0.370020, 0.370363, 0.370732, 0.371127, 0.371548, 0.371994, 0.372467, 0.372966, 0.373492, 0.374043, 0.374622, 0.375227, 0.375859, 0.376517, 0.377203, 0.377916, 0.378656, 0.379424, 0.380219, 0.381043,
0.367897, 0.367948, 0.368034, 0.368155, 0.368309, 0.368499, 0.368723, 0.368981, 0.369275, 0.369603, 0.369965, 0.370363, 0.370796, 0.371264, 0.371768, 0.372307, 0.372881, 0.373492, 0.374138, 0.374821, 0.375539, 0.376295, 0.377087, 0.377916, 0.378782, 0.379686, 0.380628, 0.381607, 0.382625, 0.383681, 0.384777, 0.385911,
0.367902, 0.367969, 0.368082, 0.368239, 0.368441, 0.368689, 0.368981, 0.369319, 0.369703, 0.370132, 0.370606, 0.371127, 0.371694, 0.372307, 0.372966, 0.373673, 0.374426, 0.375227, 0.376075, 0.376971, 0.377916, 0.378909, 0.379951, 0.381043, 0.382184, 0.383376, 0.384618, 0.385911, 0.387256, 0.388653, 0.390103, 0.391606,
0.367908, 0.367993, 0.368135, 0.368334, 0.368591, 0.368904, 0.369275, 0.369703, 0.370189, 0.370732, 0.371334, 0.371994, 0.372714, 0.373492, 0.374329, 0.375227, 0.376185, 0.377203, 0.378283, 0.379424, 0.380628, 0.381894, 0.383224, 0.384618, 0.386076, 0.387600, 0.389190, 0.390847, 0.392572, 0.394365, 0.396228, 0.398161,
0.367915, 0.368020, 0.368195, 0.368441, 0.368758, 0.369145, 0.369603, 0.370132, 0.370732, 0.371405, 0.372149, 0.372966, 0.373857, 0.374821, 0.375859, 0.376971, 0.378160, 0.379424, 0.380765, 0.382184, 0.383681, 0.385258, 0.386915, 0.388653, 0.390473, 0.392377, 0.394365, 0.396439, 0.398600, 0.400849, 0.403188, 0.405618,
0.367922, 0.368049, 0.368262, 0.368559, 0.368942, 0.369411, 0.369965, 0.370606, 0.371334, 0.372149, 0.373052, 0.374043, 0.375124, 0.376295, 0.377556, 0.378909, 0.380355, 0.381894, 0.383528, 0.385258, 0.387085, 0.389010, 0.391036, 0.393162, 0.395392, 0.397725, 0.400165, 0.402713, 0.405371, 0.408140, 0.411023, 0.414022,
0.367930, 0.368082, 0.368334, 0.368689, 0.369145, 0.369703, 0.370363, 0.371127, 0.371994, 0.372966, 0.374043, 0.375227, 0.376517, 0.377916, 0.379424, 0.381043, 0.382773, 0.384618, 0.386577, 0.388653, 0.390847, 0.393162, 0.395600, 0.398161, 0.400849, 0.403667, 0.406616, 0.409698, 0.412918, 0.416277, 0.419779, 0.423427,
0.367939, 0.368117, 0.368413, 0.368829, 0.369365, 0.370020, 0.370796, 0.371694, 0.372714, 0.373857, 0.375124, 0.376517, 0.378037, 0.379686, 0.381465, 0.383376, 0.385420, 0.387600, 0.389919, 0.392377, 0.394979, 0.397725, 0.400621, 0.403667, 0.406867, 0.410225, 0.413745, 0.417429, 0.421281, 0.425306, 0.429508, 0.433891,
0.367948, 0.368155, 0.368499, 0.368981, 0.369603, 0.370363, 0.371264, 0.372307, 0.373492, 0.374821, 0.376295, 0.377916, 0.379686, 0.381607, 0.383681, 0.385911, 0.388299, 0.390847, 0.393560, 0.396439, 0.399489, 0.402713, 0.406115, 0.409698, 0.413468, 0.417429, 0.421584, 0.425941, 0.430503, 0.435277, 0.440269, 0.445484,
0.367958, 0.368195, 0.368591, 0.369145, 0.369858, 0.370732, 0.371768, 0.372966, 0.374329, 0.375859, 0.377556, 0.379424, 0.381465, 0.383681, 0.386076, 0.388653, 0.391415, 0.394365, 0.397509, 0.400849, 0.404392, 0.408140, 0.412100, 0.416277, 0.420677, 0.425306, 0.430171, 0.435277, 0.440634, 0.446248, 0.452127, 0.458281,
0.367969, 0.368239, 0.368689, 0.369319, 0.370132, 0.371127, 0.372307, 0.373673, 0.375227, 0.376971, 0.378909, 0.381043, 0.383376, 0.385911, 0.388653, 0.391606, 0.394773, 0.398161, 0.401774, 0.405618, 0.409698, 0.414022, 0.418596, 0.423427, 0.428522, 0.433891, 0.439542, 0.445484, 0.451727, 0.458281, 0.465157, 0.472367,
0.367981, 0.368285, 0.368793, 0.369505, 0.370423, 0.371548, 0.372881, 0.374426, 0.376185, 0.378160, 0.380355, 0.382773, 0.385420, 0.388299, 0.391415, 0.394773, 0.398380, 0.402242, 0.406365, 0.410756, 0.415424, 0.420377, 0.425623, 0.431172, 0.437035, 0.443221, 0.449743, 0.456612, 0.463842, 0.471447, 0.479440, 0.487837,
0.367993, 0.368334, 0.368904, 0.369703, 0.370732, 0.371994, 0.373492, 0.375227, 0.377203, 0.379424, 0.381894, 0.384618, 0.387600, 0.390847, 0.394365, 0.398161, 0.402242, 0.406616, 0.411291, 0.416277, 0.421584, 0.427223, 0.433205, 0.439542, 0.446248, 0.453336, 0.460821, 0.468719, 0.477048, 0.485824, 0.495068, 0.504800,
0.368006, 0.368386, 0.369021, 0.369911, 0.371059, 0.372467, 0.374138, 0.376075, 0.378283, 0.380765, 0.383528, 0.386577, 0.389919, 0.393560, 0.397509, 0.401774, 0.406365, 0.411291, 0.416564, 0.422194, 0.428196, 0.434582, 0.441367, 0.448567, 0.456199, 0.464279, 0.472829, 0.481867, 0.491416, 0.501500, 0.512144, 0.523373,
0.368020, 0.368441, 0.369145, 0.370132, 0.371405, 0.372966, 0.374821, 0.376971, 0.379424, 0.382184, 0.385258, 0.388653, 0.392377, 0.396439, 0.400849, 0.405618, 0.410756, 0.416277, 0.422194, 0.428522, 0.435277, 0.442476, 0.450137, 0.458281, 0.466927, 0.476100, 0.485824, 0.496125, 0.507031, 0.518572, 0.530780, 0.543691,
0.368034, 0.368499, 0.369275, 0.370363, 0.371768, 0.373492, 0.375539, 0.377916, 0.380628, 0.383681, 0.387085, 0.390847, 0.394979, 0.399489, 0.404392, 0.409698, 0.415424, 0.421584, 0.428196, 0.435277, 0.442848, 0.450930, 0.459545, 0.468719, 0.478479, 0.488853, 0.499872, 0.511569, 0.523981, 0.537145, 0.551104, 0.565901,
0.368049, 0.368559, 0.369411, 0.370606, 0.372149, 0.374043, 0.376295, 0.378909, 0.381894, 0.385258, 0.389010, 0.393162, 0.397725, 0.402713, 0.408140, 0.414022, 0.420377, 0.427223, 0.434582, 0.442476, 0.450930, 0.459969, 0.469623, 0.479922, 0.490901, 0.502594, 0.515041, 0.528283, 0.542367, 0.557340, 0.573256, 0.590171,
0.368065, 0.368623, 0.369554, 0.370861, 0.372549, 0.374622, 0.377087, 0.379951, 0.383224, 0.386915, 0.391036, 0.395600, 0.400621, 0.406115, 0.412100, 0.418596, 0.425623, 0.433205, 0.441367, 0.450137, 0.459545, 0.469623, 0.480406, 0.491934, 0.504246, 0.517388, 0.531409, 0.546360, 0.562299, 0.579288, 0.597392, 0.616684,
0.368082, 0.368689, 0.369703, 0.371127, 0.372966, 0.375227, 0.377916, 0.381043, 0.384618, 0.388653, 0.393162, 0.398161, 0.403667, 0.409698, 0.416277, 0.423427, 0.431172, 0.439542, 0.448567, 0.458281, 0.468719, 0.479922, 0.491934, 0.504800, 0.518572, 0.533305, 0.549060, 0.565901, 0.583900, 0.603134, 0.623687, 0.645649,
0.368099, 0.368758, 0.369858, 0.371405, 0.373402, 0.375859, 0.378782, 0.382184, 0.386076, 0.390473, 0.395392, 0.400849, 0.406867, 0.413468, 0.420677, 0.428522, 0.437035, 0.446248, 0.456199, 0.466927, 0.478479, 0.490901, 0.504246, 0.518572, 0.533940, 0.550421, 0.568086, 0.587018, 0.607305, 0.629041, 0.652333, 0.677295,
0.368117, 0.368829, 0.370020, 0.371694, 0.373857, 0.376517, 0.379686, 0.383376, 0.387600, 0.392377, 0.397725, 0.403667, 0.410225, 0.417429, 0.425306, 0.433891, 0.443221, 0.453336, 0.464279, 0.476100, 0.488853, 0.502594, 0.517388, 0.533305, 0.550421, 0.568819, 0.588590, 0.609834, 0.632661, 0.657188, 0.683548, 0.711882,
0.368135, 0.368904, 0.370189, 0.371994, 0.374329, 0.377203, 0.380628, 0.384618, 0.389190, 0.394365, 0.400165, 0.406616, 0.413745, 0.421584, 0.430171, 0.439542, 0.449743, 0.460821, 0.472829, 0.485824, 0.499872, 0.515041, 0.531409, 0.549060, 0.568086, 0.588590, 0.610682, 0.634486, 0.660134, 0.687774, 0.717570, 0.749697,
0.368155, 0.368981, 0.370363, 0.372307, 0.374821, 0.377916, 0.381607, 0.385911, 0.390847, 0.396439, 0.402713, 0.409698, 0.417429, 0.425941, 0.435277, 0.445484, 0.456612, 0.468719, 0.481867, 0.496125, 0.511569, 0.528283, 0.546360, 0.565901, 0.587018, 0.609834, 0.634486, 0.661121, 0.689906, 0.721021, 0.754667, 0.791065,
0.368175, 0.369062, 0.370545, 0.372631, 0.375330, 0.378656, 0.382625, 0.387256, 0.392572, 0.398600, 0.405371, 0.412918, 0.421281, 0.430503, 0.440634, 0.451727, 0.463842, 0.477048, 0.491416, 0.507031, 0.523981, 0.542367, 0.562299, 0.583900, 0.607305, 0.632661, 0.660134, 0.689906, 0.722178, 0.757173, 0.795138, 0.836348,
0.368195, 0.369145, 0.370732, 0.372966, 0.375859, 0.379424, 0.383681, 0.388653, 0.394365, 0.400849, 0.408140, 0.416277, 0.425306, 0.435277, 0.446248, 0.458281, 0.471447, 0.485824, 0.501500, 0.518572, 0.537145, 0.557340, 0.579288, 0.603134, 0.629041, 0.657188, 0.687774, 0.721021, 0.757173, 0.796503, 0.839317, 0.885951,
0.368217, 0.369231, 0.370926, 0.373314, 0.376406, 0.380219, 0.384777, 0.390103, 0.396228, 0.403188, 0.411023, 0.419779, 0.429508, 0.440269, 0.452127, 0.465157, 0.479440, 0.495068, 0.512144, 0.530780, 0.551104, 0.573256, 0.597392, 0.623687, 0.652333, 0.683548, 0.717570, 0.754667, 0.795138, 0.839317, 0.887575, 0.940331,
0.368239, 0.369319, 0.371127, 0.373673, 0.376971, 0.381043, 0.385911, 0.391606, 0.398161, 0.405618, 0.414022, 0.423427, 0.433891, 0.445484, 0.458281, 0.472367, 0.487837, 0.504800, 0.523373, 0.543691, 0.565901, 0.590171, 0.616684, 0.645649, 0.677295, 0.711882, 0.749697, 0.791065, 0.836348, 0.885951, 0.940331, 1.000000,
};

using namespace VCA_NS;

Encoder::Encoder()
{
    m_aborted = false;
    m_param = NULL;
    epsilons = NULL;
}

vca_encoder* encoder_open(vca_param *p)
{
    if (!p)
        return NULL;

#if _MSC_VER
#pragma warning(disable: 4127) // conditional expression is constant, yes I know
#endif

#if HIGH_BIT_DEPTH
    if (VCA_DEPTH != 10 && VCA_DEPTH != 12)
#else
    if (VCA_DEPTH != 8)
#endif
    {
        vca_log(p, VCA_LOG_ERROR, "Build error, internal bit depth mismatch\n");
        return NULL;
    }

    Encoder* encoder = NULL;
    vca_param* param = param_alloc();
    if (!param)
        goto fail;

    memcpy(param, p, sizeof(vca_param));

    vca_setup_primitives(param);

    if (check_params(param))
        goto fail;

    encoder = new Encoder;

    // may change params for auto-detect, etc
    encoder->configure(param);

    encoder->create();

    /* Try to open CSV file handle */
    if (encoder->m_param->csvfn)
    {
        encoder->m_param->csvfpt = csvlog_open(encoder->m_param);
        if (!encoder->m_param->csvfpt)
        {
            vca_log(encoder->m_param, VCA_LOG_ERROR, "Unable to open CSV log file <%s>, aborting\n", encoder->m_param->csvfn);
            encoder->m_aborted = true;
        }
    }
    encoder->m_param = param;
    memcpy(encoder->m_param, param, sizeof(vca_param));
    if (encoder->m_aborted)
        goto fail;

    print_params(param);
    return encoder;

fail:
    delete encoder;
    param_free(param);
    return NULL;
}

void encoder_parameters(vca_encoder *enc, vca_param *out)
{
    if (enc && out)
    {
        Encoder *encoder = static_cast<Encoder*>(enc);
        memcpy(out, encoder->m_param, sizeof(vca_param));
    }
}

int encoder_encode(vca_encoder *enc, vca_picture *pic_in)
{
    if (!enc)
        return -1;

    if (!pic_in)
        return 0;

    Encoder *encoder = static_cast<Encoder*>(enc);
    int numEncoded;

    do
    {
        numEncoded = encoder->encode(pic_in);
        if (numEncoded)
            csvlog_frame(encoder->m_param, pic_in);
    } while (numEncoded == 0 && !pic_in);

    if (numEncoded < 0)
        encoder->m_aborted = true;

    return numEncoded;
}

void encoder_close(vca_encoder *enc)
{
    if (enc)
    {
        Encoder *encoder = static_cast<Encoder*>(enc);
        encoder->destroy();
        delete encoder;
    }
}

void encoder_shot_detect(vca_encoder *enc)
{
    Encoder *encoder = static_cast<Encoder*>(enc);
    int numFrames = encoder->m_param->totalFrames;

    /* First pass */
    encoder->isNewShot[0] = VCA_NEW_SHOT;
    encoder->isNewShot[1] = VCA_NOT_NEW_SHOT;
    encoder->prevShotPos = 0;
    int not_sure_count = 0;
    for (int i = 2, j = 0; i < numFrames; i++)
    {
        if (encoder->epsilons[i] > encoder->m_param->maxThresh)
        {
            encoder->isNewShot[i] = VCA_NEW_SHOT;
            encoder->prevShotPos = i;
        }
        else if (encoder->epsilons[i] < encoder->m_param->minThresh)
        {
            encoder->isNewShot[i] = VCA_NOT_NEW_SHOT;
        }
        else
        {
            encoder->isNewShot[i] = VCA_NOTSURE_NEW_SHOT;
            encoder->isNotSureShot[j] = i;
            encoder->prevShotDist[j] = i - encoder->prevShotPos;
            not_sure_count++;
            j++;
        }
    }
    /* If there are no VCA_NOTSURE_NEW_SHOT entries, shot detection has been completed */
    vca_log(encoder->m_param, VCA_LOG_DEBUG, "First pass of shot detection complete!\n");
    if (!not_sure_count)
        return;


    /* Second pass */
    for (int j = 0; j < not_sure_count; j++)
    {
        int fps_value = encoder->m_param->fpsNum / encoder->m_param->fpsDenom;
        if(encoder->prevShotDist[j] > fps_value && encoder->prevShotDist[j + 1] > fps_value)
        {
            encoder->isNewShot[encoder->isNotSureShot[j]] = VCA_NEW_SHOT;
        }
        else
        {
            encoder->isNewShot[encoder->isNotSureShot[j]] = VCA_NOT_NEW_SHOT;
        }
    }
    vca_log(encoder->m_param, VCA_LOG_DEBUG, "Second pass of shot detection complete!\n");
}

void encoder_shot_print(vca_encoder *enc)
{
    Encoder *encoder = static_cast<Encoder*>(enc);
    int numFrames = encoder->m_param->totalFrames;
    for (int i = 0; i < numFrames; i++)
    {
        if(encoder->isNewShot[i] == VCA_NEW_SHOT)
            vca_log(encoder->m_param, VCA_LOG_INFO, "IDR at POC %d\n", i);
    }
}

vca_picture *picture_alloc()
{
    return (vca_picture*)vca_malloc(sizeof(vca_picture));
}

void picture_init(vca_param *param, vca_picture *pic)
{
    memset(pic, 0, sizeof(vca_picture));
    pic->bitDepth = param->internalBitDepth;
    pic->colorSpace = param->internalCsp;
}

void picture_free(vca_picture *p)
{
    return vca_free(p);
}

FILE* csvlog_open(const vca_param* param)
{
    FILE *csvfp = vca_fopen(param->csvfn, "r");
    if (csvfp)
    {
        /* file already exists, re-open for append */
        fclose(csvfp);
        return vca_fopen(param->csvfn, "ab");
    }
    else
    {
        /* new CSV file, write header */
        csvfp = vca_fopen(param->csvfn, "wb");
        if (csvfp)
        {
            fprintf(csvfp, "POC, E, h, epsilon, ");
            fprintf(csvfp, "\n");
        }
        return csvfp;
    }
}

void csvlog_frame(const vca_param* param, const vca_picture* pic)
{
    if (!param->csvfpt)
        return;

    const vca_frame_stats* frameStats = &pic->frameData;
    fprintf(param->csvfpt, "%d, %d, %f, %f,", frameStats->poc, frameStats->e_value, frameStats->h_value, frameStats->epsilon);
    fprintf(param->csvfpt, "\n");
    fflush(stderr);
}

bool Encoder::compute_weighted_DCT_energy(vca_picture *pic, vca_frame_texture_t *m_texture)
{
    pixel *src = NULL;
    if (pic->bitDepth == VCA_DEPTH)
    {
        src = (pixel*)pic->planes[0];
    }
    else if (pic->bitDepth == 8 && VCA_DEPTH > 8)
    {
        int shift = (VCA_DEPTH - 8);
        uint8_t *yChar;
        yChar = (uint8_t*)pic->planes[0];
        primitives.planecopy_cp(yChar, pic->stride[0] / sizeof(*yChar), src, pic->stride[0] / sizeof(*yChar),
                                pic->stride[0], pic->height, shift);
    }
    else
    {
        uint16_t *yShort;
        uint16_t mask = (1 << VCA_DEPTH) - 1;
        int shift = abs(pic->bitDepth - VCA_DEPTH);
        yShort = (uint16_t*)pic->planes[0];
        if (pic->bitDepth > VCA_DEPTH)
        {
            primitives.planecopy_sp(yShort, pic->stride[0] / sizeof(*yShort), src, pic->stride[0] / sizeof(*yShort),
                                    pic->stride[0], pic->height, shift, mask);
        }
        else /* Case for (pic.bitDepth < VCA_DEPTH) */
        {
            primitives.planecopy_sp_shl(yShort, pic->stride[0] / sizeof(*yShort), src, pic->stride[0] / sizeof(*yShort),
                                        pic->stride[0], pic->height, shift, mask);
        }
    }

    uint32_t maxBlockSize = m_param->maxCUSize;
    uint32_t numCuInWidth = (m_param->sourceWidth + maxBlockSize - 1) / maxBlockSize;
    uint32_t numCuInHeight = (m_param->sourceHeight + maxBlockSize - 1) / maxBlockSize;
    uint32_t numCuInFrame = numCuInWidth * numCuInHeight;
    uint32_t lumaMarginX = maxBlockSize + 32;
    int maxHeight = numCuInHeight * maxBlockSize;
    int maxWidth = numCuInWidth * maxBlockSize;
    int32_t frame_texture = 0;
    intptr_t yuvstride = (numCuInWidth * maxBlockSize) + (lumaMarginX << 1);
    const double *weightFactorMatrix = weights_dct32;
    switch (maxBlockSize)
    {
    case 32: weightFactorMatrix = weights_dct32;
        break;
    case 16: weightFactorMatrix = weights_dct16;
        break;
    case 8: weightFactorMatrix = weights_dct8;
        break;
    default: weightFactorMatrix = weights_dct32;
        break;
    }
    for (int blockX = 0, ctuIdx = 0; blockX < maxWidth; blockX += maxBlockSize)
    {
        for (int blockY = 0; blockY < maxHeight; blockY += maxBlockSize, ctuIdx++)
        {
            intptr_t blockOffsetLuma = blockX + (blockY * yuvstride);
            pixel * src1 = src + blockOffsetLuma;
            int16_t * srcCoeff = VCA_MALLOC(int16_t, maxBlockSize * maxBlockSize);
            for (uint32_t p = 0; p < maxBlockSize; p++)
            {
                for (uint32_t q = 0; q < maxBlockSize; q++)
                {
                    srcCoeff[p * maxBlockSize + q] = (int16_t)src1[p * yuvstride + q];
                }
            }
            int16_t * dctCoeff = VCA_MALLOC(int16_t, maxBlockSize * maxBlockSize);
            int32_t *weightedDCTCoeff = VCA_MALLOC(int32_t, maxBlockSize * maxBlockSize);
            if (maxBlockSize == 64)
            {
                int32_t ctuEnergy = 0;
                int16_t srcCoeff1[1024] = { 0 };
                /* First 32x32 block */
                primitives.cu[3].copy_ss(srcCoeff1, 32, srcCoeff, maxBlockSize);
                primitives.cu[3].dct(srcCoeff1, dctCoeff, 32);
                for (uint32_t p = 0; p < 32; p++)
                {
                    for (uint32_t q = 0; q < 32; q++)
                    {
                        weightedDCTCoeff[p * 32 + q] = (int32_t)(weightFactorMatrix[p * 32 + q] * abs(dctCoeff[p * 32 + q]));
                        ctuEnergy += weightedDCTCoeff[p * 32 + q];
                        frame_texture += weightedDCTCoeff[p * 32 + q];
                    }
                }
                /* Second 32x32 block */
                primitives.cu[3].copy_ss(srcCoeff1, 32, srcCoeff + 32, maxBlockSize);
                primitives.cu[3].dct(srcCoeff1, dctCoeff, 32);
                for (uint32_t p = 0; p < 32; p++)
                {
                    for (uint32_t q = 0; q < 32; q++)
                    {
                        weightedDCTCoeff[p * 32 + q] = (int32_t)(weightFactorMatrix[p * 32 + q] * abs(dctCoeff[p * 32 + q]));
                        ctuEnergy += weightedDCTCoeff[p * 32 + q];
                        frame_texture += weightedDCTCoeff[p * 32 + q];
                    }
                }
                /* Third 32x32 block */
                primitives.cu[3].copy_ss(srcCoeff1, 32, srcCoeff + (32 * maxBlockSize), maxBlockSize);
                primitives.cu[3].dct(srcCoeff1, dctCoeff, 32);
                for (uint32_t p = 0; p < 32; p++)
                {
                    for (uint32_t q = 0; q < 32; q++)
                    {
                        weightedDCTCoeff[p * 32 + q] = (int32_t)(weightFactorMatrix[p * 32 + q] * abs(dctCoeff[p * 32 + q]));
                        ctuEnergy += weightedDCTCoeff[p * 32 + q];
                        frame_texture += weightedDCTCoeff[p * 32 + q];
                    }
                }
                /* Fourth 32x32 block */
                primitives.cu[3].copy_ss(srcCoeff1, 32, srcCoeff + (32 * maxBlockSize) + 32, maxBlockSize);
                primitives.cu[3].dct(srcCoeff1, dctCoeff, 32);
                for (uint32_t p = 0; p < 32; p++)
                {
                    for (uint32_t q = 0; q < 32; q++)
                    {
                        weightedDCTCoeff[p * 32 + q] = (int32_t)(weightFactorMatrix[p * 32 + q] * abs(dctCoeff[p * 32 + q]));
                        ctuEnergy += weightedDCTCoeff[p * 32 + q];
                        frame_texture += weightedDCTCoeff[p * 32 + q];
                    }
                }
                /* Absolute texture per CTU */
                m_texture->m_ctuAbsoluteEnergy[ctuIdx] = ctuEnergy;
            }
            else
            {
                const uint32_t sizeIdx = m_param->maxLog2CUSize - 2;
                primitives.cu[sizeIdx].dct(srcCoeff, dctCoeff, maxBlockSize);
                int32_t weightedSum = 0;
                for (uint32_t p = 0; p < maxBlockSize; p++)
                {
                    for (uint32_t q = 0; q < maxBlockSize; q++)
                    {
                        weightedDCTCoeff[p * maxBlockSize + q] = (int32_t)(weightFactorMatrix[p * maxBlockSize + q] * abs(dctCoeff[p * maxBlockSize + q]));
                        weightedSum += weightedDCTCoeff[p * maxBlockSize + q];
                        frame_texture += weightedDCTCoeff[p * maxBlockSize + q];
                    }
                }
                /* Absolute texture per CTU */
                m_texture->m_ctuAbsoluteEnergy[ctuIdx] = (int32_t)weightedSum;
            }
            VCA_FREE(dctCoeff);
            VCA_FREE(weightedDCTCoeff);
            VCA_FREE(srcCoeff);
        }
    }
    /* Average texture per frame */
    m_texture->m_avgEnergy = (int32_t)(frame_texture / numCuInFrame);
    for (uint32_t p = 0; p < numCuInWidth; p++)
    {
        for (uint32_t q = 0; q < numCuInHeight; q++)
        {
            /* Relative texture per CTU */
            m_texture->m_ctuRelativeEnergy[p + q * numCuInWidth] = (double)m_texture->m_ctuAbsoluteEnergy[p + q * numCuInWidth] / m_texture->m_avgEnergy;
        }
    }
    return true;
}

void Encoder::compute_dct_texture_SAD(double *relNormalizedTextureSad, vca_picture *pic)
{
    uint32_t maxBlockSize = m_param->maxCUSize;
    uint32_t numCuInWidth = (m_param->sourceWidth + maxBlockSize - 1) / maxBlockSize;
    uint32_t numCuInHeight = (m_param->sourceHeight + maxBlockSize - 1) / maxBlockSize;
    uint32_t numCuInFrame = numCuInWidth * numCuInHeight;
    double textureSad = 0;

    double rel_normTextureSad = 0.0;
    /* compute sum of absolute differences of block-wise texture between the current and prev pictures. */
    if (pic->poc > 0)
    {
        for (uint32_t j = 0; j < numCuInFrame; j++)
            textureSad += abs(cur_texture->m_ctuAbsoluteEnergy[j] - prev_texture->m_ctuAbsoluteEnergy[j]);
    }

    textureSad = (textureSad / prev_texture->m_avgEnergy);
    if (prev_norm_textureSad == 0)
        rel_normTextureSad = 0.0;
    else
        rel_normTextureSad = abs(prev_norm_textureSad - textureSad) / prev_norm_textureSad;

    *relNormalizedTextureSad = rel_normTextureSad;

    pic->frameData.poc = pic->poc;
    pic->frameData.e_value = cur_texture->m_avgEnergy;
    if (pic->poc < 1)
        pic->frameData.h_value = 0;
    else
        pic->frameData.h_value = textureSad;

    if (pic->poc > 1)
        pic->frameData.epsilon = rel_normTextureSad;
    else
        pic->frameData.epsilon = 0;

    epsilons[pic->poc] = pic->frameData.epsilon;
    /* store block-wise energy of previous frames for reference */
    memcpy(prev_texture->m_ctuAbsoluteEnergy, cur_texture->m_ctuAbsoluteEnergy, sizeof(int32_t) * numCuInFrame);
    prev_texture->m_avgEnergy = cur_texture->m_avgEnergy;
    prev_norm_textureSad = textureSad;
}

void Encoder::create()
{
    if (!primitives.cu[BLOCK_4x4].copy_ss)
    {
        // this should be an impossible condition when using our public API, and indicates a serious bug.
        vca_log(m_param, VCA_LOG_ERROR, "Primitives must be initialized before encoder is created\n");
        abort();
    }

    uint32_t maxBlockSize = m_param->maxCUSize;
    uint32_t numCuInWidth = (m_param->sourceWidth + maxBlockSize - 1) / maxBlockSize;
    uint32_t numCuInHeight = (m_param->sourceHeight + maxBlockSize - 1) / maxBlockSize;
    uint32_t numCuInFrame = numCuInWidth * numCuInHeight;
    CHECKED_MALLOC(cur_texture, vca_frame_texture_t, 1);
    CHECKED_MALLOC(prev_texture, vca_frame_texture_t, 1);
    CHECKED_MALLOC(cur_texture->m_ctuAbsoluteEnergy, int32_t, numCuInFrame);
    CHECKED_MALLOC(cur_texture->m_ctuRelativeEnergy, double, numCuInFrame);
    CHECKED_MALLOC(prev_texture->m_ctuAbsoluteEnergy, int32_t, numCuInFrame);
    CHECKED_MALLOC(prev_texture->m_ctuRelativeEnergy, double, numCuInFrame);

    CHECKED_MALLOC(epsilons, double, m_param->totalFrames);
    CHECKED_MALLOC(isNewShot, uint8_t, m_param->totalFrames);
    CHECKED_MALLOC(isNotSureShot, int, m_param->totalFrames);
    CHECKED_MALLOC(prevShotDist, int, m_param->totalFrames);
    m_encodeStartTime = vca_mdate();
    return;
fail:
    destroy();
    m_aborted = true;
}

void Encoder::destroy()
{
    VCA_FREE(cur_texture->m_ctuAbsoluteEnergy);
    VCA_FREE(cur_texture->m_ctuRelativeEnergy);
    VCA_FREE(cur_texture);
    VCA_FREE(prev_texture->m_ctuAbsoluteEnergy);
    VCA_FREE(prev_texture->m_ctuRelativeEnergy);
    VCA_FREE(prev_texture);
    VCA_FREE(epsilons);
    VCA_FREE(isNewShot);
    VCA_FREE(isNotSureShot);
    VCA_FREE(prevShotDist);
    if (m_param)
    {
        if (m_param->csvfpt)
            fclose(m_param->csvfpt);
        free((char*)m_param->csvfn);
        param_free(m_param);
    }
}

/**
 * Feed one new input frame into the encoder, get one frame out. If pic_in is
 * NULL, a flush condition is implied and pic_in must be NULL for all subsequent
 * calls for this encoder instance.
 *
 * pic_in  input original YUV picture or NULL
 * pic_out pointer to reconstructed picture struct
 *
 * returns 0 if no frames are currently available for output
 *         1 if frame was output, m_nalList contains access unit
 *         negative on malloc error or abort */
int Encoder::encode(const vca_picture* pic_in)
{
#if CHECKED_BUILD || _DEBUG
    if (g_checkFailures)
    {
        vca_log(m_param, VCA_LOG_ERROR, "encoder aborting because of internal error\n");
        return -1;
    }
#endif
    if (m_aborted)
        return -1;

    if (pic_in)
    {
        if (pic_in->bitDepth < 8 || pic_in->bitDepth > 16)
        {
            vca_log(m_param, VCA_LOG_ERROR, "Input bit depth (%d) must be between 8 and 16\n",
                     pic_in->bitDepth);
            return -1;
        }
        vca_picture *pic = (vca_picture *)pic_in;
        if (compute_weighted_DCT_energy(pic, cur_texture))
        {
            double normTextureSad = 0.0;
            compute_dct_texture_SAD(&normTextureSad, pic);
            return 1;
        }
    }
    int ret = 0;
    return ret;
}


#if defined(_MSC_VER)
#pragma warning(disable: 4800) // forcing int to bool
#pragma warning(disable: 4127) // conditional expression is constant
#endif

void Encoder::configure(vca_param *p)
{
    this->m_param = p;
}

/* The dithering algorithm is based on Sierra-2-4A error diffusion.
 * We convert planes in place (without allocating a new buffer). */
static void ditherPlane(uint16_t *src, int srcStride, int width, int height, int16_t *errors, int bitDepth)
{
    const int lShift = 16 - bitDepth;
    const int rShift = 16 - bitDepth + 2;
    const int half = (1 << (16 - bitDepth + 1));
    const int pixelMax = (1 << bitDepth) - 1;

    memset(errors, 0, (width + 1) * sizeof(int16_t));

    if (bitDepth == 8)
    {
        for (int y = 0; y < height; y++, src += srcStride)
        {
            uint8_t* dst = (uint8_t *)src;
            int16_t err = 0;
            for (int x = 0; x < width; x++)
            {
                err = err * 2 + errors[x] + errors[x + 1];
                int tmpDst = vca_clip3(0, pixelMax, ((src[x] << 2) + err + half) >> rShift);
                errors[x] = err = (int16_t)(src[x] - (tmpDst << lShift));
                dst[x] = (uint8_t)tmpDst;
            }
        }
    }
    else
    {
        for (int y = 0; y < height; y++, src += srcStride)
        {
            int16_t err = 0;
            for (int x = 0; x < width; x++)
            {
                err = err * 2 + errors[x] + errors[x + 1];
                int tmpDst = vca_clip3(0, pixelMax, ((src[x] << 2) + err + half) >> rShift);
                errors[x] = err = (int16_t)(src[x] - (tmpDst << lShift));
                src[x] = (uint16_t)tmpDst;
            }
        }
    }
}

void dither_image(vca_picture* picIn, int picWidth, int picHeight, int16_t *errorBuf, int bitDepth)
{
    if (picIn->bitDepth <= 8)
    {
        fprintf(stderr, "extras [error]: dither support enabled only for input bitdepth > 8\n");
        return;
    }

    if (picIn->bitDepth == bitDepth)
    {
        fprintf(stderr, "extras[error]: dither support enabled only if encoder depth is different from picture depth\n");
        return;
    }

    /* This portion of code is from readFrame in x264. */
    for (int i = 0; i < vca_cli_csps[picIn->colorSpace].planes; i++)
    {
        if (picIn->bitDepth < 16)
        {
            /* upconvert non 16bit high depth planes to 16bit */
            uint16_t *plane = (uint16_t*)picIn->planes[i];
            uint32_t pixelCount = vca_picturePlaneSize(picIn->colorSpace, picWidth, picHeight, i);
            int lShift = 16 - picIn->bitDepth;

            /* This loop assumes width is equal to stride which
             * happens to be true for file reader outputs */
            for (uint32_t j = 0; j < pixelCount; j++)
                plane[j] = plane[j] << lShift;
        }

        int height = (int)(picHeight >> vca_cli_csps[picIn->colorSpace].height[i]);
        int width = (int)(picWidth >> vca_cli_csps[picIn->colorSpace].width[i]);

        ditherPlane(((uint16_t*)picIn->planes[i]), picIn->stride[i] / 2, width, height, errorBuf, bitDepth);
    }
}

static int vca_atobool(const char* str, bool& bError)
{
    if (!strcmp(str, "1") ||
        !strcmp(str, "true") ||
        !strcmp(str, "yes"))
        return 1;
    if (!strcmp(str, "0") ||
        !strcmp(str, "false") ||
        !strcmp(str, "no"))
        return 0;
    bError = true;
    return 0;
}

static inline int _confirm(vca_param* param, bool bflag, const char* message)
{
    if (!bflag)
        return 0;

    vca_log(param, VCA_LOG_ERROR, "%s\n", message);
    return 1;
}

int check_params(vca_param* param)
{
#define CHECK(expr, msg) check_failed |= _confirm(param, expr, msg)
    int check_failed = 0; /* abort if there is a fatal configuration problem */
    if (check_failed == 1)
        return check_failed;
    CHECK(param->fpsNum == 0 || param->fpsDenom == 0,
        "Frame rate numerator and denominator must be specified");
    CHECK(param->sourceWidth < (int)param->maxCUSize || param->sourceHeight < (int)param->maxCUSize,
        "Picture size must be at least one CTU");
    CHECK(param->internalCsp < VCA_CSP_I400 || VCA_CSP_I444 < param->internalCsp,
        "chroma subsampling must be i400 (4:0:0 monochrome), i420 (4:2:0 default), i422 (4:2:0), i444 (4:4:4)");
    CHECK(param->sourceWidth & !!CHROMA_H_SHIFT(param->internalCsp),
        "Picture width must be an integer multiple of the specified chroma subsampling");
    CHECK(param->sourceHeight & !!CHROMA_V_SHIFT(param->internalCsp),
        "Picture height must be an integer multiple of the specified chroma subsampling");
    CHECK(param->frameNumThreads < 0 || param->frameNumThreads > VCA_MAX_FRAME_THREADS,
        "frameNumThreads (--frame-threads) must be [0 .. VCA_MAX_FRAME_THREADS)");
    CHECK(param->bEnableShotdetect < 0 || param->bEnableShotdetect > 1,
        "bEnableShotdetect (--shot-detect) must be 0 or 1");
    return check_failed;
}

int vca_atoi(const char* str, bool& bError)
{
    char *end;
    int v = strtol(str, &end, 0);

    if (end == str || *end != '\0')
        bError = true;
    return v;
}

double vca_atof(const char* str, bool& bError)
{
    char *end;
    double v = strtod(str, &end);

    if (end == str || *end != '\0')
        bError = true;
    return v;
}

static const int fixedRatios[][2] =
{
    { 1,  1 },
    { 12, 11 },
    { 10, 11 },
    { 16, 11 },
    { 40, 33 },
    { 24, 11 },
    { 20, 11 },
    { 32, 11 },
    { 80, 33 },
    { 18, 11 },
    { 15, 11 },
    { 64, 33 },
    { 160, 99 },
    { 4, 3 },
    { 3, 2 },
    { 2, 1 },
};

void setParamAspectRatio(vca_param* p, int width, int height)
{
    p->vui.aspectRatioIdc = VCA_EXTENDED_SAR;
    p->vui.sarWidth = width;
    p->vui.sarHeight = height;
    for (size_t i = 0; i < sizeof(fixedRatios) / sizeof(fixedRatios[0]); i++)
    {
        if (width == fixedRatios[i][0] && height == fixedRatios[i][1])
        {
            p->vui.aspectRatioIdc = (int)i + 1;
            return;
        }
    }
}

void getParamAspectRatio(vca_param* p, int& width, int& height)
{
    if (!p->vui.aspectRatioIdc)
        width = height = 0;
    else if ((size_t)p->vui.aspectRatioIdc <= sizeof(fixedRatios) / sizeof(fixedRatios[0]))
    {
        width = fixedRatios[p->vui.aspectRatioIdc - 1][0];
        height = fixedRatios[p->vui.aspectRatioIdc - 1][1];
    }
    else if (p->vui.aspectRatioIdc == VCA_EXTENDED_SAR)
    {
        width = p->vui.sarWidth;
        height = p->vui.sarHeight;
    }
    else
        width = height = 0;
}

void print_params(vca_param* param)
{
    if (param->logLevel < VCA_LOG_INFO)
        return;

    vca_log(param, VCA_LOG_INFO, "DCT energy block size : %d x %d\n", param->maxCUSize, param->maxCUSize);
    vca_log(param, VCA_LOG_INFO, "Shot detection algorithm : %s\n", param->bEnableShotdetect ? "enabled":"disabled");
    if(param->bEnableShotdetect)
        vca_log(param, VCA_LOG_INFO, "Shot detection Max/Min threshold : %f/ %f\n", param->maxThresh, param->minThresh);
    vca_log(param, VCA_LOG_INFO, "CSV file : %s\n", param->csvfn);
    fflush(stderr);
}

vca_param *param_alloc()
{
    return (vca_param*)vca_malloc(sizeof(vca_param));
}

void param_free(vca_param* p)
{
    vca_free(p);
}

void param_default(vca_param* param)
{
    memset(param, 0, sizeof(vca_param));

    /* Applying default values to all elements in the param structure */
    param->cpuid = 0;
    param->bEnableWavefront = 1;
    param->bEnableShotdetect = 0;
    param->frameNumThreads = 0;
    param->logLevel = VCA_LOG_INFO;

    /* Source specifications */
    param->internalBitDepth = VCA_DEPTH;
    param->internalCsp = VCA_CSP_I420;
    param->sourceHeight = 0;
    param->sourceWidth = 0;
    param->fpsNum = 0;
    param->fpsDenom = 0;

    param->maxCUSize = 64;
    param->maxLog2CUSize = 6;
    param->bLowPassDct = 0;
    param->csvfn = NULL;
    param->csvfpt = NULL;

    param->maxThresh = 50;
    param->minThresh = 10;
}

static int parseName(const char* arg, const char* const* names, bool& bError)
{
    for (int i = 0; names[i]; i++)
        if (!strcmp(arg, names[i]))
            return i;

    return vca_atoi(arg, bError);
}
/* internal versions of string-to-int with additional error checking */
#undef atobool
#undef atoi
#undef atof
#define atoi(str) vca_atoi(str, bError)
#define atof(str) vca_atof(str, bError)
#define atobool(str) (vca_atobool(str, bError))

int param_parse(vca_param* p, const char* name, const char* value)
{
    bool bError = false;
    bool bNameWasBool = false;
    bool bValueWasNull = !value;
    bool bExtraParams = false;
    char nameBuf[64];

    if (!name)
        return VCA_PARAM_BAD_NAME;

    // skip -- prefix if provided
    if (name[0] == '-' && name[1] == '-')
        name += 2;

    // s/_/-/g
    if (strlen(name) + 1 < sizeof(nameBuf) && strchr(name, '_'))
    {
        char *c;
        strcpy(nameBuf, name);
        while ((c = strchr(nameBuf, '_')) != 0)
            *c = '-';

        name = nameBuf;
    }

    if (!strncmp(name, "no-", 3))
    {
        name += 3;
        value = !value || vca_atobool(value, bError) ? "false" : "true";
    }
    else if (!strncmp(name, "no", 2))
    {
        name += 2;
        value = !value || vca_atobool(value, bError) ? "false" : "true";
    }
    else if (!value)
        value = "true";
    else if (value[0] == '=')
        value++;

#if defined(_MSC_VER)
#pragma warning(disable: 4127) // conditional expression is constant
#endif
#define OPT(STR) else if (!strcmp(name, STR))
#define OPT2(STR1, STR2) else if (!strcmp(name, STR1) || !strcmp(name, STR2))
    if (0);
    OPT("asm")
    {
        if (bValueWasNull)
            p->cpuid = 0;
        else
            p->cpuid = 0;
    }
    OPT("fps")
    {
        if (sscanf(value, "%u/%u", &p->fpsNum, &p->fpsDenom) == 2)
            ;
        else
        {
            float fps = (float)atof(value);
            if (fps > 0 && fps <= INT_MAX / 1000)
            {
                p->fpsNum = (int)(fps * 1000 + .5);
                p->fpsDenom = 1000;
            }
            else
            {
                p->fpsNum = atoi(value);
                p->fpsDenom = 1;
            }
        }
    }
    OPT("frame-threads") p->frameNumThreads = atoi(value);
    OPT("total-frames") p->totalFrames = atoi(value);
    OPT("input-res") bError |= sscanf(value, "%dx%d", &p->sourceWidth, &p->sourceHeight) != 2;
    OPT("input-csp") p->internalCsp = parseName(value, vca_source_csp_names, bError);
    OPT("csv") p->csvfn = strdup(value);
    OPT("shot-detect") p->bEnableShotdetect = atoi(value);
    OPT("max-thresh") p->maxThresh = atof(value);
    OPT("min-thresh") p->minThresh = atof(value);
    else
        bExtraParams = true;

    // solve "fatal error C1061: compiler limit : blocks nested too deeply"
    if (bExtraParams)
    {
        if (0);
        else
            return VCA_PARAM_BAD_NAME;
    }
#undef OPT
#undef atobool
#undef atoi
#undef atof

    bError |= bValueWasNull && !bNameWasBool;
    return bError ? VCA_PARAM_BAD_VALUE : 0;
}


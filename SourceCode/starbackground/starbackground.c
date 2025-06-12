/* This file is an image processing operation for GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 *
 * Credit to Øyvind Kolås (pippin) for major GEGL contributions
 * 2023  - Star background - Beaver

If you paste this syntax inside Gimp's GEGL Graph filter you can test a static
preview of it without installing the plugin.

src  aux=[  color ]  crop 

mirrors
m-angle=120
r-angle=0
n-segs=11
c-x=0.5
c-y=0.5
o-x=-0.68999999999999995
o-y=0.9
trim-x=0.105
input-scale=11

color-overlay value=#00ff33

dst-over aux=[ color value=#e061ff  ]

end of syntax

 */

/*This is an embedded GEGL Graph that allows  render plugins to be modified by other filters and work on transparent backgrounds
src is GEGL's "replace" blend mode. So it is an instruction to bea*/

#define beginfix \
" id=1 src  aux=[  color value=#ffffff ]  crop aux=[ ref=1 ]  "\


#define endfix \
" id=1 crop aux=[ ref=1 ]  box-blur radius=1 "\



#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES


property_double (scale, _("Scale"), 6.0)
   description (_("Scale of the star background"))
   value_range (1.0, 20.0)
   ui_range    (1.0, 20.0)
   ui_gamma    (3.0)

property_int (coverage, _("Coverage"), 120)
   description (_("Inner/Outer radius ratio of the star background."))
   value_range (90, 180)
   ui_range    (90, 180)
   ui_gamma    (3.0)

property_double (rotation, _("Rotate Star Background"), 0.0)
    description (_("Rotate the star background"))
    value_range (0.0, 360.0)
    ui_meta     ("unit", "degree")
    ui_meta     ("direction", "cw")

property_int (branches, _("Amount of Star Branches"), 11)
    description (_("Amount of branches on the star/shape"))
    value_range (3, 24)

property_double (duration, _("Duration"), 0.68)
    description (_("Duration, as if the shapes were in motion."))
    value_range (0.0, 1.0)


property_double (x, _("Move Horizontal (0.5 resets)"), 0.5)
    description (_("Move the central star background horizontally. 0.5 is center."))
    ui_range (0.0, 1.0)
  ui_meta     ("unit", "relative-coordinate")
  ui_meta     ("axis", "x")

property_double (y, _("Move Vertical (0.5 resets)"), 0.5)
    description (_("Move the central star background vertically. 0.5 is center."))
    ui_range (0.0, 1.0)
  ui_meta     ("unit", "relative-coordinate")
  ui_meta     ("axis", "y")

property_color (color, _("Color Shape"), "#00ff33")

property_color (color2, _("Color Background"), "#e061ff")

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     starbackground
#define GEGL_OP_C_SOURCE starbackground.c

#include "gegl-op.h"

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglNode *input, *graph1, *kali, *crop, *color, *color2, *graph2, *behind, *output;

  input    = gegl_node_get_input_proxy (gegl, "input");
  output   = gegl_node_get_output_proxy (gegl, "output");

/*This is Gimp's Kaleidoscope filter and certain settings are being embedded.*/
  kali = gegl_node_new_child (gegl,
                                  "operation", "gegl:mirrors", "trim-x", 0.105, "o-y", 0.9, 
                                  NULL);

/*Graph1 and Graph2 are GEGL Graph strings being listed and called. To find the syntax strings look for beginfix and endfix*/
  graph1 = gegl_node_new_child (gegl,
                                  "operation", "gegl:gegl", "string", beginfix,
                                  NULL);

  graph2 = gegl_node_new_child (gegl,
                                  "operation", "gegl:gegl", "string", endfix,
                                  NULL);

/*This (dst-over) is GEGL's behind blend mode*/
  behind = gegl_node_new_child (gegl,
                                  "operation", "gegl:dst-over",
                                  NULL);

  color = gegl_node_new_child (gegl,
                                  "operation", "gegl:color-overlay",
                                  NULL);

  color2 = gegl_node_new_child (gegl,
                                  "operation", "gegl:color",
                                  NULL);

  crop = gegl_node_new_child (gegl,
                                  "operation", "gegl:crop",
                                  NULL);


/*This is a simple gegl graph. It list all filters and instructs color fill to be fused
with the behind blend mode.*/
  gegl_node_link_many (input, graph1, kali, color, behind, graph2, crop, output, NULL);
  gegl_node_connect (behind, "aux", color2, "output");
  gegl_node_connect (crop, "aux", input, "output");

 gegl_operation_meta_redirect (operation, "scale", kali, "input-scale"); 
 gegl_operation_meta_redirect (operation, "duration", kali, "o-x"); 
 gegl_operation_meta_redirect (operation, "x", kali, "c-x"); 
 gegl_operation_meta_redirect (operation, "y", kali, "c-y"); 
 gegl_operation_meta_redirect (operation, "coverage", kali, "m-angle"); 
 gegl_operation_meta_redirect (operation, "rotation", kali, "r-angle"); 
 gegl_operation_meta_redirect (operation, "branches", kali, "n-segs"); 
 gegl_operation_meta_redirect (operation, "color", color, "value");
 gegl_operation_meta_redirect (operation, "color2", color2, "value");


}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;

  gegl_operation_class_set_keys (operation_class,
    "name",        "lb:starbackground",
    "title",       _("Star Background"),
    "reference-hash", "2459fgj03m0301b2ac",
    "description", _("Render a star background"),
    "gimp:menu-path", "<Image>/Filters/Render/Fun",
    "gimp:menu-label", _("Star Background..."),
    NULL);
}

#endif

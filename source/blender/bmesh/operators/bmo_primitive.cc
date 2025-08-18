/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup bmesh
 *
 * Primitive shapes.
 */

#include "MEM_guardedalloc.h"

#include "BLI_math_base_safe.h"
#include "BLI_math_matrix.h"
#include "BLI_math_rotation.h"
#include "BLI_math_vector.h"

#include "BKE_customdata.hh"

#include "bmesh.hh"
#include "intern/bmesh_operators_private.hh"

/* ************************ primitives ******************* */

static const float icovert[12][3] = {
    {0.0f, 0.0f, -200.0f},
    {144.72f, -105.144f, -89.443f},
    {-55.277f, -170.128, -89.443f},
    {-178.885f, 0.0f, -89.443f},
    {-55.277f, 170.128f, -89.443f},
    {144.72f, 105.144f, -89.443f},
    {55.277f, -170.128f, 89.443f},
    {-144.72f, -105.144f, 89.443f},
    {-144.72f, 105.144f, 89.443f},
    {55.277f, 170.128f, 89.443f},
    {178.885f, 0.0f, 89.443f},
    {0.0f, 0.0f, 200.0f},
};

static const short icoface[20][3] = {
    {0, 1, 2},  {1, 0, 5},   {0, 2, 3},  {0, 3, 4},  {0, 4, 5},  {1, 5, 10},  {2, 1, 6},
    {3, 2, 7},  {4, 3, 8},   {5, 4, 9},  {1, 10, 6}, {2, 6, 7},  {3, 7, 8},   {4, 8, 9},
    {5, 9, 10}, {6, 10, 11}, {7, 6, 11}, {8, 7, 11}, {9, 8, 11}, {10, 9, 11},
};

static const float icouvs[60][2] = {
    {0.181819f, 0.000000f}, {0.272728f, 0.157461f}, {0.090910f, 0.157461f}, {0.272728f, 0.157461f},
    {0.363637f, 0.000000f}, {0.454546f, 0.157461f}, {0.909091f, 0.000000f}, {1.000000f, 0.157461f},
    {0.818182f, 0.157461f}, {0.727273f, 0.000000f}, {0.818182f, 0.157461f}, {0.636364f, 0.157461f},
    {0.545455f, 0.000000f}, {0.636364f, 0.157461f}, {0.454546f, 0.157461f}, {0.272728f, 0.157461f},
    {0.454546f, 0.157461f}, {0.363637f, 0.314921f}, {0.090910f, 0.157461f}, {0.272728f, 0.157461f},
    {0.181819f, 0.314921f}, {0.818182f, 0.157461f}, {1.000000f, 0.157461f}, {0.909091f, 0.314921f},
    {0.636364f, 0.157461f}, {0.818182f, 0.157461f}, {0.727273f, 0.314921f}, {0.454546f, 0.157461f},
    {0.636364f, 0.157461f}, {0.545455f, 0.314921f}, {0.272728f, 0.157461f}, {0.363637f, 0.314921f},
    {0.181819f, 0.314921f}, {0.090910f, 0.157461f}, {0.181819f, 0.314921f}, {0.000000f, 0.314921f},
    {0.818182f, 0.157461f}, {0.909091f, 0.314921f}, {0.727273f, 0.314921f}, {0.636364f, 0.157461f},
    {0.727273f, 0.314921f}, {0.545455f, 0.314921f}, {0.454546f, 0.157461f}, {0.545455f, 0.314921f},
    {0.363637f, 0.314921f}, {0.181819f, 0.314921f}, {0.363637f, 0.314921f}, {0.272728f, 0.472382f},
    {0.000000f, 0.314921f}, {0.181819f, 0.314921f}, {0.090910f, 0.472382f}, {0.727273f, 0.314921f},
    {0.909091f, 0.314921f}, {0.818182f, 0.472382f}, {0.545455f, 0.314921f}, {0.727273f, 0.314921f},
    {0.636364f, 0.472382f}, {0.363637f, 0.314921f}, {0.545455f, 0.314921f}, {0.454546f, 0.472382f},
};

#define VERT_MARK 1

#define EDGE_ORIG 1
#define EDGE_MARK 2

#define FACE_MARK 1
#define FACE_NEW 2

void bmo_create_grid_exec(BMesh *bm, BMOperator *op)
{
  BMOpSlot *slot_verts_out = BMO_slot_get(op->slots_out, "verts.out");

  const float dia = BMO_slot_float_get(op->slots_in, "size");
  const uint xtot = max_ii(1, BMO_slot_int_get(op->slots_in, "x_segments"));
  const uint ytot = max_ii(1, BMO_slot_int_get(op->slots_in, "y_segments"));
  const float xtot_inv2 = 2.0f / (xtot);
  const float ytot_inv2 = 2.0f / (ytot);

  const int cd_loop_uv_offset = CustomData_get_offset(&bm->ldata, CD_PROP_FLOAT2);
  const bool calc_uvs = (cd_loop_uv_offset != -1) && BMO_slot_bool_get(op->slots_in, "calc_uvs");

  BMVert **varr;
  BMVert *vquad[4];

  float mat[4][4];
  float vec[3], tvec[3];

  uint x, y, i;

  BMO_slot_mat4_get(op->slots_in, "matrix", mat);

  BMO_slot_buffer_alloc(op, op->slots_out, "verts.out", (xtot + 1) * (ytot + 1));
  varr = (BMVert **)slot_verts_out->data.buf;

  i = 0;
  vec[2] = 0.0f;
  for (y = 0; y <= ytot; y++) {
    vec[1] = ((y * ytot_inv2) - 1.0f) * dia;
    for (x = 0; x <= xtot; x++) {
      vec[0] = ((x * xtot_inv2) - 1.0f) * dia;
      mul_v3_m4v3(tvec, mat, vec);
      varr[i] = BM_vert_create(bm, tvec, nullptr, BM_CREATE_NOP);
      BMO_vert_flag_enable(bm, varr[i], VERT_MARK);
      i++;
    }
  }

#define XY(_x, _y) ((_x) + ((_y) * (xtot + 1)))

  for (y = 1; y <= ytot; y++) {
    for (x = 1; x <= xtot; x++) {
      BMFace *f;

      vquad[0] = varr[XY(x - 1, y - 1)];
      vquad[1] = varr[XY(x, y - 1)];
      vquad[2] = varr[XY(x, y)];
      vquad[3] = varr[XY(x - 1, y)];

      f = BM_face_create_verts(bm, vquad, 4, nullptr, BM_CREATE_NOP, true);
      if (calc_uvs) {
        BMO_face_flag_enable(bm, f, FACE_MARK);
      }
    }
  }

#undef XY

  if (calc_uvs) {
    BM_mesh_calc_uvs_grid(bm, xtot, ytot, FACE_MARK, cd_loop_uv_offset);
  }
}

void BM_mesh_calc_uvs_grid(BMesh *bm,
                           const uint x_segments,
                           const uint y_segments,
                           const short oflag,
                           const int cd_loop_uv_offset)
{
  BMFace *f;
  BMLoop *l;
  BMIter iter, liter;

  const float dx = 1.0f / float(x_segments);
  const float dy = 1.0f / float(y_segments);
  const float dx_wrap = 1.0 - (dx / 2.0f);
  float x = 0.0f;
  float y = dy;

  int loop_index;

  BLI_assert(cd_loop_uv_offset != -1);

  BM_ITER_MESH (f, &iter, bm, BM_FACES_OF_MESH) {
    if (!BMO_face_flag_test(bm, f, oflag)) {
      continue;
    }

    BM_ITER_ELEM_INDEX (l, &liter, f, BM_LOOPS_OF_FACE, loop_index) {
      float *luv = BM_ELEM_CD_GET_FLOAT_P(l, cd_loop_uv_offset);

      switch (loop_index) {
        case 0:
          y -= dy;
          break;
        case 1:
          x += dx;
          break;
        case 2:
          y += dy;
          break;
        case 3:
          x -= dx;
          break;
        default:
          break;
      }

      luv[0] = x;
      luv[1] = y;
    }

    x += dx;
    if (x >= dx_wrap) {
      x = 0.0f;
      y += dy;
    }
  }
}

void bmo_create_uvsphere_exec(BMesh *bm, BMOperator *op)
{
  const float rad = BMO_slot_float_get(op->slots_in, "radius");
  const int seg = BMO_slot_int_get(op->slots_in, "u_segments");
  const int tot = BMO_slot_int_get(op->slots_in, "v_segments");

  const int cd_loop_uv_offset = CustomData_get_offset(&bm->ldata, CD_PROP_FLOAT2);
  const bool calc_uvs = (cd_loop_uv_offset != -1) && BMO_slot_bool_get(op->slots_in, "calc_uvs");

  BMOperator bmop, prevop;
  BMVert *eve, *preveve;
  BMEdge *e;
  BMIter iter;
  const float axis[3] = {0, 0, 1};
  float vec[3], mat[4][4], cmat[3][3];
  int a;

  BMO_slot_mat4_get(op->slots_in, "matrix", mat);

  const float phid = float(M_PI) / tot;
  // const float phi = 0.25f * float(M_PI); /* UNUSED. */

  /* one segment first */
  for (a = 0; a <= tot; a++) {
    /* Going in this direction, then edge extruding, makes normals face outward */
    float sin_phi, cos_phi;
    sin_cos_from_fraction(a, 2 * tot, &sin_phi, &cos_phi);

    vec[0] = 0.0f;
    vec[1] = rad * sin_phi;
    vec[2] = rad * cos_phi;
    eve = BM_vert_create(bm, vec, nullptr, BM_CREATE_NOP);
    BMO_vert_flag_enable(bm, eve, VERT_MARK);

    if (a != 0) {
      e = BM_edge_create(bm, preveve, eve, nullptr, BM_CREATE_NOP);
      BMO_edge_flag_enable(bm, e, EDGE_ORIG);
    }

    preveve = eve;
  }

  /* extrude and rotate; negative phi to make normals face outward */
  axis_angle_to_mat3(cmat, axis, -(M_PI * 2) / seg);

  for (a = 0; a < seg; a++) {
    if (a) {
      BMO_op_initf(bm, &bmop, op->flag, "extrude_edge_only edges=%S", &prevop, "geom.out");
      BMO_op_exec(bm, &bmop);
      BMO_op_finish(bm, &prevop);
    }
    else {
      BMO_op_initf(bm, &bmop, op->flag, "extrude_edge_only edges=%fe", EDGE_ORIG);
      BMO_op_exec(bm, &bmop);
    }

    BMO_slot_buffer_flag_enable(bm, bmop.slots_out, "geom.out", BM_VERT, VERT_MARK);
    BMO_op_callf(bm, op->flag, "rotate cent=%v matrix=%m3 verts=%S", vec, cmat, &bmop, "geom.out");

    prevop = bmop;
  }

  if (a) {
    BMO_op_finish(bm, &bmop);
  }

  {
    float len, len2, vec2[3];

    len = 2 * rad * sinf(phid / 2.0f);

    /* Length of one segment in shortest parallel. */
    vec[0] = rad * sinf(phid);
    vec[1] = 0.0f;
    vec[2] = rad * cosf(phid);

    mul_v3_m3v3(vec2, cmat, vec);
    len2 = len_v3v3(vec, vec2);

    /* use shortest segment length divided by 3 as merge threshold */
    BMO_op_callf(
        bm, op->flag, "remove_doubles verts=%fv dist=%f", VERT_MARK, min_ff(len, len2) / 3.0f);
  }

  if (calc_uvs) {
    BMFace *f;
    BMLoop *l;
    BMIter fiter, liter;

    /* We cannot tag faces for UVs computing above,
     * so we have to do it now, based on all its vertices being tagged. */
    BM_ITER_MESH (f, &fiter, bm, BM_FACES_OF_MESH) {
      bool valid = true;

      BM_ITER_ELEM (l, &liter, f, BM_LOOPS_OF_FACE) {
        if (!BMO_vert_flag_test(bm, l->v, VERT_MARK)) {
          valid = false;
          break;
        }
      }

      if (valid) {
        BMO_face_flag_enable(bm, f, FACE_MARK);
      }
    }

    BM_mesh_calc_uvs_sphere(bm, FACE_MARK, cd_loop_uv_offset);
  }

  /* Now apply the inverse matrix. */
  BM_ITER_MESH (eve, &iter, bm, BM_VERTS_OF_MESH) {
    if (BMO_vert_flag_test(bm, eve, VERT_MARK)) {
      mul_m4_v3(mat, eve->co);
    }
  }

  BMO_slot_buffer_from_enabled_flag(bm, op, op->slots_out, "verts.out", BM_VERT, VERT_MARK);
}

void bmo_create_icosphere_exec(BMesh *bm, BMOperator *op)
{
  const float rad = BMO_slot_float_get(op->slots_in, "radius");
  const float rad_div = rad / 200.0f;
  const int subdiv = BMO_slot_int_get(op->slots_in, "subdivisions");

  const int cd_loop_uv_offset = CustomData_get_offset(&bm->ldata, CD_PROP_FLOAT2);
  const bool calc_uvs = (cd_loop_uv_offset != -1) && BMO_slot_bool_get(op->slots_in, "calc_uvs");

  BMVert *eva[12];
  BMVert *v;
  BMIter liter;
  BMIter viter;
  BMLoop *l;
  float vec[3], mat[4][4] /* , phi, phid */;
  int a;

  BMO_slot_mat4_get(op->slots_in, "matrix", mat);

  // phid = 2.0f * float(M_PI) / subdiv; /* UNUSED. */
  // phi = 0.25f * float(M_PI);          /* UNUSED. */

  for (a = 0; a < 12; a++) {
    vec[0] = rad_div * icovert[a][0];
    vec[1] = rad_div * icovert[a][1];
    vec[2] = rad_div * icovert[a][2];
    eva[a] = BM_vert_create(bm, vec, nullptr, BM_CREATE_NOP);

    BMO_vert_flag_enable(bm, eva[a], VERT_MARK);
  }

  int uvi = 0;
  for (a = 0; a < 20; a++) {
    BMFace *f;
    BMVert *v1, *v2, *v3;

    v1 = eva[icoface[a][0]];
    v2 = eva[icoface[a][1]];
    v3 = eva[icoface[a][2]];

    f = BM_face_create_quad_tri(bm, v1, v2, v3, nullptr, nullptr, BM_CREATE_NOP);

    BM_ITER_ELEM (l, &liter, f, BM_LOOPS_OF_FACE) {
      BMO_edge_flag_enable(bm, l->e, EDGE_MARK);
    }

    /* Set the UVs here, the iteration order of the faces is not guaranteed,
     * so it's best to set the UVs right after the face is created. */
    if (calc_uvs) {
      int loop_index;
      BM_ITER_ELEM_INDEX (l, &liter, f, BM_LOOPS_OF_FACE, loop_index) {
        float *luv = BM_ELEM_CD_GET_FLOAT_P(l, cd_loop_uv_offset);
        luv[0] = icouvs[uvi][0];
        luv[1] = icouvs[uvi][1];
        uvi++;
      }
    }
  }

  if (subdiv > 1) {
    BMOperator bmop;

    BMO_op_initf(bm,
                 &bmop,
                 op->flag,
                 "subdivide_edges edges=%fe "
                 "smooth=%f "
                 "cuts=%i "
                 "use_grid_fill=%b use_sphere=%b",
                 EDGE_MARK,
                 rad,
                 (1 << (subdiv - 1)) - 1,
                 true,
                 true);

    BMO_op_exec(bm, &bmop);
    BMO_slot_buffer_flag_enable(bm, bmop.slots_out, "geom.out", BM_VERT, VERT_MARK);
    BMO_slot_buffer_flag_enable(bm, bmop.slots_out, "geom.out", BM_EDGE, EDGE_MARK);
    BMO_op_finish(bm, &bmop);
  }

  /* must transform after because of sphere subdivision */
  BM_ITER_MESH (v, &viter, bm, BM_VERTS_OF_MESH) {
    if (BMO_vert_flag_test(bm, v, VERT_MARK)) {
      mul_m4_v3(mat, v->co);
    }
  }

  BMO_slot_buffer_from_enabled_flag(bm, op, op->slots_out, "verts.out", BM_VERT, VERT_MARK);
}

static void bm_mesh_calc_uvs_sphere_face(BMFace *f, const int cd_loop_uv_offset)
{
  float *uvs[4];
  BMLoop *l;
  BMIter iter;
  float dx;
  int loop_index, loop_index_max_x;

  BLI_assert(f->len <= 4);

  /* If face has 3 vertices, it's a polar face, in which case we need to
   * compute a nearby to determine its latitude. */
  float avgx = 0.0f, avgy = 0.0f;
  BM_ITER_ELEM_INDEX (l, &iter, f, BM_LOOPS_OF_FACE, loop_index) {
    if (f->len == 3) {
      avgx += l->v->co[0];
      avgy += l->v->co[1];
    }
  }
  avgx /= 3.0f;
  avgy /= 3.0f;

  BM_ITER_ELEM_INDEX (l, &iter, f, BM_LOOPS_OF_FACE, loop_index) {
    float *luv = BM_ELEM_CD_GET_FLOAT_P(l, cd_loop_uv_offset);
    float x = l->v->co[0];
    float y = l->v->co[1];
    float z = l->v->co[2];
    float len = len_v3(l->v->co);

    /* Use neighboring point to compute angle for poles. */
    float theta;
    if (f->len == 3 && fabsf(x) < 0.0001f && fabsf(y) < 0.0001f) {
      theta = atan2f(avgy, avgx);
    }
    else {
      theta = atan2f(y, x);
    }

    /* Shift borderline coordinates to the left. */
    if (fabsf(theta - float(M_PI)) < 0.0001f) {
      theta = -M_PI;
    }

    float phi = safe_acosf(z / len);
    luv[0] = 0.5f + theta / (float(M_PI) * 2);
    luv[1] = 1.0f - phi / float(M_PI);

    uvs[loop_index] = luv;
  }

  /* Fix awkwardly-wrapping UVs */
  loop_index_max_x = 0;
  for (loop_index = 1; loop_index < f->len; loop_index++) {
    if (uvs[loop_index][0] > uvs[loop_index_max_x][0]) {
      loop_index_max_x = loop_index;
    }
  }

  for (loop_index = 0; loop_index < f->len; loop_index++) {
    if (loop_index != loop_index_max_x) {
      dx = uvs[loop_index_max_x][0] - uvs[loop_index][0];
      if (dx > 0.5f) {
        uvs[loop_index][0] += 1.0f;
      }
    }
  }
}

void BM_mesh_calc_uvs_sphere(BMesh *bm, const short oflag, const int cd_loop_uv_offset)
{
  BMFace *f;
  BMIter iter;

  BLI_assert(cd_loop_uv_offset != -1); /* caller is responsible for giving us UVs */

  BM_ITER_MESH (f, &iter, bm, BM_FACES_OF_MESH) {
    if (!BMO_face_flag_test(bm, f, oflag)) {
      continue;
    }

    bm_mesh_calc_uvs_sphere_face(f, cd_loop_uv_offset);
  }

  BMIter iter2;
  BMLoop *l;
  int loop_index;
  float minx = 1.0f;

  BM_ITER_MESH (f, &iter, bm, BM_FACES_OF_MESH) {
    if (!BMO_face_flag_test(bm, f, oflag)) {
      continue;
    }
    BM_ITER_ELEM_INDEX (l, &iter2, f, BM_LOOPS_OF_FACE, loop_index) {
      float *luv = BM_ELEM_CD_GET_FLOAT_P(l, cd_loop_uv_offset);
      if (luv[0] < minx) {
        minx = luv[0];
      }
    }
  }

  BM_ITER_MESH (f, &iter, bm, BM_FACES_OF_MESH) {
    if (!BMO_face_flag_test(bm, f, oflag)) {
      continue;
    }
    BM_ITER_ELEM_INDEX (l, &iter2, f, BM_LOOPS_OF_FACE, loop_index) {
      float *luv = BM_ELEM_CD_GET_FLOAT_P(l, cd_loop_uv_offset);
      luv[0] -= minx;
    }
  }
}

void bmo_create_circle_exec(BMesh *bm, BMOperator *op)
{
  const float radius = BMO_slot_float_get(op->slots_in, "radius");
  const int segs = BMO_slot_int_get(op->slots_in, "segments");
  const bool cap_ends = BMO_slot_bool_get(op->slots_in, "cap_ends");
  const bool cap_tris = BMO_slot_bool_get(op->slots_in, "cap_tris");

  const int cd_loop_uv_offset = CustomData_get_offset(&bm->ldata, CD_PROP_FLOAT2);
  const bool calc_uvs = (cd_loop_uv_offset != -1) && BMO_slot_bool_get(op->slots_in, "calc_uvs");

  BMVert *v1, *lastv1 = nullptr, *cent1, *firstv1 = nullptr;
  float vec[3], mat[4][4];
  int a;

  if (!segs) {
    return;
  }

  BMO_slot_mat4_get(op->slots_in, "matrix", mat);

  if (cap_ends) {
    zero_v3(vec);
    mul_m4_v3(mat, vec);

    cent1 = BM_vert_create(bm, vec, nullptr, BM_CREATE_NOP);
    BMO_vert_flag_enable(bm, cent1, VERT_MARK);
  }

  for (a = 0; a < segs; a++) {
    /* Going this way ends up with normal(s) upward */
    sin_cos_from_fraction(a, segs, &vec[0], &vec[1]);
    vec[0] *= -radius;
    vec[1] *= radius;
    vec[2] = 0.0f;
    mul_m4_v3(mat, vec);
    v1 = BM_vert_create(bm, vec, nullptr, BM_CREATE_NOP);

    BMO_vert_flag_enable(bm, v1, VERT_MARK);

    if (lastv1) {
      BM_edge_create(bm, v1, lastv1, nullptr, BM_CREATE_NOP);
    }

    if (a && cap_ends) {
      BMFace *f;

      f = BM_face_create_quad_tri(bm, cent1, lastv1, v1, nullptr, nullptr, BM_CREATE_NOP);
      BMO_face_flag_enable(bm, f, FACE_NEW);
    }

    if (!firstv1) {
      firstv1 = v1;
    }

    lastv1 = v1;
  }

  if (!a) {
    return;
  }

  BM_edge_create(bm, firstv1, lastv1, nullptr, eBMCreateFlag(0));

  if (cap_ends) {
    BMFace *f;

    f = BM_face_create_quad_tri(bm, cent1, v1, firstv1, nullptr, nullptr, BM_CREATE_NOP);
    BMO_face_flag_enable(bm, f, FACE_NEW);

    if (calc_uvs) {
      BM_mesh_calc_uvs_circle(bm, mat, radius, FACE_NEW, cd_loop_uv_offset);
    }
  }

  if (!cap_tris) {
    BMO_op_callf(bm, op->flag, "dissolve_faces faces=%ff", FACE_NEW);
  }

  BMO_slot_buffer_from_enabled_flag(bm, op, op->slots_out, "verts.out", BM_VERT, VERT_MARK);
}

void BM_mesh_calc_uvs_circle(
    BMesh *bm, float mat[4][4], const float radius, const short oflag, const int cd_loop_uv_offset)
{
  BMFace *f;
  BMLoop *l;
  BMIter fiter, liter;

  const float uv_scale = 0.5f / radius;
  const float uv_center = 0.5f;

  float inv_mat[4][4];

  BLI_assert(cd_loop_uv_offset != -1); /* caller must ensure we have UVs already */

  invert_m4_m4(inv_mat, mat);

  BM_ITER_MESH (f, &fiter, bm, BM_FACES_OF_MESH) {
    if (!BMO_face_flag_test(bm, f, oflag)) {
      continue;
    }

    BM_ITER_ELEM (l, &liter, f, BM_LOOPS_OF_FACE) {
      float *luv = BM_ELEM_CD_GET_FLOAT_P(l, cd_loop_uv_offset);

      float uv_vco[3];
      copy_v3_v3(uv_vco, l->v->co);
      /* transform back into the unit circle flat on the Z-axis */
      mul_m4_v3(inv_mat, uv_vco);

      /* then just take those coords for UVs */
      luv[0] = uv_center + uv_scale * uv_vco[0];
      luv[1] = uv_center + uv_scale * uv_vco[1];
    }
  }
}

void bmo_create_cone_exec(BMesh *bm, BMOperator *op)
{
  BMVert *v1, *v2, *lastv1 = nullptr, *lastv2 = nullptr, *cent1, *cent2, *firstv1, *firstv2;
  BMFace *f;
  float vec[3], mat[4][4];
  const float rad1 = BMO_slot_float_get(op->slots_in, "radius1");
  const float rad2 = BMO_slot_float_get(op->slots_in, "radius2");
  const float depth_half = 0.5f * BMO_slot_float_get(op->slots_in, "depth");
  int segs = BMO_slot_int_get(op->slots_in, "segments");
  const bool cap_ends = BMO_slot_bool_get(op->slots_in, "cap_ends");
  const bool cap_tris = BMO_slot_bool_get(op->slots_in, "cap_tris");

  const int cd_loop_uv_offset = CustomData_get_offset(&bm->ldata, CD_PROP_FLOAT2);
  const bool calc_uvs = (cd_loop_uv_offset != -1) && BMO_slot_bool_get(op->slots_in, "calc_uvs");

  if (!segs) {
    return;
  }

  BMO_slot_mat4_get(op->slots_in, "matrix", mat);

  if (cap_ends) {
    vec[0] = vec[1] = 0.0f;
    vec[2] = -depth_half;
    mul_m4_v3(mat, vec);

    cent1 = BM_vert_create(bm, vec, nullptr, BM_CREATE_NOP);

    vec[0] = vec[1] = 0.0f;
    vec[2] = depth_half;
    mul_m4_v3(mat, vec);

    cent2 = BM_vert_create(bm, vec, nullptr, BM_CREATE_NOP);

    BMO_vert_flag_enable(bm, cent1, VERT_MARK);
    BMO_vert_flag_enable(bm, cent2, VERT_MARK);
  }

  const int side_faces_len = segs - 1;
  BMFace **side_faces = static_cast<BMFace **>(
      MEM_mallocN(sizeof(*side_faces) * side_faces_len, __func__));

  for (int i = 0; i < segs; i++) {
    /* Calculate with higher precision, see: #87779. */
    float sin_phi, cos_phi;
    sin_cos_from_fraction(i, segs, &sin_phi, &cos_phi);

    vec[0] = rad1 * sin_phi;
    vec[1] = rad1 * cos_phi;
    vec[2] = -depth_half;
    mul_m4_v3(mat, vec);
    v1 = BM_vert_create(bm, vec, nullptr, BM_CREATE_NOP);

    vec[0] = rad2 * sin_phi;
    vec[1] = rad2 * cos_phi;
    vec[2] = depth_half;
    mul_m4_v3(mat, vec);
    v2 = BM_vert_create(bm, vec, nullptr, BM_CREATE_NOP);

    BMO_vert_flag_enable(bm, v1, VERT_MARK);
    BMO_vert_flag_enable(bm, v2, VERT_MARK);

    if (i) {
      if (cap_ends) {
        f = BM_face_create_quad_tri(bm, cent1, lastv1, v1, nullptr, nullptr, BM_CREATE_NOP);
        if (calc_uvs) {
          BMO_face_flag_enable(bm, f, FACE_MARK);
        }
        BMO_face_flag_enable(bm, f, FACE_NEW);

        f = BM_face_create_quad_tri(bm, cent2, v2, lastv2, nullptr, nullptr, BM_CREATE_NOP);
        if (calc_uvs) {
          BMO_face_flag_enable(bm, f, FACE_MARK);
        }
        BMO_face_flag_enable(bm, f, FACE_NEW);
      }

      f = BM_face_create_quad_tri(bm, lastv1, lastv2, v2, v1, nullptr, BM_CREATE_NOP);
      if (calc_uvs) {
        BMO_face_flag_enable(bm, f, FACE_MARK);
      }
      side_faces[i - 1] = f;
    }
    else {
      firstv1 = v1;
      firstv2 = v2;
    }

    lastv1 = v1;
    lastv2 = v2;
  }

  if (cap_ends) {
    f = BM_face_create_quad_tri(bm, cent1, v1, firstv1, nullptr, nullptr, BM_CREATE_NOP);
    if (calc_uvs) {
      BMO_face_flag_enable(bm, f, FACE_MARK);
    }
    BMO_face_flag_enable(bm, f, FACE_NEW);

    f = BM_face_create_quad_tri(bm, cent2, firstv2, v2, nullptr, nullptr, BM_CREATE_NOP);
    if (calc_uvs) {
      BMO_face_flag_enable(bm, f, FACE_MARK);
    }
    BMO_face_flag_enable(bm, f, FACE_NEW);
  }

  f = BM_face_create_quad_tri(bm, v1, v2, firstv2, firstv1, nullptr, BM_CREATE_NOP);
  if (calc_uvs) {
    BMO_face_flag_enable(bm, f, FACE_MARK);
  }

  if (calc_uvs) {
    BM_mesh_calc_uvs_cone(bm, mat, rad2, rad1, segs, cap_ends, FACE_MARK, cd_loop_uv_offset);
  }

  /* Collapse vertices at the first end. */
  if (rad1 == 0.0f) {
    if (cap_ends) {
      BM_vert_kill(bm, cent1);
    }
    for (int i = 0; i < side_faces_len; i++) {
      f = side_faces[i];
      BMLoop *l = BM_FACE_FIRST_LOOP(f);
      BM_edge_collapse(bm, l->prev->e, l->prev->v, true, true);
    }
  }

  /* Collapse vertices at the second end. */
  if (rad2 == 0.0f) {
    if (cap_ends) {
      BM_vert_kill(bm, cent2);
    }
    for (int i = 0; i < side_faces_len; i++) {
      f = side_faces[i];
      BMLoop *l = BM_FACE_FIRST_LOOP(f);
      BM_edge_collapse(bm, l->next->e, l->next->v, true, true);
    }
  }

  if (!cap_tris) {
    BMO_op_callf(bm, op->flag, "dissolve_faces faces=%ff", FACE_NEW);
  }

  if (side_faces != nullptr) {
    MEM_freeN(side_faces);
  }

  BMO_slot_buffer_from_enabled_flag(bm, op, op->slots_out, "verts.out", BM_VERT, VERT_MARK);
}

void BM_mesh_calc_uvs_cone(BMesh *bm,
                           float mat[4][4],
                           const float radius_top,
                           const float radius_bottom,
                           const int segments,
                           const bool cap_ends,
                           const short oflag,
                           const int cd_loop_uv_offset)
{
  BMFace *f;
  BMLoop *l;
  BMIter fiter, liter;

  const float uv_width = 1.0f / float(segments);
  const float uv_height = cap_ends ? 0.5f : 1.0f;

  /* Note that all this allows us to handle all cases
   * (real cone, truncated cone, with or without ends capped)
   * with a single common code. */
  const float uv_center_y = cap_ends ? 0.25f : 0.5f;
  const float uv_center_x_top = cap_ends ? 0.25f : 0.5f;
  const float uv_center_x_bottom = cap_ends ? 0.75f : 0.5f;
  const float uv_radius = cap_ends ? 0.24f : 0.5f;

  /* Using the opposite's end uv_scale as fallback allows us to handle 'real cone' case. */
  const float uv_scale_top = (radius_top != 0.0f) ?
                                 (uv_radius / radius_top) :
                                 ((radius_bottom != 0.0f) ? (uv_radius / radius_bottom) :
                                                            uv_radius);
  const float uv_scale_bottom = (radius_bottom != 0.0f) ? (uv_radius / radius_bottom) :
                                                          uv_scale_top;

  float local_up[3] = {0.0f, 0.0f, 1.0f};

  float x, y;
  float inv_mat[4][4];
  int loop_index;

  /* Transform the up-vector like we did the cone itself, without location. */
  mul_mat3_m4_v3(mat, local_up);
  /* Remove global scaling. */
  normalize_v3(local_up);

  invert_m4_m4(inv_mat, mat);

  BLI_assert(cd_loop_uv_offset != -1); /* caller is responsible for ensuring the mesh has UVs */

  x = 1.0f;
  y = 1.0f - uv_height;

  BM_ITER_MESH (f, &fiter, bm, BM_FACES_OF_MESH) {
    if (!BMO_face_flag_test(bm, f, oflag)) {
      continue;
    }

    if (f->len == 4 && radius_top && radius_bottom) {
      /* side face - so unwrap it in a rectangle */
      BM_ITER_ELEM_INDEX (l, &liter, f, BM_LOOPS_OF_FACE, loop_index) {
        float *luv = BM_ELEM_CD_GET_FLOAT_P(l, cd_loop_uv_offset);

        switch (loop_index) {
          case 0:
            /* Continue in the last position */
            break;
          case 1:
            y += uv_height;
            break;
          case 2:
            x -= uv_width;
            break;
          case 3:
            y -= uv_height;
            break;
          default:
            break;
        }

        luv[0] = x;
        luv[1] = y;
      }
    }
    else {
      /* Top or bottom face - so unwrap it by transforming
       * back to a circle and using the X/Y coords. */
      BM_face_normal_update(f);

      BM_ITER_ELEM (l, &liter, f, BM_LOOPS_OF_FACE) {
        float *luv = BM_ELEM_CD_GET_FLOAT_P(l, cd_loop_uv_offset);
        float uv_vco[3];

        mul_v3_m4v3(uv_vco, inv_mat, l->v->co);

        if (dot_v3v3(f->no, local_up) > 0.0f) { /* if this is a top face of the cone */
          luv[0] = uv_center_x_top + uv_vco[0] * uv_scale_top;
          luv[1] = uv_center_y + uv_vco[1] * uv_scale_top;
        }
        else {
          luv[0] = uv_center_x_bottom + uv_vco[0] * uv_scale_bottom;
          luv[1] = uv_center_y + uv_vco[1] * uv_scale_bottom;
        }
      }
    }
  }
}

void bmo_create_cube_exec(BMesh *bm, BMOperator *op)
{
  BMVert *verts[8];
  float mat[4][4];
  float off = BMO_slot_float_get(op->slots_in, "size") / 2.0f;

  const int cd_loop_uv_offset = CustomData_get_offset(&bm->ldata, CD_PROP_FLOAT2);
  const bool calc_uvs = (cd_loop_uv_offset != -1) && BMO_slot_bool_get(op->slots_in, "calc_uvs");

  /* rotation order set to match 'BM_mesh_calc_uvs_cube' */
  const char faces[6][4] = {
      {0, 1, 3, 2},
      {2, 3, 7, 6},
      {6, 7, 5, 4},
      {4, 5, 1, 0},
      {2, 6, 4, 0},
      {7, 3, 1, 5},
  };

  BMO_slot_mat4_get(op->slots_in, "matrix", mat);

  if (!off) {
    off = 0.5f;
  }
  int i = 0;

  for (int x = -1; x < 2; x += 2) {
    for (int y = -1; y < 2; y += 2) {
      for (int z = -1; z < 2; z += 2) {
        float vec[3] = {float(x) * off, float(y) * off, float(z) * off};
        mul_m4_v3(mat, vec);
        verts[i] = BM_vert_create(bm, vec, nullptr, BM_CREATE_NOP);
        BMO_vert_flag_enable(bm, verts[i], VERT_MARK);
        i++;
      }
    }
  }

  for (i = 0; i < ARRAY_SIZE(faces); i++) {
    BMFace *f;
    BMVert *quad[4] = {
        verts[faces[i][0]],
        verts[faces[i][1]],
        verts[faces[i][2]],
        verts[faces[i][3]],
    };

    f = BM_face_create_verts(bm, quad, 4, nullptr, BM_CREATE_NOP, true);
    if (calc_uvs) {
      BMO_face_flag_enable(bm, f, FACE_MARK);
    }
  }

  if (calc_uvs) {
    BM_mesh_calc_uvs_cube(bm, FACE_MARK);
  }

  BMO_slot_buffer_from_enabled_flag(bm, op, op->slots_out, "verts.out", BM_VERT, VERT_MARK);
}

void BM_mesh_calc_uvs_cube(BMesh *bm, const short oflag)
{
  BMFace *f;
  BMLoop *l;
  BMIter fiter, liter;
  const float width = 0.25f;

  const int cd_loop_uv_offset = CustomData_get_offset(&bm->ldata, CD_PROP_FLOAT2);

  float x = 0.375f;
  float y = 0.0f;

  int loop_index;

  BLI_assert(cd_loop_uv_offset != -1); /* the caller can ensure that we have UVs */

  BM_ITER_MESH (f, &fiter, bm, BM_FACES_OF_MESH) {
    if (!BMO_face_flag_test(bm, f, oflag)) {
      continue;
    }

    BM_ITER_ELEM_INDEX (l, &liter, f, BM_LOOPS_OF_FACE, loop_index) {
      float *luv = BM_ELEM_CD_GET_FLOAT_P(l, cd_loop_uv_offset);

      luv[0] = x;
      luv[1] = y;

      switch (loop_index) {
        case 0:
          x += width;
          break;
        case 1:
          y += width;
          break;
        case 2:
          x -= width;
          break;
        case 3:
          y -= width;
          break;
        default:
          break;
      }
    }

    if (y >= 0.75f && x > 0.125f) {
      x = 0.125f;
      y = 0.5f;
    }
    else if (x <= 0.125f) {
      x = 0.625f;
      y = 0.5f;
    }
    else {
      y += 0.25f;
    }
  }
}

# SPDX-FileCopyrightText: 2025 Goo Engine Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

# Goo Engine configuration - Full build WITHOUT Cycles
# This is specifically for NPR/anime-style rendering with EEVEE Legacy only
#
# Example usage:
#   cmake -C../blender/build_files/cmake/config/goo_engine.cmake  ../blender
#

set(WITH_ALEMBIC             ON  CACHE BOOL "" FORCE)
set(WITH_AUDASPACE           ON  CACHE BOOL "" FORCE)
set(WITH_BUILDINFO           ON  CACHE BOOL "" FORCE)
set(WITH_BULLET              ON  CACHE BOOL "" FORCE)
set(WITH_CODEC_AVI           ON  CACHE BOOL "" FORCE)
set(WITH_CODEC_FFMPEG        ON  CACHE BOOL "" FORCE)
set(WITH_CODEC_SNDFILE       ON  CACHE BOOL "" FORCE)
set(WITH_COMPOSITOR_CPU      ON  CACHE BOOL "" FORCE)
set(WITH_CYCLES              OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_EMBREE       OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_OSL          OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_PATH_GUIDING OFF CACHE BOOL "" FORCE)
set(WITH_DRACO               ON  CACHE BOOL "" FORCE)
set(WITH_FFTW3               ON  CACHE BOOL "" FORCE)
set(WITH_FREESTYLE           ON  CACHE BOOL "" FORCE)
set(WITH_GMP                 ON  CACHE BOOL "" FORCE)
set(WITH_HARU                ON  CACHE BOOL "" FORCE)
set(WITH_IK_ITASC            ON  CACHE BOOL "" FORCE)
set(WITH_IK_SOLVER           ON  CACHE BOOL "" FORCE)
set(WITH_IMAGE_CINEON        ON  CACHE BOOL "" FORCE)
set(WITH_IMAGE_OPENEXR       ON  CACHE BOOL "" FORCE)
set(WITH_IMAGE_OPENJPEG      ON  CACHE BOOL "" FORCE)
set(WITH_IMAGE_WEBP          ON  CACHE BOOL "" FORCE)
set(WITH_INPUT_NDOF          ON  CACHE BOOL "" FORCE)
set(WITH_INPUT_IME           ON  CACHE BOOL "" FORCE)
set(WITH_INTERNATIONAL       ON  CACHE BOOL "" FORCE)
set(WITH_LIBMV               ON  CACHE BOOL "" FORCE)
set(WITH_LIBMV_SCHUR_SPECIALIZATIONS ON CACHE BOOL "" FORCE)
set(WITH_LZMA                ON  CACHE BOOL "" FORCE)
set(WITH_LZO                 ON  CACHE BOOL "" FORCE)
set(WITH_MOD_FLUID           ON  CACHE BOOL "" FORCE)
set(WITH_MOD_OCEANSIM        ON  CACHE BOOL "" FORCE)
set(WITH_MOD_REMESH          ON  CACHE BOOL "" FORCE)
set(WITH_NANOVDB             ON  CACHE BOOL "" FORCE)
set(WITH_OPENAL              ON  CACHE BOOL "" FORCE)
set(WITH_OPENCOLLADA         ON  CACHE BOOL "" FORCE)
set(WITH_OPENCOLORIO         ON  CACHE BOOL "" FORCE)
set(WITH_OPENIMAGEDENOISE    ON  CACHE BOOL "" FORCE)
set(WITH_OPENMP              ON  CACHE BOOL "" FORCE)
set(WITH_OPENSUBDIV          ON  CACHE BOOL "" FORCE)
set(WITH_OPENVDB             ON  CACHE BOOL "" FORCE)
set(WITH_PYTHON_INSTALL      ON  CACHE BOOL "" FORCE)
set(WITH_PYTHON_INSTALL_NUMPY ON CACHE BOOL "" FORCE)
set(WITH_PYTHON_INSTALL_REQUESTS ON CACHE BOOL "" FORCE)
set(WITH_SDL                 ON  CACHE BOOL "" FORCE)
set(WITH_TBB                 ON  CACHE BOOL "" FORCE)
set(WITH_USD                 ON  CACHE BOOL "" FORCE)

# Windows-specific settings - disable Linux/Unix features
set(WITH_X11_XINPUT          OFF CACHE BOOL "" FORCE)
set(WITH_GHOST_X11           OFF CACHE BOOL "" FORCE)
set(WITH_GHOST_WAYLAND       OFF CACHE BOOL "" FORCE)

# Goo Engine specific - prioritize EEVEE Legacy
set(WITH_EEVEE_NEXT          OFF CACHE BOOL "" FORCE)

# Image formats
set(WITH_IMAGE_HDR           ON  CACHE BOOL "" FORCE)
set(WITH_IMAGE_TIFF          ON  CACHE BOOL "" FORCE)

# I/O Formats for NPR workflows
set(WITH_IO_PLY              ON  CACHE BOOL "" FORCE)
set(WITH_IO_STL              ON  CACHE BOOL "" FORCE)
set(WITH_IO_WAVEFRONT_OBJ    ON  CACHE BOOL "" FORCE)
set(WITH_IO_GPENCIL          ON  CACHE BOOL "" FORCE)

# Keep thumbnails for asset management
set(WITH_BLENDER_THUMBNAILER ON  CACHE BOOL "" FORCE)

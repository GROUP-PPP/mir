/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by:
 *   Cemil Azizoglu <cemil.azizoglu@canonical.com>
 */

#ifndef MIR_TOOLKIT_MIR_RENDER_SURFACE_H_
#define MIR_TOOLKIT_MIR_RENDER_SURFACE_H_

#include <mir_toolkit/client_types_nbs.h>

#ifdef __cplusplus
/**
 * \addtogroup mir_toolkit
 * @{
 */
extern "C" {
#endif

/** Create a render surface.
 *
 * \warning: This call is always synchronous as there is no actual rendering
 *           construct backing the surface yet. In case of error, the call
 *           will succeed but an error render surface will be returned.
 *
 * \param [in] connection       A valid connection
 *
 * \return                      The newly created render surface
 */
MirRenderSurface* mir_connection_create_render_surface(
    MirConnection* connection);

/**
 * Test for a valid render surface.
 *
 * \param [in] render_surface  The render surface
 *
 * \return                     True if the supplied render_surface is valid,
 *                             or false otherwise
 */
bool mir_render_surface_is_valid(
    MirRenderSurface* render_surface);

/**
 * Release the specified render surface.
 *
 * \warning: This will not release the content. It's an error to release
 *           the render surface without releasing the content first.
 *
 * \param [in] render_surface    The render surface to be released
 */
void mir_render_surface_release(
    MirRenderSurface* render_surface);

/**
 * Create a new buffer stream, backing the given render surface, asynchronously.
 *
 * \param [in] render_surface    The render surface
 * \param [in] width             Requested width
 * \param [in] height            Requested height
 * \param [in] format            Requested pixel format
 * \param [in] usage             Requested buffer usage
 * \param [in] callback          Callback function to be invoked when request
 *                               completes
 * \param [in] context           User data passed to the callback function
 *
 * \return                       A handle that can be supplied to mir_wait_for
 *                               to get notified when the request is complete
 */
MirWaitHandle* mir_render_surface_create_buffer_stream(
        MirRenderSurface* render_surface,
        int width, int height,
        MirPixelFormat format,
        MirBufferUsage usage,
        mir_buffer_stream_callback callback,
        void* context);

/**
 * Create a new buffer stream, backing the given render surface, synchronously.
 *
 * \param [in] render_surface    The render surface
 * \param [in] width             Requested width
 * \param [in] height            Requested height
 * \param [in] format            Requested pixel format
 * \param [in] usage             Requested buffer usage
 *
 * \return                       The newly created buffer stream
 */
MirBufferStream* mir_render_surface_create_buffer_stream_sync(
    MirRenderSurface* render_surface,
    int width, int height,
    MirPixelFormat format,
    MirBufferUsage usage);

/**
 * Set the MirSurfaceSpec to display content contained in a render surface
 *
 * \warning: The initial call to mir_surface_spec_add_render_surface will set
 *           the bottom-most content, and subsequent calls will stack the
 *           content on top.
 * \warning: It's an error to call this function with a render surface
 *           that holds no content.
 *
 * \param spec             The surface_spec to be updated
 * \param render_surface   The render surface containing the content to be displayed
 * \param scaled_width     The width that the content will be displayed at
 *                         (Ignored for buffer streams)
 * \param scaled_height    The height that the content will be displayed at
 *                         (Ignored for buffer streams)
 * \param displacement_x   The x displacement from the top-left corner of the MirSurface
 * \param displacement_y   The y displacement from the top-left corner of the MirSurface
 */
void mir_surface_spec_add_render_surface(
    MirSurfaceSpec* spec,
    MirRenderSurface* render_surface,
    int scaled_width, int scaled_height,
    int displacement_x, int displacement_y);

#ifdef __cplusplus
}
/**@}*/
#endif

#endif // MIR_TOOLKIT_MIR_RENDER_SURFACE_H_

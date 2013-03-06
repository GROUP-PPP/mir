/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Robert Carr <robert.carr@canonical.com>
 */

#ifndef MIR_TEST_CLIENT_GBM_MOCK_MESA_EGL_CLIENT_LIBRARY_H_
#define MIR_TEST_CLIENT_GBM_MOCK_MESA_EGL_CLIENT_LIBRARY_H_

#include "mir_toolkit/mir_client_library.h"

#include <gmock/gmock.h>

namespace mir
{
namespace test
{
namespace doubles
{

// A mock implementation of the C client library functions required by mesa-egl-platform-mir
class MockMesaEGLClientLibrary
{
public:
    MockMesaEGLClientLibrary();
    ~MockMesaEGLClientLibrary();

    MOCK_METHOD2(connection_get_platform, void(MirConnection*, MirPlatformPackage*));
    MOCK_METHOD2(surface_get_current_buffer, void(MirSurface*, MirBufferPackage*));
    MOCK_METHOD2(surface_get_parameters, void(MirSurface*, MirSurfaceParameters*));
    MOCK_METHOD3(surface_next_buffer, MirWaitHandle* (MirSurface*, mir_surface_lifecycle_callback, void*));
    MOCK_METHOD1(wait_for, void(MirWaitHandle*));
    
    MirConnection* a_connection_pointer();
    MirSurface* a_surface_pointer();
private:
    MirConnection* connection;
    MirSurface* surface;
};

}
}
} // namespace mir

namespace mir_toolkit
{
extern "C"
{
void mir_wait_for(MirWaitHandle* wait_handle);
void mir_connection_get_platform(MirConnection* connection, MirPlatformPackage* package);
void mir_surface_get_current_buffer(MirSurface* surface, MirBufferPackage* package);
void mir_surface_get_parameters(MirSurface* surface, MirSurfaceParameters* parameters);
MirWaitHandle* mir_surface_next_buffer(MirSurface* surface, mir_surface_lifecycle_callback callback, void* context);
}
}

#endif // MIR_TEST_CLIENT_GBM_MOCK_MESA_EGL_CLIENT_LIBRARY_H_

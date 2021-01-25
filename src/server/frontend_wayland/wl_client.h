/*
 * Copyright © 2021 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: William Wold <william.wold@canonical.com>
 */

#ifndef MIR_FRONTEND_WL_CLIENT_H_
#define MIR_FRONTEND_WL_CLIENT_H_

struct wl_client;
struct wl_listener;
struct wl_display;

#include <memory>
#include <functional>

namespace mir
{
namespace shell
{
class Shell;
}
namespace scene
{
class Session;
}

namespace frontend
{
class SessionAuthorizer;

class WlClient
{
public:
    /// Initializes a ConstructionCtx that will create a WlClient for each wl_client created on the display. Should only
    /// be called once per display. Destruction of the ConstructionCtx is handled automatically.
    static void setup_new_client_handler(
        wl_display* display,
        std::shared_ptr<shell::Shell> const& shell,
        std::shared_ptr<SessionAuthorizer> const& session_authorizer,
        std::function<void(WlClient&)>&& client_created_callback);

    static auto from(wl_client* client) -> WlClient*;

    WlClient(wl_client* raw_client, std::shared_ptr<scene::Session> const& client_session, shell::Shell* shell);
    ~WlClient();

    /// The underlying Wayland client
    wl_client* const raw_client;

    /// The Mir session associated with this client. Be careful when using this that it's actually the session you want.
    /// All clients have a session but the surfaces they create may get associated with additional sessions.
    ///
    /// For example all surfaces from a single XWayland server are attached to a single WlClient with a single cleint
    /// session, but their scene::Surfaces are associated with multiple sessions created in the XWayland frontend for
    /// individual apps.
    std::shared_ptr<scene::Session> const client_session;

private:
    /// This shell is owned by the ClientSessionConstructor, which outlives all clients.
    shell::Shell* const shell;
};
}
}

#endif // MIR_FRONTEND_WL_CLIENT_H_

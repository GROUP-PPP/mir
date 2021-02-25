/*
 * Copyright © 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 2 or 3,
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
 * Authored by: Cemil Azizoglu <cemil.azizoglu@canonical.com>
 */

#define MIR_LOG_COMPONENT "x11-error"
#include "mir/log.h"

#include "X11_resources.h"

namespace mg=mir::graphics;
namespace mx = mir::X;

//Force synchronous Xlib operation - for debugging
//#define FORCE_SYNCHRONOUS

mx::X11Resources mx::X11Resources::instance;

int mx::mir_x11_error_handler(Display* dpy, XErrorEvent* eev)
{
    char msg[80];
    XGetErrorText(dpy, eev->error_code, msg, sizeof(msg));
    log_error("X11 error %d (%s): request %d.%d\n",
        eev->error_code, msg, eev->request_code, eev->minor_code);
    return 0;
}

auto mx::X11Resources::get_conn() -> std::shared_ptr<::Display>
{
    std::lock_guard<std::mutex> lock{mutex};

    if (auto conn = connection.lock())
        return conn;

    XInitThreads();

    XSetErrorHandler(mir_x11_error_handler);

    std::shared_ptr<::Display> new_conn{
        XOpenDisplay(nullptr),
        [](::Display* display) { if (display) XCloseDisplay(display); }};

#ifdef FORCE_SYNCHRONOUS
    if (new_conn)
        XSynchronize(new_conn.get(), True);
#endif
    connection = new_conn;
    return new_conn;
}

void mx::X11Resources::set_set_output_for_window(Window win, VirtualOutput* output)
{
    std::lock_guard<std::mutex> lock{mutex};
    outputs[win] = output;
}

void mx::X11Resources::clear_output_for_window(Window win)
{
    std::lock_guard<std::mutex> lock{mutex};
    outputs.erase(win);
}

void mx::X11Resources::with_output_for_window(
    Window win,
    std::function<void(std::optional<VirtualOutput const*> output)> fn)
{
    std::lock_guard<std::mutex> lock{mutex};
    auto const iter = outputs.find(win);
    if (iter != outputs.end())
    {
        return fn(iter->second);
    }
    else
    {
        return fn(std::nullopt);
    }
}

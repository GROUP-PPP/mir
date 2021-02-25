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

#include "mir/graphics/display_report.h"
#include "mir/options/option.h"
#include "mir/options/configuration.h"
#include "platform.h"
#include "../X11_resources.h"
#include "mir/module_deleter.h"
#include "mir/assert_module_entry_point.h"
#include "mir/libname.h"
#include "mir/log.h"
#include "mir/graphics/egl_error.h"
#include "mir/graphics/egl_extensions.h"
#include "mir/graphics/egl_logger.h"

#include <boost/throw_exception.hpp>

namespace mo = mir::options;
namespace mg = mir::graphics;
namespace mx = mir::X;
namespace mgx = mg::X;
namespace geom = mir::geometry;

namespace
{
char const* x11_displays_option_name{"x11-output"};
}

mir::UniqueModulePtr<mg::Platform> create_host_platform(
    std::shared_ptr<mo::Option> const& options,
    std::shared_ptr<mir::EmergencyCleanupRegistry> const&,
    std::shared_ptr<mir::ConsoleServices> const&,
    std::shared_ptr<mg::DisplayReport> const& report,
    std::shared_ptr<mir::logging::Logger> const& logger)
{
    mir::assert_entry_point_signature<mg::CreateHostPlatform>(&create_host_platform);
    auto const conn = mx::X11Resources::instance.get_conn();
    if (!conn)
        BOOST_THROW_EXCEPTION(std::runtime_error("Need valid x11 output"));

    if (options->is_set(mir::options::debug_opt))
    {
        mg::initialise_egl_logger(logger);
    }

    auto output_sizes = mgx::Platform::parse_output_sizes(options->get<std::string>(x11_displays_option_name));

    return mir::make_module_ptr<mgx::Platform>(
        conn,
        move(output_sizes),
        report
    );
}

void add_graphics_platform_options(boost::program_options::options_description& config)
{
    mir::assert_entry_point_signature<mg::AddPlatformOptions>(&add_graphics_platform_options);
    config.add_options()
        (x11_displays_option_name,
         boost::program_options::value<std::string>()->default_value("1280x1024"),
         "[mir-on-X specific] Colon separated list of WIDTHxHEIGHT sizes for \"output\" windows."
         " ^SCALE may also be appended to any output");
}

mg::PlatformPriority probe_graphics_platform(
    std::shared_ptr<mir::ConsoleServices> const&,
        mo::ProgramOption const& /*options*/)
{
    mir::assert_entry_point_signature<mg::PlatformProbe>(&probe_graphics_platform);
    auto dpy = XOpenDisplay(nullptr);
    if (dpy)
    {
        XCloseDisplay(dpy);
        return mg::PlatformPriority::hosted;
    }
    return mg::PlatformPriority::unsupported;
}

namespace
{
mir::ModuleProperties const description = {
    "mir:x11",
    MIR_VERSION_MAJOR,
    MIR_VERSION_MINOR,
    MIR_VERSION_MICRO,
    mir::libname()
};
}

mir::ModuleProperties const* describe_graphics_module()
{
    mir::assert_entry_point_signature<mg::DescribeModule>(&describe_graphics_module);
    return &description;
}


/*
 * Copyright © 2013 Canonical Ltd.
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
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "gbm_cursor.h"
#include "gbm_platform.h"
#include "kms_output.h"
#include "kms_output_container.h"

#include <boost/exception/errinfo_errno.hpp>

#include <stdexcept>

namespace
{
int const width = 64;
int const height = 64;
uint32_t const color = 0x1c00001f;
}

namespace mgg = mir::graphics::gbm;

mgg::GBMCursor::GBMCursor(
    std::shared_ptr<GBMPlatform> const& platform,
    KMSOutputContainer const& output_container) :
        platform(platform),
        output_container(output_container),
        buffer(gbm_bo_create(
            platform->gbm.device,
            width,
            height,
            GBM_FORMAT_ARGB8888,
            GBM_BO_USE_CURSOR_64X64 | GBM_BO_USE_WRITE))
{
    if (!buffer) BOOST_THROW_EXCEPTION(std::runtime_error("failed to create gbm buffer"));

    std::vector<uint32_t> image(height*width, color);
    set_image(image.data(), geometry::Size{geometry::Width(width), geometry::Height(height)});

    output_container.for_each_output(
        [&](KMSOutput& output) { output.set_cursor(buffer); });
}

mgg::GBMCursor::~GBMCursor() noexcept
{
    gbm_bo_destroy(buffer);
}

void mgg::GBMCursor::set_image(const void* raw_argb, geometry::Size size)
{
    if (size != geometry::Size{geometry::Width(width), geometry::Height(height)})
        BOOST_THROW_EXCEPTION(std::logic_error("No support for cursors that aren't 64x64"));

    if (auto result = gbm_bo_write(
        buffer,
        raw_argb,
        size.width.as_uint32_t()*size.height.as_uint32_t()*sizeof(uint32_t)))
        BOOST_THROW_EXCEPTION(
            ::boost::enable_error_info(std::runtime_error("failed to initialize gbm buffer"))
            << (boost::error_info<GBMCursor, decltype(result)>(result)));
}

void mgg::GBMCursor::move_to(geometry::Point position)
{
    auto const x = position.x.as_uint32_t();
    auto const y = position.y.as_uint32_t();

    output_container.for_each_output([&](KMSOutput& output) { output.move_cursor(x, y); });
}

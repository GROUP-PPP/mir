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
 * Authored by: Daniel van Vugt <daniel.van.vugt@canonical.com>
 */

#include "mir/graphics/renderable.h"
#include "mir/graphics/display_buffer.h"
#include "bypass.h"

using namespace mir;
namespace mc=mir::compositor;
namespace mg=mir::graphics;

mc::BypassMatch::BypassMatch(geometry::Rectangle const& rect)
    : view_area(rect),
      bypass_is_feasible(true)
{
}

bool mc::BypassMatch::operator()(std::shared_ptr<graphics::Renderable> const& renderable)
{
    if (!bypass_is_feasible || !renderable->visible())
        return false;

    auto is_opaque = !((renderable->alpha() != 1.0f) || renderable->shaped());
    auto fits = (renderable->screen_position() == view_area);
    auto is_orthogonal = (renderable->transformation() == identity);
    return bypass_is_feasible = (is_opaque && fits && is_orthogonal);
}

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
 * Authored by: Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 */

#include <algorithm>
#include <boost/throw_exception.hpp>
#include <stdexcept>

#include "client_buffer_tracker.h"
#include "mir/graphics/buffer_id.h"

namespace mf = mir::frontend;
namespace mg = mir::graphics;

mf::ClientBufferTracker::ClientBufferTracker(unsigned int client_cache_size)
    : ids(),
      cache_size{client_cache_size}
{
}

void mf::ClientBufferTracker::add(mg::BufferID const& id)
{
    auto existing_id = std::find(ids.begin(), ids.end(), id);

    if (existing_id != ids.end())
    {
        ids.push_front(*existing_id);
        ids.erase(existing_id);
    }
    else
    {
        ids.push_front(id);
    }
    if (ids.size() > cache_size)
        ids.pop_back();
}

bool mf::ClientBufferTracker::client_has(mg::BufferID const& id) const
{
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

mf::SessionSurfaceTracker::SessionSurfaceTracker(size_t client_cache_size) :
    client_cache_size{client_cache_size}
{
}

void mf::SessionSurfaceTracker::add_buffer_to_surface(SurfaceId surface_id, mg::Buffer* buffer)
{
    auto& tracker = client_buffer_tracker[surface_id];
    if (!tracker) tracker = std::make_shared<ClientBufferTracker>(client_cache_size);
    tracker->add(buffer->id());

    client_buffer_resource[surface_id] = buffer; 
}

void mf::SessionSurfaceTracker::remove_surface(SurfaceId surface_id)
{
    auto it = client_buffer_tracker.find(surface_id);
    if (it != client_buffer_tracker.end())
        client_buffer_tracker.erase(it);

    auto last_buffer_it = client_buffer_resource.find(surface_id);
    if (last_buffer_it != client_buffer_resource.end())
        client_buffer_resource.erase(last_buffer_it);
}

mg::Buffer* mf::SessionSurfaceTracker::last_buffer(SurfaceId surface_id) const
{
    auto it = client_buffer_resource.find(surface_id);
    if (it != client_buffer_resource.end())
        return it->second;
    else
        BOOST_THROW_EXCEPTION(std::runtime_error("SurfaceId has no last buffer"));
}

bool mf::SessionSurfaceTracker::surface_has_buffer(SurfaceId surface_id, mg::Buffer* buffer) const
{
    auto it = client_buffer_tracker.find(surface_id);
    if (it == client_buffer_tracker.end())
        return false;

    return it->second->client_has(buffer->id());
}


/*
 * Copyright © 2012 Canonical Ltd.
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

#include "mir/compositor/graphic_buffer_allocator.h"
#include "mir/graphics/gbm/gbm_buffer_allocator.h"

#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace mg = mir::graphics;
namespace mgg = mir::graphics::gbm;
namespace mc = mir::compositor;
namespace geom = mir::geometry;

#include "mock_gbm_device.cpp"

class GBMBufferAllocatorTest  : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        mocker = std::shared_ptr<MockGBMDevice> (new MockGBMDevice);
        allocator = std::shared_ptr<mgg::GBMBufferAllocator> (new mgg::GBMBufferAllocator(mocker->get_device()));

        w = geom::Width(300);
        h = geom::Height(200);
        pf = mc::PixelFormat::rgba_8888;
    }

    // Defaults
    geom::Width w;
    geom::Height h;
    mc::PixelFormat pf;

    std::shared_ptr<MockGBMDevice> mocker;
    std::shared_ptr<mgg::GBMBufferAllocator> allocator;
};

TEST_F(GBMBufferAllocatorTest, allocator_returns_non_null_buffer)
{
    using namespace testing;
    EXPECT_CALL(*mocker, bo_create(_,_,_,_,_));
    EXPECT_CALL(*mocker, bo_destroy(_));

    EXPECT_TRUE(allocator->alloc_buffer(w, h, pf).get() != NULL);
}

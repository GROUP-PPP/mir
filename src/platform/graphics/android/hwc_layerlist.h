/*
 * Copyright © 2013 Canonical Ltd.
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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#ifndef MIR_GRAPHICS_ANDROID_HWC_LAYERLIST_H_
#define MIR_GRAPHICS_ANDROID_HWC_LAYERLIST_H_

#include "mir/graphics/renderable.h"
#include "mir/graphics/android/fence.h"
#include "mir/geometry/rectangle.h"
#include "hwc_layers.h"
#include <hardware/hwcomposer.h>
#include <memory>
#include <vector>
#include <initializer_list>
#include <list>

namespace mir
{
namespace graphics
{

class Renderable;
class NativeBuffer;
class Buffer;

namespace android
{

class BasicLayerList
{
public:
    hwc_display_contents_1_t* native_list() const;
    virtual ~BasicLayerList() = default;

protected:
    BasicLayerList(size_t default_size);
    void resize_layer_list(size_t size);

    std::shared_ptr<hwc_display_contents_1_t> hwc_representation;
};

class HWC10LayerList : public BasicLayerList
{
public:
    HWC10LayerList();
    void update_composition_layers(std::list<std::shared_ptr<graphics::Renderable>> const& list);
};

class FBTargetLayerList : public BasicLayerList
{
public:
    FBTargetLayerList();

    void update_composition_layers(std::list<std::shared_ptr<graphics::Renderable>> const& list);
    void set_fb_target(std::shared_ptr<NativeBuffer> const&);
    NativeFence framebuffer_fence();

private:
    FramebufferLayer fb_target;
    int fb_position;
};

}
}
}

#endif /* MIR_GRAPHICS_ANDROID_HWC_LAYERLIST_H_ */

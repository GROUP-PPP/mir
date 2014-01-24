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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "src/platform/graphics/android/display_buffer.h"
#include "src/platform/graphics/android/android_display.h"
#include "src/platform/graphics/android/gl_context.h"
#include "src/platform/graphics/android/android_format_conversion-inl.h"
#include "mir_test_doubles/mock_display_device.h"
#include "mir_test_doubles/mock_display_report.h"
#include "mir_test_doubles/mock_egl.h"
#include "mir/graphics/android/mir_native_window.h"
#include "mir_test_doubles/stub_driver_interpreter.h"
#include "mir_test_doubles/stub_display_buffer.h"
#include "mir_test_doubles/stub_display_device.h"
#include "mir_test_doubles/stub_buffer.h"
#include "mir_test_doubles/mock_framebuffer_bundle.h"
#include <memory>

namespace geom=mir::geometry;
namespace mg=mir::graphics;
namespace mga=mir::graphics::android;
namespace mtd=mir::test::doubles;

class AndroidDisplayBufferTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        stub_buffer = std::make_shared<testing::NiceMock<mtd::StubBuffer>>();
        mock_display_device = std::make_shared<testing::NiceMock<mtd::MockDisplayDevice>>();
        native_window = std::make_shared<mg::android::MirNativeWindow>(std::make_shared<mtd::StubDriverInterpreter>());

        visual_id = 5;
        dummy_display = mock_egl.fake_egl_display;
        dummy_config = mock_egl.fake_configs[0];
        dummy_context = mock_egl.fake_egl_context;
        testing::NiceMock<mtd::MockDisplayReport> report;
        gl_context = std::make_shared<mga::GLContext>(mga::to_mir_format(mock_egl.fake_visual_id),report);
        mock_fb_bundle = std::make_shared<testing::NiceMock<mtd::MockFBBundle>>();

        ON_CALL(*mock_fb_bundle, fb_format())
            .WillByDefault(testing::Return(mir_pixel_format_abgr_8888));
        ON_CALL(*mock_fb_bundle, fb_size())
            .WillByDefault(testing::Return(geom::Size{}));
    }

    testing::NiceMock<mtd::MockEGL> mock_egl;

    int visual_id;
    EGLConfig dummy_config;
    EGLDisplay dummy_display;
    EGLContext dummy_context;
    std::shared_ptr<mga::GLContext> gl_context;

    std::shared_ptr<mtd::StubBuffer> stub_buffer;
    std::shared_ptr<ANativeWindow> native_window;
    std::shared_ptr<mtd::MockDisplayDevice> mock_display_device;
    std::shared_ptr<mtd::MockFBBundle> mock_fb_bundle;
};

TEST_F(AndroidDisplayBufferTest, test_post_update)
{
    using namespace testing;

    InSequence seq;
    EXPECT_CALL(*mock_display_device, prepare_gl())
        .Times(Exactly(1));
    EXPECT_CALL(*mock_display_device, gpu_render(dummy_display, mock_egl.fake_egl_surface))
        .Times(1);
    EXPECT_CALL(*mock_fb_bundle, last_rendered_buffer())
        .Times(1)
        .WillOnce(Return(stub_buffer));
    EXPECT_CALL(*mock_display_device, post(Ref(*stub_buffer)))
        .Times(1);

    std::list<mg::Renderable> renderlist;
    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);
    db.post_update();
}

TEST_F(AndroidDisplayBufferTest, test_post_update_empty_list)
{
    using namespace testing;

    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);

    InSequence seq;
    EXPECT_CALL(*mock_display_device, prepare_gl())
        .Times(1);
    EXPECT_CALL(*mock_display_device, gpu_render(dummy_display, mock_egl.fake_egl_surface))
        .Times(1);
    EXPECT_CALL(*mock_fb_bundle, last_rendered_buffer())
        .Times(1)
        .WillOnce(Return(stub_buffer));
    EXPECT_CALL(*mock_display_device, post(Ref(*stub_buffer)))
        .Times(1);

    std::list<mg::Renderable> renderlist{};
    auto render_fn = [] (mg::Renderable const&) {};
    db.render_and_post_update(renderlist, render_fn);
}

TEST_F(AndroidDisplayBufferTest, test_post_update_list)
{
    using namespace testing;

    struct MockRenderOperator
    {
        MOCK_METHOD0(called, void());
    };

    std::list<mg::Renderable> renderlist{mg::Renderable{}, mg::Renderable{}};
    MockRenderOperator mock_call_counter;

    InSequence seq;
    EXPECT_CALL(*mock_display_device, prepare_gl_and_overlays(Ref(renderlist)))
        .Times(1);
    EXPECT_CALL(mock_call_counter, called())
        .Times(renderlist.size());
    EXPECT_CALL(*mock_display_device, gpu_render(dummy_display, mock_egl.fake_egl_surface))
        .Times(1);
    EXPECT_CALL(*mock_fb_bundle, last_rendered_buffer())
        .Times(1)
        .WillOnce(Return(stub_buffer));
    EXPECT_CALL(*mock_display_device, post(Ref(*stub_buffer)))
        .Times(1);

    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);

    db.render_and_post_update(renderlist, [&](mg::Renderable const&){
        mock_call_counter.called();
    });
}

TEST_F(AndroidDisplayBufferTest, defaults_to_normal_orientation)
{
    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window,
                          *gl_context);

    EXPECT_EQ(mir_orientation_normal, db.orientation());
}

TEST_F(AndroidDisplayBufferTest, orientation_is_passed_through)
{
    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window,
                          *gl_context);

    for (auto const& ori : {mir_orientation_normal,
                            mir_orientation_left,
                            mir_orientation_right,
                            mir_orientation_inverted})
    {
        auto config = db.configuration();
        config.orientation = ori;
        db.configure(config);
        EXPECT_EQ(ori, db.orientation());
    }
}

TEST_F(AndroidDisplayBufferTest, rotation_transposes_dimensions)
{
    using namespace testing;

    int const width = 123;
    int const height = 456;
    geom::Size const normal{width, height};
    geom::Size const transposed{height, width};

    EXPECT_CALL(*mock_fb_bundle, fb_size())
        .WillRepeatedly(Return(normal));

    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window,
                          *gl_context);

    EXPECT_EQ(normal, db.view_area().size);

    auto config = db.configuration();

    config.orientation = mir_orientation_right;
    db.configure(config);
    EXPECT_EQ(transposed, db.view_area().size);

    config.orientation = mir_orientation_inverted;
    db.configure(config);
    EXPECT_EQ(normal, db.view_area().size);

    config.orientation = mir_orientation_left;
    db.configure(config);
    EXPECT_EQ(transposed, db.view_area().size);
}

TEST_F(AndroidDisplayBufferTest, test_db_forwards_size_along)
{
    using namespace testing;

    geom::Size fake_display_size{223, 332};
    EXPECT_CALL(*mock_fb_bundle, fb_size())
        .Times(AnyNumber())
        .WillRepeatedly(Return(fake_display_size));

    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);

    auto view_area = db.view_area();

    geom::Point origin_pt{geom::X{0}, geom::Y{0}};
    EXPECT_EQ(view_area.size, fake_display_size);
    EXPECT_EQ(view_area.top_left, origin_pt);
}

TEST_F(AndroidDisplayBufferTest, db_egl_context_from_shared)
{
    using namespace testing;

    EGLint const expected_attr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

    EXPECT_CALL(mock_egl, eglCreateContext(
        dummy_display, _, dummy_context, mtd::AttrMatches(expected_attr)))
        .Times(1)
        .WillOnce(Return(mock_egl.fake_egl_context));
    EXPECT_CALL(mock_egl, eglCreateWindowSurface(
        dummy_display, _, native_window.get(), NULL))
        .Times(1)
        .WillOnce(Return(mock_egl.fake_egl_surface));
    EXPECT_CALL(mock_egl, eglDestroySurface(dummy_display, mock_egl.fake_egl_surface))
        .Times(AtLeast(1));
    EXPECT_CALL(mock_egl, eglDestroyContext(dummy_display, mock_egl.fake_egl_context))
        .Times(AtLeast(1));

    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);
    testing::Mock::VerifyAndClearExpectations(&mock_egl);
}

TEST_F(AndroidDisplayBufferTest, egl_resource_creation_failure)
{
    using namespace testing;
    EXPECT_CALL(mock_egl, eglCreateContext(_,_,_,_))
        .Times(2)
        .WillOnce(Return(EGL_NO_CONTEXT))
        .WillOnce(Return(mock_egl.fake_egl_context));
    EXPECT_CALL(mock_egl, eglCreateWindowSurface(_,_,_,_))
        .Times(1)
        .WillOnce(Return(EGL_NO_SURFACE));

    EXPECT_THROW(
    {
        mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);
    }, std::runtime_error);

    EXPECT_THROW(
    {
        mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);
    }, std::runtime_error);
}

TEST_F(AndroidDisplayBufferTest, make_current)
{
    using namespace testing;
    EGLContext fake_ctxt = reinterpret_cast<EGLContext>(0x4422);
    EGLSurface fake_surf = reinterpret_cast<EGLSurface>(0x33984);

    EXPECT_CALL(mock_egl, eglCreateContext(_,_,_,_))
        .Times(1)
        .WillOnce(Return(fake_ctxt));
    EXPECT_CALL(mock_egl, eglCreateWindowSurface(_,_,_,_))
        .Times(1)
        .WillOnce(Return(fake_surf));

    EXPECT_CALL(mock_egl, eglMakeCurrent(dummy_display, fake_surf, fake_surf, fake_ctxt))
        .Times(2)
        .WillOnce(Return(EGL_TRUE))
        .WillOnce(Return(EGL_FALSE));

    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);
    db.make_current();
    EXPECT_THROW(
    {
        db.make_current();
    }, std::runtime_error);
}

TEST_F(AndroidDisplayBufferTest, release_current)
{
    using namespace testing;

    EXPECT_CALL(mock_egl, eglMakeCurrent(dummy_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
        .Times(1);

    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);
    db.release_current();
}

TEST_F(AndroidDisplayBufferTest, display_power_mode_on_at_start)
{
    using namespace testing;
    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);
    auto config = db.configuration();
    EXPECT_EQ(config.power_mode, mir_power_mode_on);
}

TEST_F(AndroidDisplayBufferTest, display_power_mode)
{
    using namespace testing;
    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);

    Sequence seq;
    EXPECT_CALL(*mock_display_device, mode(mir_power_mode_off))
        .InSequence(seq);
    EXPECT_CALL(*mock_display_device, mode(mir_power_mode_on))
        .InSequence(seq);

    auto config = db.configuration();
    config.power_mode = mir_power_mode_off;
    db.configure(config);

    config = db.configuration();
    config.power_mode = mir_power_mode_on;
    db.configure(config); 
}

TEST_F(AndroidDisplayBufferTest, display_power_mode_disregards_double)
{
    using namespace testing;
    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);

    EXPECT_CALL(*mock_display_device, mode(mir_power_mode_off))
        .Times(1);

    auto config = db.configuration();
    config.power_mode = mir_power_mode_off;
    db.configure(config);
    config.power_mode = mir_power_mode_suspend;
    db.configure(config);
    config.power_mode = mir_power_mode_standby;
    db.configure(config);
}

//configuration tests
TEST_F(AndroidDisplayBufferTest, display_orientation_supported)
{
    using namespace testing;

    EXPECT_CALL(*mock_display_device, apply_orientation(mir_orientation_left))
        .Times(1)
        .WillOnce(Return(true));

    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);

    auto config = db.configuration();
    config.orientation = mir_orientation_left;
    db.configure(config); 

    config = db.configuration();
    EXPECT_EQ(config.orientation, mir_orientation_normal);
}

TEST_F(AndroidDisplayBufferTest, display_orientation_not_supported)
{
    using namespace testing;
    EXPECT_CALL(*mock_display_device, apply_orientation(mir_orientation_left))
        .Times(1)
        .WillOnce(Return(false));

    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);

    auto config = db.configuration();
    config.orientation = mir_orientation_left;
    db.configure(config); 

    config = db.configuration();
    EXPECT_EQ(config.orientation, mir_orientation_left);
}

TEST_F(AndroidDisplayBufferTest, incorrect_display_configure_throws)
{
    mga::DisplayBuffer db(mock_fb_bundle, mock_display_device, native_window, *gl_context);

    auto config = db.configuration();
    //error
    config.current_format = mir_pixel_format_invalid;
    EXPECT_THROW({
        db.configure(config);
    }, std::runtime_error); 
}
#if 0
//we only have single display and single mode on android for the time being
TEST_F(AndroidDisplayTest, android_display_configuration_info)
{
    mga::AndroidDisplay display(stub_db_factory, mock_display_report);
    auto config = display.configuration();

    std::vector<mg::DisplayConfigurationOutput> configurations;
    config.for_each_output([&](mg::DisplayConfigurationOutput const& config)
    {
        configurations.push_back(config);
    });

    ASSERT_EQ(1u, configurations.size());
    auto& disp_conf = configurations[0];
    ASSERT_EQ(1u, disp_conf.modes.size());
    auto& disp_mode = disp_conf.modes[0];
    EXPECT_EQ(display_size, disp_mode.size);

    EXPECT_EQ(mg::DisplayConfigurationOutputId{1}, disp_conf.id);
    EXPECT_EQ(mg::DisplayConfigurationCardId{0}, disp_conf.card_id);
    EXPECT_TRUE(disp_conf.connected);
    EXPECT_TRUE(disp_conf.used);
    auto origin = geom::Point{0,0};
    EXPECT_EQ(origin, disp_conf.top_left);
    EXPECT_EQ(0, disp_conf.current_mode_index);

    //TODO fill refresh rate accordingly
    //TODO fill physical_size_mm fields accordingly;
}
#endif

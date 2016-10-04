/*
 * Copyright © 2012-2014 Canonical Ltd.
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

#ifndef MIR_FRONTEND_SESSION_MEDIATOR_H_
#define MIR_FRONTEND_SESSION_MEDIATOR_H_

#include "display_server.h"
#include "buffer_stream_tracker.h"
#include "screencast_buffer_tracker.h"
#include "protobuf_ipc_factory.h"

#include "mir/frontend/connection_context.h"
#include "mir/frontend/surface_id.h"
#include "mir/frontend/buffer_stream_id.h"
#include "mir/graphics/platform_ipc_operations.h"
#include "mir/protobuf/display_server_debug.h"
#include "mir_toolkit/common.h"

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace mir
{
namespace cookie
{
class Authority;
}
namespace graphics
{
class Buffer;
class DisplayConfiguration;
class GraphicBufferAllocator;
}
namespace input
{
class CursorImages;
class InputDeviceHub;
}

namespace scene
{
class CoordinateTranslator;
class ApplicationNotRespondingDetector;
}

/// Frontend interface. Mediates the interaction between client
/// processes and the core of the mir system.
namespace frontend
{
class ClientBufferTracker;
class Shell;
class Session;
class Surface;
class MessageResourceCache;
class SessionMediatorReport;
class EventSink;
class EventSinkFactory;
class MessageSender;
class DisplayChanger;
class Screencast;
class PromptSession;
class BufferStream;

/**
 * SessionMediator relays requests from the client process into the server.
 *
 * Each SessionMediator is associated with exactly one client socket connection, and
 * visa versa.
 *
 * \note SessionMediator is *not* reentrant. If two threads want to process events on a client
 *       socket at the same time they must perform their own locking.
 */
class SessionMediator : public detail::DisplayServer, public mir::protobuf::DisplayServerDebug
{
public:

    SessionMediator(
        std::shared_ptr<Shell> const& shell,
        std::shared_ptr<graphics::PlatformIpcOperations> const& ipc_operations,
        std::shared_ptr<frontend::DisplayChanger> const& display_changer,
        std::vector<MirPixelFormat> const& surface_pixel_formats,
        std::shared_ptr<SessionMediatorReport> const& report,
        std::shared_ptr<EventSinkFactory> const& sink_factory,
        std::shared_ptr<MessageSender> const& message_sender,
        std::shared_ptr<MessageResourceCache> const& resource_cache,
        std::shared_ptr<Screencast> const& screencast,
        ConnectionContext const& connection_context,
        std::shared_ptr<input::CursorImages> const& cursor_images,
        std::shared_ptr<scene::CoordinateTranslator> const& translator,
        std::shared_ptr<scene::ApplicationNotRespondingDetector> const& anr_detector,
        std::shared_ptr<cookie::Authority> const& cookie_authority,
        std::shared_ptr<input::InputDeviceHub> const& hub
        );

    ~SessionMediator() noexcept;

    void client_pid(int pid) override;

    void connect(
        mir::protobuf::ConnectParameters const* request,
        mir::protobuf::Connection* response,
        google::protobuf::Closure* done) override;
    void disconnect(
        mir::protobuf::Void const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void create_surface(
        mir::protobuf::SurfaceParameters const* request,
        mir::protobuf::Surface* response,
        google::protobuf::Closure* done) override;
    void modify_surface(
        mir::protobuf::SurfaceModifications const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void release_surface(
        mir::protobuf::SurfaceId const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void platform_operation(
        mir::protobuf::PlatformOperationMessage const* request,
        mir::protobuf::PlatformOperationMessage* response,
        google::protobuf::Closure* done) override;
    void configure_surface(
        mir::protobuf::SurfaceSetting const* request,
        mir::protobuf::SurfaceSetting* response,
        google::protobuf::Closure* done) override;
    void configure_display(
        mir::protobuf::DisplayConfiguration const* request,
        mir::protobuf::DisplayConfiguration* response,
        google::protobuf::Closure* done) override;
    void set_base_display_configuration(
        mir::protobuf::DisplayConfiguration const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void preview_base_display_configuration(
        mir::protobuf::PreviewConfiguration const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void confirm_base_display_configuration(
        mir::protobuf::DisplayConfiguration const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void cancel_base_display_configuration_preview(
        mir::protobuf::Void const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void create_screencast(
        mir::protobuf::ScreencastParameters const* request,
        mir::protobuf::Screencast* response,
        google::protobuf::Closure* done) override;
    void screencast_buffer(
        mir::protobuf::ScreencastId const* request,
        mir::protobuf::Buffer* response,
        google::protobuf::Closure* done) override;
    void release_screencast(
        mir::protobuf::ScreencastId const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void create_buffer_stream(
        mir::protobuf::BufferStreamParameters const* request,
        mir::protobuf::BufferStream* response,
        google::protobuf::Closure* done) override;
    void release_buffer_stream(
        mir::protobuf::BufferStreamId const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void configure_cursor(
        mir::protobuf::CursorSetting const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void new_fds_for_prompt_providers(
        mir::protobuf::SocketFDRequest const* request,
        mir::protobuf::SocketFD* response,
        google::protobuf::Closure* done) override;
    void start_prompt_session(
        mir::protobuf::PromptSessionParameters const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void stop_prompt_session(
        mir::protobuf::Void const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void submit_buffer(
        mir::protobuf::BufferRequest const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void allocate_buffers(
        mir::protobuf::BufferAllocation const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void release_buffers(
        mir::protobuf::BufferRelease const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void request_persistent_surface_id(
        mir::protobuf::SurfaceId const* request,
        mir::protobuf::PersistentSurfaceId* response,
        google::protobuf::Closure* done) override;
    void pong(
        mir::protobuf::PingEvent const* request,
        mir::protobuf::Void* response,
        google::protobuf::Closure* done) override;
    void configure_buffer_stream(
        mir::protobuf::StreamConfiguration const* request,
        mir::protobuf::Void*,
        google::protobuf::Closure* done) override;
    void raise_surface(
        mir::protobuf::RaiseRequest const* request,
        mir::protobuf::Void*,
        google::protobuf::Closure* done) override;

    // TODO: Split this into a separate thing
    void translate_surface_to_screen(
        mir::protobuf::CoordinateTranslationRequest const* request,
        mir::protobuf::CoordinateTranslationResponse* response,
        google::protobuf::Closure* done) override;

private:
    void pack_protobuf_buffer(protobuf::Buffer& protobuf_buffer,
                              graphics::Buffer* graphics_buffer,
                              graphics::BufferIpcMsgType msg_type);

    void advance_buffer(
        BufferStreamId surf_id,
        BufferStream& buffer_stream,
        graphics::Buffer* old_buffer,
        std::function<void(graphics::Buffer*, graphics::BufferIpcMsgType)> complete);

    std::shared_ptr<graphics::DisplayConfiguration> unpack_and_sanitize_display_configuration(
        protobuf::DisplayConfiguration const*);

    virtual std::function<void(std::shared_ptr<Session> const&)> prompt_session_connect_handler() const;

    void destroy_screencast_sessions();

    pid_t client_pid_;
    std::shared_ptr<Shell> const shell;
    std::shared_ptr<graphics::PlatformIpcOperations> const ipc_operations;

    std::vector<MirPixelFormat> const surface_pixel_formats;

    std::shared_ptr<frontend::DisplayChanger> const display_changer;
    std::shared_ptr<SessionMediatorReport> const report;
    std::shared_ptr<EventSinkFactory> const sink_factory;
    std::shared_ptr<EventSink> const event_sink;
    std::shared_ptr<MessageSender> const message_sender;
    std::shared_ptr<MessageResourceCache> const resource_cache;
    std::shared_ptr<Screencast> const screencast;
    ConnectionContext const connection_context;
    std::shared_ptr<input::CursorImages> const cursor_images;
    std::shared_ptr<scene::CoordinateTranslator> const translator;
    std::shared_ptr<scene::ApplicationNotRespondingDetector> const anr_detector;
    std::shared_ptr<cookie::Authority> const cookie_authority;
    std::shared_ptr<input::InputDeviceHub> const hub;

    BufferStreamTracker buffer_stream_tracker;
    ScreencastBufferTracker screencast_buffer_tracker;

    std::weak_ptr<Session> weak_session;
    std::weak_ptr<PromptSession> weak_prompt_session;
};

}
}

#endif /* MIR_FRONTEND_SESSION_MEDIATOR_H_ */

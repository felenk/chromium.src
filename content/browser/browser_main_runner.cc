// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/browser/browser_main_runner.h"

#include "base/allocator/allocator_shim.h"
#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/debug/trace_event.h"
#include "base/logging.h"
#include "base/metrics/histogram.h"
#include "base/metrics/statistics_recorder.h"
#include "content/browser/browser_main_loop.h"
#include "content/browser/notification_service_impl.h"
#include "content/common/child_process.h"
#include "content/public/browser/compositor_util.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"

#if defined(OS_WIN)
#include "base/win/metro.h"
#include "ui/base/win/scoped_ole_initializer.h"
#include "ui/base/ime/win/tsf_bridge.h"
#endif

#if defined(OS_ANDROID)
#include "content/browser/android/surface_texture_peer_browser_impl.h"
#endif

bool g_exited_main_message_loop = false;

namespace content {

class BrowserMainRunnerImpl : public BrowserMainRunner {
 public:
  BrowserMainRunnerImpl()
      : is_initialized_(false),
        is_shutdown_(false),
        created_threads_(false) {
  }

  ~BrowserMainRunnerImpl() {
    if (is_initialized_ && !is_shutdown_)
      Shutdown();
  }

  virtual int Initialize(const MainFunctionParams& parameters)
      OVERRIDE {
    is_initialized_ = true;

#if !defined(OS_IOS)
    // ChildProcess:: is a misnomer unless you consider context.  Use
    // of --wait-for-debugger only makes sense when Chrome itself is a
    // child process (e.g. when launched by PyAuto).
    if (parameters.command_line.HasSwitch(switches::kWaitForDebugger))
      ChildProcess::WaitForDebugger("Browser");
#endif  // !defined(OS_IOS)

#if defined(OS_WIN)
    if (parameters.command_line.HasSwitch(
            switches::kEnableTextServicesFramework)) {
      base::win::SetForceToUseTsf();
    }
#endif  // OS_WIN

    base::StatisticsRecorder::Initialize();

    notification_service_.reset(new NotificationServiceImpl);

    main_loop_.reset(new BrowserMainLoop(parameters));

    main_loop_->Init();

    main_loop_->EarlyInitialization();

    // Must happen before we try to use a message loop or display any UI.
    main_loop_->InitializeToolkit();

    main_loop_->MainMessageLoopStart();

    // WARNING: If we get a WM_ENDSESSION, objects created on the stack here
    // are NOT deleted. If you need something to run during WM_ENDSESSION add it
    // to browser_shutdown::Shutdown or BrowserProcess::EndSession.

#if defined(OS_WIN)
#if !defined(NO_TCMALLOC)
    // When linking shared libraries, NO_TCMALLOC is defined, and dynamic
    // allocator selection is not supported.

    // Make this call before going multithreaded, or spawning any subprocesses.
    base::allocator::SetupSubprocessAllocator();
#endif
    ole_initializer_.reset(new ui::ScopedOleInitializer);
    if (base::win::IsTsfAwareRequired())
      ui::TsfBridge::Initialize();
#endif  // OS_WIN

#if defined(OS_ANDROID)
    SurfaceTexturePeer::InitInstance(new SurfaceTexturePeerBrowserImpl(
        parameters.command_line.HasSwitch(
            switches::kMediaPlayerInRenderProcess)));
#endif

    if (IsThreadedCompositingEnabled())
      CommandLine::ForCurrentProcess()->AppendSwitch(
          switches::kEnableThreadedCompositing);

    main_loop_->CreateThreads();
    int result_code = main_loop_->GetResultCode();
    if (result_code > 0)
      return result_code;
    created_threads_ = true;

    // Return -1 to indicate no early termination.
    return -1;
  }

  virtual int Run() OVERRIDE {
    DCHECK(is_initialized_);
    DCHECK(!is_shutdown_);
    main_loop_->RunMainMessageLoopParts();
    return main_loop_->GetResultCode();
  }

  virtual void Shutdown() OVERRIDE {
    DCHECK(is_initialized_);
    DCHECK(!is_shutdown_);
    g_exited_main_message_loop = true;

    if (created_threads_)
      main_loop_->ShutdownThreadsAndCleanUp();

#if defined(OS_WIN)
    if (base::win::IsTsfAwareRequired())
      ui::TsfBridge::GetInstance()->Shutdown();
    ole_initializer_.reset(NULL);
#endif

    main_loop_.reset(NULL);

    notification_service_.reset(NULL);

    is_shutdown_ = true;
  }

 protected:
  // True if the runner has been initialized.
  bool is_initialized_;

  // True if the runner has been shut down.
  bool is_shutdown_;

  // True if the non-UI threads were created.
  bool created_threads_;

  scoped_ptr<NotificationServiceImpl> notification_service_;
  scoped_ptr<BrowserMainLoop> main_loop_;
#if defined(OS_WIN)
  scoped_ptr<ui::ScopedOleInitializer> ole_initializer_;
#endif

  DISALLOW_COPY_AND_ASSIGN(BrowserMainRunnerImpl);
};

// static
BrowserMainRunner* BrowserMainRunner::Create() {
  return new BrowserMainRunnerImpl();
}

}  // namespace content

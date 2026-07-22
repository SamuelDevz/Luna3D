#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AppNative", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AppNative", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "AppNative", __VA_ARGS__))

struct AppState
{
    ANativeWindow* window;
    bool running;
};

static int32_t handleInput(struct android_app* app, AInputEvent* event)
{
  AppState* appState = reinterpret_cast<AppState*>(app->userData);

  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
  {
    float x = AMotionEvent_getX(event, 0);
    float y = AMotionEvent_getY(event, 0);

    switch (AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK)
    {
    case AMOTION_EVENT_ACTION_DOWN:
      LOGI("Touch down at: %f, %f", x, y);
      return 1;
    case AMOTION_EVENT_ACTION_MOVE:
      LOGI("Touch moved to: %f, %f", x, y);
      return 1;
    case AMOTION_EVENT_ACTION_UP:
      LOGI("Touch up at: %f, %f", x, y);
      return 1;
    }
  }
  else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
  {
    int32_t keyCode = AKeyEvent_getKeyCode(event);
    if (keyCode == AKEYCODE_BACK)
    {
      appState->running = false;
      return 1;
    }
  }

  return 0;
}

static void handleCmd(struct android_app* app, int32_t cmd)
{
  AppState* appState = reinterpret_cast<AppState*>(app->userData);

  switch (cmd)
  {
    case APP_CMD_INIT_WINDOW:
      if (app->window != NULL)
      {
        appState->window = app->window;
        LOGI("Janela nativa inicializada.");
      }
      break;
    case APP_CMD_TERM_WINDOW:
      appState->window = nullptr;
      LOGI("Janela nativa finalizada.");
      break;
    case APP_CMD_GAINED_FOCUS:
      break;
    case APP_CMD_LOST_FOCUS:
      break;
  }
}

void android_main(struct android_app* app)
{
  AppState appState = {};
  appState.running = true;

  app->userData = &appState;
  app->onAppCmd = handleCmd;
  app->onInputEvent = handleInput;

  while (app->destroyRequested == 0 && appState.running)
  {
    int events;
    struct android_poll_source* source;

    while (ALooper_pollOnce(0, nullptr, &events, (void**)&source) >= 0)
    {
      if (source != nullptr)
      {
        source->process(app, source);
      }
    }
  }
}
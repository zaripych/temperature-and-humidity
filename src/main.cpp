#include <Arduino.h>
#include "framework/task_scheduler.hpp"
#include "framework/actions_dispatcher.hpp"

#include "app/install.hpp"
#include "helpers/sleep.h"

task_scheduler scheduler;
actions_dispatcher dispatcher;

void setup()
{
  // should be called first for auto-correction and
  // measuring how much exactly it takes to wakeup
  deep_sleep_auto_correct_wakeup_timer();

  scheduler.dispatcher = &dispatcher;
  dispatcher.scheduler = &scheduler;

  install_app(dispatcher, scheduler);
}

void loop()
{
  scheduler.run();
}
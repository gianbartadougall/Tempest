## Tempest
Software to run automated blinds

Current bugs

task_scheduler.c
- The timer will skip an isr if it is close to end i.e if an isr is supposed to run at 65017 and the max count is 65535, the isr will be skipped 
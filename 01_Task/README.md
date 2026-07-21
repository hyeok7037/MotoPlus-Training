# 01_Task

Basic MotoPlus task example for Yaskawa robot controllers.

## Objective

Learn the basic structure of a MotoPlus application.

## Topics

- `mpUsrRoot()`
- `mpCreateTask()`
- Task execution
- Task priority
- `mpTaskDelay()`
- Controller tick

## Execution Flow

```text
Controller Start
        |
        v
   mpUsrRoot()
        |
        +---- Create Task 1
        |
        +---- Create Task 2
        |
        v
  mpExitUsrRoot
        |
        +------------------+
        |                  |
        v                  v
     Task 1             Task 2
   while(TRUE)        while(TRUE)

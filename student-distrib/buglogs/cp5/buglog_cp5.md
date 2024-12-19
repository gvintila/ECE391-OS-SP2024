# MP3.5 Buglog

## Keyboard not properly synced up between terminal switches
There were multiple issues with the keyboard encapsulated under this bug. Every variable involved with the keyboard had to be properly saved between terminal switches, so we had to create a separate keyboard variable for each terminal using a struct which would switch everytime we switch terminals. We also needed to make it so that the keyboard handler is only updating the keyboard variables associated with the shown terminal, as that's where the user is expecting the keyboard to take effect. Every other program that uses the keyboard (mainly terminal commands and putc) needed to be associated with the active terminal keyboard buffer.

## Executing shell executes in another terminal
We noticed that the flag to denote that enter has been pressed was universal so that terminal_read was executing in another terminal when enter was pressed in a different terminal. We had to incorporate the enter key so that it was unique to each terminal using the terminal keyboard struct we defined.

## Executing a program every other time causes program abnormal end error
We noticed our functionality with our program execution was odd and only worked every other time. We realized that the paging was setup incorrectly inside the scheduler. The scheduler needs to properly change the paging to the respective PID of the next process scheduled during every run.

## Program eventually pagefaults when run for long enough (stack overflow)
This bug was extremely hard to spot at first. Our scheduler worked fine in every way but we would get random pagefaults. We noticed that our esp values were decreasing extremely fast which eventually started changing our PCB values and started accessing random places in memory far from any running program. We eventually realized that our tss wasn't getting set correctly in the scheduler and we placed the saved esp from the context switch in our tss. What this eventually does is that everytime the user space launches a new process, we start the kernel stack at some place slightly above where the previous kernel stack ended, which eventually starts to overflow our stack until we start modifying the PCB values.

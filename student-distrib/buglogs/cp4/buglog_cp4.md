# MP3.4 Buglog

## George:

## Multiple Shells

### Exiting base shell and relaunching causes random page faults and weird issues
Our OS is setup so that whenever we exit base shell, it restarts back up. I realized the problem was that I was forgetting to tag the base shell as "not in use" when it goes through the execute sequence again. This was an issue because I have a function that picks up the first available process that's not tagged in use so it would actually pick up the next process and be operating on a completely different process.

## Cat

### Cat running in an infinite loop and constantly printing out file contents
At this point, we thought our read was working properly but we didn't realize that we are supposed to keep track of file position in file. This caused the cat program to infinitely loop because cat expects read to eventually return 0 which is when it finally halts. I fixed the file_read function so that it takes into account the file_pos and returns 0 after the full length of the file is read.

## Adriel:

## Counter

Page fault when pressing a key when counter runs, fixed it by disable interrupts when terminal write is called and renabling them when returning.

## Terminal Read

Did not clear keyboard buffer when terminal_read is called and did not reset the flags properly such as buf_full_flag and buf_idx = 0

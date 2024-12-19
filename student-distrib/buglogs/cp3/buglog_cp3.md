# MP3.3 Buglog

## George:

## Execute

### When a command argument string is too long, a page fault is generated
I noticed this bug whenever I would send out a command that's too long (generally one line in the terminal). I quickly realized it was because I limited the size of my command_name string to be the max file name length, which was good, but I forgot to account for the fact that it could go past the size and start dereferencing invalid memory values. I changed it so that it breaks upon getting to the very last character index.
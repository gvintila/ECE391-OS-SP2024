CP2:

3/20/2024 - BUG - Symbols would pop up when pressing number keys (0-9) and when caps lock is ON and shift is OFF
            FIX - Did not take into account that when caps lock is on we use the shifted verion of the letters but the numbers stay unshifted.

3/21/2024 - BUG - Would not insert the first character into the buffer
            FIX - I reset my buffer idx to 0 before putting the enter '\n' to the buffer, so I was esentially replacing the keyboard_buf[0] == ENTER which is not one of my print keys.

3/21//2024 - BUG - Would delete an extra character on console but not the keyboard buffer
             FIX - should not print anything to console but add char '\t' to the buffer

3/22/2024 - BUG - Inconsitency when tabbing and backspace, when typing to video mem it looked fine but the buffer would be missing some spaces.
            FIX - Outputted 0 instead of \t to the screen, initialyl I had an if statement that would return if we see a tab, but we just need to write to screen instead

3/22/2024 - BUG - Scroll works for half the screeen and outputs garbage values in other
            FIX - Forgot taht each character block in memory corresponds to 2 bytes, the first byte is atrrb and second byte is ascii so i needed to multiply by 2 or shift left by 1
            
3/23/2024 - BUG - When CTRL+L when keyboard buffer is full, it delets the last character inserted.
            FIX - I initially replace the last char in buffer to 0 because if we ctrl+L it adds the L to the buffer, so we need to delete that. But we only
                  do this if buffer is not full.

3/25/2024 - BUG - File system was giving incorrect behavior when trying to print and read data.
            FIX - We were not casting the pointer to the memory since it needed be a pointer to a uint_8 instead uint_32.

3/25/2024 - BUG - We were not able to see the full contents of the executable when we printed them out.
            FIX - They were printing NUL bytes so we skipped it when we output it.

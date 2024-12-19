# MP3.1 Buglog

George:

## GDT

### Kernel boot loops upon launching
I kept having this issue when I was initially trying to load the GDT. I realized that this was due to the fact that I wasn't loading it in correctly. I forgot to add a .word 0 padding at the beginning of the struct so it was likely loading the incorrect values into the GDT which caused it to bootloop.

## IDT

### Printf text loops back to top of screen

When I was testing my exception handlers, I saw I had this issue come up. I thought it was because the exception handler was resetting the screen_y and screen_x but it turns out it was just because the printf function is set up to loop back. I assume we are not supposed to change this for now but we'll add scrolling support later.

### Undefined reference to function initialized in .S file

I kept running into an issue with my assembly linkage where it would keep throwing back an error saying the function never got a valid reference. It was apparently because the .S file was named the same as the .h/c file. Words cannot describe how much I hate assembly.

## Paging

### Including types.h in header for ASM breaks code

So I still don't really understand this error but it seems like when the ASM was reading the types.h include, it would throw a bunch of random errors. What fixed it was using the ifndef. So, I think I should just be more careful with what I'm including in the future and only needing to include certain pieces of code for each file.

### Bootlooping when paging is enabled

We saw that whenever we changed the CR0 register to enable paging, the program would automatically crash and GDB would stop following whatever line the program was in. We realized this was because we hadn't turned the PSE bit on in CR4 before activating paging. This makes sense since our kernel code is supposed to be a 4MB page so as soon as paging was turned on, the kernel likely got confused that the kernel was partitioned into 4KB pages and crashed.
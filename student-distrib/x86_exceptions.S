/* Macro to help define basic link between 
 * interrupt/exception handler and label 
 */

    #define exception_link(name, func, vector) \
    .global name                    ;\
    name:                           ;\
        pushal                      ;\
        pushfl                      ;\
        pushl $vector               ;\
        call func                   ;\
        addl $4, %esp               ;\
        popfl                       ;\
        popal                       ;\
        iret                        ;\

/* Macro to help define basic link between 
 * interrupt/exception handler and label 
 * when there is an error code pushed to the stack
 */
    #define exception_link_error(name, func, vector) \
    .global name                    ;\
    name:                           ;\
        pushfl                      ;\
        pushal                      ;\
        pushl $vector               ;\
        movl %cr2, %eax             ;\
        pushl %eax                  ;\
        call func                   ;\
        popl %eax                   ;\
        addl $4, %esp               ;\
        popal                       ;\
        popfl                       ;\
        addl $4, %esp               ;\
        iret                        ;\
        

    exception_link(excep0, excep_handler, 0);
    exception_link(excep1, excep_handler, 1);
    exception_link(excep2, excep_handler, 2);
    exception_link(excep3, excep_handler, 3);
    exception_link(excep4, excep_handler, 4);
    exception_link(excep5, excep_handler, 5);
    exception_link(excep6, excep_handler, 6);
    exception_link(excep7, excep_handler, 7);
    exception_link_error(excep8, excep_handler_error, 8);
    exception_link(excep9, excep_handler, 9);
    exception_link_error(excep10, excep_handler_error, 10);
    exception_link_error(excep11, excep_handler_error, 11);
    exception_link_error(excep12, excep_handler_error, 12);
    exception_link_error(excep13, excep_handler_error, 13);
    exception_link_error(excep14, excep_handler_error, 14);
    
    exception_link(excep16, excep_handler, 16);
    exception_link_error(excep17, excep_handler_error, 17);
    exception_link(excep18, excep_handler, 18);
    exception_link(excep19, excep_handler, 19);


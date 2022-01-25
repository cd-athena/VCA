Application Programming Interface
============

VCA is written primarily in C++ and x86 assembly language. This API is
wholly defined within :file:`vcaLib.h` in the source/lib/ folder of our
source tree.  All of the functions and variables and enumerations meant
to be used by the end-user are present in this header.

- **vca_analyzer_open**(vca_param param)
**********************************
> Create a new encoder handler, all parameters from vca_param are copied. The returned pointer is then passed to all of the functions pertaining to this analyzer. Since **vca_param** is copied internally,  the user may release their copy after allocating the analyzer. Changes made to their copy of the param structure have no affect on the analyzer after it has been allocated.

- vca_result **vca_analyzer_push**(vca_analyzer *enc, vca_frame *frame)
**********************************


- bool **vca_result_available**(vca_analyzer *enc)
**********************************


- vca_result **vca_analyzer_pull_frame_result**(vca_analyzer *enc,
                                          vca_frame_results *result)
**********************************                                          


- void **vca_analyzer_close**(vca_analyzer *enc)
**********************************


- void **vca_analyzer_shot_detect**(vca_analyzer *enc) {}
**********************************
